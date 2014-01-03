#include "stdafx.h"
#define __IncludeInputBase__
#include "../FrameWork/FrameWork.h"
#include "Compositor.h"

//#define __DisableSmartDashboard__ //used to quickly disable the smart dashboard
#ifndef __DisableSmartDashboard__
#include "../SmartDashboard/SmartDashboard_import.h"
#else
class SmartDashboard //: public SensorBase
{
public:
	static void init() {}
	static void shutdown() {}

	//static void PutData(std::string key, Sendable *data) {}
	//static void PutData(NamedSendable *value){}
	//static Sendable* GetData(std::string keyName);

	static void PutBoolean(std::string keyName, bool value) {}
	static bool GetBoolean(std::string keyName) {return false;}

	static void PutNumber(std::string keyName, double value) {}
	static double GetNumber(std::string keyName) {return 0.0;}

	static void PutString(std::string keyName, std::string value) {}
	static int GetString(std::string keyName, char *value, unsigned int valueLen) {return 0;}
	static std::string GetString(std::string keyName) {return "";} 

	//static void PutValue(std::string keyName, ComplexData& value) {}
	//static void RetrieveValue(std::string keyName, ComplexData& value) {}

	static void SetClientMode() {}
	static void SetIPAddress(const char* address) {}
	static  bool IsConnected() {return false;}
};
#endif



Dashboard_Framework_Interface *g_Framework=NULL;
using namespace FrameWork;
event frameSync;



template<class T>
__inline T Enum_GetValue(const char *value,const char * const Table[],size_t NoItems)
{
	assert(value);  //If this fails... somebody forgot to enter a value 
	T ret=(T) 0;
	for (size_t i=0;i<NoItems;i++)
	{
		if (strcmp(value,Table[i])==0)
			ret=(T)i;
	}
	return ret;
}

const char * const csz_ReticleType_Enum[] =
{
	"none","square","pathalign","composite","bypass","line_plot"
};

struct Compositor_Props
{
	double X_Scalar;
	double Y_Scalar;

	//Reticle type:  default  (a.k.a square)
	struct SquareReticle_Props
	{
		size_t ThicknessX,ThicknessY;
		double opacity;
		BYTE rgb[3];
	};
	struct SquareReticle_Container_Props
	{
		SquareReticle_Props primary;
		SquareReticle_Props shadow;
		int PixelOffsetX,PixelOffsetY;
		bool UsingShadow,ExcludeRegion;
	};
	std::vector<SquareReticle_Container_Props> square_reticle;

	//Reticle type: Bypass
	std::string BypassPlugin;

	//Reticle type: LinePlot
	struct LinePlotReticle_Container_Props
	{
		struct VariablePacket
		{
			std::string Variable_Name;
			double Scalar;
			double Offset;
			BYTE rgb[3];
		};
		//List of all the variables we want to plot
		std::vector<VariablePacket> VariablePacket_List;
	};

	std::vector<LinePlotReticle_Container_Props> lineplot_reticle;

	struct PathAlign_Container_Props
	{
		double width,length;
		double pivot_point_length;
		double pos_x,pos_y,pos_z;
		double rot_x,rot_y,rot_z;
		double FOV_x,FOV_y;
		bool draw_cube;
	} PathAlign;
	//for now no list for path align... assuming only one instance is needed of one camera at a fixed point and orientation

	//Reticle type definitions
	enum ReticleType
	{
		eNone,
		eDefault,
		ePathAlign,
		eComposite,
		eBypass,
		eLinePlot
	};
	static ReticleType GetReticleType_Enum (const char *value)
	{	return Enum_GetValue<ReticleType> (value,csz_ReticleType_Enum,_countof(csz_ReticleType_Enum));
	}

	static const char * const GetReticleType_String(ReticleType value)
	{	return csz_ReticleType_Enum[value];
	}

	//Sequence
	struct Sequence_Packet;
	typedef std::vector<Sequence_Packet> Sequence_List;
	struct Sequence_Packet
	{
		ReticleType type;
		double PositionX,PositionY;  //These will be written dynamically properties will initialize with zero
		union type_specifics
		{
			size_t SquareReticle_SelIndex;
			Sequence_List *Composite;
		} specific_data;
	};
	Sequence_List Sequence;

	//This takes care of creating the deletion list implicitly
	Sequence_List *CreateCompositeList() 
	{
		Sequence_List *NewList=new Sequence_List;
		Composite_List.push_back(NewList);
		return NewList;
	}
	//This must be only called from within compositor's destructor
	void DestroyCompositeList()
	{
		for (size_t i=0;i<Composite_List.size();i++)
		{
			delete Composite_List[i];
			Composite_List[i]=NULL;
		}
	}

	std::vector<Sequence_List *> Composite_List;  //keep track of all composite lists created for deletion
};

class Compositor_Properties
{
	public:
		Compositor_Properties();
		const char *SetUpGlobalTable(Scripting::Script& script)
		{
			Compositor_Props &props=m_CompositorProps;
			const char* err;
			//props.ShipType=Ship_Props::eDefault;
			err = script.GetGlobalTable("Compositor");
			assert (!err);
			return err;
		}

		virtual void LoadFromScript(Scripting::Script& script);

		const Compositor_Props &GetCompositorProps() const {return m_CompositorProps;}
		Compositor_Props &GetCompositorProps_rw() {return m_CompositorProps;}
		const LUA_Controls_Properties &Get_CompositorControls() const {return m_CompositorControls;}
	private:
		Compositor_Props m_CompositorProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_CompositorControls;
};

//declared as global to avoid allocation on stack each iteration
const char * const g_Compositor_Controls_Events[] = 
{
	"SetXAxis","SetYAxis","NextSequence","PreviousSequence","SequencePOV","ToggleLinePlot","ResetPos"
};

const char *Compositor_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_Compositor_Controls_Events))?g_Compositor_Controls_Events[index] : NULL;
}
Compositor_Properties::ControlEvents Compositor_Properties::s_ControlsEvents;

Compositor_Properties::Compositor_Properties() : m_CompositorControls(&s_ControlsEvents)
{
	Compositor_Props props;
	props.X_Scalar=0.025;
	props.Y_Scalar=0.025;

	//Make one default
	Compositor_Props::SquareReticle_Props sqr_props;
	sqr_props.ThicknessX=sqr_props.ThicknessY=5;
	sqr_props.opacity=1.0;
	sqr_props.rgb[0]=sqr_props.rgb[2]=0;
	sqr_props.rgb[1]=255;
	Compositor_Props::SquareReticle_Container_Props sqr_pkt;
	sqr_pkt.primary=sqr_props;
	sqr_pkt.UsingShadow=false;
	sqr_pkt.ExcludeRegion=true;
	props.square_reticle.push_back(sqr_pkt);

	Compositor_Props::Sequence_Packet seq_pkt;
	seq_pkt.type=Compositor_Props::eDefault;
	seq_pkt.specific_data.SquareReticle_SelIndex=0;
	seq_pkt.PositionX=0.0;
	seq_pkt.PositionY=0.0;
	props.Sequence.push_back(seq_pkt);
	m_CompositorProps=props;
}

static void LoadSquareReticleProps_Internal(Scripting::Script& script,Compositor_Props &props,Compositor_Props::SquareReticle_Props &sqr_props)
{
	const char* err=NULL;
	double value;
	err=script.GetField("thickness", NULL, NULL, &value);
	if (!err)
		sqr_props.ThicknessX=sqr_props.ThicknessY=(size_t)value;
	{
		err=script.GetField("thickness_x", NULL, NULL, &value);
		if (!err)
			sqr_props.ThicknessX=(size_t)value;
		err=script.GetField("thickness_y", NULL, NULL, &value);
		if (!err)
			sqr_props.ThicknessY=(size_t)value;
		err=script.GetField("opacity", NULL, NULL, &value);
		if (!err)
			sqr_props.opacity=value;
		else
			sqr_props.opacity=1.0;
	}
	err=script.GetField("r", NULL, NULL, &value);
	if (!err)
		sqr_props.rgb[0]=(BYTE)value;
	err=script.GetField("g", NULL, NULL, &value);
	if (!err)
		sqr_props.rgb[1]=(BYTE)value;
	err=script.GetField("b", NULL, NULL, &value);
	if (!err)
		sqr_props.rgb[2]=(BYTE)value;
}

static void LoadSquareReticleProps(Scripting::Script& script,Compositor_Props &props)
{
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"square_reticle_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Compositor_Props::SquareReticle_Props sqr_props;
			LoadSquareReticleProps_Internal(script,props,sqr_props);

			Compositor_Props::SquareReticle_Container_Props sqr_pkt;
			std::string sTest;
			err = script.GetField("use_shadow",&sTest,NULL,NULL);
			sqr_pkt.UsingShadow=false;
			if (!err)
			{
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					sqr_pkt.UsingShadow=true;
			}
			err = script.GetField("exclude_region",&sTest,NULL,NULL);
			sqr_pkt.ExcludeRegion=true;
			if (!err)
			{
				if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
					sqr_pkt.ExcludeRegion=false;
			}
			err = script.GetFieldTable("shadow");
			if (!err)
			{
				Compositor_Props::SquareReticle_Props shadow_props;
				LoadSquareReticleProps_Internal(script,props,shadow_props);	
				sqr_pkt.shadow=shadow_props;
				script.Pop();
			}
			double fTest;
			err = script.GetField("x_offset",NULL,NULL,&fTest);
			if (!err)
				sqr_pkt.PixelOffsetX=(int)fTest;
			err = script.GetField("y_offset",NULL,NULL,&fTest);
			if (!err)
				sqr_pkt.PixelOffsetY=(int)fTest;
			
			sqr_pkt.primary=sqr_props;
			props.square_reticle.push_back(sqr_pkt);
			index++;

			script.Pop();
		}
	} while (!fieldtable_err);
}

static void LoadLinePlotProps_Internal(Scripting::Script& script,Compositor_Props &props,Compositor_Props::LinePlotReticle_Container_Props &lineplot_pkt)
{
	const char* err=NULL;
	double value;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"line_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Compositor_Props::LinePlotReticle_Container_Props::VariablePacket variable_pkt;

			err=script.GetField("name", &variable_pkt.Variable_Name, NULL, NULL);
			assert (!err);
			err=script.GetField("scalar",NULL , NULL, &variable_pkt.Scalar);
			if (err)
				variable_pkt.Scalar=1.0;
			err=script.GetField("offset",NULL , NULL, &variable_pkt.Offset);
			if (err)
				variable_pkt.Offset=0.0;
			err=script.GetField("r", NULL, NULL, &value);
			assert (!err);
			variable_pkt.rgb[0]=(BYTE)value;
			err=script.GetField("g", NULL, NULL, &value);
			assert (!err);
			variable_pkt.rgb[1]=(BYTE)value;
			err=script.GetField("b", NULL, NULL, &value);
			assert (!err);
			variable_pkt.rgb[2]=(BYTE)value;

			lineplot_pkt.VariablePacket_List.push_back(variable_pkt);
			index++;

			script.Pop();
		}
	} while (!fieldtable_err);
}


