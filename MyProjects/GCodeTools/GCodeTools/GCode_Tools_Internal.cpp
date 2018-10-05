//This file contains everything needed to establish a connection

#include "pch.h"
#include "GCodeTools.h"
#include "NotePlayer.h"
#include"VectorMath.h"

std::vector<std::string>& split(const std::string& s,
	char delim,
	std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

//This class can insert tabs in the GCode by providing a line number as the point of origin, offset, and size.  
//It traces in the same direction of the cut lifting the zaxis
class Tab_Generator
{
private:
	//Can be in inches or mm, depending on the units the gcode is working in
	struct Tabsize
	{
		Tabsize() : m_width(0.25),m_height(0.08)
		{}
		Tabsize(double height,double width) : m_width(width),m_height(height) 
		{}
		double m_width;
		double m_height;
	};
	//Note: that if we typically go 1mm below bottom we have to compensate for height
	Tabsize m_GlobalSize=Tabsize(0.08, 0.25);  //We can have a global size for all, but keep ability to change individual ones
	struct TabLocation
	{
		TabLocation()
		{}
		TabLocation(size_t LineNumber, double Offset) : m_LineNumber_c(LineNumber), m_Offset(Offset)
		{}
		Vec2d m_origin,m_prev_end,m_end_point,m_end_arc_axis;
		size_t m_end_point_Gtype;  // 1 straight feed, 2 clockwise arc, 3 counter arc
		double m_Z_depth; //find the last z depth to detemine height relief
		//The line number is used to compute the origin
		//The end point is the point far enough away from origin to create tab
		size_t m_LineNumber_c;  //the c means cardinal and so the origin point matches the line number on the text editor
		double m_Offset;  //how far away from origin (along the contour path) to start tab
		size_t m_NoLinesForEndPoint;  //Used to compute lines to replace
	};
	using text_line = std::string;
	using GCode = std::vector<text_line>;
	struct Tab
	{
		Tab()
		{}
		//User can contruct tab with custom size per tab
		Tab(Tabsize properties, TabLocation position) : m_properties(properties), m_position(position) 
		{}
		//Or using a global size reference
		Tab(Tab_Generator *parent,TabLocation position) : m_properties(parent->m_GlobalSize), m_position(position)
		{}
		Tabsize m_properties;
		TabLocation m_position;
		//We make it clear within the patch the intention of what is happening for each line submitted (even if it isn't needed)
		struct Patch
		{
			enum operation
			{
				e_op_insert_after_line,
				e_op_replaced_line
			};
			struct PatchLine
			{
				PatchLine(text_line _text,operation _lineType,size_t _lineNumber) : text(_text),line_type(_lineType),line_number_c(_lineNumber)
				{}
				text_line text;
				size_t line_number_c;  //cardinal line number the operation pertains to from the source... multiple lines of insertion can point to same line number
				operation line_type;
			};
			void AddLine(const char *text, operation line_type, size_t _lineNumber)
			{
				m_PatchLines.push_back(PatchLine(text,line_type, _lineNumber));
			}
			std::vector<PatchLine> m_PatchLines;
		} m_PatchCode; //this keeps a record of the changes made for the tab from beginning to end (may embed existing gcode)
		size_t m_PatchLinesToReplace;  //A count of how many lines to replace with the patch code
	};
	using Tabs = std::map<size_t,Tab>;  //found via line number
	using Tabs_iter = Tabs::iterator;
	Tabs m_Tabs; //add ability to batch process all the tabs in one setting

	GCode m_GCode;  //A container for the GCode
	std::string m_OutFileName;  //provide a different name to compare
	static __inline bool IsNumber(char value)
	{
		return (value >= '0') && (value <= '9');
	}

	static __inline int GetNumber(const char *&read_cursor)
	{
		int ret = 0;
		bool IsNegative = read_cursor[0] == '-';
		if (IsNegative)
			read_cursor++; //advance past negative sign
		//expect a number
		while (IsNumber(read_cursor[0]))
		{
			ret += read_cursor[0] - '0';
			if (IsNumber(read_cursor[1]))
				ret *= 10;
			read_cursor++; //advance
		}

		return IsNegative ? -ret : ret;
	}

	static __inline double GetNumber_Float(const char *&read_cursor)
	{
		double ret = 0;
		bool IsNegative = read_cursor[0] == '-';
		if (IsNegative)
			read_cursor++; //advance past negative sign
		//expect a number
		while (IsNumber(read_cursor[0]))
		{
			ret += (double)read_cursor[0] - '0';
			if (IsNumber(read_cursor[1]))
				ret *= 10.0;
			read_cursor++; //advance
		}
		//Do we have a decimal here?
		if (read_cursor[0] == '.')
		{
			read_cursor++; //advance past decimal
			double scaler = 0.1;
			while (IsNumber(read_cursor[0]))
			{
				const double digit = (double)read_cursor[0] - '0';
				ret += (digit * scaler);
				read_cursor++; //advance
				scaler *= .1; //move to next precision unit digit
			}
		}
		return IsNegative ? -ret : ret;
	}

	struct GCode_Variables
	{
		enum VariableEnum
		{
			eG, eX, eY, eZ, eI, eJ, eK, eF, eNoVariables
		};
		void Flush()
		{
			G = 0;
			X = Y = Z = I = J = K = F = 0.0;
			for (size_t i = 0; i < eNoVariables; i++)
				Processed[i] = false;
		}
		GCode_Variables()
		{
			Flush();
		}
		size_t G;
		double X, Y, Z, I, J, K, F;
		bool Processed[eNoVariables];
	};
	static void ParseLine(const text_line &line, GCode_Variables&value)
	{
		const char *read_cursor = line.c_str();
		const char *read_cursor_end = read_cursor + line.size();
		while ((read_cursor < read_cursor_end))
		{
			switch (*read_cursor)
			{
			case 'G':
			{
				read_cursor++;
				int result = GetNumber(read_cursor);
				assert(result >= 0 && result <= 3);  //sanity check
				value.G = result;
				break;
			}
			case 'I':
			case 'J':
			case 'K':
			case 'F':
			case 'X':
			case 'Y':
			case 'Z':
			{
				char GCodeVar = *read_cursor;  //cache to switch again
				read_cursor++;
				double result = GetNumber_Float(read_cursor);
				switch (GCodeVar)
				{
				case 'I':		value.I = result;	value.Processed[GCode_Variables::eI] = true;	break;
				case 'J':		value.J = result;	value.Processed[GCode_Variables::eJ] = true;	break;
				case 'K':		value.K = result;	value.Processed[GCode_Variables::eK] = true;	break;
				case 'F':		value.F = result;	value.Processed[GCode_Variables::eF] = true;	break;
				case 'X':		value.X = result;	value.Processed[GCode_Variables::eX] = true;	break;
				case 'Y':		value.Y = result;	value.Processed[GCode_Variables::eY] = true;	break;
				case 'Z':		value.Z = result;	value.Processed[GCode_Variables::eZ] = true;	break;
				}
				break;
			}
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				read_cursor++;
				break;
			}
		}
	}

	bool ObtainPosition(Tab &tab)
	{
		size_t line_index = tab.m_position.m_LineNumber_c-1;  //line numbers are cardinal... this becomes ordinal
		GCode_Variables gvar;
		//bool result=ParseLine_XY(m_GCode[line_index], XValue, YValue, ZValue, ZValueProcessed);
		ParseLine(m_GCode[line_index], gvar);
		if (gvar.Processed[GCode_Variables::eZ])
			tab.m_position.m_Z_depth = gvar.Z;


		//Make sure we have X and Y for the point of origin... we may have to go back some lines to get them
		bool GetX = !gvar.Processed[GCode_Variables::eX];
		bool GetY = !gvar.Processed[GCode_Variables::eY];
		//We don't know if we need these yet; however, if we are scanning backwards we want to preserve the latest value's for these once we move forward
		bool GetI = !gvar.Processed[GCode_Variables::eI];
		bool GetJ = !gvar.Processed[GCode_Variables::eJ];

		if (GetX || GetY)
		{
			double LatestI = gvar.I;
			double LatestJ = gvar.J;

			while (line_index > 5)
			{
				ParseLine(m_GCode[--line_index], gvar);
				if (GetI && gvar.Processed[GCode_Variables::eI])
				{
					GetI = false;
					LatestI = gvar.I;
				}
				if (GetJ && gvar.Processed[GCode_Variables::eJ])
				{
					GetJ = false;
					LatestJ = gvar.J;
				}
				if (
					(gvar.Processed[GCode_Variables::eX] || !GetX) &&
					(gvar.Processed[GCode_Variables::eY] || !GetY))
					break;  //we got what we need
			}

			gvar.I = LatestI;
			gvar.J = LatestJ;

			//Reset the line index for further processing!
			line_index = tab.m_position.m_LineNumber_c - 1;
			ParseLine(m_GCode[line_index], gvar);  //Ensure X and Y have the latest as these may be used again moving forward
		}
		assert(gvar.Processed[GCode_Variables::eX] && gvar.Processed[GCode_Variables::eY]);

		tab.m_position.m_origin = Vec2d(gvar.X, gvar.Y);

		//Have the point of origin... now to find the end point

		double distance_of_points = 0.0;
		Vec2d TestEndPoint = tab.m_position.m_origin;
		tab.m_position.m_prev_end = tab.m_position.m_origin;
		//If we haven't gotten Z yet... flag to get it again; otherwise we keep/prefer the anchor point's value
		bool GetZ = !gvar.Processed[GCode_Variables::eZ];
		while (distance_of_points<tab.m_properties.m_width)
		{
			line_index++;
			ParseLine(m_GCode[line_index], gvar);  //Note: don't flush as we accumulate varaibles as we go
			if (GetZ)
			{
				if (gvar.Processed[GCode_Variables::eZ])
				{
					tab.m_position.m_Z_depth = gvar.Z;
					GetZ = false;
				}
			}
			//same here... this will need to iterate until both are complete
			assert((gvar.Processed[GCode_Variables::eX] && gvar.Processed[GCode_Variables::eY]) || gvar.Processed[GCode_Variables::eZ]);  
			TestEndPoint=Vec2d(gvar.X, gvar.Y);
			distance_of_points = tab.m_position.m_origin.length(TestEndPoint);
			if (distance_of_points < tab.m_properties.m_width)
				tab.m_position.m_prev_end = TestEndPoint;  //need to keep track of previous point to inject a new point
		}
		tab.m_position.m_end_point = TestEndPoint;
		tab.m_position.m_end_arc_axis = Vec2d(gvar.I, gvar.J);  //cache radius if available
		tab.m_position.m_end_point_Gtype = gvar.G;

		tab.m_position.m_NoLinesForEndPoint = line_index - (tab.m_position.m_LineNumber_c - 1);

		//We may need to scan backwards for missing variables... doing that here:
		const bool NeedArcAxis = gvar.G == 2 || gvar.G == 3;
		GetX = !gvar.Processed[GCode_Variables::eX];
		GetY = !gvar.Processed[GCode_Variables::eY];
		GetI = NeedArcAxis && !gvar.Processed[GCode_Variables::eI];
		GetJ = NeedArcAxis && !gvar.Processed[GCode_Variables::eJ];
		
		if (GetX || GetY || GetZ || GetI || GetJ)
		{
			//Reset line index to origin but this time going in reverse
			size_t line_index = tab.m_position.m_LineNumber_c - 1;  

			while (line_index > 5)
			{
				ParseLine(m_GCode[line_index], gvar);
				line_index--;
				if (
					(gvar.Processed[GCode_Variables::eX] || !GetX) &&
					(gvar.Processed[GCode_Variables::eY] || !GetY) &&
					(gvar.Processed[GCode_Variables::eZ] || !GetZ) &&
					(gvar.Processed[GCode_Variables::eI] || !GetI) &&
					(gvar.Processed[GCode_Variables::eJ] || !GetJ))
					break;  //we got what we need
			}
			if (GetZ)
				tab.m_position.m_Z_depth = gvar.Z;
			if (GetI || GetJ)
			{
				//reconstuct the arc axis 
				Vec2d ModifiedAxis = tab.m_position.m_end_arc_axis;  //if we had either one... it's cached here
				if (GetI)
					ModifiedAxis[0] = gvar.I;
				if (GetJ)
					ModifiedAxis[1] = gvar.J;
				tab.m_position.m_end_arc_axis = ModifiedAxis;
			}
			if (GetX || GetY)
			{
				Vec2d ModifiedEndPoint = tab.m_position.m_end_point;
				Vec2d ModifiedPrevPoint = tab.m_position.m_prev_end;
				if (GetX)
					ModifiedEndPoint[0] = ModifiedPrevPoint[0] = gvar.X;
				if (GetY)
					ModifiedEndPoint[1] = ModifiedPrevPoint[1] = gvar.Y;
				tab.m_position.m_end_point=ModifiedEndPoint;
				tab.m_position.m_prev_end=ModifiedPrevPoint;
			}
			assert(gvar.Processed[GCode_Variables::eX] || !GetX);
			assert(gvar.Processed[GCode_Variables::eY] || !GetY);
			assert(gvar.Processed[GCode_Variables::eZ] || !GetZ);
			assert(gvar.Processed[GCode_Variables::eI] || !GetI);
			assert(gvar.Processed[GCode_Variables::eJ] || !GetJ);
		}
		return true;
	}

	class GCode_Writer
	{
	private:
		const GCode &m_GCode;
		//where each tab is ordered by the ordinal line number
		std::map<size_t, const Tab *> m_Tabs;  //using a map to avoid n^2 operation
		using tab_iter = std::map<size_t, const Tab *>::const_iterator;
		//We need to maintain the source index pointer as it may not increment for each call
		size_t m_SourceIndex_o=0;
		bool m_PatchInFlight = false;
		size_t m_PatchIndex = 0;
	public:
		GCode_Writer(const GCode &_GCode ) : m_GCode(_GCode)
		{
		}
		void AddTabs(const Tabs &_Tabs)
		{
			//populate the map
			for (auto &i : _Tabs)
				m_Tabs[i.first-1] = &i.second;  //insert into ordinal line number positions
		}
		//ordinal line number this function expects this to increment with no reason to seek
		const char *WriteLine(size_t line_number_o, size_t &sourceindex)
		{
			const char *ret = nullptr;
			if (m_SourceIndex_o < m_GCode.size())
			{
				//in all cases we try to find a tab line
				tab_iter Tab = m_Tabs.find(m_SourceIndex_o);
				if (Tab != m_Tabs.end())
				{
					//easy access to the patch code
					const Tab::Patch &patch_code = Tab->second->m_PatchCode;
					if (!m_PatchInFlight)
					{
						//we have a tab... do we have some patch code?
						if (patch_code.m_PatchLines.size())
						{
							m_PatchInFlight = true;
							m_PatchIndex = 0;
							//for now assume all operations are insert after this point... so this line gets returned without advancing
							ret = m_GCode[m_SourceIndex_o].c_str();
						}
						else
						{
							//Note: we should unless there are some scenarios not yet supported
							printf("warning: tab has no patch code... \n");
						}
					}
					else
					{
						//here we deal out the patched code
						if (m_PatchIndex < patch_code.m_PatchLines.size())
						{
							ret = patch_code.m_PatchLines[m_PatchIndex++].text.c_str();
						}
						else
						{
							//we are done
							m_SourceIndex_o++;  //advance to next entry
							//and skip over replaced lines
							m_SourceIndex_o += Tab->second->m_PatchLinesToReplace;
							m_PatchInFlight = false;
						}
					}
				}
				//The usual case
				if (!m_PatchInFlight)
					ret = m_GCode[m_SourceIndex_o++].c_str();
				sourceindex=m_SourceIndex_o;
			}
			return ret;
		}
		void Flush() 
		{
			m_SourceIndex_o = 0;
			m_PatchInFlight = false;
			m_PatchIndex = 0;
			m_Tabs.clear();
		}
	} m_GCode_Writer;
public:
	Tab_Generator() : m_GCode_Writer(m_GCode)
	{}
	bool Load_GCode(const char *filename)
	{
		bool ret = false;
		std::ifstream in(filename);
		if (in.is_open())
		{
			bool success = true;
			const size_t MaxBufferSize = 70;  //We know GCode is set for 70
			const size_t Padding = 30;  //this is really arbitrary
			while (!in.eof())
			{
				char Buffer[MaxBufferSize+ Padding];
				in.getline(Buffer, MaxBufferSize+ Padding);
				m_GCode.push_back(text_line(Buffer));
				if (strlen(Buffer) > MaxBufferSize)
				{
					printf("line count exceeds %d, aborting", MaxBufferSize);
					success = false;
					break;
				}
			}
			ret = success;
			in.close();
		}
		return ret;
	}
	void ProcessTab(Tab &tab)
	{
		//Obtain position from GCode... this will have both points needed to create tab
		//as well as the current z height
		bool result=ObtainPosition(tab);
		//Evaluate the Gtype
		//1 is straight
		//2 is arc clockwise
		//3 is arc counter-clockwise
		assert((tab.m_position.m_end_point_Gtype > 0) && (tab.m_position.m_end_point_Gtype < 4)); //TODO error handling of this failure
		if (tab.m_position.m_end_point_Gtype == 1)
		{
			Vec2d ContourVector = tab.m_position.m_end_point;
			ContourVector -= tab.m_position.m_origin;
			ContourVector.normalize();  // we now have a direction... now to get a few points
			Vec2d AnchorPoint = ContourVector * tab.m_position.m_Offset + tab.m_position.m_origin;
			Vec2d SegmentEnd = ContourVector * tab.m_properties.m_width + AnchorPoint;
			//Now we have everything... time to list it out
			//starting with first point to be inserted at line number's position
			char Buffer[70];
			sprintf(Buffer, "G1 X%.4f Y%.4f (tab type1 start)", AnchorPoint[0], AnchorPoint[1]);
			tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_insert_after_line, tab.m_position.m_LineNumber_c);
			sprintf(Buffer, "G1 X%.4f Y%.4f Z%.4f", AnchorPoint[0], AnchorPoint[1], tab.m_position.m_Z_depth + tab.m_properties.m_height);
			tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_insert_after_line, tab.m_position.m_LineNumber_c);
			sprintf(Buffer, "G1 X%.4f Y%.4f", SegmentEnd[0], SegmentEnd[1]);
			tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_insert_after_line, tab.m_position.m_LineNumber_c);
			sprintf(Buffer, "G1 X%.4f Y%.4f Z%.4f (tab end)", SegmentEnd[0], SegmentEnd[1], tab.m_position.m_Z_depth);
			tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_insert_after_line, tab.m_position.m_LineNumber_c);
			tab.m_PatchLinesToReplace = 0;
		}
		assert(result);  //I intend to make this more robust
	}
	//Here is the simplest form to solve tabs
	Tab CreateTab(size_t line_number, double offset = 0.0)
	{
		Tab NewTab(this, TabLocation(line_number, offset));
		ProcessTab(NewTab);
		return NewTab;
	}
	Tab CreateTab(const Tabsize &props,size_t line_number, double offset = 0.0)
	{
		Tab NewTab(props, TabLocation(line_number, offset));
		ProcessTab(NewTab);
		return NewTab;
	}
	bool ExportGCode(const char *filename)
	{
		const char *block = nullptr;
		size_t lineindex = 0;
		m_GCode_Writer.Flush();  //ensure everything is reset incase we do multiple tries
		m_GCode_Writer.AddTabs(m_Tabs);
		size_t SourceIndex=0;
		if (filename)
		{
			std::ofstream out = std::ofstream(filename, std::ios::out);
			while ((block = m_GCode_Writer.WriteLine(lineindex++, SourceIndex)) != nullptr)
			{
				out.write(block, strlen(block));
				out << '\n';
			}
			out.close();
		}
		else
		{
			//Note: These post increment so they become cardinal by coicedence when these label the line numbers
			#if 1
			while ((block = m_GCode_Writer.WriteLine(lineindex++, SourceIndex)) != nullptr)
				printf("%d: %s\n", SourceIndex, block);
			#else
			while ((block = m_GCode_Writer.WriteLine(lineindex++, SourceIndex)) != nullptr)
				printf("%d: %s\n", lineindex, block);
			#endif	
		}
		return true;
	}

	void Test()
	{
		m_GCode.clear();
		m_Tabs.clear();
		#if 1
		Load_GCode("CasterContourTest.nc");
		Tab newTab = CreateTab(293, 0.5);
		#endif	
		#if 0
		Load_GCode("TabTest.nc");
		Tab newTab = CreateTab(41, 1.0);
		SetOutFilename("TabTest_Tabbed.nc");
		#endif
		#if 1
		//Populate by cardinal line number in case we want to remove them
		m_Tabs[newTab.m_position.m_LineNumber_c] = newTab;
		//m_Tabs.push_back(ProcessTab(Tabsize(0.075, 0.25), 41, 1.0));
		ExportGCode(nullptr);
		//ExportGCode("D:/Stuff/BroncBotz/Code/MyProjects/GCodeTools/GCodeTools/TabTest_Modiied.nc");
		#endif
	}
	bool LoadToolJob(const char *filename)
	{
		m_GCode.clear();
		m_Tabs.clear();
		return Load_GCode(filename);
	}

	void SetOutFilename(const char *filename)
	{
		m_OutFileName = filename;
	}
	void SetGlobalTabSize(double height, double width)
	{
		m_GlobalSize = Tabsize(height, width);
	}
	bool AddTab(size_t line_number, double offset = 0.0)
	{
		//If we already added a tab here... remove it, and add it again (user may have changed properties or offset)
		Tabs_iter tab_entry = m_Tabs.find(line_number);
		if (tab_entry != m_Tabs.end())
			m_Tabs.erase(tab_entry);

		Tab newTab = CreateTab(line_number, offset);
		//Populate by cardinal line number in case we want to remove them
		m_Tabs[newTab.m_position.m_LineNumber_c] = newTab;
		return ExportGCode(m_OutFileName[0]==0?nullptr:m_OutFileName.c_str());
	}
	bool AddTab(double height, double width,size_t line_number, double offset = 0.0)
	{
		//If we already added a tab here... remove it, and add it again (user may have changed properties or offset)
		Tabs_iter tab_entry = m_Tabs.find(line_number);
		if (tab_entry != m_Tabs.end())
			m_Tabs.erase(tab_entry);

		Tab newTab = CreateTab(Tabsize(height,width),line_number, offset);
		//Populate by cardinal line number in case we want to remove them
		m_Tabs[newTab.m_position.m_LineNumber_c] = newTab;
		return ExportGCode(m_OutFileName[0] == 0 ? nullptr : m_OutFileName.c_str());
	}

	bool RemoveTab(size_t line_number)
	{
		bool ret = false;
		Tabs_iter tab_entry = m_Tabs.find(line_number);
		if (tab_entry != m_Tabs.end())
		{
			m_Tabs.erase(tab_entry);
			ret = true;
		}
		if (ret)
			return ExportGCode(m_OutFileName[0] == 0 ? nullptr : m_OutFileName.c_str());
		return ret;
	}

};

