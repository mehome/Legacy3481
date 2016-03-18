#include "stdafx.h"

//Can only build release with static environment
#ifndef _DEBUG
#define __BuildStaticFFMpeg__
#endif

#include "Dashboard_Interfaces.h"
#include <ddraw.h>
#include <atlbase.h>
#include <shellapi.h>
#include <strsafe.h>
#define __IncludeInputBase__  //we want lua
#include "../FrameWork/FrameWork.h"
#include "../FrameWork/Window.h"
#include "../FrameWork/Preview.h"
#include "../FFMpeg121125/FrameGrabber.h"
#pragma comment (lib,"shell32")
#pragma comment (lib,"winhttp.lib")
#ifndef __BuildStaticFFMpeg__
#pragma comment (lib,"../FFMpeg121125/lib/avcodec.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avdevice.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avfilter.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avformat.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avutil.lib")
//#pragma comment (lib , "lib/postproc.lib")
#pragma comment (lib , "../FFMpeg121125/lib/swresample.lib")
#pragma comment (lib , "../FFMpeg121125/lib/swscale.lib")
#else
#pragma comment (lib,"../FFMpeg121125/lib/libavcodec.lib")
#pragma comment (lib , "../FFMpeg121125/lib/libavdevice.lib")
#pragma comment (lib , "../FFMpeg121125/lib/libavfilter.lib")
#pragma comment (lib , "../FFMpeg121125/lib/libavformat.lib")
#pragma comment (lib , "../FFMpeg121125/lib/libavutil.lib")
#pragma comment (lib , "../FFMpeg121125/lib/libswresample.lib")
#pragma comment (lib , "../FFMpeg121125/lib/libswscale.lib")
#pragma comment( lib, "Ws2_32" )
#endif

#pragma comment (lib , "../FFMpeg121125/SDL/lib/x86/SDL.lib")
#pragma comment (lib , "../FFMpeg121125/SDL/lib/x86/SDLmain.lib")

// Converts a GUID to a string
inline void GUIDtow(GUID id,wchar_t *string) {
	wsprintfW(string,L"%x-%x-%x-%x%x-%x%x%x%x%x%x",id.Data1,id.Data2,id.Data3,
		id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]);
}

Bitmap_Handle::Bitmap_Handle(PBYTE memory,size_t xres,size_t yres,size_t stride) : frame(memory,xres,yres,stride)
{
}

Bitmap_Handle::~Bitmap_Handle()
{
	using namespace FrameWork::Bitmaps;
	bitmap_bgra_u8 *bitmap=(bitmap_bgra_u8 *)Handle;
	delete bitmap;
	Handle=NULL;
	frame.Memory=NULL; //pedantic
}

class Dashboard_Framework_Helper : public Dashboard_Framework_Interface
{
	Bitmap_Handle *CreateBGRA(const Bitmap_Frame *sourceUVYV)
	{
		using namespace FrameWork::Bitmaps;
		bitmap_bgra_u8 *bitmap=new bitmap_bgra_u8(sourceUVYV->XRes,sourceUVYV->YRes,sourceUVYV->Stride * 2);
		Bitmap_Handle *handle=new Bitmap_Handle((PBYTE)((*bitmap)()),bitmap->xres(),bitmap->yres(),bitmap->stride());
		handle->Handle=(void *)bitmap;
		return handle;
	}

	void DestroyBGRA(Bitmap_Handle *handle)
	{
		delete handle;
	}

	void UYVY_to_BGRA(const Bitmap_Frame *sourceUVYV,Bitmap_Frame *destBGRA)
	{
		using namespace FrameWork::Bitmaps;
		bitmap_ycbcr_u8 source_bitmap((pixel_ycbcr_u8 *)sourceUVYV->Memory,sourceUVYV->XRes,sourceUVYV->YRes,sourceUVYV->Stride);
		bitmap_bgra_u8 dest_bitmap((pixel_bgra_u8 *)destBGRA->Memory,destBGRA->XRes,destBGRA->YRes,destBGRA->Stride);
		dest_bitmap=source_bitmap;
	}

	void BGRA_to_UYVY(const Bitmap_Frame *sourceBGRA,Bitmap_Frame *destUYVY)
	{
		using namespace FrameWork::Bitmaps;
		bitmap_bgra_u8 source_bitmap((pixel_bgra_u8 *)sourceBGRA->Memory,sourceBGRA->XRes,sourceBGRA->YRes,sourceBGRA->Stride);
		bitmap_ycbcr_u8 dest_bitmap((pixel_ycbcr_u8 *)destUYVY->Memory,destUYVY->XRes,destUYVY->YRes,destUYVY->Stride);
		dest_bitmap=source_bitmap;
	}

	void DrawLineUYVY( Bitmap_Frame *frame, _2Dpoint p1, _2Dpoint p2, const unsigned int col[] )
	{
		using namespace FrameWork::Bitmaps;
		using namespace rasterize;
		pixel_ycbcr_u8 pix;

		int posn0[2];
		int posn1[2];

		pix.m_cb = (unsigned char)col[0];
		pix.m_y0 = (unsigned char)col[1];
		pix.m_cr = (unsigned char)col[2];
		pix.m_y1 = (unsigned char)col[3];

		posn0[0] = p1.h;
		posn0[1] = p1.v;
		posn1[0] = p2.h;
		posn1[1] = p2.v;

		bitmap_ycbcr_u8 dest((pixel_ycbcr_u8 *)frame->Memory, frame->XRes, frame->YRes, frame->Stride);
		line( dest, posn0, posn1, (const pixel_ycbcr_u8)pix );
	}

};

class ProcessingVision : public FrameWork::Outstream_Interface
{
	public:
		ProcessingVision(FrameWork::Outstream_Interface *Preview=NULL) : m_DriverProc(NULL),
			m_PlugIn(NULL),m_Outstream(Preview),m_pPluginControllerInterface(NULL) {}
		void Callback_Initialize(const char *IPAddress,const char *WindowTitle) {if (m_PlugIn) (*m_fpInitialize)(IPAddress,WindowTitle,&m_DashboardHelper);}
		void Callback_Shutdown() {if (m_PlugIn) (*m_fpShutdown)();}
		~ProcessingVision()
		{
			//Note: we can move this earlier if necessary
			Callback_Shutdown();
			FlushPlugin();
		}

		void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_Outstream=Preview;}
		void LoadPlugIn(const wchar_t Plugin[])
		{
			FlushPlugin();  //ensure its not already loaded
			m_DriverProc = NULL;  //this will avoid crashing if others fail
			m_fpInitialize = NULL;
			m_fpShutdown = NULL;
			m_CreatePluginControllerInterface=NULL;
			m_DestroyPluginControllerInterface=NULL;

			m_PlugIn=LoadLibrary(Plugin);

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
		void StartStreaming() {m_IsStreaming=true;}
		void StopStreaming() {m_IsStreaming=false;}

		virtual void process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer,bool isInterlaced,double VideoClock,float AspectRatio)
		{
			if (m_IsStreaming)
			{
				if (m_DriverProc)
				{
					using namespace FrameWork::Bitmaps;
					Bitmap_Frame frame((PBYTE)(*pBuffer)(),pBuffer->xres(),pBuffer->yres(),pBuffer->stride());
					Bitmap_Frame out_frame=*((*m_DriverProc)(&frame));
					bitmap_ycbcr_u8 dest((pixel_ycbcr_u8 *)out_frame.Memory,out_frame.XRes,out_frame.YRes,out_frame.Stride);
					m_Outstream->process_frame(&dest,isInterlaced,VideoClock,AspectRatio);
				}
				else
					m_Outstream->process_frame(pBuffer,isInterlaced,VideoClock,AspectRatio); //just passing through			
			}
		}

		Plugin_Controller_Interface* GetPluginInterface(void) 
		{
			if ((m_pPluginControllerInterface==NULL)&&(m_CreatePluginControllerInterface))
				m_pPluginControllerInterface=(*m_CreatePluginControllerInterface)();
			return m_pPluginControllerInterface;
		}

	private:
		typedef Bitmap_Frame * (*DriverProc_t)(Bitmap_Frame *Frame);
		DriverProc_t m_DriverProc;

		typedef void (*function_Initialize) (const char *IPAddress,const char *WindowTitle,Dashboard_Framework_Helper *DashboardHelper);
		function_Initialize m_fpInitialize;

		typedef void (*function_void) ();
		function_void m_fpShutdown;

		typedef Plugin_Controller_Interface * (*function_create_plugin_controller_interface) ();
		function_create_plugin_controller_interface m_CreatePluginControllerInterface;
		typedef  void (*function_destroy_plugin_controller_interface)(Plugin_Controller_Interface *);
		function_destroy_plugin_controller_interface m_DestroyPluginControllerInterface;
	
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

		HMODULE m_PlugIn;
		FrameWork::Outstream_Interface * m_Outstream; //I'm not checking for NULL so stream must be stopped while pointer is invalid
		bool m_IsStreaming;
		Dashboard_Framework_Helper m_DashboardHelper;
		Plugin_Controller_Interface *m_pPluginControllerInterface;
};


