#pragma once

namespace FrameWork
{

class LUA_Controls_Properties_Interface
{
	public:
		//The client properties class needs to have list of elements to check... return NULL when reaching the end
		virtual const char *LUA_Controls_GetEvents(size_t index) const =0;
};

//This is a helper class that makes it easy to transfer LUA script to its own contained list (included within)
class LUA_Controls_Properties
{
	public:
		enum JoyAxis_enum
		{
			eX_Axis,
			eY_Axis,
			eZ_Axis,
			eX_Rot,
			eY_Rot,
			eZ_Rot,
			eSlider0,
			eSlider1,
			ePOV_0,
			ePOV_1,
			ePOV_2,
			ePOV_3,
			eNoJoyAxis_Entries
		};

		struct Controller_Element_Properties
		{
			std::string Event;
			enum ElementType
			{
				eJoystickAnalog,
				eJoystickCulver,
				eJoystickButton,
				eKeyboard			//currently only available on simulation
			} Type;
			union ElementTypeSpecific
			{
				struct AnalogSpecifics_rw
				{
					JoyAxis_enum JoyAxis;
					bool IsFlipped;
					double Multiplier;
					double FilterRange;
					double CurveIntensity;
				} Analog;
				struct CulverSpecifics_rw
				{
					JoyAxis_enum JoyAxis_X;
					JoyAxis_enum JoyAxis_Y;
					double MagnitudeScalarArc,MagnitudeScalarBase;
					bool IsFlipped;
					double Multiplier;
					double FilterRange;
					double CurveIntensity;
				} Culver;
				struct ButtonSpecifics_rw
				{
					size_t WhichButton;
					size_t WhichKey;  //for keyboard... bundled together so events are duplicated -1 if not used
					bool useOnOff;
					bool dbl_click;
				} Button;
				struct KeyboardSpecifics_rw
				{
					size_t WhichKey;
					bool useOnOff;
					bool dbl_click;
				} Keyboard;

			} Specifics;
		};

		struct Control_Props
		{
			std::vector<Controller_Element_Properties> EventList;
			std::string Controller;
		};
		typedef std::vector<Control_Props> Controls_List;
	private:
		//Return if element was successfully created (be sure to check as some may not be present)
		static const char *ExtractControllerElementProperties(Controller_Element_Properties &Element,const char *Eventname,Scripting::Script& script);

		Controls_List m_Controls;
		LUA_Controls_Properties_Interface * m_pParent;
	public:
		LUA_Controls_Properties(LUA_Controls_Properties_Interface *parent);

		const Controls_List &Get_Controls() const {return m_Controls;}
		//call from within GetFieldTable controls
		void LoadFromScript(Scripting::Script& script);
		//Just have the client (from ship) call this
		void BindAdditionalUIControls(bool Bind,void *joy,void *key) const;
		LUA_Controls_Properties &operator= (const LUA_Controls_Properties &CopyFrom);
};

}