static void LoadLinePlotProps(Scripting::Script& script,Compositor_Props &props)
{
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"line_plot_list_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Compositor_Props::LinePlotReticle_Container_Props lineplot_pkt;
			LoadLinePlotProps_Internal(script,props,lineplot_pkt);

			props.lineplot_reticle.push_back(lineplot_pkt);
			index++;

			script.Pop();
		}
	} while (!fieldtable_err);
}

static bool LoadMeasuredValue(Scripting::Script& script,const char *root_name,double &value)
{
	const char* err=NULL;
	err = script.GetField(root_name, NULL, NULL,&value);
	if (err)
	{
		std::string name=root_name;
		name+="_ft";
		err = script.GetField(name.c_str(), NULL, NULL,&value);
		if (!err)
			value=Feet2Meters(value);
		else
		{
			std::string name=root_name;
			name+="_in";
			err = script.GetField(name.c_str(), NULL, NULL,&value);
			if (!err)
				value=Inches2Meters(value);
		}
	}
	return err==NULL;  //return success if no error
}

static void LoadPathAlignProps(Scripting::Script& script,Compositor_Props &props)
{
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	//Note: I've left this in form to handle list of settings... not yet sure if user will need multiple views (e.g. multiple sequences)
	//do 
	{
		fieldtable_err = script.GetFieldTable("path_align");
		if (!fieldtable_err)
		{
			Compositor_Props::PathAlign_Container_Props pal_props;

			double fTest;
			if (LoadMeasuredValue(script,"width",fTest))
				pal_props.width=fTest;
			else
				pal_props.width= 15 * 0.0254;	// 15 inches in meters (robot is 24)	This is the width of our drawn path.
			if (LoadMeasuredValue(script,"length",fTest))
				pal_props.length=fTest;
			else
				pal_props.length= 33 * 0.0254;	// 33 inches in meters					This is how far to project our path.
			if (LoadMeasuredValue(script,"pivot",fTest))
				pal_props.pivot_point_length=fTest;
			else
				pal_props.pivot_point_length= 22 * 0.0254;	// assumes pivot point is 2/3s from front.		This is the distance from the pivot point to the front, center of the robot frame.

			err = script.GetFieldTable("camera_position");
			if (!err)
			{
				if (LoadMeasuredValue(script,"x",fTest))
					pal_props.pos_x=fTest;
				else
					pal_props.pos_x=0.0;
				if (LoadMeasuredValue(script,"y",fTest))
					pal_props.pos_y=fTest;
				else
					pal_props.pos_y= -10 * 0.0254;
				if (LoadMeasuredValue(script,"z",fTest))
					pal_props.pos_z=fTest;
				else
					pal_props.pos_z=18 * 0.0254;
				script.Pop();
			}
			err = script.GetFieldTable("camera_rotation");
			if (!err)
			{
				err=script.GetField("x_deg", NULL, NULL, &fTest);
				if (!err) pal_props.rot_x=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("x",NULL,NULL,&fTest);
					if (!err)
						pal_props.rot_x=fTest;
					else 
						pal_props.rot_x=0.0;
				}
				err=script.GetField("y_deg", NULL, NULL, &fTest);
				if (!err) pal_props.rot_y=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("y",NULL,NULL,&fTest);
					if (!err)
						pal_props.rot_y=fTest;
					else
						pal_props.rot_y=0.33;
				}
				err=script.GetField("z_deg", NULL, NULL, &fTest);
				if (!err) pal_props.rot_z=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("z",NULL,NULL,&fTest);
					if (!err)
						pal_props.rot_z=fTest;
					else
						pal_props.rot_z=0.0;
				}
				script.Pop();
			}
			err = script.GetField("fov",NULL,NULL,&fTest);
			if (!err)
				pal_props.FOV_x=pal_props.FOV_y=fTest;
			else
			{
				err = script.GetField("fov_x",NULL,NULL,&fTest);
				if (!err)
				{
					pal_props.FOV_x=fTest;
					err = script.GetField("fov_y",NULL,NULL,&fTest);
					assert(!err);  //gotta have y if we have x
					pal_props.FOV_y=fTest;
				}
				else
					pal_props.FOV_x=pal_props.FOV_y=47.0;  //using default
			}
			std::string sTest;
			err = script.GetField("draw_cube",&sTest,NULL,NULL);
			pal_props.draw_cube=false;
			if (!err)
			{
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					pal_props.draw_cube=true;
			}
			props.PathAlign=pal_props;
			//props.square_reticle.push_back(sqr_pkt);
			//index++;
			script.Pop();
		}
		else
		{
			Compositor_Props::PathAlign_Container_Props pal_props;
			pal_props.width= 15 * 0.0254;
			pal_props.length= 33 * 0.0254;
			pal_props.pivot_point_length= 22 * 0.0254;
			pal_props.pos_x=0.0;
			pal_props.pos_y= -10 * 0.0254;
			pal_props.pos_z=18 * 0.0254;
			pal_props.rot_x=0.0;
			pal_props.rot_y=0.33;
			pal_props.rot_z=0.0;
			pal_props.FOV_x=pal_props.FOV_y=47.0;
			pal_props.draw_cube=false;
			props.PathAlign=pal_props;
		}
	} //while (!fieldtable_err);
}

static void LoadSequenceProps(Scripting::Script& script,Compositor_Props::Sequence_List &sequence,Compositor_Props &props,bool IsRecursive=false)
{
	const char* err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		if (!IsRecursive)
			sprintf_s(Buffer,128,"sequence_%d",index);
		else
			sprintf_s(Buffer,128,"composite_%d",index);
		err = script.GetFieldTable(Buffer);
		if (!err)
		{
			Compositor_Props::Sequence_Packet seq_pkt;

			std::string sTest;
			err = script.GetField("type",&sTest,NULL,NULL);
			assert(!err);  //gotta have it if we are making a sequence
			seq_pkt.type=Compositor_Props::GetReticleType_Enum(sTest.c_str());
			switch (seq_pkt.type)
			{
			case Compositor_Props::eDefault:
			case Compositor_Props::eLinePlot:  //line plot uses same selection mechanism
				{
					double fTest;

					err = script.GetField("selection",NULL,NULL,&fTest);
					seq_pkt.specific_data.SquareReticle_SelIndex=(size_t)fTest;
					seq_pkt.specific_data.SquareReticle_SelIndex--;  // translate cardinal to ordinal 
				}
				break;
			case Compositor_Props::eComposite:

				err = script.GetFieldTable("composite");
				if (!err)
				{
					Compositor_Props::Sequence_List *NewList=props.CreateCompositeList();
					seq_pkt.specific_data.Composite=NewList;
					//recursively call this with a new composite list to be used as the sequence
					LoadSequenceProps(script,*NewList,props,true);
					script.Pop();
				}
				break;
			}

			//These always start out zero'd
			seq_pkt.PositionX=0.0;
			seq_pkt.PositionY=0.0;

			sequence.push_back(seq_pkt);
			index++;

			script.Pop();
		}
	} while (!err);
}

static void LoadSequence_PersistentData(Scripting::Script& script,Compositor_Props::Sequence_List &sequence,Compositor_Props &props,bool IsRecursive=false)
{
	const char* err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		if (!IsRecursive)
			sprintf_s(Buffer,128,"sequence_%d",index);
		else
			sprintf_s(Buffer,128,"composite_%d",index);
		err = script.GetFieldTable(Buffer);
		if (!err)
		{
			Compositor_Props::Sequence_Packet &seq_pkt=sequence[index-1]; //ordinal translation

			std::string sTest;
			err = script.GetField("type",&sTest,NULL,NULL);
			assert(!err);  //gotta have it if we are making a sequence
			//If someone modifies the lua and the types do not match break out
			if (seq_pkt.type!=Compositor_Props::GetReticleType_Enum(sTest.c_str()))
			{
				script.Pop();
				break;
			}

			switch (seq_pkt.type)
			{
			case Compositor_Props::eDefault:
			case Compositor_Props::ePathAlign:
				{
					err = script.GetField("x",NULL,NULL,&seq_pkt.PositionX);
					err = script.GetField("y",NULL,NULL,&seq_pkt.PositionY);
				}
				break;
			case Compositor_Props::eComposite:
				{
					err = script.GetField("x",NULL,NULL,&seq_pkt.PositionX);
					err = script.GetField("y",NULL,NULL,&seq_pkt.PositionY);
					err = script.GetFieldTable("composite");
					if (!err)
					{
						Compositor_Props::Sequence_List *Composite=seq_pkt.specific_data.Composite;
						//recursively call this within the composite list to be used as the sequence
						LoadSequence_PersistentData(script,*Composite,props,true);
						script.Pop();
					}
				}
				break;
			}

			index++;

			script.Pop();
		}
	} while (!err);
}

void Compositor_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	{
		double version;
		err=script.GetField("version", NULL, NULL, &version);
		if (!err)
			printf ("Version=%.2f\n",version);
	}

	Compositor_Props &props=m_CompositorProps;
	script.GetField("bypass_plugin",&props.BypassPlugin);  //just get it here... implement later

	//note: call super here if we derived from other props
	err = script.GetFieldTable("settings");
	if (!err)
	{
		script.GetField("x_scalar", NULL, NULL, &props.X_Scalar);
		script.GetField("y_scalar", NULL, NULL, &props.Y_Scalar);
		err = script.GetFieldTable("square_reticle_props");
		if (!err)
		{
			//clear the default one
			props.square_reticle.clear();
			LoadSquareReticleProps(script,props);
			script.Pop();
		}
		err = script.GetFieldTable("line_plot_props");
		if (!err)
		{
			//clear the default one
			props.lineplot_reticle.clear();
			LoadLinePlotProps(script,props);
			script.Pop();
		}

		LoadPathAlignProps(script,props);

		err = script.GetFieldTable("sequence");
		if (!err)
		{
			//clear defaults
			props.Sequence.clear();  //for now it is assumed the default will not allocate a composite
			LoadSequenceProps(script,props.Sequence,props);

			err = script.GetFieldTable("load_settings");
			{
				if (!err)
				{
					LoadSequence_PersistentData(script,props.Sequence,props);
					script.Pop();
				}
			}

			script.Pop();
		}
		script.Pop();
	}

	err = script.GetFieldTable("controls");
	if (!err)
	{
		m_CompositorControls.LoadFromScript(script);
		script.Pop();
	}
}

