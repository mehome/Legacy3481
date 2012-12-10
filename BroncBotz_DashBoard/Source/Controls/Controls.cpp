#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

static void DebugOutput(const char *format, ... )
{	char Temp[2048];
	va_list marker;
	va_start(marker,format);
	vsprintf(Temp,format,marker);
	OutputDebugStringA(Temp);
	va_end(marker);		
}

extern HMODULE g_hModule;
Dashboard_Controller_Interface *g_Controller=NULL;
DLGPROC g_WinProc;

#define TBM_GETPOS              (WM_USER)
#define TBM_SETPOS              (WM_USER+5)

class DialogBase : public MessageBase_Interface
{
    public:
        DialogBase();
        virtual ~DialogBase(void);

		// Runs the Dialog.
        virtual bool Run(HWND pParent);
		void OnEndDialog(void);

	protected:
		virtual size_t GetDialogResource() const =0;
		virtual LPARAM GetInstance() const =0;
		virtual const wchar_t * const GetTitlePrefix() const =0;
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		HWND        m_hDlg;                     // HWND of Dialog.
    private:
        int OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam );
		bool m_IsClosing;
};

class FileControls : public DialogBase
{
	public:
		FileControls();
		~FileControls();
	protected:
		virtual size_t GetDialogResource() const {return IDD_FILE_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"File controls for ";}
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	private:
		int m_ScrubValue;
} *g_pFileControls;

class ProcampControls : public DialogBase
{
	public:
		ProcampControls();
		~ProcampControls();
		virtual bool Run(HWND pParent);
	protected:
		virtual size_t GetDialogResource() const {return IDD_PROCAMP_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"Procamp for ";}
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	private:
		void UpdateText(ProcAmp_enum setting, bool ForceUpdate=false);
		void UpdateSlider(ProcAmp_enum setting, bool ForceUpdate=false);
		double m_ProcAmpValues[e_no_procamp_items];  
		double m_OldEdit_ProcAmpValues[e_no_procamp_items];   //flood control for edit
		double m_OldSlider_ProcAmpValues[e_no_procamp_items];   //flood control for sliders
		//Avoid recursion on messages
		bool m_UpdatingEdit[e_no_procamp_items],m_UpdatingSlider[e_no_procamp_items];
} *g_pProcamp;

  /***********************************************************************************************************************/
 /*														DialogBase														*/
/***********************************************************************************************************************/


DialogBase::DialogBase() : m_IsClosing(false)
{
	//DebugOutput("DialogBase() starting %p\n",this);
}

DialogBase::~DialogBase(void)
{
	//DebugOutput("~DialogBase() ending %p\n",this);
}

int DialogBase::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return(TRUE);
}

void DialogBase::OnEndDialog(void)
{
	if (!m_IsClosing)
	{
		m_IsClosing=true;
		::DestroyWindow( m_hDlg ); 
		m_hDlg=NULL;
		delete this;
	}
}

long DialogBase::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		OnInitDialog( uMsg, wParam, lParam );
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		OnEndDialog();
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

bool DialogBase::Run(HWND pParent)
{
	// Display the main dialog box.
	m_hDlg= CreateDialog( g_hModule, MAKEINTRESOURCE(GetDialogResource()), pParent, g_WinProc );
	SetWindowLongPtr(m_hDlg,GWLP_USERDATA, (LONG_PTR)GetInstance());

	bool bResult_=  m_hDlg!=NULL;
	if (bResult_)
	{
		wchar_t Buffer[128];
		GetWindowText(pParent,Buffer,128);
		std::wstring Name=GetTitlePrefix();
		Name+=Buffer;
		SetWindowText(m_hDlg,Name.c_str());
		ShowWindow(m_hDlg, SW_SHOW);
	}
	return (bResult_);
}


//Agghhh this is not perfect but gets the job done... we have to keep track of our own adjustments since the GetScrollInfo() is not working
//  [12/7/2012 James]
static void ScrollAdjust(WPARAM wParam,SCROLLINFO &si,int numrows=20)
{
	switch (LOWORD(wParam)) 
	{
		case SB_LINEDOWN: //Scrolls one line down. 
			//if (si.nPos+(numrows-1)<si.nMax) si.nPos+=1;
			if (si.nPos<si.nMax) si.nPos+=1;
			break;
		case SB_LINEUP: //Scrolls one line up. 
			if (si.nPos>0) si.nPos-=1;
			break;
		case SB_PAGEDOWN: //Scrolls one page down. 
			//if (numrows+si.nPos<si.nMax-(numrows-1))
			//	si.nPos+=numrows;
			//else
			//	si.nPos=si.nMax-(numrows-1);
			si.nPos+=numrows;
			if (si.nPos>si.nMax)
				si.nPos=si.nMax;
			break;
		case SB_PAGEUP: //Scrolls one page up. 
			si.nPos-=numrows;
			if (si.nPos<si.nMin)
				si.nPos=si.nMin;
			break;
		case SB_THUMBTRACK: 
			si.nPos=HIWORD(wParam);
			break;
	} //End switch
}


  /***********************************************************************************************************************/
 /*														FileControls													*/