class GCodeTools_Internal
{
private:
	DirectSound::Output::DirectSound_Initializer m_ds_init;
	DirectSound::Output::DS_Output m_DS;
	NotePlayer m_NotePlayer;
	Tab_Generator m_TabGenerator;
public:
	GCodeTools_Internal()
	{
		m_NotePlayer.Link_DSound(std::make_shared<DirectSound::Output::DS_Output>(m_DS));
	}
	void connect()
	{
		printf("Connecting");
	}
	void TestSound_Start() { m_DS.StartStreaming(); }
	void TestSound_Stop() { m_DS.StopStreaming(); }

	bool LoadSequence_CT(const char *filename) { return m_NotePlayer.LoadSequence_CT(filename); }
	void PlayBlock(size_t block_number) { m_NotePlayer.PlayBlock(block_number); }
	void PlaySong(double position) { m_NotePlayer.PlaySong(position); }
	void Stop_NotePlayer() { m_NotePlayer.Stop(); }
	void Pause_NotePlayer(bool IsPaused) { m_NotePlayer.Pause(IsPaused); }
	void ReveseChannels(bool IsReversed) { m_NotePlayer.ReveseChannels(IsReversed); }
	bool ExportGCode(const char *filename) { return m_NotePlayer.ExportGCode(filename); }
	void SetBounds(double x, double y, double z) {m_NotePlayer.SetBounds(x, y, z); }