static void PixelSystem_to_AimingSystem(int Px,int Py,double &Ax,double &Ay,double XRes=640.0,double YRes=480.0,double AspectRatio=(4.0/3.0))
{
	//Pixel aspect The inverse aspect ratio * the screen res
	//const double PixelAspectRatio=(3.0/4.0)*(XRes/YRes);
	//const double SquareWidthCoefficient=YRes * PixelAspectRatio;

	Ax = ((Px - (XRes/2.0)) / (XRes/2.0)) * AspectRatio;
	Ay = -((Py - (YRes/2.0)) / (YRes/2.0));
}

static void AimingSystem_to_PixelSystem(int &Px,int &Py,double Ax,double Ay,double XRes=640.0,double YRes=480.0,double AspectRatio=(4.0/3.0))
{
	Px=(int)((XRes*AspectRatio + XRes*Ax) / (2.0*AspectRatio));
	Py=(int)(-(((Ay-1)*YRes) / 2.0));
}

const size_t c_RoundRobimBufferSize=2048; //so far 1920 xres is typically the number for a limit
class LinePlot_Retical
{
private:
	template<class T>
	class RoundRobinBuffer
	{
		public:
			RoundRobinBuffer() : m_Head(0), m_Tail(0)
			{
			}
			void push(T value)
			{
				m_Buffer[m_Head++]=value;
				if (m_Head>=c_RoundRobimBufferSize)
					m_Head=0;
			}
			void pop()
			{
				m_Tail++;
				if (m_Tail>=c_RoundRobimBufferSize)
					m_Tail=0;
			}
			size_t size()
			{
				size_t ret=0;
				if (m_Head>m_Tail)
					ret=m_Head-m_Tail;
				else if (m_Head<m_Tail)
					ret=(c_RoundRobimBufferSize-m_Tail) + m_Head;  //wrap around
				return ret;
			}
			inline T& operator [] (size_t index) 
			{ 
				size_t IndexOffset=m_Tail+index;
				if (IndexOffset>=c_RoundRobimBufferSize)
					IndexOffset-=c_RoundRobimBufferSize;
				assert(IndexOffset<c_RoundRobimBufferSize);
				return m_Buffer[IndexOffset];
			}
			inline T operator [] (size_t index) const 
			{ 
				size_t IndexOffset=m_Tail+index;
				if (IndexOffset>=c_RoundRobimBufferSize)
					IndexOffset-=c_RoundRobimBufferSize;
				assert(IndexOffset<c_RoundRobimBufferSize);
				return m_Buffer[IndexOffset];
			}

		private:
			size_t m_Head;
			size_t m_Tail;
			T m_Buffer[c_RoundRobimBufferSize];
	};
	typedef RoundRobinBuffer<size_t> LineQueue;
	struct LineData
	{
		DWORD pixel;
		LineQueue line_queue;
	};
	std::vector<LineData> m_LineDataList;
	bool m_PixelColorCache;
public:
	LinePlot_Retical() : m_PixelColorCache(false)
	{
	}

	void FlushCache()
	{
		//Note: This does not need to be thread safe
		m_PixelColorCache=false;
		m_LineDataList.clear();
	}
	Bitmap_Frame *RenderLinePlotReticle(Bitmap_Frame *Frame,const Compositor_Props::LinePlotReticle_Container_Props &props,bool AddNewData=true)
	{
		typedef Compositor_Props::LinePlotReticle_Container_Props LineProps;

		//first run loop
		if (!m_PixelColorCache)
		{
			for (size_t i=0;i<props.VariablePacket_List.size();i++)
			{
				const LineProps::VariablePacket &variable_pkt=props.VariablePacket_List[i];

				const double R = (double)variable_pkt.rgb[0];
				const double G = (double)variable_pkt.rgb[1];
				const double B = (double)variable_pkt.rgb[2];
				const DWORD Y =(DWORD)( (0.257 * R + 0.504 * G + 0.098 * B) + 16.0);
				const DWORD U =(DWORD)((-0.148 * R - 0.291 * G + 0.439 * B) + 128.0);
				const DWORD V =(DWORD)( (0.439 * R - 0.368 * G - 0.071 * B) + 128.0);

				LineData line_data;
				DWORD &pixel=line_data.pixel;
				pixel=U + (Y<<8) + (V<<16) + (Y<<24);
				//pixel=Y + (V<<8) + (Y<<16) + (U<<24);
				//Ensure the variable is initialized
				SmartDashboard::PutNumber(variable_pkt.Variable_Name,0.0);
				m_LineDataList.push_back(line_data);
			}
			m_PixelColorCache=true;
		}
		for (size_t i=0;i<props.VariablePacket_List.size();i++)
		{
			//push new read onto queue
			const LineProps::VariablePacket &variable_pkt=props.VariablePacket_List[i];
			LineData &line_data=m_LineDataList[i];
			LineQueue &line_queue=line_data.line_queue;
			//Now to compute value to push
			double Value=SmartDashboard::GetNumber(variable_pkt.Variable_Name);
			if (AddNewData)
			{
				size_t YHalfRes=Frame->YRes>>1;
				//Note the Y coordinates have negative above so the scalar has a negative sign to compensate
				size_t PositionY=((size_t)(((Value*-variable_pkt.Scalar)+variable_pkt.Offset)*(double)YHalfRes))+YHalfRes;
				if (PositionY>=Frame->YRes)
					PositionY=Frame->YRes-1;
				line_queue.push(PositionY);
			}

			size_t eox=line_queue.size();
			//Trim the size down to less than x res
			while (eox>Frame->XRes)
			{
				line_queue.pop();
				eox=line_queue.size();
			}
			size_t LineWidthInBytes=Frame->Stride * 4;
			union PixelUVYY
			{
				struct UYVY
				{
					BYTE U,Y,V,Y2;
				} component;
				DWORD raw;
			};
			for (size_t x=0;x<eox;x+=2)
			{
				PixelUVYY source_pixel;
				size_t y=line_queue[x];
				PixelUVYY *pPixel1 =(PixelUVYY *)(Frame->Memory+ (x*2) + (LineWidthInBytes * y));
				source_pixel.raw=pPixel1->raw;
				pPixel1->raw=line_data.pixel;
				pPixel1->component.Y2=source_pixel.component.Y2;


				if ((x+1) < eox)
				{
					size_t y2=line_queue[x+1];
					PixelUVYY *pPixel2 =(PixelUVYY *)(Frame->Memory+ (x*2) + (LineWidthInBytes * y2));
					source_pixel.raw=pPixel2->raw;
					pPixel2->raw=line_data.pixel;
					pPixel2->component.Y=source_pixel.component.Y;
				}
			}
		}
		return Frame;
	}
};

static Bitmap_Frame *RenderSquareReticle(Bitmap_Frame *Frame,double XPos,double YPos,const Compositor_Props::SquareReticle_Props &props,
										 int PixelOffsetX=0,int PixelOffsetY=0,RECT *ExcludeRegion=NULL)
{
	if (g_Framework)
	{
		const size_t XRes=Frame->XRes;
		const size_t YRes=Frame->YRes;
		#if 0
		//Test... make a green box in the center of the frame
		size_t PositionY=bgra_frame.YRes / 2;
		size_t PositionX=bgra_frame.XRes / 2;
		#else
		int Px,Py;
		AimingSystem_to_PixelSystem(Px,Py,XPos,YPos,XRes,YRes,((double)XRes/(double)YRes));
		size_t PositionY=(size_t)(Py + PixelOffsetY);
		size_t PositionX=(size_t)(Px + PixelOffsetX);
		#endif

		//Test bounds
		size_t ThicknessX=props.ThicknessX;
		const size_t ThicknessY=props.ThicknessY;
		const double Opacity=props.opacity;

		if (PositionX<ThicknessX)
			PositionX=ThicknessX;
		else if (PositionX>XRes-ThicknessX)
			PositionX=XRes-ThicknessX;
		if (PositionY<ThicknessY)
			PositionY=ThicknessY;
		else if (PositionY>YRes-ThicknessY)
			PositionY=YRes-ThicknessY;


		size_t LineWidthInBytes=Frame->Stride * 4;

		const double R = (double)props.rgb[0];
		const double G = (double)props.rgb[1];
		const double B = (double)props.rgb[2];

		//These coefficients can work, but I'm using ones that I have tested before
		//http://softpixel.com/~cwright/programming/colorspace/yuv/
		//http://msdn.microsoft.com/en-us/library/aa917087.aspx
		
		//These come from old code in the TargaReader plugin
		const double Y = (0.257 * R + 0.504 * G + 0.098 * B) + 16.0;
		const double U =(-0.148 * R - 0.291 * G + 0.439 * B) + 128.0;
		const double V = (0.439 * R - 0.368 * G - 0.071 * B) + 128.0;

		//for UVYV x has to be even
		PositionX=PositionX&0xFFFE; 
		ThicknessX=ThicknessX&0xFFFE;
		bool ExcludeRowDetected=false;

		for (size_t y=PositionY-ThicknessY;y<PositionY+ThicknessY;y++)
		{
			if (ExcludeRegion)
				ExcludeRowDetected=(((LONG)y>ExcludeRegion->top) && ((LONG)y<ExcludeRegion->bottom)) ? true : false;

			for (size_t x=PositionX-ThicknessX; x<PositionX+ThicknessX; x+=2)
			{
				if ((ExcludeRegion) && (ExcludeRowDetected) && ((LONG)x>ExcludeRegion->left) && ((LONG)x<ExcludeRegion->right)) continue;
				PBYTE pU =(Frame->Memory+ (x*2 + 0) + (LineWidthInBytes * y));
				PBYTE pY =(Frame->Memory+ (x*2 + 1) + (LineWidthInBytes * y));
				PBYTE pV =(Frame->Memory+ (x*2 + 2) + (LineWidthInBytes * y));
				PBYTE pY2=(Frame->Memory+ (x*2 + 3) + (LineWidthInBytes * y));


				*pU =(BYTE)((Opacity*U)+((1.0-Opacity)* (double)(*pU)));
				*pY =(BYTE)((Opacity*Y)+((1.0-Opacity)* (double)(*pY)));
				*pV =(BYTE)((Opacity*V)+((1.0-Opacity)* (double)(*pV)));
				*pY2 =(BYTE)((Opacity*Y)+((1.0-Opacity)* (double)(*pY2)));
			}
		}
	}
	return Frame;
}