const wchar_t * const cwsz_DefaultSmartFile=L"C:\\WindRiver\\WPILib\\SmartDashboard.jar";
const wchar_t * const cwsz_PlugInFile=L"ProcessingVision.dll";
const wchar_t * const cwsz_ClassName=L"SunAwtFrame";
const wchar_t * const cwsz_WindowName=L"SmartDashboard - ";
std::wstring g_IP_Address=L"none";
std::wstring g_Robot_IP_Address=L"none";

class DDraw_Preview 
{
	public:
		struct DDraw_Preview_Props
		{
			enum WindowType
			{
				eStandAlone,
				eSmartDashboard
			} window_type;
			std::wstring source_name;
			std::wstring IP_Address;
			//If this is non-zero this may embed the port number within the URL... typically this is for RTSP if it needs to be
			//other than 554... like 1180 as specified in the FMS white paper.  The camera setting must reflect the port used as well
			//see options here: system options/network/tcp_ip/advanced
			LONG Port;
			FrameGrabber::ReaderFormat ReaderFormat;
			std::wstring smart_file;			//startup this parent window app
			std::wstring ClassName,WindowName;  //find this window
			std::wstring plugin_file;
			std::wstring controls_plugin_file;
			std::wstring aux_startup_file,aux_startup_file_Args;	//startup an aux... (no window finding for this)
			LONG XRes,YRes,XPos,YPos;
			//provide an alternate way to determine window coordinates
			// -1 for x and y res will revert to hard coded defaults (this keeps tweaking inside the cpp file)
			void SetDefaults(LONG XRes_=-1,LONG YRes_=-1,float XPos_=0.5f,float YPos_=0.5f);
		};
		typedef DDraw_Preview_Props::WindowType WindowType;

		DDraw_Preview(const DDraw_Preview_Props &props);
		virtual ~DDraw_Preview();
		//returns true to quit
		bool CommandLineInterface();
		void RunApp();
		void SetInitRecord(bool RecordFrames) {m_InitRecord=RecordFrames;}
		bool GetInitRecord() const {return m_InitRecord;}
		const char *GetRecordPath() const {return m_RecordPath.c_str();}
		void SetRecordPath(const char *Path) {m_RecordPath=Path;}
		void SignalQuit() 
		{ 
			m_Terminate.set(); 
			Callback_Shutdown();
		}

		Preview *GetPreview() {return m_DD_StreamOut;}
		void Reset_DPC();  //This launches reset on a deferred procedure call
		void StopStreaming();
		void StartStreaming();

		void Callback_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset) {m_Controls_PlugIn.Callback_AddMenuItems(hPopupMenu,StartingOffset);}
		//Note: it is imperative to enter client code on the GUI thread
		void Callback_On_Selection(int selection,HWND pParent) {m_Controls_PlugIn.Callback_On_Selection(selection,pParent);}
		double GetAspectRatio() const {return m_DD_StreamOut->GetAspectRatio();}
		Dashboard_Controller_Interface *GetDashboard_Controller_Interface() {return m_FrameGrabber.GetDashboard_Controller_Interface();}
	protected:
		virtual void CloseResources();
		virtual void OpenResources();
	private:
		struct Controls_Plugin
		{
			void LoadPlugIn(const wchar_t Plugin[]);
			void FlushPlugin();

			HMODULE m_PlugIn;
			typedef void (*function_AddMenuItems) (HMENU hPopupMenu,size_t StartingOffset);
			function_AddMenuItems m_fpAddMenuItems;
			typedef void (*function_On_Selection) (int selection,HWND pParent);
			function_On_Selection m_fpOn_Selection;
			typedef void (*function_Initialize) (Dashboard_Controller_Interface *controller,DLGPROC gWinProc);
			function_Initialize m_fpInitialize;
			typedef void (*function_initiailze_plugin) (Plugin_Controller_Interface *plugin);
			function_initiailze_plugin m_fpInitializePlugin;
			typedef void (*function_void) ();
			function_void m_fpShutdown;
			typedef void (*function_StartedStreaming) (HWND pParent);
			function_StartedStreaming m_fpStartedStreaming;

			void Callback_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset) {if (m_PlugIn) (*m_fpAddMenuItems)(hPopupMenu,StartingOffset);}
			void Callback_On_Selection(int selection,HWND pParent) {if (m_PlugIn) (*m_fpOn_Selection)(selection,pParent);}
			void Callback_Initialize(Dashboard_Controller_Interface *controller,DLGPROC gWinProc) {if (m_PlugIn) (*m_fpInitialize)(controller,gWinProc);}
			void Callback_StartedStreaming(HWND pParent) {if (m_PlugIn) (*m_fpStartedStreaming)(pParent);}
			void Callback_Shutdown() {if (m_PlugIn) (*m_fpShutdown)();}

		} m_Controls_PlugIn;
		void Callback_Initialize(Dashboard_Controller_Interface *controller,DLGPROC gWinProc) {m_Controls_PlugIn.Callback_Initialize(controller,gWinProc);}
		void Callback_StartedStreaming(HWND pParent) {m_Controls_PlugIn.Callback_StartedStreaming(pParent);}
		void Callback_Shutdown() {m_Controls_PlugIn.Callback_Shutdown();}

		void Reset();
		void DisplayHelp();
		void SetDefaults(LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		void UpdateDefaultsFromWindowPlacement();
		void Deferred_AttachToParent();
		void Deferred_AttachToParent_DPC();
		FrameWork::event m_Terminate;
		FrameWork::Work::thread m_thread;  //For DPC support
		//This is needed to ensure open is finished before a reset operation when using Deferred_AttachToParent_DPC
		FrameWork::critical_section m_BlockOpenCloseResources;  

		HWND m_ParentHwnd;
		Window *m_Window;
		Preview *m_DD_StreamOut;

		DDraw_Preview_Props m_Props;
		RECT m_DefaultWindow;  //left=xRes top=yRes right=xPos bottom=YPos
		std::string m_RecordPath;

		#if 0
		FrameGrabber_TestPattern m_FrameGrabber;
		#else
		FrameGrabber m_FrameGrabber;
		#endif

		ProcessingVision m_ProcessingVision;

		bool m_IsPopup_LastOpenedState;  //This is only written at the point when window is created
		bool m_IsStreaming;
		bool m_InitRecord;
};

using namespace std;

  /*******************************************************************************************************/
 /*											DDraw_Window												*/
/*******************************************************************************************************/

WINDOWPLACEMENT g_WindowInfo;
bool g_IsPopup=true;
bool g_IsSmartDashboardStarted=false;  //only run shell execute one time

void HighlightWindow( HWND hwnd, BOOL fDraw )
{
	HWND parent=GetParent(hwnd);
	assert(parent);
	if (!parent) return;
	const LONG DINV=3;
	HDC hdc;
	RECT rc,parent_rect;
	BOOL bBorderOn;
	bBorderOn = fDraw;

	if (hwnd == NULL || !IsWindow(hwnd))
		return;

	hdc = ::GetWindowDC(parent);
	::GetWindowRect(parent, &parent_rect);
	::GetWindowRect(hwnd, &rc);
	::OffsetRect(&rc, -parent_rect.left, -parent_rect.top);
	rc.left-=DINV<<1;
	rc.top-=DINV<<1;
	rc.right+=DINV<<1;
	rc.bottom+=DINV<<1;

	if (!IsRectEmpty(&rc))
	{
		PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, DINV,  DSTINVERT);
		PatBlt(hdc, rc.left, rc.bottom - DINV, DINV,
			-(rc.bottom - rc.top - 2 * DINV), DSTINVERT);
		PatBlt(hdc, rc.right - DINV, rc.top + DINV, DINV,
			rc.bottom - rc.top - 2 * DINV, DSTINVERT);
		PatBlt(hdc, rc.right, rc.bottom - DINV, -(rc.right - rc.left),
			DINV, DSTINVERT);
	}

	::ReleaseDC(parent, hdc);
} 

