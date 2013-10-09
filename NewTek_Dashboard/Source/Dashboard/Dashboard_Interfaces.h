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

struct Bitmap_Frame
{
	Bitmap_Frame(PBYTE memory,size_t xres,size_t yres,size_t stride) : Memory(memory),XRes(xres),YRes(yres),Stride(stride) {}
	PBYTE Memory;  //A pointer to memory of the frame... this can be written over for compositing effects
	size_t XRes,YRes;  //X and Y res in pixels, these may also change but typically should stay the same (and return same memory)
	size_t Stride;  //This is >= Xres some memory buffers need extra room for processing... this can be changed (and might need to be)
};
struct Bitmap_Handle
{
	Bitmap_Handle(PBYTE memory,size_t xres,size_t yres,size_t stride);
	~Bitmap_Handle();
	Bitmap_Frame frame;
	void *Handle; //used internally
};

class Dashboard_Framework_Interface
{
public:
	//helper functions to easily construct blank BGRA frame
	virtual Bitmap_Handle *CreateBGRA(const Bitmap_Frame *sourceUVYV)=0;
	virtual void DestroyBGRA(Bitmap_Handle *handle)=0;

	//converter functions
	virtual void UYVY_to_BGRA(const Bitmap_Frame *sourceUVYV,Bitmap_Frame *destBGRA)=0;
	virtual void BGRA_to_UYVY(const Bitmap_Frame *sourceBGRA,Bitmap_Frame *destUYVY)=0;
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
	virtual void Record(bool)=0;
	virtual bool GetRecordState(void)=0;
	virtual void SetRecordPath(const char *Path)=0;
	virtual const char *GetRecordPath()=0;

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

class Plugin_Controller_Interface
{
	public:
		virtual const char *GetPlugInName() const=0;
		virtual ~Plugin_Controller_Interface() {}
};