/***********************************************************************************************************************/


FileControls::FileControls() : m_ScrubValue(0)
{
}

FileControls::~FileControls()
{
	g_pFileControls=NULL;
}

long FileControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_COMMAND: 
			{
				WORD notifycode = HIWORD(wParam);
				WORD buttonid = LOWORD(wParam);
				if (notifycode==BN_CLICKED) 
				{
					//Handle our button up
					switch (buttonid) 
					{
					case IDC_PAUSE:
						g_Controller->Pause();
						break;
					case IDC_STOP:
						g_Controller->Stop();
						break;
					case IDC_PLAY:
						g_Controller->Run();
						break;
					case IDC_BROWSE:
						break;
					case IDC_SCRUB:
						DebugOutput("Inside Scrub\n");
						break;
					}
				}
			}
			break;
		case WM_HSCROLL:
			{
				HWND hWndScroller=(HWND)lParam;
				SCROLLINFO si;
				ZeroMemory(&si,sizeof(SCROLLINFO));
				si.nMax=100;

				if (hWndScroller==GetDlgItem(m_hDlg, IDC_SCRUB))
				{
					si.nPos=m_ScrubValue;
					ScrollAdjust(wParam,si);
					m_ScrubValue=si.nPos;
					DebugOutput("Position= %d\n",SendMessage(hWndScroller,TBM_GETPOS,0,0));
				}
			}
			break;
		default:
			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
	}
	return TRUE;
}

  /***********************************************************************************************************************/
 /*														ProcampControls													*/
/***********************************************************************************************************************/

//These should never change
const double c_procamp_NonCalibrated_brightness	= 0.0;
const double c_procamp_NonCalibrated_contrast	= 1.0;
const double c_procamp_NonCalibrated_hue		= 0.0;
const double c_procamp_NonCalibrated_saturation	= 1.0;
const double c_procamp_NonCalibrated_u_offset	= 0.0;
const double c_procamp_NonCalibrated_v_offset	= 0.0;
const double c_procamp_NonCalibrated_u_gain		= 1.0;
const double c_procamp_NonCalibrated_v_gain		= 1.0;

double ProcAmp_Defaults[e_no_procamp_items]=
{
	c_procamp_NonCalibrated_brightness,
	c_procamp_NonCalibrated_contrast,
	c_procamp_NonCalibrated_hue,
	c_procamp_NonCalibrated_saturation,
	c_procamp_NonCalibrated_u_offset,
	c_procamp_NonCalibrated_v_offset,
	c_procamp_NonCalibrated_u_gain,
	c_procamp_NonCalibrated_v_gain,
	0.0, //pedestal
};

//These entries correspond to the ProcAmp enumeration... where -1 is used for all entries not supported
static size_t s_ProcampResourceTable_TrackerBar[]=
{
	IDC_SliderBrightness,
	IDC_SliderContrast,
	-1,-1,
	-1,-1,-1,-1,
	-1
};

static size_t s_ProcampResourceTable_Edit[]=
{
	IDC_EditBrightness,
	IDC_EditContrast,
	-1,-1,
	-1,-1,-1,-1,
	-1
};

const int c_MaxTrackbarRange=100;
const int c_HalfTrackbarMaxRange=c_MaxTrackbarRange>>1;

static int GetTrackerBarPos(double Input,ProcAmp_enum setting)
{
	int Position=0;
	switch (setting)
	{
		//-1 to 1 range
		case e_procamp_brightness:
		case e_procamp_u_offset:
		case e_procamp_v_offset:
			Position= (int)((Input + 1.0) * (double)c_HalfTrackbarMaxRange);
			break;
		case e_procamp_contrast:
		case e_procamp_saturation:
		case e_procamp_u_gain:
		case e_procamp_v_gain:
			Position = (int)(Input * 0.25 * (double)c_MaxTrackbarRange);
			break;
	}
	return Position;
}

