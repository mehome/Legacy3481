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
	"none","square","composite","bypass"
};

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
	enum ReticleType
	{
		eNone,
		eDefault,
		eComposite,
		eBypass
	};
	static ReticleType GetReticleType_Enum (const char *value)
	{	return Enum_GetValue<ReticleType> (value,csz_ReticleType_Enum,_countof(csz_ReticleType_Enum));
	}

	static const char * const GetReticleType_String(ReticleType value)
	{	return csz_ReticleType_Enum[value];
	}

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
	"SetXAxis","SetYAxis","NextSequence","PreviousSequence","SequencePOV"
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

static void LoadSequenceProps(Scripting::Script& script,Compositor_Props &props)
{
	const char* err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"sequence_%d",index);
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
				{
					double fTest;

					err = script.GetField("selection",NULL,NULL,&fTest);
					seq_pkt.specific_data.SquareReticle_SelIndex=(size_t)fTest;
					seq_pkt.specific_data.SquareReticle_SelIndex--;  // translate cardinal to ordinal 
				}
				break;
			}

			//These always start out zero'd
			seq_pkt.PositionX=0.0;
			seq_pkt.PositionY=0.0;

			props.Sequence.push_back(seq_pkt);
			index++;

			script.Pop();
		}
	} while (!err);
}

