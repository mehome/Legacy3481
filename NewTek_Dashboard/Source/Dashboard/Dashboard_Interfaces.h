#pragma once

//Here are the ranges for each
//brightness	-1.0 - 1.0
//contrast		 0.0 - 1.0 (unity) - about 4.0
//hue		  -180.0 - 180.0
//saturation     0.0 - 1.0 (unity) - about 4.0
//u_offset		-1.0 - 1.0
//v_offset		-1.0 - 1.0
//u_gain		 0.0 - 1.0 (unity) - about 4.0
//v_gain	 	 0.0 - 1.0 (unity) - about 4.0


enum ProcAmp_enum
{
	e_procamp_brightness,
	e_procamp_contrast,
	e_procamp_hue,
	e_procamp_saturation,
	e_procamp_u_offset,
	e_procamp_v_offset,
	e_procamp_u_gain,
	e_procamp_v_gain,
	e_procamp_pedestal,		//Currently not used reserved for future use
	e_no_procamp_items
};

class Dashboard_Controller_Interface
{
public:
	virtual ~Dashboard_Controller_Interface() {}
	virtual void Flush()=0;
	virtual int Run (void)=0;					
	virtual int Stop (void)=0;					
	virtual int Pause (void)=0;					
	virtual void Seek (double fraction)=0;							//enter a fractional value range 0 - 1.0
	virtual int SetRate (int)=0;									//currently not supported
	virtual void SwitchFilename(const wchar_t FileToUse[])=0;		//switches to next file (will play it once switched)
	virtual void GetFileName(std::wstring &Output) const=0;

	virtual bool Set_ProcAmp(ProcAmp_enum ProcSetting,double value)=0;
	virtual double Get_ProcAmp(ProcAmp_enum ProcSetting) const=0;
};

//All modeless Dialogs and Windows must inherit from this dispatcher for best performance, this will ensure the UI thread and parent message pump
//is used
class MessageBase_Interface
{
public:
	virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
};