static double GetTrackerBarValue (int Position,ProcAmp_enum setting)
{
	double Value=0.0;
	assert((Position>=0) && (Position<=c_MaxTrackbarRange));
	double ScaledPosition=(double)Position/c_MaxTrackbarRange; //converted to a proportion value from 0-1
	switch (setting)
	{
	//-1 to 1 range
	case e_procamp_brightness:
	case e_procamp_u_offset:
	case e_procamp_v_offset:
		//Note: due to the setup of the sliders we'll decrease the range to -.5 to .5... this will allow the sliders to remain 100 pixels wide
		//and this range is plenty good for these
		//Value= (ScaledPosition - 0.5) * 2.0;
		Value= (ScaledPosition - 0.5) * 1.0;
		break;
	case e_procamp_contrast:
	case e_procamp_saturation:
	case e_procamp_u_gain:
	case e_procamp_v_gain:
		Value=(ScaledPosition) * 4.0;  
		break;
	}
	return Value;
}

//returns true if successful
static bool Edit_CheckBounds (double Value,ProcAmp_enum setting)
{
	bool ret=false;
	switch (setting)
	{
		//-1 to 1 range
	case e_procamp_brightness:
	case e_procamp_u_offset:
	case e_procamp_v_offset:
		ret=((Value>=-1.0)&&(Value<=1.0));
		break;
	case e_procamp_contrast:
	case e_procamp_saturation:
	case e_procamp_u_gain:
	case e_procamp_v_gain:
		ret=((Value>=0.0)&&(Value<=4.0));
		break;
	}
	return ret;
}

static void GetFormattedStringForValue(double Input,std::wstring &Output,ProcAmp_enum setting)
{
	wchar_t Buffer[128];
	//These may appear to all work the same, but I'd like to reserve the design to allow for them to be formatted differently
	switch (setting)
	{
		//-1 to 1 range
	case e_procamp_brightness:
	case e_procamp_u_offset:
	case e_procamp_v_offset:
		swprintf(Buffer,L"%.1f",Input*100.0);
		Output=Buffer;
		break;
	case e_procamp_contrast:
	case e_procamp_saturation:
	case e_procamp_u_gain:
	case e_procamp_v_gain:
		swprintf(Buffer,L"%.1f",Input*100.0);
		Output=Buffer;
		break;
	}
}

static double GetValueForFormattedString(wchar_t Input[],ProcAmp_enum setting)
{
	double ret;
	//These may appear to all work the same, but I'd like to reserve the design to allow for them to be formatted differently
	switch (setting)
	{
		//-1 to 1 range
	case e_procamp_brightness:
	case e_procamp_u_offset:
	case e_procamp_v_offset:
		ret=_wtof(Input) / 100.0;
		break;
	case e_procamp_contrast:
	case e_procamp_saturation:
	case e_procamp_u_gain:
	case e_procamp_v_gain:
		ret=_wtof(Input) / 100.0;
		break;
	}
	return ret;
}

ProcampControls::ProcampControls()
{
	for (size_t i=0;i<e_no_procamp_items;i++)
	{
		m_ProcAmpValues[i]=ProcAmp_Defaults[i];
		m_UpdatingEdit[i]=false;
		m_UpdatingSlider[i]=false;
	}
	//TO BLS load values here
}

void ProcampControls::UpdateText(ProcAmp_enum setting, bool ForceUpdate)
{
	//flood control check... and if we have a resource to draw to
	if	(	((m_OldEdit_ProcAmpValues[setting]!=m_ProcAmpValues[setting]) || (ForceUpdate)) &&
		(s_ProcampResourceTable_Edit[setting]!=-1)	)
	{
		m_UpdatingEdit[setting]=true;
		HWND hWndScroller=GetDlgItem(m_hDlg, s_ProcampResourceTable_Edit[setting]);
		std::wstring Value;
		GetFormattedStringForValue(m_ProcAmpValues[setting],Value,setting);
		SetWindowText(hWndScroller,Value.c_str());
		m_OldEdit_ProcAmpValues[setting]=m_ProcAmpValues[setting];  //keep note of last value written
		m_UpdatingEdit[setting]=false;
	}
}

void ProcampControls::UpdateSlider(ProcAmp_enum setting, bool ForceUpdate)
{
	if	(	((m_OldSlider_ProcAmpValues[setting]!=m_ProcAmpValues[setting]) || (ForceUpdate)) &&
		(s_ProcampResourceTable_TrackerBar[setting]!=-1)	)
	{
		m_UpdatingSlider[setting]=true;
		HWND hWndScroller=GetDlgItem(m_hDlg, s_ProcampResourceTable_TrackerBar[setting]);
		SendMessage(hWndScroller,TBM_SETPOS,TRUE,GetTrackerBarPos(m_ProcAmpValues[setting],setting));
		m_OldSlider_ProcAmpValues[setting]=m_ProcAmpValues[setting];  //keep note of last value written
		m_UpdatingSlider[setting]=false;
	}
}