	bool LoadToolJob(const char *filename)
	{
		return m_TabGenerator.LoadToolJob(filename);
	}
	void SetWorkingFile(const char *filename)
	{
		m_TabGenerator.SetOutFilename(filename);
	}
	void SetGlobalTabSize(double height, double width)
	{
		m_TabGenerator.SetGlobalTabSize(height, width);
	}
	bool AddTab(size_t line_number, double offset)
	{
		return m_TabGenerator.AddTab(line_number, offset);
	}
	bool AddTab(double height, double width, size_t line_number, double offset)
	{
		return m_TabGenerator.AddTab(height, width, line_number, offset);
	}
	bool RemoveTab(size_t line_number)
	{
		return m_TabGenerator.RemoveTab(line_number);
	}

	void Test() 
	{ 
		m_TabGenerator.Test();
		//LoadSequence_CT("FugaVIII_tc.txt");
		//printf("Loaded FugaVIII\n");
	}
};

  /*******************************************************************************************************/
 /*													GCodeTools											*/
/*******************************************************************************************************/

void GCodeTools::GCodeTools_init(void)
{
	m_p_GCodeTools = std::make_shared<GCodeTools_Internal>();
}

void GCodeTools::GCodeTools_connect()
{
	m_p_GCodeTools->connect();
}