// Note: _2Dpoint is defined in Dashboard_Interface.h because it's handy for use with the line drawing function.
struct _3Dpoint
{
	double x, y, z;
	
	_3Dpoint(double xx, double yy, double zz)
	{            
		x = xx;
		y = yy;
		z = zz;
	}
	
	_3Dpoint()
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	inline bool operator==(const _3Dpoint& _Right) const
	{
		return ((x==_Right.x)&&(y==_Right.y)&&(z==_Right.z));
	}
};

class CAMERA
{
public:
	_3Dpoint from;			// camera origin
	_3Dpoint to;			// the point we are looking at in 3 space (NOT a vector)
	_3Dpoint up;			// up vector.
	double angleh, anglev;	// FOVs (in degrees)
	double zoom;			
	double front, back;		// clipping planes
	double roll;			// roll
	short projection;

	CAMERA()
	{
		// These just give us sane starting values. 
		// Don't make camera adjustments here.
		from = _3Dpoint(0, -.5 ,0);
		to = _3Dpoint(0, 1, 0);
		roll = 0;
		up = _3Dpoint(sin(roll), 0, cos(roll));
		angleh = 47.0;
		anglev = 47.0;
		zoom = 1.0;
		front = 0.0;
		back = 250.0;
		projection = 0;
	}

	// A more convenient way to set the camera.
	void SetLookAtFromAngles(_3Dpoint orientation)
	{
		// forward facing vector
		_3Dpoint new_look = _3Dpoint(0, 1, 0);
		new_look.x = sin(orientation.x);
		new_look.y = cos(orientation.x);
		new_look.y *= cos(orientation.y);
		new_look.z = sin(orientation.y);
		to.x = from.x + new_look.x;	
		to.y = from.y + new_look.y;
		to.z = from.z + new_look.z;
		roll = orientation.z;
		up = _3Dpoint(sin(roll), 0, cos(roll));
	}
};

class SCREEN
{
public:
	_2Dpoint center;
	_2Dpoint size;

	SCREEN()
	{
		center = _2Dpoint(640/2, 480/2);
		size = _2Dpoint(640, 480);
	}

	SCREEN(int width, int hight)
	{
		SetSize(width, hight);
	}

	void SetSize(int width, int hight)
	{
		center = _2Dpoint(width/2, hight/2);
		size = _2Dpoint(width, hight);
	}
};

class projection
{
public:
	_2Dpoint p1;	// these are effectively our output points
	_2Dpoint p2;

	CAMERA camera;
	SCREEN screen;

private:
	_3Dpoint origin;
	_3Dpoint e1, e2, n1, n2;
	double tanthetah, tanthetav;
	_3Dpoint basisa, basisb, basisc;
	double EPSILON;
	
public:
	projection()
	{
		EPSILON = 0.001;
		camera = CAMERA();
		screen = SCREEN(640, 480);
		origin = _3Dpoint();
		basisa = _3Dpoint();
		basisb = _3Dpoint();
		basisc = _3Dpoint();
		p1 = _2Dpoint();
		p2 = _2Dpoint();
		e1 = _3Dpoint();
		e2 = _3Dpoint();
		n1 = _3Dpoint();
		n2 = _3Dpoint();
		assert(Trans_Initialise() == true);
	}

	// This is for convenience.
	// Note that if you want to change camera settings like position, or orientation, 
	// you have to change camera members, then call Trans_Initialize() 
	projection(_3Dpoint Cam_Origin, _3Dpoint Cam_LookAt, double FOV, double Cam_Zoom )
	{
		EPSILON = 0.001;
		camera = CAMERA();
		camera.from = Cam_Origin;
		camera.to = Cam_LookAt;
		camera.angleh = FOV;
		camera.anglev = FOV;
		screen = SCREEN(640, 480);
		origin = _3Dpoint();
		basisa = _3Dpoint();
		basisb = _3Dpoint();
		basisc = _3Dpoint();
		p1 = _2Dpoint();
		p2 = _2Dpoint();
		e1 = _3Dpoint();
		e2 = _3Dpoint();
		n1 = _3Dpoint();
		n2 = _3Dpoint();
		assert(Trans_Initialise() == true);
	}

	bool Trans_Initialise()
	{
		/* Is the camera position and view vector coincident ? */
		if (EqualVertex(camera.to, camera.from))
		{
			return (false);
		}

		/* Is there a legal camera up vector ? */
		if (EqualVertex(camera.up, origin))
		{
			return (false);
		}

		basisb.x = camera.to.x - camera.from.x;
		basisb.y = camera.to.y - camera.from.y;
		basisb.z = camera.to.z - camera.from.z;
		Normalise(basisb);
		basisa= CrossProduct(camera.up, basisb);
		Normalise(basisa);

		/* Are the up vector and view direction colinear */
		if (EqualVertex(basisa, origin))
		{
			return (false);
		}

		basisc=CrossProduct(basisb, basisa);

		/* Do we have legal camera apertures ? */
		if (camera.angleh < EPSILON || camera.anglev < EPSILON)
		{
			return (false);
		}

		/* Calculate camera aperture statics, note: angles in degrees */
		tanthetah = tan(DEG_2_RAD(camera.angleh) / 2);
		tanthetav = tan(DEG_2_RAD(camera.anglev) / 2);

		/* Do we have a legal camera zoom ? */
		if (camera.zoom < EPSILON)
		{
			return (false);
		}

		/* Are the clipping planes legal ? */
		if (camera.front < 0 || camera.back < 0 || camera.back <= camera.front)
		{
			return (false);
		}

		return true;
	}

	bool Trans_Line(_3Dpoint w1, _3Dpoint w2)
	{
		Trans_World2Eye(w1, e1);
		Trans_World2Eye(w2, e2);
		if (Trans_ClipEye(e1, e2))
		{
			Trans_Eye2Norm(e1, n1);
			Trans_Eye2Norm(e2, n2);
			if (Trans_ClipNorm(n1, n2))
			{
				Trans_Norm2Screen(n1, p1);
				Trans_Norm2Screen(n2, p2);
				return (true);
			}
		}

		return (true);
	}

	void Normalise(_3Dpoint& v)
	{
		double length;
		length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}

	_3Dpoint CrossProduct(_3Dpoint p1,_3Dpoint p2)
	{	
		_3Dpoint p3;
		p3 = _3Dpoint(0,0,0);
		p3.x = p1.y * p2.z - p1.z * p2.y;
		p3.y = p1.z * p2.x - p1.x * p2.z;
		p3.z = p1.x * p2.y - p1.y * p2.x;

		return p3;
	}

	bool EqualVertex(_3Dpoint p1, _3Dpoint p2)
	{
		if (abs(p1.x - p2.x) > EPSILON)
			return(false);
		if (abs(p1.y - p2.y) > EPSILON)
			return(false);
		if (abs(p1.z - p2.z) > EPSILON)
			return(false);

		return(true);
	}

private:
	void Trans_World2Eye(_3Dpoint w, _3Dpoint& e)
	{
		/* Translate world so that the camera is at the origin */
		w.x -= camera.from.x;
		w.y -= camera.from.y;
		w.z -= camera.from.z;

		/* Convert to eye coordinates using basis vectors */
		e.x = w.x * basisa.x + w.y * basisa.y + w.z * basisa.z;
		e.y = w.x * basisb.x + w.y * basisb.y + w.z * basisb.z;
		e.z = w.x * basisc.x + w.y * basisc.y + w.z * basisc.z;
	}

	bool Trans_ClipEye(_3Dpoint& e1, _3Dpoint& e2)
	{
		double mu;

		/* Is the vector totally in front of the front cutting plane ? */
		if (e1.y <= camera.front && e2.y <= camera.front)
		{
			return (false);
		}

		/* Is the vector totally behind the back cutting plane ? */
		if (e1.y >= camera.back && e2.y >= camera.back)
		{
			return (false);
		}

		/* Is the vector partly in front of the front cutting plane ? */
		if ((e1.y < camera.front && e2.y > camera.front) ||
			(e1.y > camera.front && e2.y < camera.front))
		{
			mu = (camera.front - e1.y) / (e2.y - e1.y);
			if (e1.y < camera.front)
			{
				e1.x = e1.x + mu * (e2.x - e1.x);
				e1.z = e1.z + mu * (e2.z - e1.z);
				e1.y = camera.front;
			}
			else
			{
				e2.x = e1.x + mu * (e2.x - e1.x);
				e2.z = e1.z + mu * (e2.z - e1.z);
				e2.y = camera.front;
			}
		}

		/* Is the vector partly behind the back cutting plane ? */
		if ((e1.y < camera.back && e2.y > camera.back) ||
			(e1.y > camera.back && e2.y < camera.back))
		{ 
			mu = (camera.back - e1.y) / (e2.y - e1.y);
			if (e1.y < camera.back)
			{
				e2.x = e1.x + mu * (e2.x - e1.x);
				e2.z = e1.z + mu * (e2.z - e1.z);
				e2.y = camera.back;
			}
			else
			{
				e1.x = e1.x + mu * (e2.x - e1.x);
				e1.z = e1.z + mu * (e2.z - e1.z);
				e1.y = camera.back;
			}
		}

		return (true);
	}

	void Trans_Eye2Norm(_3Dpoint e, _3Dpoint& n)
	{
		double d;
		if (camera.projection == 0)
		{
			d = camera.zoom / e.y;
			n.x = d * e.x / tanthetah;
			n.y = e.y;
			n.z = d * e.z / tanthetav;
		}
		else
		{
			n.x = camera.zoom * e.x / tanthetah;
			n.y = e.y;
			n.z = camera.zoom * e.z / tanthetav;
		}
	}