class DDraw_Window : public Window
{
	public:
		DDraw_Window(DDraw_Preview *pParent, HWND HWND_Parent=NULL , const bool IsPopup=true , 
			const wchar_t *pWindowName=L"Window" , const RECT *pWindowPosition=NULL ) : Window(HWND_Parent,IsPopup,pWindowName,pWindowPosition), 
			m_pParent(pParent),m_AspectRatio(4.0/3.0),m_Editable(false),m_IsDragging(false),m_Rotate90(false)
		{
		}
		~DDraw_Window()
		{
			//if (!m_PopupWindow)
			//	KillTimer(m_hWnd,1);
		}
	protected:
		enum MenuSelection
		{
			eMenu_NoSelection,
			eMenu_Floating=100,	//typically win32 starts these at 100  (not sure why, but it is probably optional)
			eMenu_Dockable,
			eMenu_Editable,
			eMenu_LockAuto,
			eMenu_Lock4x3,
			eMenu_Lock16x9,
			eMenu_Stretch,
			eMenu_Show600i,
			eMenu_Show480i,
			eMenu_Show300i,
			eMenu_Show240i,
			eMenu_Rotate90,
			eMenu_NoEntries
		};

		virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
		{
			long ret=0;
			switch (message)
			{
			case WM_LBUTTONDOWN:
				if (m_Editable)
				{
					WINDOWPLACEMENT preview_pos;
					GetWindowPlacement(*this,&preview_pos);
					struct tagPOINT mouseloc;
					GetCursorPos(&mouseloc);
					m_XGrab=preview_pos.rcNormalPosition.left-mouseloc.x;
					m_YGrab=preview_pos.rcNormalPosition.top-mouseloc.y;
					m_IsDragging=true;
					HighlightWindow(*this,false);
				}
				break;
			case WM_LBUTTONUP:
				if (m_IsDragging)
					HighlightWindow(*this,true);

				m_IsDragging=false;
				break;
			case WM_MOUSEMOVE:
				if (m_IsDragging)
				{
					struct tagPOINT mouseloc;
					GetCursorPos(&mouseloc);
					SetWindowPos(*this,NULL,mouseloc.x+m_XGrab,mouseloc.y+m_YGrab,0,0,SWP_NOSIZE|SWP_NOZORDER);
				}
				break;
			//This sizing was translated from here:
			//http://cboard.cprogramming.com/windows-programming/98998-uniform-resize-window.html
			//It works well...the only discrepancy is the the aspect ratio is height/width form (not the end of the world)
			//  [11/21/2012 James]
			case WM_SIZING:
				{
					double aspectRatio=(m_AspectRatio!=0.0)?1.0 / m_AspectRatio : 0.0;
					WPARAM &wParam=w;
					LPARAM &lParam=l;

					if (m_AspectRatio!=0.0)
					{
						RECT sz;    
						int w, h;
						// Copy over the new size that was determined by windows        
						memcpy(&sz, (const void *) lParam, sizeof(RECT));         
						// Calculate the width and height of the window        
						w = sz.right - sz.left;        
						h = sz.bottom - sz.top;         
						switch (wParam)        
						{            
						case WMSZ_LEFT:            
						case WMSZ_RIGHT:                
							// Modify the Height of the window                
							sz.bottom = LONG(w * aspectRatio) + sz.top;                
							break;             
						case WMSZ_TOP:            
						case WMSZ_BOTTOM:                
							// Modify the Width of the window                
							sz.right = LONG(h * (1 / aspectRatio)) + sz.left;                
							break;             
						case WMSZ_TOPRIGHT:            
						case WMSZ_TOPLEFT:            
						case WMSZ_BOTTOMRIGHT:            
						case WMSZ_BOTTOMLEFT:                
							// Adjust the width and height of the window to match aspect ratio                
							if (double(h) / double(w) > aspectRatio)
								w = int(double(h) / aspectRatio);                
							else
								h = int(double(w) * aspectRatio);

							// Adjust Height                
							if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)                
								sz.top = sz.bottom - h;                
							else
								sz.bottom = sz.top + h;

							// Adjust Width                
							if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
								sz.left = sz.right - w;
							else
								sz.right = sz.left + w;
							break;        
						}         
						// Copy back the size of the window        
						memcpy((void *) lParam, &sz, sizeof(RECT));
					}
				}
				break;
			case WM_RBUTTONDOWN:
				{
					HMENU hPopupMenu = CreatePopupMenu();
					if (!g_IsPopup)
						InsertMenu(hPopupMenu, -1, ( m_Editable?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Editable, L"Editable");
					InsertMenu(hPopupMenu, -1, ( g_IsPopup?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Floating, L"Floating");
					InsertMenu(hPopupMenu, -1, (!g_IsPopup?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Dockable, L"Dockable");
					if (g_IsPopup)
					{
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==(double)m_pParent->GetAspectRatio())?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_LockAuto, L"Lock file aspect");
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==4.0/3.0)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Lock4x3, L"Lock 4x3 aspect");
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==16.0/9.0)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Lock16x9, L"Lock 16x9 aspect");
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==0.0)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Stretch, L"Stretch");
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_Show600i, L"Size 600i");
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_Show480i, L"Size 480i");
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_Show300i, L"Size 300i");
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_Show240i, L"Size 240i");
						InsertMenu(hPopupMenu, -1, ((m_Rotate90)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Rotate90, L"Rotate 90");
					}
					//go to plug-in for addition items
					m_pParent->Callback_AddMenuItems(hPopupMenu,eMenu_NoEntries);
					SetForegroundWindow(window);

					int XPos=0;
					int YPos=0;
					if (!g_IsPopup)
					{
						WINDOWPLACEMENT parent;
						GetWindowPlacement(GetParent(*this),&parent);
						XPos=parent.rcNormalPosition.left;
						YPos=parent.rcNormalPosition.top;
					}
					GetWindowPlacement(*this,&g_WindowInfo);
					XPos+=(g_WindowInfo.rcNormalPosition.left+g_WindowInfo.rcNormalPosition.right) >> 1;
					YPos+=(g_WindowInfo.rcNormalPosition.top+g_WindowInfo.rcNormalPosition.bottom) >> 1;
					MenuSelection selection=(MenuSelection)TrackPopupMenuEx(hPopupMenu, TPM_CENTERALIGN | TPM_RETURNCMD, XPos, YPos	, window, NULL);
					if (selection!=eMenu_NoSelection)
					{
						FrameWork::DebugOutput("Selection=%d\n",selection);
						switch (selection)
						{
							case eMenu_Editable:
								m_Editable=!m_Editable;
								HighlightWindow(*this,m_Editable);
								break;
							case eMenu_Floating:
								if (!g_IsPopup)		//only commit if it was changed
								{
									g_IsPopup=true;
									m_pParent->Reset_DPC();
								}
								break;
							case eMenu_Dockable:
								if (g_IsPopup)		//only commit if it was changed
								{
									g_IsPopup=false;
									m_pParent->Reset_DPC();
								}
								break;
							case eMenu_LockAuto:
								m_AspectRatio=m_pParent->GetAspectRatio();
								break;
							case eMenu_Lock4x3:
								m_AspectRatio=!m_Rotate90 ? 4.0/3.0 : 3.0/4.0;
								break;
							case eMenu_Lock16x9:
								m_AspectRatio=!m_Rotate90? 16.0/9.0 : 9.0/16.0;
								break;
							case eMenu_Stretch:
								m_AspectRatio=0.0;
								break;
							case eMenu_Show600i:
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									const double YRes=m_Rotate90 ? 800.0 : 600.0;
									SetWindowPos(*this,NULL,0,0,(int)(m_AspectRatio*YRes),(int)YRes,SWP_NOMOVE|SWP_NOZORDER);
								}
								break;
							case eMenu_Show480i:
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									const double YRes=m_Rotate90 ? 640.0 : 480.0;
									SetWindowPos(*this,NULL,0,0,(int)(m_AspectRatio*YRes),(int)YRes,SWP_NOMOVE|SWP_NOZORDER);
								}
								break;
							case eMenu_Show300i:
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									const double YRes=m_Rotate90 ? 400.0 : 300.0;
									SetWindowPos(*this,NULL,0,0,(int)(m_AspectRatio*YRes),(int)YRes,SWP_NOMOVE|SWP_NOZORDER);
								}
								break;
							case eMenu_Show240i:
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									const double YRes=m_Rotate90 ? 320.0 : 240.0;
									SetWindowPos(*this,NULL,0,0,(int)(m_AspectRatio*YRes),(int)YRes,SWP_NOMOVE|SWP_NOZORDER);
								}
								break;
							case eMenu_Rotate90:
								m_Rotate90=!m_Rotate90;
								m_AspectRatio=1.0/m_AspectRatio; //get reciprocal of whatever it currently is
								break;
							default:
								{
									int entry=(int)selection - eMenu_NoEntries;
									assert (entry>=0);
									m_pParent->Callback_On_Selection(entry,*this);
								}
						}
					}
				}
				break;
			case WM_CLOSE:
			case WM_DESTROY:
				m_pParent->SignalQuit();
				break;
			default:
				ret=DefWindowProc(window,message,w,l);
			}
			return ret;
		}
		virtual void InitializeWindow() 
		{
			//if (!m_PopupWindow)
			//	SetTimer(m_hWnd,1,1000,NULL);
		}

	private:
		DDraw_Preview * const m_pParent;
		LONG m_XGrab,m_YGrab;
		//if this is 0 it does not lock
		double m_AspectRatio;
		bool m_Editable,m_IsDragging;
		bool m_Rotate90;
};


