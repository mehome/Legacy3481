#include "stdafx.h"
#define __IncludeInputBase__
#include "../FrameWork/FrameWork.h"
#include "Compositor.h"

//Enable this to include FOV in the SmartDashboard
#undef __Edit_FOV__
#undef  __Edit_Camera_Position__
#undef __ShowBottomHandle__

#if 0
#define UnitMeasure2UI Meters2Feet
#define UI2UnitMeasure Feet2Meters
#else
#define UnitMeasure2UI Meters2Inches
#define UI2UnitMeasure Inches2Meters
#endif

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

#define SCRIPT_TEST_BOOL_YES(x,y)  			err = script.GetField(y,&sTest,NULL,NULL);\
x=false;\
if (!err)\
{\
	if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))\
		x=true;\
}



#define SCRIPT_TEST_BOOL_NO(x,y)  			err = script.GetField(y,&sTest,NULL,NULL);\
x=true;\
if (!err)\
{\
	if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))\
		x=false;\
}


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
	"none","square","pathalign","shape","composite","bypass","line_plot"
};

const char * const csz_ShapeType_Enum[] =
{
	"cube","square","circle"
};

const char * const csz_Shape2D_PlaneSelection_Enum[] =
{
	"xy","xz","yz","xy_and_xz"
};

const char * const csz_PathAlign_Enum[] =
{
	"none","path","runner"
};

struct Compositor_Props
{
	double X_Scalar;
	double Y_Scalar;
	double Z_Scalar;

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
		//This can be left empty for no remote control
		//If filled in the following suffix on names are as follows:
		// _x,_y,_z controls the position of the camera
		// _rot_x,_rot_y,_rot_z controls the orientation of the camera
		std::string RemoteVariableName;

		double width,length;
		double pivot_point_length;
		double pos_x,pos_y,pos_z;
		double rot_x,rot_y,rot_z;
		double FOV_x,FOV_y;
		BYTE rgb[3];  //shape color
		enum path_types
		{
			eNone,  //still need path align for shapes, but we can not draw anything for it
			eDefaultPath,
			eDistanceRunner  //TODO this one may look cool, but probably not going to do it
		} path_type;  //this will become depreciated
		static path_types GetPathAlign_Enum (const char *value)
		{	return Enum_GetValue<path_types> (value,csz_PathAlign_Enum,_countof(csz_PathAlign_Enum));
		}
		size_t NumberSegments;  //This can slice the path up into this many segment squares
		bool IgnoreLinearVelocity;  //avoid motion when true
		bool IgnoreAngularVelocity; //avoid the curving when true
		bool IsRearView;  //Used for if camera is mounted for rear view
	} PathAlign;
	//No list for path align... assuming only one instance is needed of one camera at a fixed point and orientation

	//Shapes will use project properties set in the Path Align container... which should be renamed to projection properties
	struct Shape3D_Renderer_Props
	{
		//This can be left empty for no remote control
		//If filled in the following suffix on names are as follows:
		// _x,_y,_z controls the position of the shape
		// _rot_x,_rot_y,_rot_z controls the orientation of the shape (Not all shapes have this)
		// _enabled controls whether or not it is enabled
		std::string RemoteVariableName;
		BYTE rgb[3];  //shape color
		struct Shapes2D_Orientation_Props  //Squares use this
		{
			double Yaw,Pitch,Roll;
		};

		union type_specifics
		{
			double Size_1D;  //All shapes use this... kept as generic name for shapes that just need size
			struct Shapes2D_Props  //circles use this
			{
				double Size_1D;  
				//Where x and y are width height and z is depth
				enum PlaneSelection_enum
				{
					e_xy_plane,  //typical front face drawing
					e_xz_plane,  //like drawn on the floor or ceiling
					e_yz_plane,  //like drawn on a side wall (probably rarely used)
					e_xy_and_xz_plane  //A double render on both planes
				} PlaneSelection;
				static PlaneSelection_enum GetPlaneSelection_Enum (const char *value)
				{	return Enum_GetValue<PlaneSelection_enum> (value,csz_Shape2D_PlaneSelection_Enum,_countof(csz_Shape2D_PlaneSelection_Enum));
				}
			} Shapes2D;
			struct Square_Props  //Squares use this
			{
				Shapes2D_Props Props2D;
				double Length,Width;
				Shapes2D_Orientation_Props Orientation;
				//These help define where to position the origin of the handle typically 0.5,0.5 is center
				double XBisect,YBisect;
			} Square;
			struct Cube_Props  //Cubes use this
			{
				double Size_1D;
				double Length,Width,Depth;
				Shapes2D_Orientation_Props Orientation;
				//These help define where to position the origin of the handle typically 0.5,0.5 is center
				double XBisect,YBisect,ZBisect;
			} Cube;
		} specific_data;

		enum shape_types 
		{
			e_Cube,
			e_Square,
			e_Circle
		} draw_shape;
		static shape_types GetShapeType_Enum (const char *value)
		{	return Enum_GetValue<shape_types> (value,csz_ShapeType_Enum,_countof(csz_ShapeType_Enum));
		}
	};
	std::vector<Shape3D_Renderer_Props> shapes_3D_reticle;

	//Reticle type definitions
	enum ReticleType
	{
		eNone,
		eDefault,
		ePathAlign,
		eShape3D,
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
			struct ShapeDynamics
			{
				size_t SelIndex;
				double PositionZ;
			} ShapeReticle_props;

			//Not yet supported
			//struct ShapeDynamics_Orientation
			//{
			//	size_t SelIndex;
			//	double PositionZ;
			//	double rot_x,rot_y,rot_z;
			//} ShapeReticle_Orientation_props;

			double PositionZ; //for path align
		} specific_data;
		bool IsEnabled;  //all reticles can have ability to not be drawn TODO
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
	"SetXAxis","SetYAxis","SetZAxis","NextSequence","PreviousSequence","SequencePOV","ToggleLinePlot","ResetPos"
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
	props.Z_Scalar=0.025;

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
			SCRIPT_TEST_BOOL_YES(sqr_pkt.UsingShadow,"use_shadow");
			SCRIPT_TEST_BOOL_NO(sqr_pkt.ExcludeRegion,"exclude_region");
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