void GCodeTools::TestSound_Start() 
{
	m_p_GCodeTools->TestSound_Start();
}
void GCodeTools::TestSound_Stop() 
{
	m_p_GCodeTools->TestSound_Stop();
}
bool GCodeTools::LoadSequence_CT(const char *filename)
{
	return m_p_GCodeTools->LoadSequence_CT(filename);
}
void GCodeTools::PlayBlock(size_t block_number)
{
	m_p_GCodeTools->PlayBlock(block_number);
}
void GCodeTools::PlaySong(double position)
{
	m_p_GCodeTools->PlaySong(position);
}
void GCodeTools::Stop_NotePlayer()
{
	m_p_GCodeTools->Stop_NotePlayer();
}

void GCodeTools::Pause_NotePlayer(bool IsPaused)
{
	m_p_GCodeTools->Pause_NotePlayer(IsPaused);
}
void GCodeTools::ReveseChannels(bool IsReversed)
{
	m_p_GCodeTools->ReveseChannels(IsReversed);
}
bool GCodeTools::ExportGCode(const char *filename)
{
	return m_p_GCodeTools->ExportGCode(filename);
}
void GCodeTools::SetBounds(double x, double y, double z) 
{ 
	m_p_GCodeTools->SetBounds(x, y, z);
}


bool GCodeTools::LoadToolJob(const char *filename)
{
	return m_p_GCodeTools->LoadToolJob(filename);
}
void GCodeTools::SetWorkingFile(const char *filename)
{
	m_p_GCodeTools->SetWorkingFile(filename);
}
void GCodeTools::SetGlobalTabSize(double height, double width)
{
	m_p_GCodeTools->SetGlobalTabSize(height, width);
}
bool GCodeTools::AddTab(size_t line_number, double offset)
{
	return m_p_GCodeTools->AddTab(line_number, offset);
}
bool GCodeTools::AddTab(double height, double width, size_t line_number, double offset)
{
	return m_p_GCodeTools->AddTab(height,width,line_number, offset);
}

bool GCodeTools::RemoveTab(size_t line_number)
{
	return m_p_GCodeTools->RemoveTab(line_number);
}




void GCodeTools::Test()
{
	m_p_GCodeTools->Test();
}