using namespace FrameWork;

  /*******************************************************************************************************/
 /*									DDraw_Preview::DDraw_Preview_Props									*/
/*******************************************************************************************************/

void DDraw_Preview::DDraw_Preview_Props::SetDefaults(LONG XRes_,LONG YRes_,float XPos_,float YPos_)
{
	XRes=(XRes_==-1)?480:XRes_;
	YRes=(YRes_==-1)?270:YRes_;

	XPos=(LONG)(((float)(::GetSystemMetrics(SM_CXSCREEN)-XRes))*XPos_);
	YPos=(LONG)(((float)(::GetSystemMetrics(SM_CYSCREEN)-YRes))*YPos_);
}

  /*******************************************************************************************************/
 /*											DDraw_Preview												*/
/*******************************************************************************************************/



//http://stackoverflow.com/questions/3922488/use-wildcards-with-findwindow-api-call-with-mfc
//Added wildcard find window implementation to ensure all EuCon 3 versions dialog windows are found
//This code is slightly modified to our coding conventions

struct FindWindowData 
{
	FindWindowData(const wchar_t *windowTitle) : WindowTitle(windowTitle) , ResultHandle(0)
	{}
	std::wstring WindowTitle;
	HWND ResultHandle;
};

BOOL CALLBACK FindWindowImpl( HWND hWnd, LPARAM lParam ) 
{
	FindWindowData * p = (FindWindowData *) lParam ;
	if( !p ) 
	{
		// Finish enumerating we received an invalid parameter
		return FALSE;
	}

	int length = GetWindowTextLength( hWnd ) + 1;
	if( length > 0 ) 
	{
		std::vector<wchar_t> buffer( std::size_t( length ), 0 );      
		if( GetWindowText( hWnd, &buffer[0], length ) ) 
		{
			// Comparing the string - If you want to add some features you can do it here
			if( _wcsnicmp( &buffer[0], p->WindowTitle.c_str(), std::min( buffer.size(), p->WindowTitle.size() )  ) == 0 ) 
			{
				p->ResultHandle = hWnd;
				// Finish enumerating we found what we need
				return FALSE;
			}
		}
	}
	// Continue enumerating
	return TRUE;
}

// Returns the window handle when found if it returns 0 GetLastError() will return more information
HWND FindWindowStart( const wchar_t *windowTitle ) 
{

	if( !windowTitle ) 
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	FindWindowData data( windowTitle );
	if( !EnumWindows( FindWindowImpl, PtrToLong(&data) ) && data.ResultHandle != 0 ) 
	{
		SetLastError( ERROR_SUCCESS );
		return data.ResultHandle;
	}

	// Return ERROR_FILE_NOT_FOUND in GetLastError
	SetLastError( ERROR_FILE_NOT_FOUND );
	return 0;
}




void DDraw_Preview::Controls_Plugin::FlushPlugin()
{
	if (m_PlugIn)
	{
		FreeLibrary(m_PlugIn);
		m_PlugIn = NULL;
	}
}

void DDraw_Preview::Controls_Plugin::LoadPlugIn(const wchar_t Plugin[])
{
	FlushPlugin();  //ensure its not already loaded
	m_PlugIn=LoadLibrary(Plugin);
	if (m_PlugIn)
	{
		//Let's ensure they all work to avoid having to checking for NULL in each in implementation
		try
		{
			m_fpAddMenuItems=(function_AddMenuItems) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_AddMenuItems");
			if (!m_fpAddMenuItems) throw 0;
			m_fpOn_Selection=(function_On_Selection) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_On_Selection");
			if (!m_fpOn_Selection) throw 1;
			m_fpInitialize=(function_Initialize) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_Initialize");
			if (!m_fpInitialize) throw 2;
			m_fpShutdown=(function_void) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_Shutdown");
			if (!m_fpShutdown) throw 3;
			m_fpStartedStreaming=(function_StartedStreaming) GetProcAddress(m_PlugIn,"Callback_SmartCppDashboard_StartedStreaming");
			if (!m_fpStartedStreaming) throw 4;
			m_fpInitializePlugin=(function_initiailze_plugin) GetProcAddress(m_PlugIn, "CallBack_SmartCppDashboard_Initialize_Plugin");
			if (!m_fpInitializePlugin) throw 5;
		}
		catch (int ErrorCode)
		{
			DebugOutput("Controls Plugin failed error code=%d",ErrorCode);
			FlushPlugin();
		}
	}
}


DDraw_Preview::DDraw_Preview(const DDraw_Preview_Props &props) : m_Window(NULL),m_ParentHwnd(NULL),m_DD_StreamOut(NULL),
	m_FrameGrabber(NULL,props.IP_Address.c_str(),props.ReaderFormat,props.Port),m_ProcessingVision(NULL),m_IsStreaming(false),m_InitRecord(false)
{
	m_Controls_PlugIn.LoadPlugIn(props.controls_plugin_file.c_str());
	Dashboard_Controller_Interface *l_Dashboard_Interface=m_FrameGrabber.GetDashboard_Controller_Interface();
	Callback_Initialize(l_Dashboard_Interface,m_Window->GetDispatcherBase());
	m_Props=props;
	SetDefaults(props.XRes,props.YRes,props.XPos,props.YPos);
}

void DDraw_Preview::UpdateDefaultsFromWindowPlacement()
{
	//If we never associated the parent window on startup because we were a pop-up, and invoke a docking operation we'll need to associate it
	//now... if its there we can successfully dock
	if ((!g_IsPopup)&&(!m_ParentHwnd))
		m_ParentHwnd=FindWindowStart(m_Props.WindowName.c_str());

	//If we still have our parent window or if we have always been a pop-up
	if ((m_ParentHwnd) || (g_IsPopup && m_IsPopup_LastOpenedState))
	{
		//we'll want to to set the defaults to the last set position
		LONG XPos=g_WindowInfo.rcNormalPosition.left;
		LONG YPos=g_WindowInfo.rcNormalPosition.top;
		LONG XRes=g_WindowInfo.rcNormalPosition.right - XPos;
		LONG YRes=g_WindowInfo.rcNormalPosition.bottom - YPos;
		//translate from child to pop-up or vise versa

		if (g_IsPopup!=m_IsPopup_LastOpenedState)
		{
			RECT Parent;
			GetWindowRect(m_ParentHwnd,&Parent);
			if (g_IsPopup)  //child to pop-up
				XPos+=Parent.left,YPos+=Parent.top;
			else
				XPos-=Parent.left,YPos-=Parent.top;
		}
		//Pedantic but we should keep this updated
		SetDefaults(XRes,YRes,XPos,YPos);
	}
}