static void LoadShapeReticleProps_Internal(Scripting::Script& script,Compositor_Props &props,Compositor_Props::Shape3D_Renderer_Props &shape_props)
{
	const char* err=NULL;
	double value;

	typedef Compositor_Props::Shape3D_Renderer_Props ShapeProps;
	typedef ShapeProps::type_specifics::Shapes2D_Props Shapes2D_Props;
	typedef Compositor_Props::Shape3D_Renderer_Props::type_specifics::Square_Props SquareProps;
	typedef Compositor_Props::Shape3D_Renderer_Props::type_specifics::Cube_Props CubeProps;

	std::string sTest;
	err = script.GetField("remote_name",&shape_props.RemoteVariableName,NULL,NULL);
	if (shape_props.RemoteVariableName.c_str()[0]!=0)
	{
		sTest=shape_props.RemoteVariableName;
		sTest+="_x";
		SmartDashboard::PutNumber(sTest,0.0);
		sTest=shape_props.RemoteVariableName;
		sTest+="_y";
		SmartDashboard::PutNumber(sTest,0.0);
		sTest=shape_props.RemoteVariableName;
		sTest+="_z";
		SmartDashboard::PutNumber(sTest,0.0);
		sTest=shape_props.RemoteVariableName;
		sTest+="_enabled";
		SmartDashboard::PutBoolean(sTest,true);
	}
	err = script.GetField("draw_shape",&sTest,NULL,NULL);
	assert(!err);  //gotta have it if we are making a shape
	ShapeProps::shape_types shape=ShapeProps::GetShapeType_Enum(sTest.c_str());
	shape_props.draw_shape=shape;

	switch (shape)
	{
	case ShapeProps::e_Cube:
		{
			CubeProps &cube_props=shape_props.specific_data.Cube;
			if (LoadMeasuredValue(script,"size",value))
			{
				cube_props.Size_1D=value;
				cube_props.Length=cube_props.Width=cube_props.Depth=value;
			}
			else
				shape_props.specific_data.Size_1D= 12 * 0.0254;	
			if (LoadMeasuredValue(script,"length",value))
				cube_props.Length=value;
			else
				cube_props.Length= cube_props.Size_1D;	

			if (LoadMeasuredValue(script,"width",value))
				cube_props.Width=value;
			else
				cube_props.Width= cube_props.Length;	

			if (LoadMeasuredValue(script,"depth",value))
				cube_props.Depth=value;
			else
				cube_props.Depth= cube_props.Depth;	

			//TODO move this in sequence packet when live edit comes on-line
			double fTest;
			err = script.GetField("x_bisect",NULL,NULL,&fTest);
			cube_props.XBisect=(err==NULL) ? fTest : 0.5;

			err = script.GetField("y_bisect",NULL,NULL,&fTest);
			cube_props.YBisect=(err==NULL) ? fTest : 0.5;

			err = script.GetField("z_bisect",NULL,NULL,&fTest);
			cube_props.ZBisect=(err==NULL) ? fTest : 0.5;

			err = script.GetFieldTable("rotation");
			if (!err)
			{
				err=script.GetField("x_deg", NULL, NULL, &fTest);
				if (!err) cube_props.Orientation.Yaw=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("x",NULL,NULL,&fTest);
					assert(!err);
					cube_props.Orientation.Yaw=fTest;
				}
				err=script.GetField("y_deg", NULL, NULL, &fTest);
				if (!err) cube_props.Orientation.Pitch=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("y",NULL,NULL,&fTest);
					assert(!err);
					cube_props.Orientation.Pitch=fTest;
				}
				err=script.GetField("z_deg", NULL, NULL, &fTest);
				if (!err) cube_props.Orientation.Roll=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("z",NULL,NULL,&fTest);
					assert(!err);
					cube_props.Orientation.Roll=fTest;
				}
				script.Pop();
			}
			else
			{

				cube_props.Orientation.Yaw=0.0;
				cube_props.Orientation.Pitch=0.0;
				cube_props.Orientation.Roll=0.0;
			}
			if (shape_props.RemoteVariableName.c_str()[0]!=0)
			{
				sTest=shape_props.RemoteVariableName;
				sTest+="_rot_x";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(cube_props.Orientation.Yaw));
				sTest=shape_props.RemoteVariableName;
				sTest+="_rot_y";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(cube_props.Orientation.Pitch));
				sTest=shape_props.RemoteVariableName;
				sTest+="_rot_z";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(cube_props.Orientation.Roll));
			}

		}		

		break;
	case ShapeProps::e_Circle:
		//The rest should be the shapes 2D kind
		if (LoadMeasuredValue(script,"size",value))
			shape_props.specific_data.Shapes2D.Size_1D=value;
		else
			shape_props.specific_data.Shapes2D.Size_1D= 12 * 0.0254;	
		//plane selection 
		err = script.GetField("plane_selection",&sTest,NULL,NULL);
		if (!err)
		{
			Shapes2D_Props::PlaneSelection_enum selection=Shapes2D_Props::GetPlaneSelection_Enum(sTest.c_str());
			shape_props.specific_data.Shapes2D.PlaneSelection=selection;
		}
		else
			shape_props.specific_data.Shapes2D.PlaneSelection=ShapeProps::type_specifics::Shapes2D_Props::e_xy_plane;
		break;
	case ShapeProps::e_Square:
		{

			SquareProps &sqr_props=shape_props.specific_data.Square;
			if (LoadMeasuredValue(script,"size",value))
				sqr_props.Length=sqr_props.Width=value;
			else
			{
				if (LoadMeasuredValue(script,"length",value))
					sqr_props.Length=value;
				else
					sqr_props.Length= 12 * 0.0254;	

				if (LoadMeasuredValue(script,"width",value))
					sqr_props.Width=value;
				else
					sqr_props.Width= 12 * 0.0254;	
			}

			//TODO move this in sequence packet when live edit comes on-line
			double fTest;
			err = script.GetField("x_bisect",NULL,NULL,&fTest);
			sqr_props.XBisect=(err==NULL) ? fTest : 0.5;

			err = script.GetField("y_bisect",NULL,NULL,&fTest);
			sqr_props.YBisect=(err==NULL) ? fTest : 0.5;

			err = script.GetFieldTable("rotation");
			if (!err)
			{
				err=script.GetField("x_deg", NULL, NULL, &fTest);
				if (!err) sqr_props.Orientation.Yaw=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("x",NULL,NULL,&fTest);
					assert(!err);
					sqr_props.Orientation.Yaw=fTest;
				}
				err=script.GetField("y_deg", NULL, NULL, &fTest);
				if (!err) sqr_props.Orientation.Pitch=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("y",NULL,NULL,&fTest);
					assert(!err);
					sqr_props.Orientation.Pitch=fTest;
				}
				err=script.GetField("z_deg", NULL, NULL, &fTest);
				if (!err) sqr_props.Orientation.Roll=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("z",NULL,NULL,&fTest);
					assert(!err);
					sqr_props.Orientation.Roll=fTest;
				}
				script.Pop();
			}
			else
			{

				sqr_props.Orientation.Yaw=0.0;
				sqr_props.Orientation.Pitch=0.0;
				sqr_props.Orientation.Roll=0.0;
			}
			if (shape_props.RemoteVariableName.c_str()[0]!=0)
			{
				sTest=shape_props.RemoteVariableName;
				sTest+="_rot_x";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(sqr_props.Orientation.Yaw));
				sTest=shape_props.RemoteVariableName;
				sTest+="_rot_y";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(sqr_props.Orientation.Pitch));
				sTest=shape_props.RemoteVariableName;
				sTest+="_rot_z";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(sqr_props.Orientation.Roll));
			}

			//plane selection 
			err = script.GetField("plane_selection",&sTest,NULL,NULL);
			if (!err)
			{
				Shapes2D_Props::PlaneSelection_enum selection=Shapes2D_Props::GetPlaneSelection_Enum(sTest.c_str());
				sqr_props.Props2D.PlaneSelection=selection;
			}
			else
				sqr_props.Props2D.PlaneSelection=ShapeProps::type_specifics::Shapes2D_Props::e_xy_plane;
		}		
		break;

	default:
		//The rest should be the shapes 2D kind
		if (LoadMeasuredValue(script,"size",value))
			shape_props.specific_data.Shapes2D.Size_1D=value;
		else
			shape_props.specific_data.Shapes2D.Size_1D= 12 * 0.0254;	// 12 inches default value
	}

	err=script.GetField("r", NULL, NULL, &value);
	if (!err)
		shape_props.rgb[0]=(BYTE)value;
	err=script.GetField("g", NULL, NULL, &value);
	if (!err)
		shape_props.rgb[1]=(BYTE)value;
	err=script.GetField("b", NULL, NULL, &value);
	if (!err)
		shape_props.rgb[2]=(BYTE)value;
}