	bool Trans_ClipNorm(_3Dpoint& n1, _3Dpoint& n2)
	{
		double mu;
	
		/* Is the line segment totally right of x = 1 ? */
		if (n1.x >= 1 && n2.x >= 1)
			return (false);

		/* Is the line segment totally left of x = -1 ? */
		if (n1.x <= -1 && n2.x <= -1)
			return (false);

		/* Does the vector cross x = 1 ? */
		if ((n1.x > 1 && n2.x < 1) || (n1.x < 1 && n2.x > 1))
		{
			mu = (1 - n1.x) / (n2.x - n1.x);
			if (n1.x < 1)
			{
				n2.z = n1.z + mu * (n2.z - n1.z);
				n2.x = 1;
			}
			else
			{
				n1.z = n1.z + mu * (n2.z - n1.z);
				n1.x = 1;
			}
		}

		/* Does the vector cross x = -1 ? */
		if ((n1.x < -1 && n2.x > -1) || (n1.x > -1 && n2.x < -1))
		{
			mu = (-1 - n1.x) / (n2.x - n1.x);
			if (n1.x > -1)
			{
				n2.z = n1.z + mu * (n2.z - n1.z);
				n2.x = -1;
			}
			else
			{
				n1.z = n1.z + mu * (n2.z - n1.z);
				n1.x = -1;
			}
		}

		/* Is the line segment totally above z = 1 ? */
		if (n1.z >= 1 && n2.z >= 1)
			return (false);

		/* Is the line segment totally below z = -1 ? */
		if (n1.z <= -1 && n2.z <= -1)
			return (false);

		/* Does the vector cross z = 1 ? */
		if ((n1.z > 1 && n2.z < 1) || (n1.z < 1 && n2.z > 1))
		{
			mu = (1 - n1.z) / (n2.z - n1.z);
			if (n1.z < 1)
			{
				n2.x = n1.x + mu * (n2.x - n1.x);
				n2.z = 1;
			}
			else
			{
				n1.x = n1.x + mu * (n2.x - n1.x);
				n1.z = 1;
			}
		}

		/* Does the vector cross z = -1 ? */
		if ((n1.z < -1 && n2.z > -1) || (n1.z > -1 && n2.z < -1))
		{
			mu = (-1 - n1.z) / (n2.z - n1.z);
			if (n1.z > -1)
			{
				n2.x = n1.x + mu * (n2.x - n1.x);
				n2.z = -1;
			}
			else
			{
				n1.x = n1.x + mu * (n2.x - n1.x);
				n1.z = -1;
			}
		}

		return (true);

	}

	void Trans_Norm2Screen(_3Dpoint norm, _2Dpoint& projected)
	{
		double aspect = (double)screen.size.h / (double)screen.size.v;
		projected.h = (int)(screen.center.h - screen.size.h * norm.x / 2);
		projected.v = (int)(screen.center.v - screen.size.v * aspect * norm.z / 2);
	}
};


class PathRenderer
{
public:
	PathRenderer(void)
	{
		NumPoints = 10;
		Left = new _3Dpoint[NumPoints + 1];
		Center = new _3Dpoint[NumPoints + 1];
		Right = new _3Dpoint[NumPoints + 1];
		Translation = new _3Dpoint[NumPoints + 1];

		angle = -45;
		dir = 1;

		draw_cube = false;
		// poitns for cube - 1 x 1 meter centered at 0,0,0.
		cube[0].x = -0.5, cube[0].y =  0.5, cube[0].z =  0.5;
		cube[1].x =  0.5, cube[1].y =  0.5, cube[1].z =  0.5;
		cube[2].x = -0.5, cube[2].y =  0.5, cube[2].z = -0.5;
		cube[3].x =  0.5, cube[3].y =  0.5, cube[3].z = -0.5;
		cube[4].x = -0.5, cube[4].y = -0.5, cube[4].z =  0.5;
		cube[5].x =  0.5, cube[5].y = -0.5, cube[5].z =  0.5;
		cube[6].x = -0.5, cube[6].y = -0.5, cube[6].z = -0.5;
		cube[7].x =  0.5, cube[7].y = -0.5, cube[7].z = -0.5;

		// these should probably be pulled from LUA values.
		width = 15 * 0.0254;	// 15 inches in meters (robot is 24)	This is the width of our drawn path.
		length = 33 * 0.0254;	// 33 inches in meters					This is how far to project our path.
		pivot_point_length = 22 * 0.0254;	// assumes pivot point is 2/3s from front.		This is the distance from the pivot point to the front, center of the robot frame.

		forward_velocity = 0;	// just setting to known values. these should be set per frame.
		angular_velocity = 0;
	}

	void Initialize(const Compositor_Props::PathAlign_Container_Props &props)
	{
		width=props.width;
		length=props.length;
		pivot_point_length=props.pivot_point_length;
		_3Dpoint Camera_position = _3Dpoint(props.pos_x,props.pos_y,props.pos_z);
		_3Dpoint Camara_LookAt = _3Dpoint(props.rot_x,props.rot_y,props.rot_z);
		m_lastOrientation=Camara_LookAt;
		draw_cube=props.draw_cube;

		projector = projection();
		projector.camera.from = Camera_position;
		projector.camera.SetLookAtFromAngles(Camara_LookAt);
		projector.camera.anglev = props.FOV_y;
		projector.camera.angleh = props.FOV_x;
		projector.Trans_Initialise();
	}

	~PathRenderer(void)
	{
		delete[] Left;
		delete[] Center;
		delete[] Right;
		delete[] Translation;
	}

	int NumPoints;

	double width;				// width in meters of the path (generally width of the robot drive)
	double length;				// length in meters of predicted path
	double pivot_point_length;	// length in meters from the drive pivot point to the front of the robot.

	double forward_velocity;	// meters per second
	double angular_velocity;	// radians per second

	bool draw_cube;				// if true, render a cube.
	_3Dpoint cube[8];
	int angle;
	int dir;
	
	projection projector;		// projection class

	// Compute the path mesh. This is for the front of the robot which is offset from the pivot point at ground level.
	// This should be called prior to each render, but could be called only when velocities change.
	void ComputePathPoints(double Foward_Vel, double Angular_Vel)
	{
		if( draw_cube || Translation == NULL || Center == NULL || Right == NULL || Left == NULL ) 
			return;

		forward_velocity = Foward_Vel;
		angular_velocity = Angular_Vel;

		double time_to_length = forward_velocity > 0 ? length / forward_velocity : 0;
		double end_angle = angular_velocity != 0 ? angular_velocity * time_to_length : length;
		
		double path_radius = angular_velocity != 0 ? forward_velocity / angular_velocity : 0;
		double left_radius = path_radius - width / 2;
		double right_radius = path_radius + width / 2;

		double angle = 0;
		for(int i = 0; i <= NumPoints; i++)
		{
			// calculate the front end translation first
			Translation[i].x = angular_velocity != 0 ? cos(angle + M_PI/2) * pivot_point_length : 0;
			Translation[i].y = angular_velocity != 0 ? sin(angle + M_PI/2) * pivot_point_length : pivot_point_length;

			Center[i].x = angular_velocity != 0 ? (cos(angle) * path_radius - path_radius) + Translation[i].x : 0 + Translation[i].x;
			Center[i].y = angular_velocity != 0 ? (sin(angle) * path_radius) + Translation[i].y - pivot_point_length : angle + Translation[i].y - pivot_point_length;
			Center[i].z = 0.0;

			Left[i].x = angular_velocity != 0 ? (cos(angle) * left_radius - path_radius) + Translation[i].x : left_radius + Translation[i].x;
			Left[i].y = angular_velocity != 0 ? (sin(angle) * left_radius) + Translation[i].y - pivot_point_length : angle + Translation[i].y - pivot_point_length;
			Left[i].z = 0.0;

			Right[i].x = angular_velocity != 0 ? (cos(angle) * right_radius - path_radius) + Translation[i].x : right_radius + Translation[i].x;
			Right[i].y = angular_velocity != 0 ? (sin(angle) * right_radius) + Translation[i].y - pivot_point_length : angle + Translation[i].y - pivot_point_length;
			Right[i].z = 0.0;

			angle += end_angle/NumPoints;
		}
	}

	
	//This is almost a wrapper to the projection call... except that it will implicitly check for difference before making the call
	void Compute_LookAtFromAngles(double Yaw,double Pitch)
	{
		_3Dpoint orientation(Yaw,Pitch,m_lastOrientation.z);
		if (!(orientation==m_lastOrientation))
		{
			projector.camera.SetLookAtFromAngles(orientation);
			projector.Trans_Initialise();
			m_lastOrientation=orientation;
		}
	}

	// draw the path mesh
	Bitmap_Frame* RenderPath(Bitmap_Frame* Frame)
	{
		if (g_Framework)
		{
			unsigned int col[] = { 128, 255, 128, 255 };

			projector.screen.SetSize(Frame->XRes, Frame->YRes);

			if( !draw_cube )
			{	// draw the normal path
				projector.Trans_Line(Left[0], Right[0]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				for (int i = 0; i < NumPoints; i++)
				{
					projector.Trans_Line(Left[i], Left[i + 1]);
					g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

					projector.Trans_Line(Center[i], Center[i + 1]);
					g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

					projector.Trans_Line(Right[i], Right[i + 1]);
					g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

					projector.Trans_Line(Left[i + 1], Right[i + 1]);
					g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);
				}
			}
			else	// test cube.
			{	// animation. sweep back and forth.
				//_3Dpoint Camera_LookAt = _3Dpoint(DEG_2_RAD(angle),DEG_2_RAD(-45), 0);
				//angle += dir;
				//if(angle < -45) dir = 1;
				//if(angle > 45) dir = -1;

				//projector.camera.SetLookAtFromAngles(Camera_LookAt);
				//projector.Trans_Initialise();

				projector.Trans_Line(cube[0], cube[1]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[0], cube[2]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[1], cube[3]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[2], cube[3]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[4], cube[5]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[4], cube[6]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[5], cube[7]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[6], cube[7]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[0], cube[4]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[1], cube[5]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[2], cube[6]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(cube[3], cube[7]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);
			}
		}

		return Frame;
	}

private:
	_3Dpoint* Left;
	_3Dpoint* Center;
	_3Dpoint* Right;
	_3Dpoint* Translation;

	_3Dpoint m_lastOrientation;  //used by Compute_LookAtFromAngles
};


class Bypass_Reticle
{
	private:
	HMODULE m_PlugIn;
	std::string IPAddress;
	Dashboard_Framework_Interface *DashboardHelper;

	typedef Bitmap_Frame * (*DriverProc_t)(Bitmap_Frame *Frame);
	DriverProc_t m_DriverProc;

	typedef void (*function_Initialize) (const char *IPAddress,Dashboard_Framework_Interface *DashboardHelper);
	function_Initialize m_fpInitialize;

	typedef void (*function_void) ();
	function_void m_fpShutdown;

