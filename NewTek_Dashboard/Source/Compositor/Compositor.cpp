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


struct Compositor_Props
{
	double X_Scalar;
	double Y_Scalar;
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
		bool UsingShadow;
	};
	std::vector<SquareReticle_Container_Props> square_reticle;
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
	"SetXAxis","SetYAxis","NextSequence","PreviousSequence"
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
	props.square_reticle.push_back(sqr_pkt);

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
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"square_reticle_%d",index);
		err = script.GetFieldTable(Buffer);
		if (!err)
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
	//note: call super here if we derived from other props
	err = script.GetFieldTable("settings");
	if (!err)
	{
		Compositor_Props &props=m_CompositorProps;
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

static Bitmap_Frame *RenderSquareReticle(Bitmap_Frame *Frame,double XPos,double YPos,const Compositor_Props::SquareReticle_Props &props,
										 int PixelOffsetX=0,int PixelOffsetY=0)
{
	if (g_Framework)
	{
		Bitmap_Handle *bgra_handle=g_Framework->CreateBGRA(Frame);
		Bitmap_Frame &bgra_frame=bgra_handle->frame;
		g_Framework->UYVY_to_BGRA(Frame,&bgra_frame);
		#if 0
		//Test... make a green box in the center of the frame
		size_t PositionY=bgra_frame.YRes / 2;
		size_t PositionX=bgra_frame.XRes / 2;
		#else
		int Px,Py;
		AimingSystem_to_PixelSystem(Px,Py,XPos,YPos,bgra_frame.XRes,bgra_frame.YRes,((double)bgra_frame.XRes/(double)bgra_frame.YRes));
		size_t PositionY=(size_t)(Py + PixelOffsetY);
		size_t PositionX=(size_t)(Px + PixelOffsetX);
		#endif

		//Test bounds
		const size_t ThicknessX=props.ThicknessX;
		const size_t ThicknessY=props.ThicknessY;
		const double Opacity=props.opacity;

		if (PositionX<ThicknessX)
			PositionX=ThicknessX;
		else if (PositionX>bgra_frame.XRes-ThicknessX)
			PositionX=bgra_frame.XRes-ThicknessX;
		if (PositionY<ThicknessY)
			PositionY=ThicknessY;
		else if (PositionY>bgra_frame.YRes-ThicknessY)
			PositionY=bgra_frame.YRes-ThicknessY;


		size_t LineWidthInBytes=bgra_frame.Stride * 4;
		for (size_t y=PositionY-ThicknessY;y<PositionY+ThicknessY;y++)
		{
			for (size_t x=PositionX-ThicknessX; x<PositionX+ThicknessX; x++)
			{
				PBYTE pBlue=(bgra_frame.Memory+ (x*4 + 0) + (LineWidthInBytes * y));
				PBYTE pGreen=(bgra_frame.Memory+ (x*4 + 1) + (LineWidthInBytes * y));
				PBYTE pRed=(bgra_frame.Memory+ (x*4 + 2) + (LineWidthInBytes * y));
				*pBlue =(BYTE)((Opacity*(double)props.rgb[2])+((1.0-Opacity)* (double)(*pBlue )));  //blue
				*pGreen=(BYTE)((Opacity*(double)props.rgb[1])+((1.0-Opacity)* (double)(*pGreen)));  //green
				*pRed  =(BYTE)((Opacity*(double)props.rgb[0])+((1.0-Opacity)* (double)(*pRed  )));  //red
			}
		}
		g_Framework->BGRA_to_UYVY(&bgra_frame,Frame);
		g_Framework->DestroyBGRA(bgra_handle);
	}
	return Frame;
}

class Compositor
{
	public:
		void SetXAxis(double value)
		{
			if (m_IsEditable)
			{
				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				SmartDashboard::PutNumber("X Position",value);
				m_Xpos+= (value * props.X_Scalar);
				const double x_limit=(m_Frame->XRes>m_Frame->YRes)?(double)m_Frame->XRes/(double)m_Frame->YRes : 1.0;
				if (m_Xpos>x_limit)
					m_Xpos=x_limit;
				else if (m_Xpos<-x_limit)
					m_Xpos=-x_limit;
			}
		}
		void SetYAxis(double value)
		{
			if (m_IsEditable)
			{
				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				SmartDashboard::PutNumber("Y Position",value);
				m_Ypos+= (value * props.Y_Scalar);
				const double y_limit=(m_Frame->YRes>m_Frame->XRes)?(double)m_Frame->YRes/(double)m_Frame->XRes : 1.0;
				if (m_Ypos>y_limit)
					m_Ypos=y_limit;
				else if (m_Ypos<-y_limit)
					m_Ypos=-y_limit;
			}
		}

		IEvent::HandlerList ehl;
		Compositor() : m_JoyBinder(FrameWork::GetDirectInputJoystick()),m_Xpos(0.0),m_Ypos(0.0),m_IsEditable(false)
		{
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Subscribe(ehl,*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Subscribe(ehl,*this, &Compositor::SetYAxis);
		}
		~Compositor()
		{
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Remove(*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Remove(*this, &Compositor::SetYAxis);
			m_CompositorProperties.Get_CompositorControls().BindAdditionalUIControls(false,&m_JoyBinder,NULL);
		}
		virtual void Initialize(EventMap& em, const Compositor_Properties *props=NULL)
		{
			//props may be NULL if there is no lua
			if (props)
			{
				m_CompositorProperties=*props;
				m_CompositorProperties.Get_CompositorControls().BindAdditionalUIControls(true,&m_JoyBinder,NULL);
			}

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

		Bitmap_Frame *TimeChange(Bitmap_Frame *Frame)
		{

			const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
			const time_type current_time=time_type::get_current_time();
			const double dTime_s=(double)(current_time-m_LastTime);
			m_LastTime=current_time;
			m_Frame=Frame; //to access frame properties during the event callback
			m_JoyBinder.UpdateJoyStick(dTime_s);

			if (SmartDashboard::IsConnected())
				m_IsEditable=SmartDashboard::GetBoolean("Edit Position");

			const Compositor_Props::SquareReticle_Container_Props &sqr_props=props.square_reticle[0];
			if (sqr_props.UsingShadow)
				RenderSquareReticle(Frame,m_Xpos,m_Ypos,sqr_props.shadow,sqr_props.PixelOffsetX,sqr_props.PixelOffsetY);
			return RenderSquareReticle(Frame,m_Xpos,m_Ypos,sqr_props.primary);
		}

	private:
		time_type m_LastTime;
		FrameWork::UI::JoyStick_Binder m_JoyBinder;
		FrameWork::EventMap m_EventMap;
		Compositor_Properties m_CompositorProperties;
		Bitmap_Frame *m_Frame;
		double m_Xpos,m_Ypos;
		bool m_IsEditable;
} *g_pCompositor;


extern "C" COMPOSITER_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame)
{
	if (g_pCompositor)
		return g_pCompositor->TimeChange(Frame);
	else
		return Frame;
}

extern "C" COMPOSITER_API void Callback_SmartCppDashboard_Initialize(char *IPAddress,Dashboard_Framework_Interface *DashboardHelper)
{
	g_Framework=DashboardHelper;
	SmartDashboard::SetClientMode();
	SmartDashboard::SetIPAddress(IPAddress);
	SmartDashboard::init();
	SmartDashboard::PutBoolean("Edit Position",false);

	g_pCompositor = new Compositor();
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