void DDraw_Preview::StopStreaming()
{
	if (m_IsStreaming)
	{
		if (m_FrameGrabber.GetDashboard_Controller_Interface())
		{
			//Make note of last state for client bls... as early as possible
			m_InitRecord=m_FrameGrabber.GetDashboard_Controller_Interface()->GetRecordState();
			m_RecordPath=m_FrameGrabber.GetDashboard_Controller_Interface()->GetRecordPath();
		}
		//before closing the resources ensure the upstream is not streaming to us
		m_FrameGrabber.StopStreaming();
		m_ProcessingVision.StopStreaming();
		m_FrameGrabber.SetOutstream_Interface(NULL);  //pedantic
		m_ProcessingVision.SetOutstream_Interface(NULL);
		m_IsStreaming=false;
	}
}

void DDraw_Preview::CloseResources()
{
	auto_lock FunctionBlock(m_BlockOpenCloseResources);
	{
		//See if the filename has changed... so that we can save it on exit
		Dashboard_Controller_Interface *dci=m_FrameGrabber.GetDashboard_Controller_Interface();
		if (dci)
		{
			std::wstring NewName;
			dci->GetFileName(NewName);
			if (NewName[0]!=0)
				g_IP_Address=NewName;
		}
	}
	StopStreaming();
	delete m_DD_StreamOut;
	m_DD_StreamOut=NULL;
	if (m_Window)
	{
		GetWindowPlacement(*m_Window,&g_WindowInfo);
		UpdateDefaultsFromWindowPlacement();
		delete m_Window;
		m_Window=NULL;
	}
	if (m_ParentHwnd)
	{
		//TODO determine if popup can listen to WM_DESTROY from parent (and Nullify m_ParentHwnd)
		//or use timer (I think I may just leave this alone)
		#if 0
		if (g_IsPopup)
			PostMessage(m_ParentHwnd,WM_CLOSE,0,0);
		#endif
		m_ParentHwnd=NULL;
	}

	m_ProcessingVision.Callback_Shutdown();
}

DDraw_Preview::~DDraw_Preview()
{
	CloseResources();
	m_Controls_PlugIn.FlushPlugin();
}


void DDraw_Preview::StartStreaming()
{
	if (!m_IsStreaming && m_DD_StreamOut)
	{
		m_IsStreaming=true;
		#if 0
		m_FrameGrabber.SetOutstream_Interface(m_DD_StreamOut);
		#else
		m_ProcessingVision.SetOutstream_Interface(m_DD_StreamOut);
		m_FrameGrabber.SetOutstream_Interface(&m_ProcessingVision);
		#endif
		//Now to start the frame grabber
		m_FrameGrabber.StartStreaming();
		m_ProcessingVision.StartStreaming();
		Callback_StartedStreaming(*m_Window);
		if (m_FrameGrabber.GetDashboard_Controller_Interface())
		{
			 //transfer the init state as late as possible
			if (m_RecordPath.c_str()[0]!=0)
				m_FrameGrabber.GetDashboard_Controller_Interface()->SetRecordPath(m_RecordPath.c_str());
			m_FrameGrabber.GetDashboard_Controller_Interface()->Record(m_InitRecord);
		}
	}

}

void DDraw_Preview::Deferred_AttachToParent()
{
	HWND ParentHwnd=NULL;
	do 
	{
		Sleep(2000);  //keep a nice big interval since this is an idle check
		ParentHwnd=FindWindowStart(m_Props.WindowName.c_str());
	} while ((ParentHwnd==NULL)&&(m_IsStreaming)); //This may take a while on cold start
	if ((ParentHwnd)&&(m_IsStreaming))
	{
		//Force a match... when it lost its parent it must not save a pop-up offset flagged as a pop-up these are relative offsets to
		//the parent window... so we can keep this this way by fooling the logic to thinking it was child all along
		m_IsPopup_LastOpenedState=false;
		//This is pretty much the identical implementation as eMenu_Dockable
		g_IsPopup=false;
		Reset();  //since we are already in a DPC we can call this directly
	}
}

void DDraw_Preview::Deferred_AttachToParent_DPC()
{
	cpp::threadcall_ex( do_not_wait, m_thread, this, &DDraw_Preview::Deferred_AttachToParent);
}



void DDraw_Preview::OpenResources()
{
	auto_lock FunctionBlock(m_BlockOpenCloseResources);
	typedef DDraw_Preview::DDraw_Preview_Props PrevProps;
	CloseResources(); //just ensure all resources are closed

	if( !m_Props.plugin_file.empty() )
	{
		m_ProcessingVision.LoadPlugIn(m_Props.plugin_file.c_str());
		{
			std::string IpToUse="localhost";
			if (g_Robot_IP_Address.c_str()[0]!=0)
			{
				wchar2char(g_Robot_IP_Address.c_str());
				IpToUse=wchar2char_pchar;
			}
			std::string WindowTitle="default";
			{
				wchar2char(m_Props.source_name.c_str());
				WindowTitle=wchar2char_pchar;
			}
			m_ProcessingVision.Callback_Initialize(IpToUse.c_str(),WindowTitle.c_str());
			m_Controls_PlugIn.m_fpInitializePlugin(m_ProcessingVision.GetPluginInterface());
		}
	}
	LONG XRes=m_DefaultWindow.left, YRes=m_DefaultWindow.top, XPos=m_DefaultWindow.right, YPos=m_DefaultWindow.bottom;
	const wchar_t *source_name=m_Props.source_name.c_str();

	HWND hWnd_ForDDraw=NULL;
	HWND ParentHwnd=NULL;
	bool IsPopup=g_IsPopup;
	bool IsSmartDashboardStarted=g_IsSmartDashboardStarted;
	if ((m_Props.smart_file.c_str()[0]!=0)&&(m_Props.window_type!=PrevProps::eStandAlone))
	{
		if (!g_IsSmartDashboardStarted)
		{
			//Sanity check... see if the window already exists from a previous session:
			#if 0
			if (m_Props.WindowName.c_str()[0]!=0)
				ParentHwnd=FindWindow(m_Props.ClassName.c_str(),m_Props.WindowName.c_str());
			#else
			if (m_Props.WindowName.c_str()[0]!=0)
				ParentHwnd=FindWindowStart(m_Props.WindowName.c_str());
			#endif

			//No window found (typical case) launch one
			if (!ParentHwnd)
			{
				std::vector<std::string> Args;
				wchar2char(m_Props.smart_file.c_str());
				split_arguments(std::string(wchar2char_pchar),Args);

				LPTSTR szCmdline;
				{
					char2wchar(Args[0].c_str());
					szCmdline= _tcsdup(char2wchar_pwchar);
				}
				std::wstring paramaters;
				for (size_t i=1;i<Args.size();i++)
				{
					char2wchar(Args[i].c_str());
					paramaters+=char2wchar_pwchar;
					paramaters+=L" ";
				}
				//Note:
				//The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications. It is not a true HINSTANCE, 
				//however. The only thing that can be done with the returned HINSTANCE is to cast it to an int and compare it with the value 32 or one 
				//of the error codes below.
				HINSTANCE test=ShellExecute(NULL,L"open",szCmdline,paramaters.c_str(),NULL,SW_SHOWNORMAL);
			}
			IsSmartDashboardStarted=true;
			//There was a sleep here, but we can offload the attach to a DPC
		}
	}

	bool Enable_Deferred_AttachToParent=false;
	if ((!g_IsPopup)&&(m_Props.WindowName.c_str()[0]!=0)&&(m_Props.window_type!=PrevProps::eStandAlone))
	{
		#if 0
		//Give this some time to open
		size_t TimeOut=0;
		do 
		{
			Sleep(100);
			#if 0
			//SmartDashboard - /SunAwtFrame
			//ParentHwnd=FindWindow(L"SunAwtFrame",L"SmartDashboard - ");
			ParentHwnd=FindWindow(m_Props.ClassName.c_str(),m_Props.WindowName.c_str());
			#else
			ParentHwnd=FindWindowStart(m_Props.WindowName.c_str());
			#endif
		} while ((ParentHwnd==NULL)&&(TimeOut++<50)); //This may take a while on cold start
		m_ParentHwnd=ParentHwnd;
		#else
		if (!ParentHwnd)
		{
			//See if we already attached to it from a previous session
			ParentHwnd=FindWindowStart(m_Props.WindowName.c_str());
			if (!ParentHwnd)
				Enable_Deferred_AttachToParent=true;
			else
				m_ParentHwnd=ParentHwnd;
		}
		#endif
	}

	//If we don't have a parent window then we must be a Popup
	if (!ParentHwnd)
		g_IsPopup=IsPopup=true;

	{
		assert (!m_Window);
		//apparently there is a racing condition where the window needs a thread to set its handle
		size_t TimeOut=0;
		LONG X=XPos;
		LONG Y=YPos;
		RECT lWindowPosition = {  X,Y,X+XRes,Y+YRes};
		//Note: A child option can only be presented if we have the parent window
		assert(IsPopup || ParentHwnd);
		m_IsPopup_LastOpenedState=IsPopup;  //this is the only place this is written
		m_Window=new DDraw_Window(this,ParentHwnd,IsPopup,source_name,&lWindowPosition);
		while ((!(HWND)*m_Window)&&(TimeOut++<100))
			Sleep(10);
		assert((HWND)*m_Window);
		hWnd_ForDDraw=(HWND)*m_Window;
	}

	if (Enable_Deferred_AttachToParent)
		Deferred_AttachToParent_DPC();

	if (hWnd_ForDDraw)
	{
		assert (!m_DD_StreamOut);
		m_DD_StreamOut=new Preview(hWnd_ForDDraw);
		if (!m_DD_StreamOut->Get_IsError())
		{
			m_DD_StreamOut->StartStreaming();
		}
		else
			printf("DDraw_Preview::OpenResources Error detected in setting up DDraw environment \n");
	}
	else
		printf("No HWnd for DDraw\n");
	StartStreaming();
	//see if there is another file to launch
	if ((!g_IsSmartDashboardStarted)&& (m_Props.aux_startup_file.c_str()[0]!=0))
	{
		const wchar_t *Args=m_Props.aux_startup_file_Args.c_str();
		if (Args[0]==0)
			Args=NULL;
		HINSTANCE test=ShellExecute(NULL,L"open",m_Props.aux_startup_file.c_str(),Args,NULL,SW_SHOWNORMAL);
		DebugOutput("Launching %ls, result=%x",m_Props.aux_startup_file.c_str(),test);
	}

	g_IsSmartDashboardStarted=IsSmartDashboardStarted;
}