	typedef Plugin_Controller_Interface * (*function_create_plugin_controller_interface) ();
	function_create_plugin_controller_interface m_CreatePluginControllerInterface;
	typedef  void (*function_destroy_plugin_controller_interface)(Plugin_Controller_Interface *);
	function_destroy_plugin_controller_interface m_DestroyPluginControllerInterface;

	Plugin_Controller_Interface *m_pPluginControllerInterface;

	public:
	Bypass_Reticle(const char *_IPAddress,Dashboard_Framework_Interface *_DashboardHelper) : m_PlugIn(NULL),IPAddress(_IPAddress),DashboardHelper(_DashboardHelper), 
		m_DriverProc(NULL),m_pPluginControllerInterface(NULL)
	{
	}

	void FlushPlugin()
	{
		if (m_pPluginControllerInterface)
		{
			(*m_DestroyPluginControllerInterface)(m_pPluginControllerInterface);
			m_pPluginControllerInterface=NULL;
		}

		if (m_PlugIn)
		{
			FreeLibrary(m_PlugIn);
			m_PlugIn = NULL;
		}
	}

	void Callback_Initialize() {if (m_PlugIn) (*m_fpInitialize)(IPAddress.c_str(),DashboardHelper);}
	void Callback_Shutdown() {if (m_PlugIn) (*m_fpShutdown)();}
	~Bypass_Reticle()
	{
		//Note: we can move this earlier if necessary
		Callback_Shutdown();
		FlushPlugin();
	}

	Bitmap_Frame *Callback_ProcessFrame_UYVY(Bitmap_Frame *Frame)
	{
		Bitmap_Frame *ret=Frame;
			if (m_PlugIn)
				ret=(*m_DriverProc)(Frame);
		return ret;
	}

	Plugin_Controller_Interface* GetPluginInterface(void) 
	{
		if ((m_PlugIn)&&(m_pPluginControllerInterface==NULL)&&(m_CreatePluginControllerInterface))
			m_pPluginControllerInterface=(*m_CreatePluginControllerInterface)();
		return m_pPluginControllerInterface;
	}

	void LoadPlugIn(const char Plugin[])
	{
		FlushPlugin();  //ensure its not already loaded
		m_DriverProc = NULL;  //this will avoid crashing if others fail
		m_fpInitialize = NULL;
		m_fpShutdown = NULL;
		m_CreatePluginControllerInterface=NULL;
		m_DestroyPluginControllerInterface=NULL;

		m_PlugIn=LoadLibraryA(Plugin);

		if (m_PlugIn)
		{
			try
			{

				m_DriverProc=(DriverProc_t) GetProcAddress(m_PlugIn,"ProcessFrame_UYVY");
				if (!m_DriverProc) throw 1;
				m_fpInitialize=(function_Initialize) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_Initialize");
				if (!m_fpInitialize) throw 2;
				m_fpShutdown=(function_void) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_Shutdown");
				if (!m_fpShutdown) throw 3;
				size_t Tally=0;
				//This may be NULL if there are no controls for it
				m_CreatePluginControllerInterface=
					(function_create_plugin_controller_interface) GetProcAddress(m_PlugIn,"Callback_CreatePluginControllerInterface");
				m_DestroyPluginControllerInterface=
					(function_destroy_plugin_controller_interface) GetProcAddress(m_PlugIn,"Callback_DestroyPluginControllerInterface");

				//either all or nothing of this group of functions
				if ((Tally!=0)&&(Tally!=3))
				{
					assert(false);
					throw 4;
				}
			}
			catch (int ErrorCode)
			{
				m_DriverProc = NULL;  //this will avoid crashing if others fail
				m_fpInitialize = NULL;
				m_fpShutdown = NULL;
				FrameWork::DebugOutput("ProcessingVision Plugin failed error code=%d",ErrorCode);
				FlushPlugin();
			}
		}
	}
};

class Compositor
{
	public:
		void ResetPos()
		{
			if (m_IsEditable)
			{
				m_Xpos=0.0,m_Ypos=0.0;
				SmartDashboard::PutNumber("X Position",0.0);
				SmartDashboard::PutNumber("Y Position",0.0);
			}
		}
		void SetXAxis(double value)
		{
			if (m_IsEditable)
			{
				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				SmartDashboard::PutNumber("X Position",value);
				m_Xpos+= (value * props.X_Scalar);
			}
		}
		void SetYAxis(double value)
		{
			if (m_IsEditable)
			{
				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				SmartDashboard::PutNumber("Y Position",value);
				m_Ypos+= (value * props.Y_Scalar);
			}
		}
		void UpdateSequence(size_t NewSequenceIndex,bool forceUpdate=false)
		{
			if ((m_SequenceIndex!=NewSequenceIndex)||forceUpdate)
			{
				Compositor_Props::Sequence_List &Sequence=*(const_cast<Compositor_Props::Sequence_List *>(m_pSequence));
				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				//Save current position to this sequence packet
				Sequence[m_SequenceIndex].PositionX=m_Xpos;
				Sequence[m_SequenceIndex].PositionY=m_Ypos;

				//For recursive stepping we can determine if we start the index from the end or beginning from looking at the old and new value
				const bool FromNext=NewSequenceIndex<m_SequenceIndex;
				//issue the update
				m_SequenceIndex=NewSequenceIndex;
				m_LinePlot.FlushCache();  //there may be a new list of colors on the next sequence
				const Compositor_Props::Sequence_Packet &seq_pkt=Sequence[m_SequenceIndex];
				bool CompositeUpdate=false;
				if ((m_IsEditable)&&(m_RecurseIntoComposite)&&(seq_pkt.type==Compositor_Props::eComposite))
				{
					//check this before pushing onto the stack
					if (m_PositionTracker.empty())
						SmartDashboard::PutNumber("Sequence",(double)(m_SequenceIndex+1));
					PositionPacket pkt;
					pkt.pSequence=m_pSequence;
					pkt.SequenceIndex=m_SequenceIndex;
					pkt.m_Xpos=m_Xpos_Offset;
					pkt.m_Ypos=m_Ypos_Offset;
					m_PositionTracker.push(pkt);  //keep track of where we were we can pop out once we hit the ends
					//Now to step in to the sub sequence
					m_pSequence=seq_pkt.specific_data.Composite;
					//add the overall parent offset
					m_Xpos_Offset+=seq_pkt.PositionX;
					m_Ypos_Offset+=seq_pkt.PositionY;
					//set the index to the begin or end based on the direction it happened
					assert(m_pSequence->size()); //we should not have any empty sequences! (LUA need to have at least one per level)
					m_SequenceIndex=FromNext?m_pSequence->size()-1:0;
					NewSequenceIndex=m_SequenceIndex;
					CompositeUpdate=true;
				}
		
				if (m_SequenceIndex==-1)
				{
					//reached beginning
					if (!m_PositionTracker.empty())
					{
						const PositionPacket &pkt=m_PositionTracker.top();
						m_PositionTracker.pop();
						//restore to parent
						m_pSequence=pkt.pSequence;
						m_SequenceIndex=pkt.SequenceIndex;  //go back where we were in a previous case
						m_Xpos_Offset=pkt.m_Xpos;
						m_Ypos_Offset=pkt.m_Ypos;
						NewSequenceIndex=m_SequenceIndex-1;
						CompositeUpdate=true;
					}
					else
						m_SequenceIndex=Sequence.size()-1;
				}
				else if (m_SequenceIndex>=Sequence.size())
				{
					//reached end
					if (!m_PositionTracker.empty())
					{
						const PositionPacket &pkt=m_PositionTracker.top();
						m_PositionTracker.pop();
						//restore to parent
						m_pSequence=pkt.pSequence;
						m_SequenceIndex=pkt.SequenceIndex;  
						m_Xpos_Offset=pkt.m_Xpos;
						m_Ypos_Offset=pkt.m_Ypos;
						//advance to next
						NewSequenceIndex=m_SequenceIndex+1;
						CompositeUpdate=true;
					}
					else
						m_SequenceIndex=0;
				}
				if (m_PositionTracker.empty())
					SmartDashboard::PutNumber("Sequence",(double)(m_SequenceIndex+1));
				//Modify position to last saved... using m_pSequence... since this may change when stepping into composite
				m_Xpos=(*m_pSequence)[m_SequenceIndex].PositionX;
				m_Ypos=(*m_pSequence)[m_SequenceIndex].PositionY;
				if (CompositeUpdate)
					UpdateSequence(NewSequenceIndex,true);
			}
		}