static void LoadShapeReticleProps(Scripting::Script& script,Compositor_Props &props)
{
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"shape_reticle_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Compositor_Props::Shape3D_Renderer_Props shape_props;
			LoadShapeReticleProps_Internal(script,props,shape_props);

			props.shapes_3D_reticle.push_back(shape_props);
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

static void LoadPathAlignProps(Scripting::Script& script,Compositor_Props &props)
{
	typedef Compositor_Props::PathAlign_Container_Props PathProps; 
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
					pal_props.pos_z=fTest;
				else
					pal_props.pos_y= -10 * 0.0254;
				if (LoadMeasuredValue(script,"z",fTest))
					pal_props.pos_y=fTest;
				else
					pal_props.pos_z=18 * 0.0254;
				script.Pop();

				#ifdef __Edit_Camera_Position__
				SmartDashboard::PutNumber("Camera_x",Meters2Inches(pal_props.pos_x));
				SmartDashboard::PutNumber("Camera_y",Meters2Inches(pal_props.pos_z));
				SmartDashboard::PutNumber("Camera_z",Meters2Inches(pal_props.pos_y));
				#endif
			}
			err = script.GetFieldTable("camera_rotation");
			if (!err)
			{
				err=script.GetField("x_deg", NULL, NULL, &fTest);
				if (!err) pal_props.rot_x=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("x",NULL,NULL,&fTest);
					assert(!err);
					pal_props.rot_x=fTest;
				}
				err=script.GetField("y_deg", NULL, NULL, &fTest);
				if (!err) pal_props.rot_y=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("y",NULL,NULL,&fTest);
					assert(!err);
					pal_props.rot_y=fTest;
				}
				err=script.GetField("z_deg", NULL, NULL, &fTest);
				if (!err) pal_props.rot_z=DEG_2_RAD(fTest);
				else
				{
					err = script.GetField("z",NULL,NULL,&fTest);
					assert(!err);
					pal_props.rot_z=fTest;
				}
				script.Pop();
			}
			else
			{
				pal_props.rot_x=0.0;
				pal_props.rot_y=0.33;
				pal_props.rot_z=0.0;
			}


			std::string sTest;
			err = script.GetField("remote_name",&pal_props.RemoteVariableName,NULL,NULL);
			if (pal_props.RemoteVariableName.c_str()[0]!=0)
			{
				sTest=pal_props.RemoteVariableName;
				sTest+="_x";
				SmartDashboard::PutNumber(sTest,UnitMeasure2UI(pal_props.pos_x));
				sTest=pal_props.RemoteVariableName;
				sTest+="_y";
				SmartDashboard::PutNumber(sTest,UnitMeasure2UI(pal_props.pos_z));
				sTest=pal_props.RemoteVariableName;
				sTest+="_z";
				SmartDashboard::PutNumber(sTest,UnitMeasure2UI(pal_props.pos_y));

				sTest=pal_props.RemoteVariableName;
				sTest+="_rot_x";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(pal_props.rot_x));
				sTest=pal_props.RemoteVariableName;
				sTest+="_rot_y";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(pal_props.rot_y));
				sTest=pal_props.RemoteVariableName;
				sTest+="_rot_z";
				SmartDashboard::PutNumber(sTest,RAD_2_DEG(pal_props.rot_z));
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

			err = script.GetField("draw_selection",&sTest,NULL,NULL);
			if (!err)
			{
				PathProps::path_types selection=PathProps::GetPathAlign_Enum(sTest.c_str());
				pal_props.path_type=selection;
			}
			else
				pal_props.path_type=PathProps::eDefaultPath;


			err=script.GetField("r", NULL, NULL, &fTest);
			if (!err)
				pal_props.rgb[0]=(BYTE)fTest;
			err=script.GetField("g", NULL, NULL, &fTest);
			if (!err)
				pal_props.rgb[1]=(BYTE)fTest;
			err=script.GetField("b", NULL, NULL, &fTest);
			if (!err)
				pal_props.rgb[2]=(BYTE)fTest;

			err=script.GetField("num_segments", NULL, NULL, &fTest);
			pal_props.NumberSegments=err?10:(size_t)fTest;

			SCRIPT_TEST_BOOL_YES(pal_props.IsRearView,"rear_view");
			SCRIPT_TEST_BOOL_YES(pal_props.IgnoreLinearVelocity,"disable_motion");
			SCRIPT_TEST_BOOL_YES(pal_props.IgnoreAngularVelocity,"disable_turns");

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
			pal_props.path_type=PathProps::eDefaultPath;
			pal_props.NumberSegments=10;
			props.PathAlign=pal_props;
		}
	} //while (!fieldtable_err);
}