void DDraw_Preview::Reset()
{
	CloseResources();
	OpenResources();
}

void DDraw_Preview::Reset_DPC()
{
	//This has to be a deferred procedure call because the primitive callback must complete!
	using namespace FrameWork;
	cpp::threadcall_ex( do_not_wait, m_thread, this, &DDraw_Preview::Reset);
}

void DDraw_Preview::SetDefaults(LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
	//Note: we use the rect as such
	//left=xRes top=yRes right=xPos bottom=YPos
	m_DefaultWindow.left =XRes,
	m_DefaultWindow.top=YRes,
	m_DefaultWindow.right=XPos,
	m_DefaultWindow.bottom=YPos;
}

void DDraw_Preview::RunApp()
{
	OpenResources();
	m_Terminate.wait();
	CloseResources();
}

void AssignInput(wstring &output,const char *input)
{
	if (strlen(input) == 0)
	{	// can't do anything with this.
		DebugOutput("Input arg for %s is empty\n", output.c_str());
		output=L"";
		return;
	}
	std::vector<std::string> Args;
	split_arguments(std::string(input),Args);
	//just use the first argument found
	char2wchar(Args[0].c_str());
	output=char2wchar_pwchar;
	if (wcscmp(output.c_str(),L"none")==0)
		output=L"";
}

void AssignOutput(string &output,const wchar_t *input)
{
	wchar2char(input);
	output=wchar2char_pchar;
	if (output[0]==0)
		output="none";
}

class DDraw_Preview_Properties
{
public:
	struct AncillaryProps
	{
		std::wstring Robot_IP_Address;
		std::wstring StreamProfile;
		std::wstring RecordPath;
		bool RecordFrames;
		bool IsPopup;
	};
private:
	DDraw_Preview::DDraw_Preview_Props m_PreviewProps;
	AncillaryProps m_AncillaryProps;
	std::string m_FileName;
	bool m_UsingLUA;
	bool m_SaveOnExit;
private:
	void AssignWstring(Scripting::Script& script,const char *Parameter,const char *DefaultValue,std::wstring &Dest)
	{
		const char* err=NULL;
		std::string s_value;
		err=script.GetField(Parameter, &s_value, NULL, NULL );
		if (!err)
		{
			char2wchar(s_value.c_str());
			Dest=char2wchar_pwchar;
		}
		else
		{
			char2wchar(DefaultValue);
			Dest=char2wchar_pwchar;
		}
	}

	const char *SetUpGlobalTable(Scripting::Script& script)
	{
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		const char* err;
		//props.ShipType=Ship_Props::eDefault;
		err = script.GetGlobalTable("Dashboard");
		assert (!err);
		return err;
	}

	//This is common for all readers once the StreamProfile is set
	void InitReaderFormatProp()
	{
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		const AncillaryProps &props_anc=m_AncillaryProps;

		props.ReaderFormat=FrameGrabber::eFFMPeg_Reader;
		if  ((wcsicmp(props_anc.StreamProfile.c_str(),L"mjpg")==0) || (wcsicmp(props_anc.StreamProfile.c_str(),L"mjpeg")==0))
			props.ReaderFormat=FrameGrabber::eHttpReader;
		else if  ((wcsicmp(props_anc.StreamProfile.c_str(),L"mjpg2")==0) || (wcsicmp(props_anc.StreamProfile.c_str(),L"mjpeg2")==0))
			props.ReaderFormat=FrameGrabber::eHttpReader2;
	}

	struct IgnoreList
	{
		bool Ignore_URL;
		bool Ignore_Position;
		bool Ignore_Popup;
		bool Ignore_Record;
	};
	void Load_PersistentData(Scripting::Script& script, const IgnoreList &ignore_list)
	{
		const char* err=NULL;
		std::string sTest;
		double dTest;
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		//TODO add ignore capabilities
		if (!ignore_list.Ignore_URL)
			AssignWstring(script,"url","Black",props.IP_Address);
		if (!ignore_list.Ignore_Position)
		{
			err = script.GetField("left",NULL,NULL,&dTest);
			props.XPos=err?20:(LONG)dTest;
			err = script.GetField("top",NULL,NULL,&dTest);
			props.YPos=err?10:(LONG)dTest;
			err = script.GetField("right",NULL,NULL,&dTest);
			props.XRes=err?320:((LONG)dTest)-props.XPos;
			err = script.GetField("bottom",NULL,NULL,&dTest);
			props.YRes=err?240:((LONG)dTest)-props.YPos;
		}
		if (!ignore_list.Ignore_Popup)
		{
			err = script.GetField("is_popup",&sTest,NULL,NULL);
			if (!err)
			{
				if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
					m_AncillaryProps.IsPopup=false;
				else
					m_AncillaryProps.IsPopup=true;
			}
			else
				m_AncillaryProps.IsPopup=true;
		}
		if (!ignore_list.Ignore_Record)
		{
			err = script.GetField("record_frames",&sTest,NULL,NULL);
			if (!err)
			{
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					m_AncillaryProps.RecordFrames=true;
				else
					m_AncillaryProps.RecordFrames=false;
			}
			else
				m_AncillaryProps.RecordFrames=false;

			//RecordPath= D:/media/Robot_Capture/
			AssignWstring(script,"record_path","D:/media/Robot_Capture/",m_AncillaryProps.RecordPath);
		}
	}

