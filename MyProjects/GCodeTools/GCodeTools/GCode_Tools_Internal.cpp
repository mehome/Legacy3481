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
		Tabsize(double height,double width) : m_width(width),m_height(height) 
		{}
		double m_width;
		double m_height;
	};
	Tabsize m_GlobalSize=Tabsize(0.02, 0.25);  //We can have a global size for all, but keep ability to change individual ones
	struct TabLocation
	{
		TabLocation(size_t LineNumber, double Offset) : m_LineNumber_c(LineNumber), m_Offset(Offset)
		{}
		Vec2d m_origin,m_prev_end,m_end_point;
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
	using Tabs = std::vector<Tab>;
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

	static bool ParseLine_XY(const text_line &line, double &XValue, double &YValue,double &ZValue, bool &ZProcessed, bool wait_for_z=false)
	{
		bool XProcessed = false;
		bool YProcessed = false;
		ZProcessed = false;
		const char *read_cursor = line.c_str();
		const char *read_cursor_end = read_cursor + line.size();
		while ((read_cursor<read_cursor_end)&&((!XProcessed)||(!YProcessed)||(wait_for_z && !ZProcessed) ))
		{
			switch (*read_cursor)
			{
			case 'G':
			{
				read_cursor++;
				int result = GetNumber(read_cursor);
				assert(result == 1 || result == 2 || result==3);  //sanity check
				break;
			}
			case 'I':
			case 'J':
			case 'F':
			{
				read_cursor++;
				double result = GetNumber_Float(read_cursor);
				break;
			}
			case 'X':
			{
				read_cursor++;
				double result = GetNumber_Float(read_cursor);
				XValue = result;
				XProcessed = true;
				break;
			}
			case 'Y':
			{
				read_cursor++;
				double result = GetNumber_Float(read_cursor);
				YValue = result;
				YProcessed = true;
				break;
			}
			case 'Z':
			{
				read_cursor++;
				double result = GetNumber_Float(read_cursor);
				ZValue = result;
				ZProcessed = true;
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
		return XProcessed&&YProcessed;
	}
	bool ObtainPosition(Tab &tab)
	{
		size_t line_index = tab.m_position.m_LineNumber_c-1;  //line numbers are cardinal... this becomes ordinal
		double XValue;
		double YValue;
		double ZValue;
		bool ZValueProcessed;
		bool result=ParseLine_XY(m_GCode[line_index], XValue, YValue, ZValue, ZValueProcessed);
		if (ZValueProcessed)
			tab.m_position.m_Z_depth = ZValue;
		//TODO make this more robust... currently the line we pick has to have both coordinates on it
		assert(result);
		tab.m_position.m_origin = Vec2d(XValue, YValue);
		double distance_of_points = 0.0;
		Vec2d TestEndPoint = tab.m_position.m_origin;
		tab.m_position.m_prev_end = tab.m_position.m_origin;
		bool GetZ = !ZValueProcessed;
		while (distance_of_points<tab.m_properties.m_width)
		{
			line_index++;
			result = ParseLine_XY(m_GCode[line_index], XValue, YValue, ZValue, ZValueProcessed);
			if (GetZ)
			{
				if (ZValueProcessed)
				{
					tab.m_position.m_Z_depth = ZValue;
					GetZ = false;
				}
			}
			assert(result);  //same here... this will need to iterate until both are complete
			TestEndPoint=Vec2d(XValue, YValue);
			distance_of_points = tab.m_position.m_origin.length(TestEndPoint);
			if (distance_of_points < tab.m_properties.m_width)
				tab.m_position.m_prev_end = TestEndPoint;  //need to keep track of previous point to inject a new point
		}
		tab.m_position.m_end_point = TestEndPoint;
		tab.m_position.m_NoLinesForEndPoint = line_index - (tab.m_position.m_LineNumber_c - 1);
		if (GetZ)
		{
			//Reset line index to origin but this time going in reverse
			size_t line_index = tab.m_position.m_LineNumber_c - 1;  
			while ((!ZValueProcessed) && (line_index > 5))
			{
				ParseLine_XY(m_GCode[line_index], XValue, YValue, ZValue, ZValueProcessed,true);
				line_index--;
			}
			tab.m_position.m_Z_depth = ZValue;
			assert(ZValueProcessed);
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
				m_Tabs[i.m_position.m_LineNumber_c - 1] = &i;  //insert into ordinal line number positions
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
					if (!m_PatchInFlight)
					{
						m_PatchInFlight = true;
						m_PatchIndex = 0;
						//for now assume all operations are insert after this point... so this line gets returned without advancing
						ret = m_GCode[m_SourceIndex_o].c_str();
					}
					else
					{
						const Tab::Patch &patch_code = Tab->second->m_PatchCode;
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
		Vec2d ContourVector = tab.m_position.m_end_point;
		ContourVector -= tab.m_position.m_origin;
		ContourVector.normalize();  // we now have a direction... now to get a few points
		Vec2d AnchorPoint = ContourVector * tab.m_position.m_Offset + tab.m_position.m_origin;
		Vec2d SegmentEnd = ContourVector * tab.m_properties.m_width + AnchorPoint;
		//Now we have everything... time to list it out
		//starting with first point to be inserted at line number's position
		char Buffer[70];
		sprintf(Buffer,"G2 X%.4f Y%.4f Z%.4f",AnchorPoint[0],AnchorPoint[1],tab.m_position.m_Z_depth+tab.m_properties.m_height);
		tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_insert_after_line,tab.m_position.m_LineNumber_c);
		sprintf(Buffer, "G2 X%.4f Y%.4f", SegmentEnd[0], SegmentEnd[1]);
		tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_insert_after_line, tab.m_position.m_LineNumber_c);
		sprintf(Buffer, "G2 X%.4f Y%.4f Z%.4f", tab.m_position.m_end_point.x(), tab.m_position.m_end_point.y(), tab.m_position.m_Z_depth);
		tab.m_PatchCode.AddLine(Buffer, Tab::Patch::e_op_replaced_line, tab.m_position.m_LineNumber_c+tab.m_position.m_NoLinesForEndPoint);
		tab.m_PatchLinesToReplace = tab.m_position.m_NoLinesForEndPoint;
		assert(result);  //I intend to make this more robust
	}
	//Here is the simplest form to solve tabs
	Tab ProcessTab(size_t line_number, double offset = 0.0)
	{
		Tab NewTab(this, TabLocation(line_number, offset));
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
				out.write(block, strlen(block));
			out.close();
		}
		else
		{
			//Note: These post increment so they become cardinal by coicedence when these label the line numbers
			#if 0
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
		Load_GCode("TabTest.nc");
		m_Tabs.push_back(ProcessTab(41, 1.0));
		ExportGCode(nullptr);
	}
	#if 0
	void SetOutFilename(const char *filename)
	{
		m_OutFileName = filename;
	}
	void AddTab(size_t line_number, double offset = 0.0)
	{
		m_Tabs.push_back(Tab(this, TabLocation(line_number, offset)));
	}

	bool Process()
	{
		for (auto &iter : m_Tabs)
		{

		}
		if (m_OutFileName[0] != 0)
		{
			std::ofstream out = std::ofstream(m_OutFileName.c_str(), std::ios::out);
			for (auto &iter : m_GCode)
			{
				const text_line &element = iter;
				out.write(element.c_str(), strlen(element.c_str()));
			}
			out.close();
		}
	}
	#endif
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
void GCodeTools::Test()
{
	m_p_GCodeTools->Test();
}