static void LoadSequenceProps(Scripting::Script& script,Compositor_Props::Sequence_List &sequence,Compositor_Props &props,bool IsRecursive=false)
{
	const char* err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	bool GotFieldTable;
	do 
	{
		if (!IsRecursive)
			sprintf_s(Buffer,128,"sequence_%d",index);
		else
			sprintf_s(Buffer,128,"composite_%d",index);
		err = script.GetFieldTable(Buffer);
		GotFieldTable=!err;
		if (GotFieldTable)
		{
			Compositor_Props::Sequence_Packet seq_pkt;
			//Start with the position's initialized
			seq_pkt.PositionX=0.0;
			seq_pkt.PositionY=0.0;

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
			case Compositor_Props::ePathAlign:
				seq_pkt.specific_data.PositionZ=0.0;
				break;
			case Compositor_Props::eShape3D:
				{
					double fTest;

					err = script.GetField("selection",NULL,NULL,&fTest);
					seq_pkt.specific_data.ShapeReticle_props.SelIndex=(size_t)fTest;
					seq_pkt.specific_data.ShapeReticle_props.SelIndex--;  // translate cardinal to ordinal 

					if (LoadMeasuredValue(script,"x",fTest))
						seq_pkt.PositionX=fTest;
					else
						seq_pkt.PositionX= 0.0;

					if (LoadMeasuredValue(script,"y",fTest))
						seq_pkt.PositionY=fTest;
					else
						seq_pkt.PositionY= 0.0;	

					if (LoadMeasuredValue(script,"z",fTest))
						seq_pkt.specific_data.ShapeReticle_props.PositionZ=fTest;
					else
						seq_pkt.specific_data.ShapeReticle_props.PositionZ= 60 * 0.0254;  //a 5 foot depth default makes easier to see it to start

					const Compositor_Props::Shape3D_Renderer_Props &shape_props=props.shapes_3D_reticle[seq_pkt.specific_data.ShapeReticle_props.SelIndex];
					if (shape_props.RemoteVariableName.c_str()[0]!=0)
					{
						sTest=shape_props.RemoteVariableName;
						sTest+="_x";
						SmartDashboard::PutNumber(sTest,UnitMeasure2UI(seq_pkt.PositionX));
						sTest=shape_props.RemoteVariableName;
						sTest+="_y";
						SmartDashboard::PutNumber(sTest,UnitMeasure2UI(seq_pkt.PositionY));
						sTest=shape_props.RemoteVariableName;
						sTest+="_z";
						SmartDashboard::PutNumber(sTest,UnitMeasure2UI(seq_pkt.specific_data.ShapeReticle_props.PositionZ));
					}
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

			sequence.push_back(seq_pkt);
			index++;

			script.Pop();
		}
	} while (GotFieldTable);
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
				err = script.GetField("x",NULL,NULL,&seq_pkt.PositionX);
				err = script.GetField("y",NULL,NULL,&seq_pkt.PositionY);
				break;
			case Compositor_Props::ePathAlign:
				err = script.GetField("x",NULL,NULL,&seq_pkt.PositionX);
				err = script.GetField("y",NULL,NULL,&seq_pkt.PositionY);
				err = script.GetField("z",NULL,NULL,&seq_pkt.specific_data.PositionZ);
				break;
			case Compositor_Props::eShape3D:
				{
					err = script.GetField("x",NULL,NULL,&seq_pkt.PositionX);
					err = script.GetField("y",NULL,NULL,&seq_pkt.PositionY);
					err = script.GetField("z",NULL,NULL,&seq_pkt.specific_data.ShapeReticle_props.PositionZ);
					const Compositor_Props::Shape3D_Renderer_Props &shape_props=props.shapes_3D_reticle[seq_pkt.specific_data.ShapeReticle_props.SelIndex];
					if (shape_props.RemoteVariableName.c_str()[0]!=0)
					{
						sTest=shape_props.RemoteVariableName;
						sTest+="_x";
						SmartDashboard::PutNumber(sTest,UnitMeasure2UI(seq_pkt.PositionX));
						sTest=shape_props.RemoteVariableName;
						sTest+="_y";
						SmartDashboard::PutNumber(sTest,UnitMeasure2UI(seq_pkt.PositionY));
						sTest=shape_props.RemoteVariableName;
						sTest+="_z";
						SmartDashboard::PutNumber(sTest,UnitMeasure2UI(seq_pkt.specific_data.ShapeReticle_props.PositionZ));
					}
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
		script.GetField("z_scalar", NULL, NULL, &props.Z_Scalar);
		err = script.GetFieldTable("square_reticle_props");
		if (!err)
		{
			//clear the default one
			props.square_reticle.clear();
			LoadSquareReticleProps(script,props);
			script.Pop();
		}
		err = script.GetFieldTable("shape_reticle_props");
		if (!err)
		{
			//pedantic
			props.shapes_3D_reticle.clear();
			LoadShapeReticleProps(script,props);
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
		back = 1000.0;
		projection = 0;
	}

	// A more convenient way to set the camera.
	void SetLookAtFromAngles(_3Dpoint orientation)
	{
		// forward facing vector
		_3Dpoint new_look = _3Dpoint(0, 1, 0);	// unit vector
		// rotate it.
		new_look.x = sin(orientation.x);
		new_look.y = cos(orientation.x);
		new_look.y *= cos(orientation.y);
		new_look.z = sin(orientation.y);
		// scale it.
		//new_look.x *= 10.0;
		//new_look.y *= 10.0;
		//new_look.z *= 10.0;
		// add it to the camera origin.
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
	mutable _2Dpoint center;
	mutable _2Dpoint size;

	SCREEN()
	{
		center = _2Dpoint(640/2, 480/2);
		size = _2Dpoint(640, 480);
	}

	SCREEN(int width, int hight)
	{
		SetSize(width, hight);
	}

	void SetSize(int width, int hight) const
	{
		center = _2Dpoint(width/2, hight/2);
		size = _2Dpoint(width, hight);
	}
};

class projection
{
public:
	mutable _2Dpoint p1;	// these are effectively our output points
	mutable _2Dpoint p2;

	CAMERA camera;
	SCREEN screen;

private:
	_3Dpoint origin;
	mutable _3Dpoint e1, e2, n1, n2;
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

	bool Trans_Line(_3Dpoint w1, _3Dpoint w2) const
	{
		Trans_World2Eye(w1, e1);
		Trans_World2Eye(w2, e2);
		if (Trans_ClipEye(e1, e2))
		{
			Trans_Eye2Norm(e1, n1);
			Trans_Eye2Norm(e2, n2);
//			if (Trans_ClipNorm(n1, n2))
			{
				Trans_Norm2Screen(n1, p1);
				Trans_Norm2Screen(n2, p2);
				return (true);
			}
		}

		return (true);
	}

	bool Trans_Line(osg::Vec3d w1, osg::Vec3d w2) const
	{
		return Trans_Line(_3Dpoint(w1[0],w1[1],w1[2]),_3Dpoint(w2[0],w2[1],w2[2]));
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
	void Trans_World2Eye(_3Dpoint w, _3Dpoint& e) const
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

	bool Trans_ClipEye(_3Dpoint& e1, _3Dpoint& e2) const
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

	void Trans_Eye2Norm(_3Dpoint e, _3Dpoint& n) const
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

	bool Trans_ClipNorm(_3Dpoint& n1, _3Dpoint& n2) const
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

	void Trans_Norm2Screen(_3Dpoint norm, _2Dpoint& projected) const
	{
		double aspect = (double)screen.size.h / (double)screen.size.v;
		projected.h = (int)(screen.center.h - screen.size.h * norm.x / 2);
		projected.v = (int)(screen.center.v - screen.size.v * aspect * norm.z / 2);
	}
};

class PathRenderer
{
public:
	typedef Compositor_Props::PathAlign_Container_Props PathProps; 
	PathRenderer(void)
	{
		NumPoints = 10;
		Left = new _3Dpoint[NumPoints + 1];
		Center = new _3Dpoint[NumPoints + 1];
		Right = new _3Dpoint[NumPoints + 1];
		Translation = new _3Dpoint[NumPoints + 1];

		angle = -45;
		dir = 1;

		path_type = PathProps::eDefaultPath;

		// these should probably be pulled from LUA values.
		width = 15 * 0.0254;	// 15 inches in meters (robot is 24)	This is the width of our drawn path.
		length = 33 * 0.0254;	// 33 inches in meters					This is how far to project our path.
		pivot_point_length = 22 * 0.0254;	// assumes pivot point is 2/3s from front.		This is the distance from the pivot point to the front, center of the robot frame.

		forward_velocity = 0;	// just setting to known values. these should be set per frame.
		angular_velocity = 0;

		m_SegmentOffset=0.0;
	}

	void SetNumPoints(size_t NumSegments)
	{
		//TODO ask Cary why this check was here
		//if (NumSegments>(size_t)NumPoints)

		{
			delete[] Left;
			delete[] Center;
			delete[] Right;
			delete[] Translation;
			NumPoints=NumSegments;
			Left = new _3Dpoint[NumPoints + 1];
			Center = new _3Dpoint[NumPoints + 1];
			Right = new _3Dpoint[NumPoints + 1];
			Translation = new _3Dpoint[NumPoints + 1];
		}
	}

	void Initialize(const Compositor_Props::PathAlign_Container_Props &props)
	{
		width=props.width;
		length=props.length;
		pivot_point_length=props.pivot_point_length;
		_3Dpoint Camera_position = _3Dpoint(props.pos_x,props.pos_y,props.pos_z);
		_3Dpoint Camara_LookAt = _3Dpoint(props.rot_x,props.rot_y,props.rot_z);
		m_lastOrientation=Camara_LookAt;
		path_type=props.path_type;

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

	PathProps::path_types path_type;		// what to draw

	int angle;
	int dir;
	
	projection projector;		// projection class

	//TODO work out unresolved external symbol of Framework::IsZero()
	inline bool Is_Zero(double value,double tolerance=1e-5)
	{
		return fabs(value)<tolerance;
	}


	// Compute the path mesh. This is for the front of the robot which is offset from the pivot point at ground level.
	// This should be called prior to each render, but could be called only when velocities change.
	void ComputePathPoints(double Forward_Vel, double Angular_Vel, double dTime_s, bool ImplementLinearMotion)
	{
		if( path_type != PathProps::eDefaultPath || Translation == NULL || Center == NULL || Right == NULL || Left == NULL ) 
			return;

		//Normalize the angular velocity
		Angular_Vel=m_KalFilter_AngularVelocity(Angular_Vel);
		Angular_Vel=m_AngularVelocity_Averager.GetAverage(Angular_Vel);
		if (Is_Zero(Angular_Vel))
			Angular_Vel=0.0;

		forward_velocity = fabs(Forward_Vel);  //reverse needs to keep this positive to render the curves properly
		angular_velocity = Angular_Vel;

		//Going backwards we switch the forward to positve and reverse the curve
		if (Forward_Vel<0.0)
			angular_velocity=-angular_velocity;

		//I'm not quite sure whether to make this vanish if it is stopped... hard coded for now, but may make a lua option
		#if 0
		if ((!IsZero(angular_velocity))&&(IsZero(forward_velocity)))
			forward_velocity=1.0;
		#else
		if (Is_Zero(forward_velocity))
			forward_velocity=1.0;
		#endif

		const double time_to_length = forward_velocity > 0 ? length / forward_velocity : 0;
		const double end_angle = angular_velocity != 0 ? angular_velocity * time_to_length : length;
		
		const double path_radius = angular_velocity != 0 ? forward_velocity / angular_velocity : 0;
		const double left_radius = path_radius - width / 2;
		const double right_radius = path_radius + width / 2;

		const double SegmentLength=end_angle/NumPoints;
		//dTime_s=0.033;  //breakpoint test
		double direction=(Angular_Vel>=0.0)?-1.0:1.0;
		if ((Forward_Vel<0.0) && (Angular_Vel!=0.0))
			direction=-direction;
		//hack TODO find the derivate for angular velocity
		if (Angular_Vel!=0.0)
		{
			direction=(direction>0.0)?0.1:-0.1;
		}
		//hack 2... as it gets closer to zero velocity it goes very fast
		if ((fabs(Angular_Vel)<0.1)&&(angular_velocity!=0))
		{
			direction=0.0;
			m_SegmentOffset=0.0;  //avoid bug when going from backwards to forwards
		}

		m_SegmentOffset=SegmentLength!=0.0?fmod((((Forward_Vel * direction) * dTime_s)+m_SegmentOffset),SegmentLength):m_SegmentOffset;
		double angle = m_SegmentOffset;
		if (!ImplementLinearMotion)
			angle=0.0;
		//TODO motion with angular velocity doesn't look right
		//if (angular_velocity!=0)
		//	angle=0;
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

			angle += SegmentLength;
		}
	}

	
	//This is almost a wrapper to the projection call... except that it will implicitly check for difference before making the call
	void Compute_LookAtFromAngles(double Yaw,double Pitch,double Roll)
	{
		_3Dpoint orientation(Yaw,Pitch,Roll);
		if (!(orientation==m_lastOrientation))
		{
			projector.camera.SetLookAtFromAngles(orientation);
			projector.Trans_Initialise();
			m_lastOrientation=orientation;
			#ifdef __Edit_Camera_Position__
			SmartDashboard::PutNumber("Camera_Yaw",RAD_2_DEG(Yaw));
			SmartDashboard::PutNumber("Camera_Pitch",RAD_2_DEG(Pitch));
			SmartDashboard::PutNumber("Camera_Roll",RAD_2_DEG(Roll));
			#endif
		}
	}

	// draw the path mesh
	Bitmap_Frame* RenderPath(Bitmap_Frame* Frame,const Compositor_Props::PathAlign_Container_Props &props,bool EnableFlash)
	{
		if (g_Framework)
		{
			const double R = (double)props.rgb[0];
			const double G = (double)props.rgb[1];
			const double B = (double)props.rgb[2];

			//These come from old code in the TargaReader plugin
			const double Y = (0.257 * R + 0.504 * G + 0.098 * B) + 16.0;
			const double U =(-0.148 * R - 0.291 * G + 0.439 * B) + 128.0;
			const double V = (0.439 * R - 0.368 * G - 0.071 * B) + 128.0;
			//gotta love constants to do this :)
			unsigned int col[] = { (unsigned int)U, (unsigned int)Y, (unsigned int)V, (unsigned int)Y };
			if (EnableFlash)
			{
				col[0]=16;
				col[1]=150;
				col[2]=245;
				col[3]=150;
			}

			projector.screen.SetSize(Frame->XRes, Frame->YRes);

			switch (path_type)
			{
			case PathProps::eDefaultPath:
				// draw the normal path
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
				break;
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
	double m_SegmentOffset;
	KalmanFilter m_KalFilter_AngularVelocity;
	Averager<double,10> m_AngularVelocity_Averager;
};

class Shape3D_Renderer
{
public:
	typedef Compositor_Props::Shape3D_Renderer_Props ShapeProps;
	typedef ShapeProps::type_specifics::Shapes2D_Props Shapes2D_Props;
	Shape3D_Renderer(const projection &Projection) : m_Projection(Projection)
	{}

	inline osg::Quat From_Rot_Radians(double Yaw, double Pitch, double Roll)
	{
		return osg::Quat(
			-Pitch, osg::Vec3d(1,0,0),
			Roll, osg::Vec3d(0,1,0),
			-Yaw, osg::Vec3d(0,0,1));
	}

	void InitCube(double XPos,double YPos,double ZPos,const Compositor_Props::Shape3D_Renderer_Props &props)
	{
		using namespace osg;
		typedef Compositor_Props::Shape3D_Renderer_Props::type_specifics::Cube_Props CubeProps;
		const CubeProps &cube_props=props.specific_data.Cube;
		const double Length=cube_props.Length;
		const double Width=cube_props.Width;
		const double Depth=cube_props.Depth;
		Quat orientation=From_Rot_Radians(cube_props.Orientation.Yaw,cube_props.Orientation.Pitch,cube_props.Orientation.Roll);

		Vec3d FwdDir(orientation*Vec3d(0,1,0));
		Vec3d BackDir=FwdDir;
		Vec3d UpDir(orientation*Vec3d(0,0,1));
		Vec3d DownDir=UpDir;
		Vec3d RightDir(orientation*Vec3d(1,0,0));
		Vec3d LeftDir=RightDir;

		const double YBisect=cube_props.YBisect,XBisect=cube_props.XBisect,ZBisect=cube_props.ZBisect;
		//Scale normalized by size
		UpDir   *=(Length * YBisect); //bisect length and width to work with that origin
		DownDir *=(Length * (1.0-YBisect));
		LeftDir *=(Width  * XBisect);
		RightDir*=(Width  * (1.0-XBisect));
		FwdDir *=(Depth * ZBisect);
		BackDir *=(Depth * (1.0-ZBisect));
		//Now to apply position offset
		Vec3d Offset(XPos,ZPos,YPos);

		cube[0]= -LeftDir +   UpDir + FwdDir + Offset;
		cube[1]= RightDir +   UpDir + FwdDir + Offset;
		cube[2]= -LeftDir + -DownDir + FwdDir + Offset;
		cube[3]= RightDir + -DownDir + FwdDir + Offset;

		cube[4]= -LeftDir +   UpDir + -BackDir + Offset;
		cube[5]= RightDir +   UpDir + -BackDir + Offset;
		cube[6]= -LeftDir + -DownDir + -BackDir + Offset;
		cube[7]= RightDir + -DownDir + -BackDir + Offset;
	}
	void InitSquare(double XPos,double YPos,double ZPos,const Compositor_Props::Shape3D_Renderer_Props &props)
	{
		using namespace osg;
		typedef Compositor_Props::Shape3D_Renderer_Props::type_specifics::Square_Props SquareProps;
		const SquareProps &sqr_props=props.specific_data.Square;
		Quat orientation=From_Rot_Radians(sqr_props.Orientation.Yaw,sqr_props.Orientation.Pitch,sqr_props.Orientation.Roll);

		switch (sqr_props.Props2D.PlaneSelection)
		{
		case Shapes2D_Props::e_xy_plane:
		case Shapes2D_Props::e_xy_and_xz_plane:
			break;
		case Shapes2D_Props::e_xz_plane:
			orientation*=From_Rot_Radians(0,PI_2,0);
			break;
		case Shapes2D_Props::e_yz_plane:
			orientation*=From_Rot_Radians(PI_2,0,0);
			break;
		}

		//A square is a 2D object therefore has no depth or the forward direction
		//Vec3d ForwardDir(orientation*Vec3d(0,1,0));

		Vec3d UpDir(orientation*Vec3d(0,0,1));
		Vec3d DownDir=UpDir;
		Vec3d RightDir(orientation*Vec3d(1,0,0));
		Vec3d LeftDir=RightDir;

		//Scale normalized by the length and width
		UpDir   *=(sqr_props.Length * sqr_props.YBisect); //bisect length and width to work with that origin
		DownDir *=(sqr_props.Length * (1.0-sqr_props.YBisect));
		LeftDir *=(sqr_props.Width  * sqr_props.XBisect);
		RightDir*=(sqr_props.Width  * (1.0-sqr_props.XBisect));
		//Now to apply position offset
		Vec3d Offset(XPos,ZPos,YPos);

		square[0]= -LeftDir +   UpDir + Offset;
		square[1]= RightDir +   UpDir + Offset;
		square[2]= -LeftDir + -DownDir + Offset;
		square[3]= RightDir + -DownDir + Offset;

		#ifdef __ShowBottomHandle__
		std::string sTest;
		sTest=props.RemoteVariableName;
		sTest+="_bottom_y";
		SmartDashboard::PutNumber(sTest,UnitMeasure2UI(square[3].z()));
		sTest=props.RemoteVariableName;
		sTest+="_bottom_z";
		SmartDashboard::PutNumber(sTest,UnitMeasure2UI(square[3].y()));
		#endif
	}
	void InitCircle(double XPos,double YPos,double ZPos,const Compositor_Props::Shape3D_Renderer_Props &props)
	{
		// point for circle - 25 inch diameter centered at 0,0,0.
		// render twice, with translation.
		double radius = props.specific_data.Shapes2D.Size_1D * 0.5;  //The props stores as diameter so we half it
		int idx = 0;
		for(double rad = 0; rad < 2*M_PI; rad += 2*M_PI/40, idx++ )
		{
			switch (props.specific_data.Shapes2D.PlaneSelection)
			{
			case Shapes2D_Props::e_xy_plane:
			case Shapes2D_Props::e_xy_and_xz_plane:
				circle[idx].x = (cos(rad) * radius) + XPos;
				circle[idx].y = ZPos;
				circle[idx].z = (sin(rad) * radius) + YPos;
				break;
			case Shapes2D_Props::e_xz_plane:
				circle[idx].x = (cos(rad) * radius) + XPos;
				circle[idx].y = (sin(rad) * radius) + ZPos;
				circle[idx].z = YPos;
				break;
			case Shapes2D_Props::e_yz_plane:
				circle[idx].x = ZPos;
				circle[idx].y = (cos(rad) * radius) + ZPos;
				circle[idx].z = (sin(rad) * radius) + YPos;
				break;
			}
		}
	}
	Bitmap_Frame *operator()(Bitmap_Frame *Frame,double XPos,double YPos,double ZPos,const Compositor_Props::Shape3D_Renderer_Props &props,bool EnableFlash)
	{
		if (g_Framework)
		{
			const projection &projector=m_Projection;

			const double R = (double)props.rgb[0];
			const double G = (double)props.rgb[1];
			const double B = (double)props.rgb[2];

			//These come from old code in the TargaReader plugin
			const double Y = (0.257 * R + 0.504 * G + 0.098 * B) + 16.0;
			const double U =(-0.148 * R - 0.291 * G + 0.439 * B) + 128.0;
			const double V = (0.439 * R - 0.368 * G - 0.071 * B) + 128.0;
			//gotta love constants to do this :)
			unsigned int col[] = { (unsigned int)U, (unsigned int)Y, (unsigned int)V, (unsigned int)Y };
			if (EnableFlash)
			{
				col[0]=16;
				col[1]=150;
				col[2]=245;
				col[3]=150;
			}

			projector.screen.SetSize(Frame->XRes, Frame->YRes);

			switch (props.draw_shape)
			{
			case ShapeProps::e_Cube:
				InitCube(XPos,YPos,ZPos,props);
				// test cube.
				// animation. sweep back and forth.
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

				break;
			case ShapeProps::e_Square:
				InitSquare(XPos,YPos,ZPos,props);
				projector.Trans_Line(square[0], square[1]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(square[0], square[2]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(square[1], square[3]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				projector.Trans_Line(square[2], square[3]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);
				break;
			case ShapeProps::e_Circle:
				InitCircle(XPos,YPos,ZPos,props);
				for (int p = 0; p < 39; p++)
				{
					projector.Trans_Line(circle[p], circle[p+1]);
					g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);
				}
				projector.Trans_Line(circle[39], circle[0]);
				g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);

				//For xy and xz we render two circles and xy has already been rendered in the default loop
				if (props.specific_data.Shapes2D.PlaneSelection==Shapes2D_Props::e_xy_and_xz_plane)
				{
					//Copy the props and alter the plane selection to xz (since the xy and xz selection has already rendered the xy selection)
					ShapeProps props_copy=props;
					props_copy.specific_data.Shapes2D.PlaneSelection=Shapes2D_Props::e_xz_plane;
					InitCircle(XPos,YPos,ZPos,props_copy);
					for (int p = 0; p < 39; p++)
					{
						projector.Trans_Line(circle[p], circle[p+1]);
						g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);
					}
					projector.Trans_Line(circle[39], circle[0]);
					g_Framework->DrawLineUYVY(Frame, projector.p1, projector.p2, col);
				}
				break;
			}
		}

		return Frame;

	}

private:
	const projection &m_Projection;
	osg::Vec3d cube[8];
	osg::Vec3d square[4];
	_3Dpoint circle[40];
};

class Bypass_Reticle
{
	private:
	HMODULE m_PlugIn;
	std::string IPAddress,WindowTitle;
	Dashboard_Framework_Interface *DashboardHelper;

	typedef Bitmap_Frame * (*DriverProc_t)(Bitmap_Frame *Frame);
	DriverProc_t m_DriverProc;

	typedef void (*function_Initialize) (const char *IPAddress,const char *WindowTitle,Dashboard_Framework_Interface *DashboardHelper);
	function_Initialize m_fpInitialize;

	typedef void (*function_void) ();
	function_void m_fpShutdown;

	typedef Plugin_Controller_Interface * (*function_create_plugin_controller_interface) ();
	function_create_plugin_controller_interface m_CreatePluginControllerInterface;
	typedef  void (*function_destroy_plugin_controller_interface)(Plugin_Controller_Interface *);
	function_destroy_plugin_controller_interface m_DestroyPluginControllerInterface;

	Plugin_Controller_Interface *m_pPluginControllerInterface;

	public:
	Bypass_Reticle(const char *_IPAddress,const char *_WindowTitle,Dashboard_Framework_Interface *_DashboardHelper) : m_PlugIn(NULL),IPAddress(_IPAddress),WindowTitle(_WindowTitle),
		DashboardHelper(_DashboardHelper),m_DriverProc(NULL),m_pPluginControllerInterface(NULL)
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

	void Callback_Initialize() {if (m_PlugIn) (*m_fpInitialize)(IPAddress.c_str(),WindowTitle.c_str(),DashboardHelper);}
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
				m_Xpos=0.0,m_Ypos=0.0,m_Zpos=0.0;
				SmartDashboard::PutNumber("X Position",0.0);
				SmartDashboard::PutNumber("Y Position",0.0);
				SmartDashboard::PutNumber("Z Position",0.0);
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
		void SetZAxis(double value)
		{
			if (m_IsEditable)
			{
				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				SmartDashboard::PutNumber("Z Position",value);
				m_Zpos+= (value * props.Z_Scalar);
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
				//Only certain 3D reticles have a Z position in the specific data structure
				switch (Sequence[m_SequenceIndex].type)
				{
				case Compositor_Props::ePathAlign:
					Sequence[m_SequenceIndex].specific_data.PositionZ=m_Zpos;
					break;
				case Compositor_Props::eShape3D:
					Sequence[m_SequenceIndex].specific_data.ShapeReticle_props.PositionZ=m_Zpos;
					break;
				}

				//For recursive stepping we can determine if we start the index from the end or beginning from looking at the old and new value
				const bool FromNext=NewSequenceIndex<m_SequenceIndex;
				//issue the update
				m_SequenceIndex=NewSequenceIndex;

				bool CompositeUpdate=false;

				//bounds check the new value
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

				m_LinePlot.FlushCache();  //there may be a new list of colors on the next sequence
				const Compositor_Props::Sequence_Packet &seq_pkt=Sequence[m_SequenceIndex];

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
		
				if (m_PositionTracker.empty())
					SmartDashboard::PutNumber("Sequence",(double)(m_SequenceIndex+1));
				//Modify position to last saved... using m_pSequence... since this may change when stepping into composite
				m_Xpos=(*m_pSequence)[m_SequenceIndex].PositionX;
				m_Ypos=(*m_pSequence)[m_SequenceIndex].PositionY;
				//Only certain 3D reticles have a Z position in the specific data structure
				switch (Sequence[m_SequenceIndex].type)
				{
				case Compositor_Props::ePathAlign:
					m_Zpos=(*m_pSequence)[m_SequenceIndex].specific_data.PositionZ;
					break;
				case Compositor_Props::eShape3D:
					m_Zpos=(*m_pSequence)[m_SequenceIndex].specific_data.ShapeReticle_props.PositionZ;
					break;
				}
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
		Compositor(const char *IPAddress,const char *WindowTitle,Dashboard_Framework_Interface *DashboardHelper) : m_Bypass(IPAddress,WindowTitle,DashboardHelper),m_JoyBinder(FrameWork::GetDirectInputJoystick()),
			m_SequenceIndex(0),m_pSequence(NULL),m_BlinkCounter(0),m_Xpos(0.0),m_Ypos(0.0),m_Zpos(0.0),m_Xpos_Offset(0.0),m_Ypos_Offset(0.0),
			#ifdef __Edit_FOV__
			m_LastFOV(47.0),
			#endif
			m_PathPlotter(),m_ShapeRender(m_PathPlotter.projector),
			m_WindowTitle(WindowTitle),
			m_IsEditable(false),m_PreviousIsEditable(false),m_RecurseIntoComposite(false),m_Flash(false),m_ToggleLinePlot(true)
		{
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Subscribe(ehl,*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Subscribe(ehl,*this, &Compositor::SetYAxis);
			em->EventValue_Map["SetZAxis"].Subscribe(ehl,*this, &Compositor::SetZAxis);
			em->Event_Map["NextSequence"].Subscribe(ehl, *this, &Compositor::NextSequence);
			em->Event_Map["PreviousSequence"].Subscribe(ehl, *this, &Compositor::PreviousSequence);
			em->EventValue_Map["SequencePOV"].Subscribe(ehl,*this, &Compositor::SetPOV);
			em->Event_Map["ToggleLinePlot"].Subscribe(ehl,*this, &Compositor::ToggleLinePlot);
			em->Event_Map["ResetPos"].Subscribe(ehl, *this, &Compositor::ResetPos);

			//m_RecurseIntoComposite=true; //testing  (TODO implement in menu)
			SmartDashboard::PutNumber("Velocity",1.0);
			SmartDashboard::PutNumber("Rotation Velocity",0.0);
			//Setup the edit position name (to be unique per window instance) 
			m_EditPositionName="Edit Position ";
			m_EditPositionName+=WindowTitle;
			SmartDashboard::PutBoolean(m_EditPositionName.c_str(),false);
			#ifdef __Edit_FOV__
			SmartDashboard::PutNumber("FOV",m_LastFOV);
			#endif
			#ifdef __Edit_Camera_Position__
			SmartDashboard::PutNumber("Camera_x",0.0);
			SmartDashboard::PutNumber("Camera_y",Inches2Meters(12));
			SmartDashboard::PutNumber("Camera_z",0.0);
			#endif
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
				switch (seq_pkt.type)
				{
				case Compositor_Props::eComposite:
					{
						out << ",\n";
						Compositor_Props::Sequence_List *Composite=seq_pkt.specific_data.Composite;
						//recursively call this within the composite list to be used as the sequence
						SaveData_Sequence(out,*Composite,RecursiveCount+1);
					}
					break;
				case Compositor_Props::ePathAlign:
					out << ", " << "z=" << seq_pkt.specific_data.PositionZ << " ";
					break;
				case Compositor_Props::eShape3D:
					out << ", " << "z=" << seq_pkt.specific_data.ShapeReticle_props.PositionZ << " ";
					break;
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
			string OutFile = "CompositorSave_";
			OutFile+=m_WindowTitle.c_str();
			OutFile+=".lua";
			string output;

			ofstream out(OutFile.c_str(), std::ios::out );

			const Compositor_Props::Sequence_List &sequence= m_CompositorProperties.GetCompositorProps().Sequence;
			SaveData_Sequence(out,sequence);
		}
		~Compositor()
		{
			//ensure the current position is set on the properties
			UpdateSequence(m_SequenceIndex,true);
		
			SaveData();
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Remove(*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Remove(*this, &Compositor::SetYAxis);
			em->EventValue_Map["SetZAxis"].Remove(*this, &Compositor::SetZAxis);
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
				#ifdef __Edit_FOV__
				m_LastFOV=m_CompositorProperties.GetCompositorProps().PathAlign.FOV_x;
				SmartDashboard::PutNumber("FOV",m_LastFOV);
				#endif
				m_PathPlotter.Initialize(m_CompositorProperties.GetCompositorProps().PathAlign);
				m_PathPlotter.SetNumPoints(m_CompositorProperties.GetCompositorProps().PathAlign.NumberSegments);  //assign the segment count
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
				SmartDashboard::PutBoolean(m_EditPositionName.c_str(),Edit);
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
			Compositor_Props &props_rw=m_CompositorProperties.GetCompositorProps_rw();
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
					Compositor_Props::PathAlign_Container_Props &pa_props_rw=props_rw.PathAlign;
					#if 0
					m_PathPlotter.ComputePathPoints(1.0, 0.25);
					#else
					double Xpos,Ypos,Zpos;
					//We don't want to associate the position with the group offset... its always tuned separately 
					Xpos=(EnableFlash&&m_RecurseIntoComposite)?m_Xpos:seq_pkt.PositionX;
					Ypos=(EnableFlash&&m_RecurseIntoComposite)?m_Ypos:seq_pkt.PositionY;
					Zpos=(EnableFlash&&m_RecurseIntoComposite)?m_Zpos:seq_pkt.specific_data.PositionZ;

					#ifdef __Edit_FOV__
					{
						const double NewFOV=SmartDashboard::GetNumber("FOV");
						if (NewFOV!=m_LastFOV)
						{
							m_PathPlotter.projector.camera.angleh=NewFOV;
							m_PathPlotter.projector.camera.anglev=NewFOV;
							m_PathPlotter.projector.Trans_Initialise();
							m_LastFOV=NewFOV;
						}
					}
					#endif

					#ifdef __Edit_Camera_Position__
					{

						const double Camera_x=SmartDashboard::GetNumber("Camera_x");
						const double Camera_y=SmartDashboard::GetNumber("Camera_y");
						const double Camera_z=SmartDashboard::GetNumber("Camera_z");
						if ((Camera_x!=pa_props.pos_x)||(Camera_y!=pa_props.pos_y)||(Camera_z!=pa_props.pos_z))
						{
							pa_props_rw.pos_x=Inches2Meters(Camera_x);
							pa_props_rw.pos_y=Inches2Meters(Camera_z);
							pa_props_rw.pos_z=Inches2Meters(Camera_y);
							_3Dpoint Camera_position = _3Dpoint(pa_props.pos_x,pa_props.pos_y,pa_props.pos_z);
							m_PathPlotter.projector.camera.from=Camera_position;
							m_PathPlotter.projector.Trans_Initialise();
						}
					}
					#endif

					std::string sTest=pa_props.RemoteVariableName;
					if ((!m_IsEditable)&&(pa_props.RemoteVariableName.c_str()[0]!=0))
					{
						sTest+="_x";
						const double Camera_x=SmartDashboard::GetNumber(sTest);
						sTest=pa_props.RemoteVariableName;
						sTest+="_y";
						const double Camera_y=SmartDashboard::GetNumber(sTest);
						sTest=pa_props.RemoteVariableName;
						sTest+="_z";
						const double Camera_z=SmartDashboard::GetNumber(sTest);

						if ((Camera_x!=pa_props.pos_x)||(Camera_y!=pa_props.pos_y)||(Camera_z!=pa_props.pos_z))
						{
							pa_props_rw.pos_x=UI2UnitMeasure(Camera_x);
							pa_props_rw.pos_y=UI2UnitMeasure(Camera_z);
							pa_props_rw.pos_z=UI2UnitMeasure(Camera_y);
							_3Dpoint Camera_position = _3Dpoint(pa_props.pos_x,pa_props.pos_y,pa_props.pos_z);
							m_PathPlotter.projector.camera.from=Camera_position;
							m_PathPlotter.projector.Trans_Initialise();
						}

						sTest=pa_props.RemoteVariableName;
						sTest+="_rot_x";
						pa_props_rw.rot_x=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
						sTest=pa_props.RemoteVariableName;
						sTest+="_rot_y";
						pa_props_rw.rot_y=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
						sTest=pa_props.RemoteVariableName;
						sTest+="_rot_z";
						pa_props_rw.rot_z=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
					}

					//Treating Xpos and Ypos as degrees will keep the scale to feel about right
					m_PathPlotter.Compute_LookAtFromAngles(DEG_2_RAD(Xpos)+pa_props.rot_x,DEG_2_RAD(Ypos)+pa_props.rot_y,DEG_2_RAD(Zpos)+pa_props.rot_z);
					double Velocity=SmartDashboard::GetNumber("Velocity");
					if (pa_props.IsRearView)
						Velocity=-Velocity;
					double Rotation_Velocity=!pa_props.IgnoreAngularVelocity ? SmartDashboard::GetNumber("Rotation Velocity") : 0.0;
					m_PathPlotter.ComputePathPoints(Feet2Meters(Velocity),-Rotation_Velocity,m_dTime_s,!pa_props.IgnoreLinearVelocity);
					#endif
					ret = m_PathPlotter.RenderPath(Frame,pa_props,m_RecurseIntoComposite&&EnableFlash&&!m_Flash);
				}
				break;
			case Compositor_Props::eShape3D:
				{
					const Compositor_Props::Shape3D_Renderer_Props &shape_props=props.shapes_3D_reticle[seq_pkt.specific_data.SquareReticle_SelIndex];
					Compositor_Props::Shape3D_Renderer_Props &shape_props_rw=props_rw.shapes_3D_reticle[seq_pkt.specific_data.SquareReticle_SelIndex];
					double Xpos,Ypos,Zpos;
					std::string sTest=shape_props.RemoteVariableName;
					bool RenderShape=true;
					if ((!m_IsEditable)&&(shape_props.RemoteVariableName.c_str()[0]!=0))
					{
						sTest+="_x";
						Xpos=UI2UnitMeasure(SmartDashboard::GetNumber(sTest));
						sTest=shape_props.RemoteVariableName;
						sTest+="_y";
						Ypos=UI2UnitMeasure(SmartDashboard::GetNumber(sTest));
						sTest=shape_props.RemoteVariableName;
						sTest+="_z";
						Zpos=UI2UnitMeasure(SmartDashboard::GetNumber(sTest));
						sTest=shape_props.RemoteVariableName;
						sTest+="_enabled";
						RenderShape=SmartDashboard::GetBoolean(sTest);

						typedef Compositor_Props::Shape3D_Renderer_Props ShapeProps;
						if (shape_props.draw_shape==ShapeProps::e_Square)
						{
							ShapeProps::type_specifics::Square_Props &orientation_rw=shape_props_rw.specific_data.Square;
							sTest=shape_props.RemoteVariableName;
							sTest+="_rot_x";
							orientation_rw.Orientation.Yaw=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
							sTest=shape_props.RemoteVariableName;
							sTest+="_rot_y";
							orientation_rw.Orientation.Pitch=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
							sTest=shape_props.RemoteVariableName;
							sTest+="_rot_z";
							orientation_rw.Orientation.Roll=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
						}
						else if (shape_props.draw_shape==ShapeProps::e_Cube)
						{
							ShapeProps::type_specifics::Cube_Props &orientation_rw=shape_props_rw.specific_data.Cube;
							sTest=shape_props.RemoteVariableName;
							sTest+="_rot_x";
							orientation_rw.Orientation.Yaw=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
							sTest=shape_props.RemoteVariableName;
							sTest+="_rot_y";
							orientation_rw.Orientation.Pitch=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
							sTest=shape_props.RemoteVariableName;
							sTest+="_rot_z";
							orientation_rw.Orientation.Roll=DEG_2_RAD(SmartDashboard::GetNumber(sTest));
						}
					}
					else
					{
						if (m_RecurseIntoComposite)
						{
							Xpos=EnableFlash?m_Xpos+XOffset:seq_pkt.PositionX+XOffset;
							Ypos=EnableFlash?m_Ypos+YOffset:seq_pkt.PositionY+YOffset;
							Zpos=EnableFlash?m_Zpos:seq_pkt.specific_data.ShapeReticle_props.PositionZ;
							if (EnableFlash)
							{
								m_Xpos=Xpos-XOffset;
								m_Ypos=Ypos-YOffset;
							}
						}
						else
						{
							//If we are not the top layer then we are always locked down if we are not recursing
							bool UseInput=(&sequence==&props.Sequence) && EnableFlash;
							Xpos=UseInput?m_Xpos:seq_pkt.PositionX+XOffset;
							Ypos=UseInput?m_Ypos:seq_pkt.PositionY+YOffset;
							Zpos=UseInput?m_Zpos:seq_pkt.specific_data.ShapeReticle_props.PositionZ;
							if (EnableFlash)
							{
								double TempXpos=UseInput?Xpos:Xpos-seq_pkt.PositionX;
								m_Xpos=(TempXpos<0)?std::max(TempXpos,m_Xpos):std::min(TempXpos,m_Xpos);

								double TempYpos=UseInput?Ypos:Ypos-seq_pkt.PositionY;
								m_Ypos=(TempXpos<0)?std::max(TempYpos,m_Ypos):std::min(TempYpos,m_Ypos);
							}
						}
						if (shape_props.RemoteVariableName.c_str()[0]!=0)
						{
							sTest=shape_props.RemoteVariableName;
							sTest+="_x";
							SmartDashboard::PutNumber(sTest,UnitMeasure2UI(Xpos));
							sTest=shape_props.RemoteVariableName;
							sTest+="_y";
							SmartDashboard::PutNumber(sTest,UnitMeasure2UI(Ypos));
							sTest=shape_props.RemoteVariableName;
							sTest+="_z";
							SmartDashboard::PutNumber(sTest,UnitMeasure2UI(Zpos));

							//TODO enable once I have check box for rotation
							//typedef Compositor_Props::Shape3D_Renderer_Props ShapeProps;
							//if (shape_props.draw_shape==ShapeProps::e_Square)
							//{
							//	sTest+="_rot_x";
							//	SmartDashboard::PutNumber(sTest,RAD_2_DEG(Xpos));
							//	sTest=shape_props.RemoteVariableName;
							//	sTest+="_rot_y";
							//	SmartDashboard::PutNumber(sTest,RAD_2_DEG(Ypos));
							//	sTest=shape_props.RemoteVariableName;
							//	sTest+="_rot_z";
							//	SmartDashboard::PutNumber(sTest,RAD_2_DEG(Zpos));
							//}
						}
					}
					if (RenderShape)
						m_ShapeRender(Frame,Xpos,Ypos,Zpos,shape_props,EnableFlash&&!m_Flash);
				}
				break;
			case Compositor_Props::eBypass:
				{
					//Enable this to profile the bypass plugin
					//Keep this disabled... use one with processing vision instead
					#if 0
					using namespace FrameWork;
					time_type Profile=time_type::get_current_time();
					ret=m_Bypass.Callback_ProcessFrame_UYVY(Frame);
					time_type Profile2=time_type::get_current_time();
					SmartDashboard::PutNumber("Bypass Time ms",(double)(Profile2-Profile)*1000.0);
					#else
					ret=m_Bypass.Callback_ProcessFrame_UYVY(Frame);
					#endif
				}
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
			m_dTime_s=dTime_s;
			m_LastTime=current_time;
			m_Frame=Frame; //to access frame properties during the event callback
			m_JoyBinder.UpdateJoyStick(dTime_s);

			if (SmartDashboard::IsConnected())
			{
				m_IsEditable=SmartDashboard::GetBoolean(m_EditPositionName.c_str());
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
		double m_dTime_s;
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
		double m_Xpos,m_Ypos,m_Zpos;
		double m_Xpos_Offset,m_Ypos_Offset;  //only used when stepping through recursion
		#ifdef __Edit_FOV__
		double m_LastFOV;
		#endif
		Bypass_Reticle m_Bypass;
		LinePlot_Retical m_LinePlot;
		PathRenderer m_PathPlotter;
		Shape3D_Renderer m_ShapeRender;
		std::string m_EditPositionName;
		std::string m_WindowTitle;

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

extern "C" COMPOSITER_API void Callback_SmartCppDashboard_Initialize(const char *IPAddress,const char *WindowTitle,Dashboard_Framework_Interface *DashboardHelper)
{
	g_Framework=DashboardHelper;
	SmartDashboard::SetClientMode();
	SmartDashboard::SetIPAddress(IPAddress);
	SmartDashboard::init();

	g_pCompositor = new Compositor(IPAddress,WindowTitle,DashboardHelper);
	{
		Compositor_Properties props;
		Scripting::Script script;
		const char *err;

		{	//Try the unique name first
			std::string UniqueName="Compositor_";
			UniqueName+=WindowTitle;
			UniqueName+=".lua";

			err=script.LoadScript(UniqueName.c_str(),true);
			if (err!=NULL)
			{
				UniqueName="../Compositor/Compositor_";
				UniqueName+=WindowTitle;
				UniqueName+=".lua";
				err=script.LoadScript(UniqueName.c_str(),true);
			}
		}

		//If that doesn't work try the legacy name
		if (err!=NULL)
		{
			err=script.LoadScript("Compositor.lua",true);
			if (err!=NULL)
				err=script.LoadScript("../Compositor/Compositor.lua",true);
		}

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