	virtual void LoadFromScript(Scripting::Script& script)
	{
		const char* err=NULL;
		{
			double version;
			err=script.GetField("version", NULL, NULL, &version);
			if (!err)
				printf ("Version=%.2f\n",version);
		}

		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		props.window_type=DDraw_Preview::DDraw_Preview_Props::eSmartDashboard; //always use SmartDashboard
		err = script.GetFieldTable("settings");
		if (!err)
		{
			std::string sTest;
			double dTest;
			//title= Main
			AssignWstring(script,"title","Main",props.source_name);
			//IP_Address= Black
			AssignWstring(script,"url","Black",props.IP_Address);  //It was so confusing to call this IP_Address url should be a better name
			//Robot_IP_Address= localhost
			AssignWstring(script,"robot_ip_address","localhost",m_AncillaryProps.Robot_IP_Address);

			err = script.GetField("port",NULL,NULL,&dTest);
			props.Port=err?20:(LONG)dTest;

			//StreamProfile= default
			AssignWstring(script,"stream_profile","default",m_AncillaryProps.StreamProfile);
			InitReaderFormatProp();
			//left= 20
			err = script.GetField("left",NULL,NULL,&dTest);
			props.XPos=err?20:(LONG)dTest;
			//top= 10
			err = script.GetField("top",NULL,NULL,&dTest);
			props.YPos=err?10:(LONG)dTest;
			//right= 320
			err = script.GetField("right",NULL,NULL,&dTest);
			props.XRes=err?320:((LONG)dTest)-props.XPos;
			//bottom= 240
			err = script.GetField("bottom",NULL,NULL,&dTest);
			props.YRes=err?240:((LONG)dTest)-props.YPos;
			//SmartDashboard= "java -jar C:\WindRiver\WPILib\SmartDashboard.jar ip localhost"
			AssignWstring(script,"smart_dashboard","java -jar C:\\WindRiver\\WPILib\\SmartDashboard.jar ip localhost",props.smart_file);
			//ClassName= SunAwtFrame  --depreciated
			//WindowName= "SmartDashboard"  //we have wildcard capabilities so we can omit endings
			AssignWstring(script,"window_name","SmartDashboard",props.WindowName);
			//IsPopup= 0
			err = script.GetField("is_popup",&sTest,NULL,NULL);
			if (!err)
			{
				if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
					m_AncillaryProps.IsPopup=false;
				else
					m_AncillaryProps.IsPopup=true;
			}
			else
				m_AncillaryProps.IsPopup=true;
			//PlugIn= Compositor.dll
			AssignWstring(script,"plug_in","",props.plugin_file);
			//AuxStartupFile= TestServer.exe
			AssignWstring(script,"aux_startup_file","",props.aux_startup_file);
			//AuxStartupFileArgs= none
			AssignWstring(script,"aux_startup_file_args","",props.aux_startup_file_Args);
			//RecordFrames= 0
			err = script.GetField("record_frames",&sTest,NULL,NULL);
			if (!err)
			{
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					m_AncillaryProps.RecordFrames=true;
				else
					m_AncillaryProps.RecordFrames=false;
			}
			else
				m_AncillaryProps.RecordFrames=false;

			//RecordPath= D:/media/Robot_Capture/
			AssignWstring(script,"record_path","D:/media/Robot_Capture/",m_AncillaryProps.RecordPath);

			//TODO may want to allow user to specify none
			AssignWstring(script,"controls_plugin_file","Controls.dll",props.controls_plugin_file);
			//props.controls_plugin_file=L"Controls.dll";

			IgnoreList ignore_list;
			memset(&ignore_list,0,sizeof(IgnoreList));
			err = script.GetField("ignore_url",NULL,&ignore_list.Ignore_URL,NULL);
			err = script.GetField("ignore_position",NULL,&ignore_list.Ignore_Position,NULL);
			err = script.GetField("ignore_popup",NULL,&ignore_list.Ignore_Popup,NULL);
			err = script.GetField("ignore_record",NULL,&ignore_list.Ignore_Record,NULL);

			err = script.GetFieldTable("load_settings");
			{
				if (!err)
				{
					Load_PersistentData(script,ignore_list);
					script.Pop();
				}
			}

			script.Pop();
		}
	}
	void Load_INI()
	{
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		AncillaryProps &props_anc=m_AncillaryProps;

		wstring SmartDashboard=cwsz_DefaultSmartFile;
		wstring Title=L"Preview";
		wstring Plugin=cwsz_PlugInFile;
		wstring AuxStart=L"none";
		wstring AuxArgs=L"none";
		wstring ClassName=cwsz_ClassName;
		wstring WindowName=cwsz_WindowName;
		wstring URL;
		props_anc.StreamProfile=L"default";
		props_anc.RecordPath=L"none";
		props_anc.RecordFrames=false;  //local cache until the app becomes instantiated

		{
			string InFile = m_FileName.c_str();
			std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				const size_t NoEnties=17;
				string StringEntry[NoEnties<<1];
				{
					char Buffer[1024];
					for (size_t i=0;i<NoEnties;i++)
					{
						in>>StringEntry[i<<1];
						in.getline(Buffer,1024);
						StringEntry[(i<<1)+1]=Buffer;
					}
				}
				in.close();
				AssignInput(Title,StringEntry[1].c_str());
				AssignInput(URL,StringEntry[3].c_str());
				AssignInput(props_anc.Robot_IP_Address,StringEntry[5].c_str());
				AssignInput(props_anc.StreamProfile,StringEntry[7].c_str());
				int left=atoi(StringEntry[9].c_str());
				int top=atoi(StringEntry[11].c_str());
				int right=atoi(StringEntry[13].c_str());
				int bottom=atoi(StringEntry[15].c_str());
				props.XRes=right-left;
				props.YRes=bottom-top;
				props.XPos=left;
				props.YPos=top;
				AssignInput(SmartDashboard,StringEntry[17].c_str());
				AssignInput(ClassName,StringEntry[19].c_str());
				AssignInput(WindowName,StringEntry[21].c_str());
				props_anc.IsPopup=atoi(StringEntry[23].c_str())==0?false:true;
				AssignInput(Plugin,StringEntry[25].c_str());
				AssignInput(AuxStart,StringEntry[27].c_str());
				AssignInput(AuxArgs,StringEntry[29].c_str());
				props_anc.RecordFrames=atoi(StringEntry[31].c_str())==0?false:true;
				AssignInput(props_anc.RecordPath,StringEntry[33].c_str());
			}
			else
			{
				URL=L"";
				props.XRes=320;
				props.YRes=240;
				props.XPos=20;
				props.YPos=10;
			}
		}

		InitReaderFormatProp();
		props.window_type=DDraw_Preview::DDraw_Preview_Props::eSmartDashboard; //always use SmartDashboard

		props.source_name=Title;
		props.IP_Address=URL;
		props.smart_file=SmartDashboard;
		props.ClassName=ClassName;
		props.WindowName=WindowName;
		props.plugin_file=Plugin;
		props.controls_plugin_file=L"Controls.dll"; //TODO may want to allow user to specify none
		props.aux_startup_file=AuxStart;
		props.aux_startup_file_Args=AuxArgs;

	}
	void Save_INI()
	{
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		AncillaryProps &props_anc=m_AncillaryProps;

		string OutFile = m_FileName.c_str();
		string output;

		ofstream out(OutFile.c_str(), std::ios::out );

		AssignOutput(output,props.source_name.c_str());
		out << "title= " << output << endl;
		AssignOutput(output,props.IP_Address.c_str());
		out << "IP_Address= " << output << endl;
		AssignOutput(output,props_anc.Robot_IP_Address.c_str());
		out << "Robot_IP_Address= " << output << endl;
		AssignOutput(output,props_anc.StreamProfile.c_str());
		out << "StreamProfile= " << output << endl;

		out << "left= " << props.XPos << endl;
		out << "top= "  << props.YPos << endl;
		out << "right= " << (props.XPos+props.XRes) << endl;
		out << "bottom= "  << (props.YPos+props.YRes) << endl;

		AssignOutput(output,props.smart_file.c_str());
		out << "SmartDashboard= " << "\"" << output << "\"" << endl;
		AssignOutput(output,props.ClassName.c_str());
		out << "ClassName= " << output << endl;
		AssignOutput(output,props.WindowName.c_str());
		out << "WindowName= " << "\"" << output << "\"" << endl;
		out << "IsPopup= " << props_anc.IsPopup << endl;
		AssignOutput(output,props.plugin_file.c_str());
		out << "PlugIn= " << output << endl;
		AssignOutput(output,props.aux_startup_file.c_str());
		out << "AuxStartupFile= " << output << endl;
		AssignOutput(output,props.aux_startup_file_Args.c_str());
		out << "AuxStartupFileArgs= " << output << endl;
		out << "RecordFrames= " << props_anc.RecordFrames << endl;
		AssignOutput(output,props_anc.RecordPath.c_str());
		out << "RecordPath= " << output << endl;
		out.close();
	}


	void Save_LUA_internal(std::ofstream &out)
	{
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		AncillaryProps &props_anc=m_AncillaryProps;

		using namespace std;
		string output;
		out << "settings_load = " << endl;    //header
		out << "{" << endl;

		AssignOutput(output,props.IP_Address.c_str());
		out << "\t" << "url= " << "\"" << output << "\"," << endl;

		out << "\t" << "left= " << props.XPos << "," << endl;
		out << "\t" << "top= " << props.YPos << "," << endl;
		out << "\t" << "right= " << (props.XPos+props.XRes) << "," << endl;
		out << "\t" << "bottom= " << (props.YPos+props.YRes) << "," << endl;

		out << "\t" << "is_popup= " << props_anc.IsPopup << "," << endl;
		out << "\t" << "record_frames= " << props_anc.RecordFrames << "," << endl;
		AssignOutput(output,props_anc.RecordPath.c_str());
		out << "\t" << "record_path= " << "\"" << output << "\"" << endl;
		out << "}" << endl;  //footer
	}

	void Save_LUA()
	{
		using namespace std;
		char Buffer[1024];
		StringCchCopyA(Buffer,1024,m_FileName.c_str());
		//chop extension
		char *EndPeriodPtr=strrchr(Buffer,'.');
		if (EndPeriodPtr)
			*EndPeriodPtr=0;

		string OutFile = Buffer;
		OutFile += "Save.lua";

		ofstream out(OutFile.c_str(), std::ios::out );

		Save_LUA_internal(out);
	}
