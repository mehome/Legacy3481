#include "stdafx.h"
#define __IncludeInputBase__
#include "../FrameWork/FrameWork.h"
#include "Compositer.h"
#include "../SmartDashboard2/SmartDashboard_Import.h"

Dashboard_Framework_Interface *g_Framework=NULL;
using namespace FrameWork;
event frameSync;



struct Compositor_Props
{
	public:	

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

static Bitmap_Frame *RenderGreenReticle(Bitmap_Frame *Frame,double XPos,double YPos)
{
	if (g_Framework)
	{
		Bitmap_Handle *bgra_handle=g_Framework->CreateBGRA(Frame);
		Bitmap_Frame &bgra_frame=bgra_handle->frame;
		g_Framework->UYVY_to_BGRA(Frame,&bgra_frame);
		#if 0
		//Test... make a green box in the center of the frame
		const size_t PositionY=bgra_frame.YRes / 2;
		const size_t PositionX=bgra_frame.XRes / 2;
		#else
		int Px,Py;
		AimingSystem_to_PixelSystem(Px,Py,XPos,YPos,bgra_frame.XRes,bgra_frame.YRes,((double)bgra_frame.XRes/(double)bgra_frame.YRes));
		const size_t PositionY=(size_t)Py;
		const size_t PositionX=(size_t)Px;
		#endif
		size_t LineWidthInBytes=bgra_frame.Stride * 4;
		for (size_t y=PositionY-5;y<PositionY+5;y++)
		{
			for (size_t x=PositionX-5; x<PositionX+5; x++)
			{
				*(bgra_frame.Memory+ (x*4 + 0) + (LineWidthInBytes * y))=0;
				*(bgra_frame.Memory+ (x*4 + 1) + (LineWidthInBytes * y))=255;
				*(bgra_frame.Memory+ (x*4 + 2) + (LineWidthInBytes * y))=0;
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
				//Temp testing
				SmartDashboard::PutNumber("X Position",value);
				m_Xpos+= (value * 0.025);
				if (m_Xpos>1.0)
					m_Xpos=1.0;
				else if (m_Xpos<-1.0)
					m_Xpos=-1.0;
			}
		}
		void SetYAxis(double value)
		{
			if (m_IsEditable)
			{
				//Temp testing
				SmartDashboard::PutNumber("Y Position",value);
				m_Ypos+= (value * 0.025);
				if (m_Ypos>0.8)
					m_Ypos=0.8;
				else if (m_Ypos<-0.8)
					m_Ypos=-0.8;
			}
		}

		IEvent::HandlerList ehl;
		Compositor() : m_JoyBinder(FrameWork::GetDirectInputJoystick()),m_Xpos(0.0),m_Ypos(0.0)
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

		Bitmap_Frame *TimeChange(Bitmap_Frame *Frame)
		{

			const time_type current_time=time_type::get_current_time();
			const double dTime_s=(double)(current_time-m_LastTime);
			m_LastTime=current_time;
			m_JoyBinder.UpdateJoyStick(dTime_s);

			m_IsEditable=SmartDashboard::GetBoolean("Edit Position");
			return RenderGreenReticle(Frame,m_Xpos,m_Ypos);
		}

	private:
		time_type m_LastTime;
		FrameWork::UI::JoyStick_Binder m_JoyBinder;
		FrameWork::EventMap m_EventMap;
		Compositor_Properties m_CompositorProperties;
		double m_Xpos,m_Ypos;
		bool m_IsEditable;
} *g_pCompositor;

//Give something cool to look at
class SineWaveMaker
{
	public:
		SineWaveMaker() : m_rho(0.0),m_rho2(0.0) {}
		void operator()(double &Sample,double &Sample2,double freq_hz=1000,double SampleRate=48000,double amplitude=1.0)
		{
			double			 theta,theta2;
			size_t index=0; //array index of buffer

			const double pi2 = 3.1415926 * 2.0;
			//Compute the angle ratio unit we are going to use
			//Multiply times pi 2 to Convert the angle ratio unit into radians
			theta = (freq_hz / SampleRate) * pi2;
			theta2 = ((freq_hz*0.5) / SampleRate) * pi2;

			Sample = sin( m_rho ) * amplitude;
			Sample2 = sin (m_rho2) * amplitude;

			//Find Y given the hypotenuse (scale) and the angle (rho)
			//Note: using sin will solve for Y, and give us an initial 0 size
			//increase our angular measurement
			m_rho += theta;
			m_rho2 += theta2;
			//bring back the angular measurement by the length of the circle when it has completed a revolution
			if ( m_rho > pi2 )
				m_rho -= pi2;
			if ( m_rho2 > pi2 )
				m_rho2 -= pi2;
		}
	private:
		double m_rho,m_rho2;
} g_TestSample;

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
			err=script.LoadScript("../Compositer/Compositor.lua",true);

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