		void NextSequence()
		{
			UpdateSequence(m_SequenceIndex+1);
		}
		void PreviousSequence()
		{
			UpdateSequence(m_SequenceIndex-1);
		}
		void SetPOV (double value)
		{
			//We put the typical case first (save the amount of branching)
			if (value!=-1)
			{
				if (!m_POVSetValve)
				{
					m_POVSetValve=true;
					//so breaking down the index
					//0 = up
					//1 = up right
					//2 = right
					//3 = down right
					//4 = down
					//5 = down left
					//6 = left
					//7 = left up
					size_t index=(size_t)(value/45.0);
					switch (index)
					{
					case 2: NextSequence();	break;
					case 6: PreviousSequence();	break;
					}
				}
			}
			else 
				m_POVSetValve=false;
		}
		void ToggleLinePlot()
		{
			m_ToggleLinePlot=!m_ToggleLinePlot;
		}
		IEvent::HandlerList ehl;
		Compositor(const char *IPAddress,Dashboard_Framework_Interface *DashboardHelper) : m_Bypass(IPAddress,DashboardHelper),m_JoyBinder(FrameWork::GetDirectInputJoystick()),
			m_SequenceIndex(0),m_pSequence(NULL),m_BlinkCounter(0),m_Xpos(0.0),m_Ypos(0.0),m_Xpos_Offset(0.0),m_Ypos_Offset(0.0),
			m_IsEditable(false),m_PreviousIsEditable(false),m_RecurseIntoComposite(false),m_Flash(false),m_ToggleLinePlot(true)
		{
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Subscribe(ehl,*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Subscribe(ehl,*this, &Compositor::SetYAxis);
			em->Event_Map["NextSequence"].Subscribe(ehl, *this, &Compositor::NextSequence);
			em->Event_Map["PreviousSequence"].Subscribe(ehl, *this, &Compositor::PreviousSequence);
			em->EventValue_Map["SequencePOV"].Subscribe(ehl,*this, &Compositor::SetPOV);
			em->Event_Map["ToggleLinePlot"].Subscribe(ehl,*this, &Compositor::ToggleLinePlot);
			em->Event_Map["ResetPos"].Subscribe(ehl, *this, &Compositor::ResetPos);

			//m_RecurseIntoComposite=true; //testing  (TODO implement in menu)
			SmartDashboard::PutNumber("Velocity",1.0);
			SmartDashboard::PutNumber("Rotation Velocity",0.0);
		}

		void SaveData_Sequence(std::ofstream &out,const Compositor_Props::Sequence_List &sequence,size_t RecursiveCount=0)
		{
			using namespace std;
			if (RecursiveCount==0)
				out << "sequence_load = " << endl;    //header
			else
			{
				for (size_t i=0;i<RecursiveCount;i++)
					out << "\t\t";
				out << "composite = " << endl;    //header
			}

			for (size_t i=0;i<RecursiveCount;i++)
				out << "\t\t";
			out << "{" << endl;
			for (size_t i=0;i<sequence.size();i++)
			{
				const Compositor_Props::Sequence_Packet &seq_pkt=sequence[i];
				if (RecursiveCount==0)
					out << "\t" << "sequence_" << (i+1) << " = {";
				else
				{
					for (size_t j=0;j<RecursiveCount;j++)
						out << "\t\t";
					out << "\t" << "composite_" << (i+1) << " = {";
				}
				out << "type=" << "\"" << Compositor_Props::GetReticleType_String(seq_pkt.type) << "\", ";
				//TODO write specific data types here... which should be the same expect for the composite type
				out << "x=" << seq_pkt.PositionX << ", " << "y=" << seq_pkt.PositionY << " ";
				if (seq_pkt.type == Compositor_Props::eComposite)
				{
					out << ",\n";
					Compositor_Props::Sequence_List *Composite=seq_pkt.specific_data.Composite;
					//recursively call this within the composite list to be used as the sequence
					SaveData_Sequence(out,*Composite,RecursiveCount+1);
				}
				out << "}," << endl;
			}

			for (size_t i=0;i<RecursiveCount;i++)
				out << "\t\t";
			out << "}" << endl;  //footer

			for (size_t j=0;j<RecursiveCount;j++)
				out << "\t";
		}

		void SaveData()
		{
			using namespace std;
			string OutFile = "CompositorSave.lua";
			string output;

			ofstream out(OutFile.c_str(), std::ios::out );

			const Compositor_Props::Sequence_List &sequence= m_CompositorProperties.GetCompositorProps().Sequence;
			SaveData_Sequence(out,sequence);
		}
		~Compositor()
		{
			//ensure the current position is set on the properties
			{
				Compositor_Props::Sequence_List &sequence_rw= m_CompositorProperties.GetCompositorProps_rw().Sequence;
				Compositor_Props::Sequence_Packet &seq_pkt_rw= sequence_rw[m_SequenceIndex];
				seq_pkt_rw.PositionX=m_Xpos,  seq_pkt_rw.PositionY=m_Ypos;
			}
		
			SaveData();
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Remove(*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Remove(*this, &Compositor::SetYAxis);
			em->Event_Map["NextSequence"].Remove(*this, &Compositor::NextSequence);
			em->Event_Map["PreviousSequence"].Remove(*this, &Compositor::PreviousSequence);
			em->EventValue_Map["SequencePOV"].Remove(*this, &Compositor::SetPOV);
			em->Event_Map["ToggleLinePlot"].Remove(*this, &Compositor::ToggleLinePlot);
			em->Event_Map["ResetPos"].Remove(*this, &Compositor::ResetPos);
			
			m_CompositorProperties.Get_CompositorControls().BindAdditionalUIControls(false,&m_JoyBinder,NULL);
			//For now limit the sequence to advance and previous calls to save network bandwidth... we can move this to the time change if necessary
			SmartDashboard::PutNumber("Sequence",(double)(m_SequenceIndex+1));
			m_CompositorProperties.GetCompositorProps_rw().DestroyCompositeList();
		}
		virtual void Initialize(EventMap& em, const Compositor_Properties *props=NULL)
		{
			//props may be NULL if there is no lua
			if (props)
			{
				m_CompositorProperties=*props;
				m_CompositorProperties.Get_CompositorControls().BindAdditionalUIControls(true,&m_JoyBinder,NULL);
				//Setup the initial coordinates
				const Compositor_Props::Sequence_List &sequence= props->GetCompositorProps().Sequence;
				m_pSequence=&m_CompositorProperties.GetCompositorProps().Sequence;
				//Setup our sequence display and positions
				if (sequence.size())
				{
					const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
					m_Xpos=props.Sequence[m_SequenceIndex].PositionX;
					m_Ypos=props.Sequence[m_SequenceIndex].PositionY;
					UpdateSequence(m_SequenceIndex,true);
				}
				if (props->GetCompositorProps().BypassPlugin.c_str()[0]!=0)
				{
					m_Bypass.LoadPlugIn(props->GetCompositorProps().BypassPlugin.c_str());					
					m_Bypass.Callback_Initialize();  //will implicitly handle error
				}
				m_PathPlotter.Initialize(m_CompositorProperties.GetCompositorProps().PathAlign);
			}
			else
				m_pSequence=&m_CompositorProperties.GetCompositorProps().Sequence;  //Assign pointer for default properties case

			//Bind the compositor's eventmap to the joystick
			m_JoyBinder.SetControlledEventMap(&m_EventMap);
		}
		FrameWork::EventMap &GetEventMap_rw() {return m_EventMap;}

		void SetIsEditable(bool Edit)
		{
			if (SmartDashboard::IsConnected())
				SmartDashboard::PutBoolean("Edit Position",Edit);
			else
				m_IsEditable=Edit;
		}

		bool GetIsEditable() const
		{
			return m_IsEditable;
		}


		
		double CheckX(double Xpos) const
		{
			const double x_limit=(m_Frame->XRes>m_Frame->YRes)?(double)m_Frame->XRes/(double)m_Frame->YRes : 1.0;
			if (Xpos>x_limit)
				Xpos=x_limit;
			else if (m_Xpos<-x_limit)
				Xpos=-x_limit;
			return Xpos;
		}

		double CheckY(double Ypos) const
		{
			const double y_limit=(m_Frame->YRes>m_Frame->XRes)?(double)m_Frame->YRes/(double)m_Frame->XRes : 1.0;
			if (Ypos>y_limit)
				Ypos=y_limit;
			else if (Ypos<-y_limit)
				Ypos=-y_limit;
			return Ypos;
		}

		//TODO work out unresolved external symbol of Framework::IsZero()
		inline bool IsZero(double value,double tolerance=1e-5)
		{
			return fabs(value)<tolerance;
		}

		Bitmap_Frame *Render_Reticle(Bitmap_Frame *Frame,const Compositor_Props::Sequence_List &sequence,size_t SequenceIndex,double XOffset=0.0,double YOffset=0.0)
		{
			const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
			Bitmap_Frame *ret=Frame;
			bool EnableFlash=false;
			if (m_IsEditable)
			{
				if ((&sequence==m_pSequence)&&(SequenceIndex==m_SequenceIndex))
				{
					m_Flash=(m_BlinkCounter++&0x10)!=0;
					m_BlinkCounter=(m_BlinkCounter&0x1f);
					EnableFlash=true;
				}
				else if (!m_RecurseIntoComposite)
					EnableFlash=true;
			}
			const Compositor_Props::Sequence_Packet &seq_pkt=sequence[SequenceIndex];
			switch (seq_pkt.type)
			{
			case Compositor_Props::eDefault:
				{
					//copy the props to alter the opacity for blinking
					Compositor_Props::SquareReticle_Container_Props sqr_props=props.square_reticle[seq_pkt.specific_data.SquareReticle_SelIndex];
					double Xpos,Ypos;
					if (m_RecurseIntoComposite)
					{
						Xpos=EnableFlash?m_Xpos+XOffset:seq_pkt.PositionX+XOffset;
						Ypos=EnableFlash?m_Ypos+YOffset:seq_pkt.PositionY+YOffset;
						if (EnableFlash)
						{
							Xpos=CheckX(Xpos);
							m_Xpos=Xpos-XOffset;
							Ypos=CheckY(Ypos);
							m_Ypos=Ypos-YOffset;
						}
					}
					else
					{
						//If we are not the top layer then we are always locked down if we are not recursing
						bool UseInput=(&sequence==&props.Sequence) && EnableFlash;
						Xpos=UseInput?m_Xpos:seq_pkt.PositionX+XOffset;
						Ypos=UseInput?m_Ypos:seq_pkt.PositionY+YOffset;
						if (EnableFlash)
						{
							Xpos=CheckX(Xpos);
							double TempXpos=UseInput?Xpos:Xpos-seq_pkt.PositionX;
							m_Xpos=(TempXpos<0)?std::max(TempXpos,m_Xpos):std::min(TempXpos,m_Xpos);

							Ypos=CheckY(Ypos);
							double TempYpos=UseInput?Ypos:Ypos-seq_pkt.PositionY;
							m_Ypos=(TempXpos<0)?std::max(TempYpos,m_Ypos):std::min(TempYpos,m_Ypos);
						}
					}

					if (EnableFlash)
					{
						if (!m_Flash)
							sqr_props.primary.opacity*=0.5;
					}
					if (sqr_props.UsingShadow)
					{
						sqr_props.shadow.opacity*=0.5;
						RECT ExcludeRegion,*pExcludeRegion=NULL;
						if (sqr_props.ExcludeRegion)
						{
							const size_t XRes=Frame->XRes;
							const size_t YRes=Frame->YRes;
							int Px,Py;
							AimingSystem_to_PixelSystem(Px,Py,Xpos,Ypos,XRes,YRes,((double)XRes/(double)YRes));
							pExcludeRegion=&ExcludeRegion;
							size_t PositionY=(size_t)(Py);
							size_t PositionX=(size_t)(Px) & 0xFFFE;
							size_t ThicknessX=sqr_props.primary.ThicknessX & 0xFFFE;
							ExcludeRegion.top=(LONG)(PositionY-sqr_props.primary.ThicknessY);
							ExcludeRegion.bottom=(LONG)(PositionY+sqr_props.primary.ThicknessY);
							ExcludeRegion.left=(LONG)(PositionX-ThicknessX);
							ExcludeRegion.right=(LONG)(PositionX+ThicknessX);
						}
						
						RenderSquareReticle(Frame,Xpos,Ypos,sqr_props.shadow,sqr_props.PixelOffsetX,sqr_props.PixelOffsetY,pExcludeRegion);
					}
					ret=RenderSquareReticle(Frame,Xpos,Ypos,sqr_props.primary);
				}
				break;
			case Compositor_Props::ePathAlign:
				{
					const Compositor_Props::PathAlign_Container_Props &pa_props=props.PathAlign;
					#if 0
					m_PathPlotter.ComputePathPoints(1.0, 0.25);
					#else
					double Xpos,Ypos;
					Xpos=EnableFlash?m_Xpos+XOffset:seq_pkt.PositionX+XOffset;
					Ypos=EnableFlash?m_Ypos+YOffset:seq_pkt.PositionY+YOffset;
					//Treating Xpos and Ypos as degrees will keep the scale to feel about right
					m_PathPlotter.Compute_LookAtFromAngles(DEG_2_RAD(Xpos)+pa_props.rot_x,DEG_2_RAD(Ypos)+pa_props.rot_y);
					double Velocity=SmartDashboard::GetNumber("Velocity");
					double Rotation_Velocity=SmartDashboard::GetNumber("Rotation Velocity");
					if ((!IsZero(Rotation_Velocity))&&(IsZero(Velocity)))
						Velocity=1.0;
					//Flip direction when going backwards
					if (Velocity<0.0)
					{
						Velocity=-Velocity;
						Rotation_Velocity=-Rotation_Velocity;
					}
					m_PathPlotter.ComputePathPoints(Feet2Meters(Velocity),-Rotation_Velocity);
					#endif
					if (!IsZero(Velocity))
						ret = m_PathPlotter.RenderPath(Frame);
				}
				break;
			case Compositor_Props::eBypass:
				ret=m_Bypass.Callback_ProcessFrame_UYVY(Frame);
				break;
			case Compositor_Props::eLinePlot:
				{
					Compositor_Props::LinePlotReticle_Container_Props lineplot_props=props.lineplot_reticle[seq_pkt.specific_data.SquareReticle_SelIndex];
					ret=m_LinePlot.RenderLinePlotReticle(Frame,lineplot_props,m_ToggleLinePlot);
					break;
				}
			case Compositor_Props::eComposite:
				{
					const Compositor_Props::Sequence_List &composite=*seq_pkt.specific_data.Composite;
					//For group mode the specific offsets are locked down, we just need to make sure we are not inside recursion... otherwise refer to the locked down value
					bool UseInput=(&sequence==&props.Sequence) && EnableFlash;
					const double Xpos=UseInput?m_Xpos+XOffset:seq_pkt.PositionX+XOffset;
					const double Ypos=UseInput?m_Ypos+YOffset:seq_pkt.PositionY+YOffset;

					for (size_t i=0;i<composite.size();i++)
						ret=Render_Reticle(Frame,composite,i,Xpos,Ypos);
				}
				break;
			};

			return ret;
		}

		Bitmap_Frame *TimeChange(Bitmap_Frame *Frame)
		{
			const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
			const time_type current_time=time_type::get_current_time();
			const double dTime_s=(double)(current_time-m_LastTime);
			m_LastTime=current_time;
			m_Frame=Frame; //to access frame properties during the event callback
			m_JoyBinder.UpdateJoyStick(dTime_s);

			if (SmartDashboard::IsConnected())
			{
				m_IsEditable=SmartDashboard::GetBoolean("Edit Position");
				//This is only read for cases like autonomous... so we'll cut to the quick during edit mode and not read it
				//I could almost write back the value not changing, but I'd need to track the top level index... I don't think it is justifiable for this
				//especially since the workflow should not be changing this in the java client
				if (!m_IsEditable)
					UpdateSequence((size_t)SmartDashboard::GetNumber("Sequence")-1); //convert to ordinal
			}
			//If edit status changes... issue update this will ensure its in the right mode
			if (m_PreviousIsEditable != m_IsEditable )
				UpdateSequence(m_SequenceIndex,true);
			
			Bitmap_Frame *ret=Frame;
			if (m_PositionTracker.empty())
				ret=Render_Reticle(Frame,*m_pSequence,m_SequenceIndex);
			else
			{
				for (size_t i=0;i<m_pSequence->size();i++)
					ret=Render_Reticle(ret,*m_pSequence,i,m_Xpos_Offset,m_Ypos_Offset);
			}
			m_PreviousIsEditable=m_IsEditable;
			return ret;
		}

		Plugin_Controller_Interface* GetBypassPluginInterface(void) {return m_Bypass.GetPluginInterface();}
		Compositor_Props::ReticleType GetCurrentReticalType() const {return m_CompositorProperties.GetCompositorProps().Sequence[m_SequenceIndex].type;}
		void SetRecurseIntoComposite(bool enableRecursiveStep) 
		{
			m_RecurseIntoComposite=enableRecursiveStep;
			//if we are no longer recursing... we'll need to get out of the recursion
			if (!enableRecursiveStep)
			{
				{
					Compositor_Props::Sequence_List &Sequence_rw=*(const_cast<Compositor_Props::Sequence_List *>(m_pSequence));
					Sequence_rw[m_SequenceIndex].PositionX=m_Xpos;
					Sequence_rw[m_SequenceIndex].PositionY=m_Ypos;
				}
				while(!m_PositionTracker.empty())
				{
					const PositionPacket &pkt=m_PositionTracker.top();
					m_PositionTracker.pop();
					//restore to parent
					m_pSequence=pkt.pSequence;
					m_SequenceIndex=pkt.SequenceIndex;  
					m_Xpos_Offset=pkt.m_Xpos;
					m_Ypos_Offset=pkt.m_Ypos;
					m_Xpos=(*m_pSequence)[m_SequenceIndex].PositionX;
					m_Ypos=(*m_pSequence)[m_SequenceIndex].PositionY;
				}
			}
			else
				UpdateSequence(m_SequenceIndex,true);  //this will check if we are on composite to break it up
		}
		bool GetRecurseIntoComposite() const {return m_RecurseIntoComposite;}
	private:
		time_type m_LastTime;
		FrameWork::UI::JoyStick_Binder m_JoyBinder;
		FrameWork::EventMap m_EventMap;
		Compositor_Properties m_CompositorProperties;
		Bitmap_Frame *m_Frame;
		size_t m_SequenceIndex;
		const Compositor_Props::Sequence_List *m_pSequence;  //keep track of which sequence level where we are
		struct PositionPacket
		{
			double m_Xpos,m_Ypos;  //Keep track of parent offset
			size_t SequenceIndex;
			const Compositor_Props::Sequence_List *pSequence;
		};
		std::stack<PositionPacket> m_PositionTracker;  //used to recurse into the sequence
		size_t m_BlinkCounter; //very simple blink mechanism
		double m_Xpos,m_Ypos;
		double m_Xpos_Offset,m_Ypos_Offset;  //only used when stepping through recursion
		Bypass_Reticle m_Bypass;
		LinePlot_Retical m_LinePlot;
		PathRenderer m_PathPlotter;

		bool m_IsEditable;
		bool m_PreviousIsEditable;  //detect when Editable has switched to off to issue an update
		bool m_POVSetValve;
		bool m_RecurseIntoComposite;  //If true it will step into composite list for edit during previous/next sequence
		bool m_Flash;  //Making this a member makes it possible to enable flash for a whole group in composite
		bool m_ToggleLinePlot;
} *g_pCompositor;


extern "C" COMPOSITER_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame)
{
	if (g_pCompositor)
		return g_pCompositor->TimeChange(Frame);
	else
		return Frame;
}

extern "C" COMPOSITER_API void Callback_SmartCppDashboard_Initialize(const char *IPAddress,Dashboard_Framework_Interface *DashboardHelper)
{
	g_Framework=DashboardHelper;
	SmartDashboard::SetClientMode();
	SmartDashboard::SetIPAddress(IPAddress);
	SmartDashboard::init();
	SmartDashboard::PutBoolean("Edit Position",false);

	g_pCompositor = new Compositor(IPAddress,DashboardHelper);
	{
		Compositor_Properties props;
		Scripting::Script script;
		const char *err;
		err=script.LoadScript("Compositor.lua",true);
		if (err!=NULL)
			err=script.LoadScript("../Compositor/Compositor.lua",true);

		if (err==NULL)
		{
			script.NameMap["EXISTING_DASHBOARD"] = "EXISTING_COMPOSITER";
			props.SetUpGlobalTable(script);
			props.LoadFromScript(script);
			g_pCompositor->Initialize(g_pCompositor->GetEventMap_rw(),&props);
		}
		else
			g_pCompositor->Initialize(g_pCompositor->GetEventMap_rw(),NULL);
	}
}

extern "C" COMPOSITER_API void Callback_SmartCppDashboard_Shutdown()
{
	delete g_pCompositor;
	g_pCompositor=NULL;
	SmartDashboard::shutdown();
}

class Plugin_Compositor_Interface : public Plugin_Controller_Interface
{
	public:
		virtual void SetIsEditable(bool Edit)=0;
		virtual bool GetIsEditable() const=0;
		virtual Plugin_Controller_Interface* GetBypassPluginInterface(void)=0;
		virtual Compositor_Props::ReticleType GetCurrentReticalType() const=0;
		virtual void SetStepIntoComposite(bool enableRecursiveStep)=0;
		virtual bool GetStepIntoComposite() const=0;
};

class Plugin_Compositor : public Plugin_Compositor_Interface
{
	public:
		Plugin_Compositor(Compositor *implementation) : m_internal(implementation) 	{assert(implementation);}
	protected:
		void SetIsEditable(bool Edit)
		{
			m_internal->SetIsEditable(Edit);
		}
		bool GetIsEditable() const {return m_internal->GetIsEditable();}
		virtual const char *GetPlugInName() const {return "Plugin_Compositor";}

		virtual Plugin_Controller_Interface* GetBypassPluginInterface(void)  {return m_internal->GetBypassPluginInterface();}
		Compositor_Props::ReticleType GetCurrentReticalType() const {return m_internal->GetCurrentReticalType();}
		virtual void SetStepIntoComposite(bool enableRecursiveStep) {m_internal->SetRecurseIntoComposite(enableRecursiveStep);}
		virtual bool GetStepIntoComposite() const {return m_internal->GetRecurseIntoComposite();}
	private:
		Compositor *m_internal;
};

extern "C" COMPOSITER_API Plugin_Controller_Interface *Callback_CreatePluginControllerInterface()
{
	Plugin_Compositor *plugin=new Plugin_Compositor(g_pCompositor);
	return plugin;
}

extern "C" COMPOSITER_API void Callback_DestroyPluginControllerInterface(Plugin_Controller_Interface *plugin)
{
	delete plugin;
}