public:
	const DDraw_Preview::DDraw_Preview_Props &GetDDraw_Preview_Props() const {return m_PreviewProps;}
	DDraw_Preview::DDraw_Preview_Props &GetDDraw_Preview_Props_rw() {return m_PreviewProps;}
	const AncillaryProps &GetAncillaryProps() const {return m_AncillaryProps;}
	AncillaryProps &GetAncillaryProps_rw() {return m_AncillaryProps;}

	DDraw_Preview_Properties(const char *FileName=NULL) : m_UsingLUA(false),m_SaveOnExit(false)
	{
		DDraw_Preview::DDraw_Preview_Props &props=m_PreviewProps;
		Scripting::Script script;
		const char *err;
		if (FileName==NULL)
			FileName="Video1.lua";

		//file name is not null go ahead assign... we should assume it will have an extension
		m_FileName=FileName;

		const char *ext=strchr(FileName,'.');

		if ((ext)&&(stricmp(ext,".lua")==0))
		{
			err=script.LoadScript(FileName,true);
			if (err==NULL)
			{
				script.NameMap["EXISTING_DASHBOARD_MAIN"] = "EXISTING_DASHMAIN";
				SetUpGlobalTable(script);
				LoadFromScript(script);
				m_UsingLUA=true;
			}
			else
			{
				//In the default case if we don't have a lua fallback to ini
				m_FileName="Video1.ini";
				Load_INI();
			}
		}
		else if ((ext)&&(stricmp(ext,".ini")==0))
		{
			Load_INI();
		}
		else
			assert(false);

	}
	void SetSaveOnExit(bool SaveOnExit) {m_SaveOnExit=SaveOnExit;}
	~DDraw_Preview_Properties()
	{
		if (m_SaveOnExit)
		{
			if (!m_UsingLUA)
				Save_INI();
			else
				Save_LUA();
		}
	}
};



//http://stackoverflow.com/questions/865152/how-can-i-get-a-process-handle-by-its-name-in-c
#include <TlHelp32.h>


int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	// Create a system wide named event
	std::wstring SystemWideName=L"SmartCPPDashboard_";
	SystemWideName+=lpCmdLine;  //keep it unique to allow multiple instances (but not of the same kind)
	HANDLE	SystemWideNamedEvent = ::CreateEventW( NULL, FALSE, FALSE, SystemWideName.c_str() );		
	if ( SystemWideNamedEvent )
	{	// Check for another app with the same name running
		if ( ::GetLastError() == ERROR_ALREADY_EXISTS )
		{
			DebugOutput("Another copy of this application is already running.\n");

			const wchar_t *name=L"Dashboard.exe";
			{
				DWORD pid = 0;

				// Create toolhelp snapshot.
				HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
				PROCESSENTRY32 process;
				ZeroMemory(&process, sizeof(process));
				process.dwSize = sizeof(process);

				// Walk through all processes.
				if (Process32First(snapshot, &process))
				{
					do
					{
						// Compare process.szExeFile based on format of name, i.e., trim file path
						// trim .exe if necessary, etc.
						if (wcsicmp(process.szExeFile, name)==0)
						{
							pid = process.th32ProcessID;
							//make sure this is not this process
							//We may want to refine this more for multiple instance case, but if it is locked up there may not be anymore information available
							if ((pid!=GetCurrentProcessId()) && (pid!=0))
							{
								HANDLE process=OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
								TerminateProcess(process,0);
								CloseHandle(process);
							}
						}
					} while (Process32Next(snapshot, &process));
				}

				CloseHandle(snapshot);
			}
		}
	}

	{

		std::string Filename;
		{
			wchar2char(lpCmdLine);
			Filename=wchar2char_pchar;
		}
		//For release build... we need to restore our working directory read .ini from driver station
		#ifndef _DEBUG
		{
			wchar_t Buffer[MAX_PATH];
			GetModuleFileName(hInstance,Buffer,MAX_PATH);
			wchar_t *LastSlash=wcsrchr(Buffer,'\\');
			*LastSlash=0;
			SetCurrentDirectory(Buffer);
		}
		#endif
		DDraw_Preview_Properties main_props(Filename.c_str()[0]!=0?Filename.c_str():NULL);
		//Grab the ancillary properties and assign them to their respective places
		const DDraw_Preview_Properties::AncillaryProps &props_anc=main_props.GetAncillaryProps();
		DDraw_Preview_Properties::AncillaryProps &props_anc_rw=main_props.GetAncillaryProps_rw();
		const DDraw_Preview::DDraw_Preview_Props &props=main_props.GetDDraw_Preview_Props();
		DDraw_Preview::DDraw_Preview_Props &props_rw=main_props.GetDDraw_Preview_Props_rw();

		g_Robot_IP_Address=props_anc.Robot_IP_Address;
		g_IsPopup=props_anc.IsPopup;
		g_IP_Address=main_props.GetDDraw_Preview_Props().IP_Address;

		g_WindowInfo.rcNormalPosition.left=props.XPos;
		g_WindowInfo.rcNormalPosition.top=props.YPos;
		g_WindowInfo.rcNormalPosition.right=props.XPos+props.XRes;
		g_WindowInfo.rcNormalPosition.bottom=props.YPos+props.YRes;

		DDraw_Preview TheApp(main_props.GetDDraw_Preview_Props());
		//Note: ensure the CWD is maintained on exit... the file requester can change where it goes
		wchar_t CWD[MAX_PATH];
		GetCurrentDirectory(MAX_PATH,CWD);
		TheApp.SetInitRecord(props_anc.RecordFrames);
		{
			wchar2char(props_anc.RecordPath.c_str());
			TheApp.SetRecordPath(wchar2char_pchar);
		}
		TheApp.RunApp();
		SetCurrentDirectory(CWD);
		bool SaveOnExit=true;
		//TODO Hack bandaid... fixme
		if (g_WindowInfo.rcNormalPosition.left + g_WindowInfo.rcNormalPosition.top + g_WindowInfo.rcNormalPosition.right + g_WindowInfo.rcNormalPosition.bottom == 0)
			SaveOnExit=false;

		//set the globals back into the properties for save
		props_anc_rw.Robot_IP_Address=g_Robot_IP_Address;
		props_anc_rw.IsPopup=g_IsPopup;
		props_rw.IP_Address=g_IP_Address;

		props_rw.XPos=g_WindowInfo.rcNormalPosition.left;
		props_rw.YPos=g_WindowInfo.rcNormalPosition.top;
		props_rw.XRes=g_WindowInfo.rcNormalPosition.right-g_WindowInfo.rcNormalPosition.left;
		props_rw.YRes=g_WindowInfo.rcNormalPosition.bottom-g_WindowInfo.rcNormalPosition.top;

		props_anc_rw.RecordFrames=TheApp.GetInitRecord();
		if (TheApp.GetRecordPath())
		{
			char2wchar(TheApp.GetRecordPath());
			props_anc_rw.RecordPath=char2wchar_pwchar;
		}

		main_props.SetSaveOnExit(SaveOnExit);
	}
	CloseHandle(SystemWideNamedEvent);
	return 0;
}