static void LoadSequence_PersistentData(Scripting::Script& script,Compositor_Props &props)
{
	const char* err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"sequence_%d",index);
		err = script.GetFieldTable(Buffer);
		if (!err)
		{
			Compositor_Props::Sequence_Packet &seq_pkt=props.Sequence[index-1]; //ordinal translation

			std::string sTest;
			err = script.GetField("type",&sTest,NULL,NULL);
			assert(!err);  //gotta have it if we are making a sequence
			assert(seq_pkt.type==Compositor_Props::GetReticleType_Enum(sTest.c_str()));  //handle recovery later

			switch (seq_pkt.type)
			{
			case Compositor_Props::eDefault:
				{
					err = script.GetField("x",NULL,NULL,&seq_pkt.PositionX);
					err = script.GetField("y",NULL,NULL,&seq_pkt.PositionY);
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
		err = script.GetFieldTable("sequence");
		if (!err)
		{
			//clear defaults
			props.Sequence.clear();  //for now it is assumed the default will not allocate a composite
			LoadSequenceProps(script,props);

			err = script.GetFieldTable("load_settings");
			{
				if (!err)
				{
					LoadSequence_PersistentData(script,props);
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

static Bitmap_Frame *RenderSquareReticle(Bitmap_Frame *Frame,double XPos,double YPos,const Compositor_Props::SquareReticle_Props &props,
										 int PixelOffsetX=0,int PixelOffsetY=0)
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

		for (size_t y=PositionY-ThicknessY;y<PositionY+ThicknessY;y++)
		{
			for (size_t x=PositionX-ThicknessX; x<PositionX+ThicknessX; x+=2)
			{
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
		void UpdateSequence(size_t NewSequenceIndex,bool forceUpdate=false)
		{
			if ((m_SequenceIndex!=NewSequenceIndex)||forceUpdate)
			{
				Compositor_Props &props_rw=m_CompositorProperties.GetCompositorProps_rw();
				//Save current position to this sequence packet
				props_rw.Sequence[m_SequenceIndex].PositionX=m_Xpos;
				props_rw.Sequence[m_SequenceIndex].PositionY=m_Ypos;

				//issue the update
				m_SequenceIndex=NewSequenceIndex;

				const Compositor_Props &props=m_CompositorProperties.GetCompositorProps();
				if (m_SequenceIndex==-1)
					m_SequenceIndex=props.Sequence.size()-1;
				else if (m_SequenceIndex>=props.Sequence.size())
					m_SequenceIndex=0;

				SmartDashboard::PutNumber("Sequence",(double)(m_SequenceIndex+1));
				//Modify position to last saved
				m_Xpos=props.Sequence[m_SequenceIndex].PositionX;
				m_Ypos=props.Sequence[m_SequenceIndex].PositionY;

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

		IEvent::HandlerList ehl;
		Compositor() : m_JoyBinder(FrameWork::GetDirectInputJoystick()),m_SequenceIndex(0),m_BlinkCounter(0),m_Xpos(0.0),m_Ypos(0.0),m_IsEditable(false)
		{
			FrameWork::EventMap *em=&m_EventMap; 
			em->EventValue_Map["SetXAxis"].Subscribe(ehl,*this, &Compositor::SetXAxis);
			em->EventValue_Map["SetYAxis"].Subscribe(ehl,*this, &Compositor::SetYAxis);
			em->Event_Map["NextSequence"].Subscribe(ehl, *this, &Compositor::NextSequence);
			em->Event_Map["PreviousSequence"].Subscribe(ehl, *this, &Compositor::PreviousSequence);
			em->EventValue_Map["SequencePOV"].Subscribe(ehl,*this, &Compositor::SetPOV);
		}
		void SaveData()
		{
			using namespace std;
			string OutFile = "CompositorSave.lua";
			string output;

			ofstream out(OutFile.c_str(), std::ios::out );

			out << "sequence_load = " << endl;    //header

			out << "{" << endl;
			const Compositor_Props::Sequence_List &sequence= m_CompositorProperties.GetCompositorProps().Sequence;
			for (size_t i=0;i<sequence.size();i++)
			{
				const Compositor_Props::Sequence_Packet &seq_pkt=sequence[i];
				out << "\t" << "sequence_" << (i+1) << " = {";
				out << "type=" << "\"" << Compositor_Props::GetReticleType_String(seq_pkt.type) << "\", ";
				//TODO write specific data types here... which should be the same expect for the composite type
				out << "x=" << seq_pkt.PositionX << ", " << "y=" << seq_pkt.PositionY << " ";
				out << "}," << endl;
			}

			out << "}" << endl;  //footer
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
			
			m_CompositorProperties.Get_CompositorControls().BindAdditionalUIControls(false,&m_JoyBinder,NULL);
			//For now limit the sequence to advance and previous calls to save network bandwidth... we can move this to the time change if necessary
			SmartDashboard::PutNumber("Sequence",(double)(m_SequenceIndex+1));
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

				//Setup our sequence display and positions
				if (sequence.size())
					UpdateSequence(m_SequenceIndex,true);
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
			{
				m_IsEditable=SmartDashboard::GetBoolean("Edit Position");
				UpdateSequence((size_t)SmartDashboard::GetNumber("Sequence")-1); //convert to ordinal
			}

			Bitmap_Frame *ret=Frame;
			bool Flash=true;
			if (m_IsEditable)
			{
				Flash=(m_BlinkCounter++&0x10)!=0;
				m_BlinkCounter=(m_BlinkCounter&0x1f);
			}
			switch (props.Sequence[m_SequenceIndex].type)
			{
			case Compositor_Props::eDefault:
				{
					//copy the props to alter the opacity for blinking
					Compositor_Props::SquareReticle_Container_Props sqr_props=props.square_reticle[props.Sequence[m_SequenceIndex].specific_data.SquareReticle_SelIndex];
					if (!Flash)
						sqr_props.primary.opacity*=0.5;
					if (sqr_props.UsingShadow)
					{
						sqr_props.shadow.opacity*=0.5;
						RenderSquareReticle(Frame,m_Xpos,m_Ypos,sqr_props.shadow,sqr_props.PixelOffsetX,sqr_props.PixelOffsetY);
					}
					ret=RenderSquareReticle(Frame,m_Xpos,m_Ypos,sqr_props.primary);
				}
			};

			return ret;
		}

	private:
		time_type m_LastTime;
		FrameWork::UI::JoyStick_Binder m_JoyBinder;
		FrameWork::EventMap m_EventMap;
		Compositor_Properties m_CompositorProperties;
		Bitmap_Frame *m_Frame;
		size_t m_SequenceIndex;
		size_t m_BlinkCounter; //very simple blink mechanism
		double m_Xpos,m_Ypos;
		bool m_IsEditable;
		bool m_POVSetValve;
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