bool ProcampControls::Run(HWND pParent)
{
	bool ret=__super::Run(pParent);
	for (int i=0;i<e_no_procamp_items;i++)
	{
		UpdateSlider((ProcAmp_enum)i,true);
		UpdateText((ProcAmp_enum)i,true);
	}
	return ret;
}

ProcampControls::~ProcampControls()
{
	g_pProcamp=NULL;
	//TODO BLS save values here
}

long ProcampControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_COMMAND: 
			{
				HWND hWndEdit=(HWND)lParam;
				WORD notifycode = HIWORD(wParam);
				WORD buttonid = LOWORD(wParam);
				if (notifycode==EN_CHANGE) 
				{
					for (size_t i=0;i<e_no_procamp_items;i++)
					{
						if ((!m_UpdatingEdit[i]) &&
							(s_ProcampResourceTable_TrackerBar[i]!=-1)&&(buttonid==s_ProcampResourceTable_Edit[i]))
						{
							//Sanity check... leaving disabled to avoid overhead
							//assert(hWndEdit==GetDlgItem(m_hDlg, s_ProcampResourceTable_Edit[i]));

							unsigned TextLength=GetWindowTextLength(hWndEdit);
							if (TextLength<128)
							{
								wchar_t Buffer[128];
								GetWindowText(hWndEdit,Buffer,128);
								double value=GetValueForFormattedString(Buffer,(ProcAmp_enum)i);
								if (Edit_CheckBounds(value,(ProcAmp_enum)i))
								{
									m_ProcAmpValues[i]=value;
									DebugOutput("Edit[%d]= %.3f\n",i,value);
									g_Controller->Set_ProcAmp((ProcAmp_enum)i,m_ProcAmpValues[i]);
									UpdateSlider((ProcAmp_enum)i); //implicitly checks for flood control
								}
							}
							else
								assert(false);  //what happened?
							
						}
					}
				}
			}			
			break;
		case WM_HSCROLL:
			{
				HWND hWndScroller=(HWND)lParam;
				for (size_t i=0;i<e_no_procamp_items;i++)
				{
					if ((s_ProcampResourceTable_TrackerBar[i]!=-1)&&(hWndScroller==GetDlgItem(m_hDlg, s_ProcampResourceTable_TrackerBar[i])))
					{
						int position=SendMessage(hWndScroller,TBM_GETPOS,0,0);
						DebugOutput("setting[%d]= %d\n",i,position);
						m_ProcAmpValues[i]=GetTrackerBarValue(position,(ProcAmp_enum)i);
						g_Controller->Set_ProcAmp((ProcAmp_enum)i,m_ProcAmpValues[i]);
						UpdateText((ProcAmp_enum)i); //implicitly checks for flood control
						break;
					}
				}
			}
			break;
		default:
			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
	}
	return TRUE;
}

  /***********************************************************************************************************************/
 /*													C Global functions													*/
/***********************************************************************************************************************/


enum MenuSelection
{
	eMenu_NoSelection,
	eMenu_Controls,
	eMenu_Procamp,
	eMenu_NoEntries
};

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Initialize (Dashboard_Controller_Interface *controller,DLGPROC gWinProc)
{
	g_Controller=controller;
	g_WinProc=gWinProc;
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
{
	//only populate this if we have a controller
	if (g_Controller)
	{
		InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
		InsertMenu(hPopupMenu, -1, (g_pFileControls?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Controls+StartingOffset, L"File Controls...");
		InsertMenu(hPopupMenu, -1, (g_pProcamp?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Procamp+StartingOffset, L"Procamp...");
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection,HWND pParent)
{
	DebugOutput("Selection=%d\n",selection);
	switch (selection)
	{
		case eMenu_Controls:
			if (!g_pFileControls)
			{
				g_pFileControls=new FileControls;
				g_pFileControls->Run(pParent);
			}
			else
			{
				DebugOutput("Controls Dialog already running\n");
				assert(false);
			}
			break;
		case eMenu_Procamp:
			if (!g_pProcamp)
			{
				g_pProcamp=new ProcampControls;
				g_pProcamp->Run(pParent);
			}
			else
			{
				DebugOutput("Procamp Dialog already running\n");
				assert(false);
			}
			break;
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Shutdown()
{
	//Just signal... it will destroy itself from within the message pump
	if (g_pFileControls)
		g_pFileControls->OnEndDialog();
	if (g_pProcamp)
		g_pProcamp->OnEndDialog();
}
