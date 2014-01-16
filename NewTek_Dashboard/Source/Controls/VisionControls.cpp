#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"
#include "../ProcessingVision/Plugin_Control_Interface.h"

DialogBase *g_pVisionControls=NULL;  //cjt
DialogBase *g_pTargetEnableControls=NULL;

extern Dashboard_Controller_Interface *g_Controller;
static Plugin_SquareTargeting *g_plugin_SquareTargeting=NULL;


  /***********************************************************************************************************************/
 /*													TargetEnableControls												*/
/***********************************************************************************************************************/

void VisionControls_SaveChanges();

class TargetEnableControls : public DialogBase
{
public:
	TargetEnableControls() : m_IgnoreUpdate(false) {}
	~TargetEnableControls();
	virtual bool Run(HWND pParent);
protected:
	virtual size_t GetDialogResource() const {return IDD_Target_DIALOG;}
	virtual LPARAM GetInstance() const {return (LPARAM) this;}
	virtual const wchar_t * const GetTitlePrefix() const  {return L"Target Enabling";}
	virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
private:
	void UpdateControls();
	bool m_IgnoreUpdate;
};

DialogBase *CreateTargetEnableDialog() {return new TargetEnableControls;}

TargetEnableControls::~TargetEnableControls()
{
	g_pTargetEnableControls=NULL;
	VisionControls_SaveChanges();  //the targeting is bundled in this file
}

void TargetEnableControls::UpdateControls()
{
	const bool IsTargeting=g_plugin_SquareTargeting->Get_Vision_Settings(eIsTargeting)==0.0?false:true;
	SendDlgItemMessage(m_hDlg, IDC_DisableTarget, BM_SETCHECK, (!IsTargeting), 0);
	SendDlgItemMessage(m_hDlg, IDC_EnableTarget, BM_SETCHECK,  ( IsTargeting), 0);
}

bool TargetEnableControls::Run(HWND pParent)
{
	m_IgnoreUpdate=true;
	bool ret=__super::Run(pParent);
	m_IgnoreUpdate=false;
	UpdateControls();
	return ret;
}

long TargetEnableControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND: 
		{
			WORD notifycode = HIWORD(wParam);
			WORD buttonid = LOWORD(wParam);
			if (notifycode==BN_CLICKED) 
			{
				if (m_IgnoreUpdate) break;
				//Handle our button up
				switch (buttonid) 
				{
				case IDC_DisableTarget:
					printf("Disable Target\n");
					g_plugin_SquareTargeting->Set_Vision_Settings(eIsTargeting,(double)false);
					break;
				case IDC_EnableTarget:
					printf("Enable Target\n");
					g_plugin_SquareTargeting->Set_Vision_Settings(eIsTargeting,(double)true);
					break;
				}
			}
		}
		break;
	default:
		return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
	}
	UpdateControls();  //for this simple dialog everything we capture we'll want an update
	return TRUE;
}


  /***********************************************************************************************************************/
 /*														VisionControls													*/
/***********************************************************************************************************************/


class VisionControls : public DialogBase
{
	public:
		VisionControls();
		~VisionControls();
		virtual bool Run(HWND pParent);
	protected:
		virtual size_t GetDialogResource() const {return IDD_VISION_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"Vision controls for ";}
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	private:
		struct VisionSettings
		{
			TrackerType vsTrackerType;
			DisplayType vsDisplayType;
			bool vsSolidMask;
			bool vsOverlays;
			bool vsAimingText;
			bool vsBoundsText;
			bool vs3ptGoal;
			ThresholdColorSpace vsThresholdMode;	
			int ThresholdValues[eNumThresholdSettings];
		} CurrentSettings;
		void GetVisionSettings(void);
		void UpdateControls(void);
		void UpdateText(VisionSetting_enum setting, bool ForceUpdate=false);
		void UpdateSlider(VisionSetting_enum setting, bool ForceUpdate=false);
		void EnableGBSVSliders(bool bEnable);
		double m_OldEdit_ThresholdValues[eNumThresholdSettings];   //flood control for edit
		double m_OldSlider_ThresholdValues[eNumThresholdSettings];   //flood control for sliders
		//Avoid recursion on messages
		bool m_UpdatingEdit[eNumThresholdSettings],
			m_UpdatingSlider[eNumThresholdSettings];
};

DialogBase *CreateVisionControlsDialog() {return new VisionControls;}

const char * const csz_Filename="Vision_";
static void GetVisionFilename(std::string &Output)
{
	Output = csz_Filename;
	Output += DashBoard_GetWindowText();
	Output += ".ini";
}



class MenuSelection_Vision : public MenuSelection_Interface
{
	virtual void Initialize(HWND pParent,Plugin_Controller_Interface *plugin)
	{
		g_plugin_SquareTargeting=dynamic_cast<Plugin_SquareTargeting *>(plugin);
		if( g_plugin_SquareTargeting == NULL ) return;

		using namespace std;
		string InFile;
		GetVisionFilename(InFile);

		std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			string StringEntry;
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eTrackerType, (double)(TrackerType)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eDisplayType, (double)(DisplayType)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eSolidMask, (double)(bool)(atoi(StringEntry.c_str()) > 0));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eOverlays, (double)(bool)(atoi(StringEntry.c_str()) > 0));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eAimingText, (double)(bool)(atoi(StringEntry.c_str()) > 0));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eBoundsText, (double)(bool)(atoi(StringEntry.c_str()) > 0));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(e3PtGoal, (double)(bool)(atoi(StringEntry.c_str()) > 0));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdMode, (double)(ThresholdColorSpace)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane1Min, (double)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane2Min, (double)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane3Min, (double)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane1Max, (double)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane2Max, (double)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane3Max, (double)atoi(StringEntry.c_str()));
			in >> StringEntry; in >> StringEntry;
			g_plugin_SquareTargeting->Set_Vision_Settings(eIsTargeting, (double)(bool)(atoi(StringEntry.c_str()) > 0));
			in.close();
		}
	}

	enum MenuSelection
	{
		eMenu_Vision,	//cjt
		eMenu_Targeting,
		eMenu_NoEntries
	};

	virtual size_t AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
	{
		InsertMenu(hPopupMenu, -1, (g_pVisionControls?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Vision+StartingOffset, L"Vision...");  //cjt
		InsertMenu(hPopupMenu, -1, (g_pTargetEnableControls?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Targeting+StartingOffset, L"Targeting...");
		return 2;
	}

	virtual void On_Selection(int selection,HWND pParent)
	{
		switch (selection)
		{
		case eMenu_Vision:
			if (!g_pVisionControls)
			{
				g_pVisionControls=CreateVisionControlsDialog();
				g_pVisionControls->Run(pParent);
			}
			else
			{
				DebugOutput("Vision Dialog already running\n");
				assert(false);
			}
			break;
		case eMenu_Targeting:
			if (!g_pTargetEnableControls)
			{
				g_pTargetEnableControls=CreateTargetEnableDialog();
				g_pTargetEnableControls->Run(pParent);
			}
			else
			{
				DebugOutput("Target Dialog already running\n");
				assert(false);
			}
			break;
		}
	}
} g_MenuSelection_Vision_Instance;

MenuSelection_Interface *g_MenuSelection_Vision=&g_MenuSelection_Vision_Instance;

void Vision_Shutdown()
{
	if (g_pVisionControls)	//cjt
		g_pVisionControls->OnEndDialog();
	if (g_pTargetEnableControls)
		g_pTargetEnableControls->OnEndDialog();
}

  /***********************************************************************************************************************/
 /*														VisionControls													*/
/***********************************************************************************************************************/

//These entries correspond to the Threshold enumerations...
static size_t s_ThresholdResourceTable_TrackerBar[]=
{
	IDC_RHMin,
	IDC_GSMin,
	IDC_BLMin,
	IDC_RHMax,
	IDC_GSMax,
	IDC_BLMax
};

static size_t s_ThresholdResourceTable_Edit[]=
{
	IDC_RHMin_Ed,
	IDC_GSMin_Ed,
	IDC_BLMin_Ed,
	IDC_RHMax_Ed,
	IDC_GSMax_Ed,
	IDC_BLMax_Ed 
};

VisionControls::VisionControls() 
{
	for (size_t i=0; i < eNumThresholdSettings; i++)
	{
		m_UpdatingEdit[i]=false;
		m_UpdatingSlider[i]=false;
	}
}

bool VisionControls::Run(HWND pParent)
{
	bool ret=__super::Run(pParent);

	if( g_plugin_SquareTargeting == NULL ) return ret;

	for( int i = 0; i < eNumThresholdSettings; i++ )
	{
		SendDlgItemMessage(m_hDlg, s_ThresholdResourceTable_TrackerBar[i], TBM_SETRANGE, 1, (LPARAM) MAKELONG(0, 255));
		SendDlgItemMessage(m_hDlg, s_ThresholdResourceTable_TrackerBar[i], TBM_SETTICFREQ, 21, 0);
	}

	using namespace std;
	string InFile;
	GetVisionFilename(InFile);
	GetVisionSettings();

	std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		string StringEntry;
		in >> StringEntry; in >> StringEntry;
		if( CurrentSettings.vsTrackerType != (TrackerType)atoi(StringEntry.c_str()) )
		{
			CurrentSettings.vsTrackerType = (TrackerType)atoi(StringEntry.c_str());
			g_plugin_SquareTargeting->Set_Vision_Settings(eTrackerType, (double)CurrentSettings.vsTrackerType);
			GetVisionSettings();	// need to update the rest.
		}
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsDisplayType = (DisplayType)atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eDisplayType, (double)CurrentSettings.vsDisplayType);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsSolidMask = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin_SquareTargeting->Set_Vision_Settings(eSolidMask, (double)CurrentSettings.vsSolidMask);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsOverlays = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin_SquareTargeting->Set_Vision_Settings(eOverlays, (double)CurrentSettings.vsOverlays);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsAimingText = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin_SquareTargeting->Set_Vision_Settings(eAimingText, (double)CurrentSettings.vsAimingText);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsBoundsText = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin_SquareTargeting->Set_Vision_Settings(eBoundsText, (double)CurrentSettings.vsBoundsText);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vs3ptGoal = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin_SquareTargeting->Set_Vision_Settings(e3PtGoal, (double)CurrentSettings.vs3ptGoal);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsThresholdMode = (ThresholdColorSpace)atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdMode, (double)CurrentSettings.vsThresholdMode);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[0] = atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane1Min, (double)CurrentSettings.ThresholdValues[0]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[1] = atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane2Min, (double)CurrentSettings.ThresholdValues[1]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[2] = atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane3Min, (double)CurrentSettings.ThresholdValues[2]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[3] = atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane1Max, (double)CurrentSettings.ThresholdValues[3]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[4] = atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane2Max, (double)CurrentSettings.ThresholdValues[4]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[5] = atoi(StringEntry.c_str());
		g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdPlane3Max, (double)CurrentSettings.ThresholdValues[5]);
		in.close();
	}

	UpdateControls();

	return ret;
}

void VisionControls_SaveChanges()
{
	using namespace std;
	string OutFile;
	GetVisionFilename(OutFile);
	ofstream out(OutFile.c_str(), std::ios::out );
	//this is unrolled to look pretty... :)
	out << "TrackerType= " << (int)g_plugin_SquareTargeting->Get_Vision_Settings(eTrackerType) << endl;
	out << "DisplayType= " << (int)g_plugin_SquareTargeting->Get_Vision_Settings(eDisplayType) << endl;
	out << "SolidMask= " <<   (int)g_plugin_SquareTargeting->Get_Vision_Settings(eSolidMask) << endl;
	out << "Overlays= " <<    (int)g_plugin_SquareTargeting->Get_Vision_Settings(eOverlays) << endl;
	out << "Aiming= " <<      (int)g_plugin_SquareTargeting->Get_Vision_Settings(eAimingText) << endl;
	out << "Bounds= " <<      (int)g_plugin_SquareTargeting->Get_Vision_Settings(eBoundsText) << endl;
	out << "3ptgoal= " <<     (int)g_plugin_SquareTargeting->Get_Vision_Settings(e3PtGoal) << endl;
	out << "ThresholdType= "<<(int)g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdMode) << endl;
	out << "Plane1Min= " << g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdPlane1Min) << endl;
	out << "Plane2Min= " << g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdPlane2Min) << endl;
	out << "Plane3Min= " << g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdPlane3Min) << endl;
	out << "Plane1Max= " << g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdPlane1Max) << endl;
	out << "Plane2Max= " << g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdPlane2Max) << endl;
	out << "Plane3Max= " << g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdPlane3Max) << endl;
	out << "IsTargeting= " << (int)g_plugin_SquareTargeting->Get_Vision_Settings(eIsTargeting) << endl;
	out.close();
}

VisionControls::~VisionControls()
{
	g_pVisionControls=NULL;
	VisionControls_SaveChanges();
}

void VisionControls::GetVisionSettings()
{
	if( g_plugin_SquareTargeting == NULL ) return;

	CurrentSettings.vsTrackerType = (TrackerType)(int)g_plugin_SquareTargeting->Get_Vision_Settings(eTrackerType);
	CurrentSettings.vsDisplayType = (DisplayType)(int)g_plugin_SquareTargeting->Get_Vision_Settings(eDisplayType);
	CurrentSettings.vsSolidMask = (bool)((int)g_plugin_SquareTargeting->Get_Vision_Settings(eSolidMask) > 0);
	CurrentSettings.vsOverlays = (bool)((int)g_plugin_SquareTargeting->Get_Vision_Settings(eOverlays) > 0);
	CurrentSettings.vsAimingText = (bool)((int)g_plugin_SquareTargeting->Get_Vision_Settings(eAimingText) > 0);
	CurrentSettings.vsBoundsText = (bool)((int)g_plugin_SquareTargeting->Get_Vision_Settings(eBoundsText) > 0);
	CurrentSettings.vs3ptGoal = (bool)((int)g_plugin_SquareTargeting->Get_Vision_Settings(e3PtGoal) > 0);
	CurrentSettings.vsThresholdMode = (ThresholdColorSpace)(int)g_plugin_SquareTargeting->Get_Vision_Settings(eThresholdMode);
	for( int i = 0; i < eNumThresholdSettings; i++ )
	{
		CurrentSettings.ThresholdValues[i] = (int)g_plugin_SquareTargeting->Get_Vision_Settings((VisionSetting_enum)(eThresholdPlane1Min + i));
	}
}

void VisionControls::UpdateControls()
{
	//SendDlgItemMessage(m_hDlg, IDC_TargetFrisbe, BM_SETCHECK, ( CurrentSettings.vsTrackerType == eFrisbeTracker ), 0);
	SendDlgItemMessage(m_hDlg, IDC_TargetBall, BM_SETCHECK, ( CurrentSettings.vsTrackerType == eBallTracker ), 0);
	SendDlgItemMessage(m_hDlg, IDC_TargetGoal, BM_SETCHECK, ( CurrentSettings.vsTrackerType == eGoalTracker ), 0);

	SendDlgItemMessage(m_hDlg, IDC_DisplayNormal, BM_SETCHECK, ( CurrentSettings.vsDisplayType == eNormal ), 0);
	SendDlgItemMessage(m_hDlg, IDC_DisplayThreshold, BM_SETCHECK, ( CurrentSettings.vsDisplayType == eThreshold ), 0);
	SendDlgItemMessage(m_hDlg, IDC_DisplayMask, BM_SETCHECK, ( CurrentSettings.vsDisplayType == eMasked ), 0);

	SendDlgItemMessage(m_hDlg, IDC_SOLID, BM_SETCHECK, CurrentSettings.vsSolidMask, 0);
	SendDlgItemMessage(m_hDlg, IDC_ShowAiming, BM_SETCHECK, CurrentSettings.vsAimingText, 0);
	SendDlgItemMessage(m_hDlg, IDC_ShowBounds, BM_SETCHECK, CurrentSettings.vsBoundsText, 0);
	SendDlgItemMessage(m_hDlg, IDC_ShowOverlay, BM_SETCHECK, CurrentSettings.vsOverlays, 0);

	SendDlgItemMessage(m_hDlg, IDC_3PT, BM_SETCHECK, CurrentSettings.vs3ptGoal, 0);
	SendDlgItemMessage(m_hDlg, IDC_2PT, BM_SETCHECK, !CurrentSettings.vs3ptGoal, 0);

	SendDlgItemMessage(m_hDlg, IDC_ThresholdRGB, BM_SETCHECK, ( CurrentSettings.vsThresholdMode == eThreshRGB ), 0);
	SendDlgItemMessage(m_hDlg, IDC_ThresholdHSV, BM_SETCHECK, ( CurrentSettings.vsThresholdMode == eThreshHSV ), 0);
	SendDlgItemMessage(m_hDlg, IDC_ThresholdLuma, BM_SETCHECK, ( CurrentSettings.vsThresholdMode == eThreshLuma ), 0);

	for( int i = 0; i < eNumThresholdSettings; i++ )
	{
		UpdateText((VisionSetting_enum)(eThresholdPlane1Min + i), true);
		UpdateSlider((VisionSetting_enum)(eThresholdPlane1Min + i), true);
	}
}

void VisionControls::UpdateText(VisionSetting_enum setting, bool ForceUpdate)
{
	//flood control check... and if we have a resource to draw to
	if( ((m_OldEdit_ThresholdValues[setting - eThresholdStart] != CurrentSettings.ThresholdValues[setting - eThresholdStart]) || (ForceUpdate)) )
	{
		m_UpdatingEdit[setting - eThresholdStart] = true;
		HWND hWndScroller = GetDlgItem(m_hDlg, s_ThresholdResourceTable_Edit[setting - eThresholdStart]);
		std::wstring Value;
		wchar_t Buffer[128];
		swprintf(Buffer, L"%d", CurrentSettings.ThresholdValues[setting - eThresholdStart]);
		Value = Buffer;
		SetWindowText(hWndScroller, Value.c_str());
		m_OldEdit_ThresholdValues[setting - eThresholdStart] = CurrentSettings.ThresholdValues[setting - eThresholdStart];  //keep note of last value written
		m_UpdatingEdit[setting - eThresholdStart] = false;
	}
}

void VisionControls::UpdateSlider(VisionSetting_enum setting, bool ForceUpdate)
{
	if(	((m_OldSlider_ThresholdValues[setting - eThresholdStart] != CurrentSettings.ThresholdValues[setting - eThresholdStart]) || (ForceUpdate)) )
	{
		m_UpdatingSlider[setting - eThresholdStart] = true;
		HWND hWndScroller = GetDlgItem(m_hDlg, s_ThresholdResourceTable_TrackerBar[setting - eThresholdStart]);
		SendMessage(hWndScroller, TBM_SETPOS, TRUE, CurrentSettings.ThresholdValues[setting - eThresholdStart]);
		m_OldSlider_ThresholdValues[setting - eThresholdStart] = CurrentSettings.ThresholdValues[setting - eThresholdStart];  //keep note of last value written
		m_UpdatingSlider[setting - eThresholdStart] = false;
	}
}

void VisionControls::EnableGBSVSliders(bool bEnable)
{
	for( int i = 0; i < eNumThresholdSettings; i++ )
	{
		if( ((VisionSetting_enum)(eThresholdPlane1Min + i) != eThresholdPlane1Min ) &&
			((VisionSetting_enum)(eThresholdPlane1Min + i) != eThresholdPlane1Max ) )
		{
			HWND hWndScroller = GetDlgItem(m_hDlg, s_ThresholdResourceTable_TrackerBar[eThresholdPlane1Min + i - eThresholdStart]);
			EnableWindow(hWndScroller, bEnable);
			hWndScroller = GetDlgItem(m_hDlg, s_ThresholdResourceTable_Edit[eThresholdPlane1Min + i - eThresholdStart]);
			EnableWindow(hWndScroller, bEnable);
		}
	}
}

long VisionControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	assert(g_Controller);
	assert(g_plugin_SquareTargeting);

	switch(uMsg)
	{
		case WM_COMMAND: 
			{
				HWND hWndEdit=(HWND)lParam;
				WORD notifycode = HIWORD(wParam);
				WORD buttonid = LOWORD(wParam);
				bool bChecked = false;

				if (notifycode==BN_CLICKED) 
				{
					//Handle our button up
					switch (buttonid) 
					{
					case IDC_TargetGoal:
						g_plugin_SquareTargeting->Set_Vision_Settings(eTrackerType, eGoalTracker);
						GetVisionSettings();
						UpdateControls();
						break;
					//case IDC_TargetFrisbe:
					//	g_plugin_SquareTargeting->Set_Vision_Settings(eTrackerType, eFrisbeTracker);
					case IDC_TargetBall:
						g_plugin_SquareTargeting->Set_Vision_Settings(eTrackerType, eBallTracker);
						GetVisionSettings();
						UpdateControls();
						break;
					case IDC_DisplayNormal:
						g_plugin_SquareTargeting->Set_Vision_Settings(eDisplayType, eNormal);
						CurrentSettings.vsDisplayType = eNormal;
						break;
					case IDC_DisplayThreshold:
						g_plugin_SquareTargeting->Set_Vision_Settings(eDisplayType, eThreshold);
						CurrentSettings.vsDisplayType = eThreshold;
						break;
					case IDC_DisplayMask:
						g_plugin_SquareTargeting->Set_Vision_Settings(eDisplayType, eMasked);
						CurrentSettings.vsDisplayType = eMasked;
						break;
					case IDC_SOLID:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_SOLID, BM_GETCHECK, 0, 0) > 0;
						g_plugin_SquareTargeting->Set_Vision_Settings(eSolidMask, bChecked);
						CurrentSettings.vsSolidMask = bChecked;
						break;
					case IDC_ShowOverlay:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_ShowOverlay, BM_GETCHECK, 0, 0) > 0;
						g_plugin_SquareTargeting->Set_Vision_Settings(eOverlays, bChecked);
						CurrentSettings.vsOverlays = bChecked;
						break;
					case IDC_ShowAiming:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_ShowAiming, BM_GETCHECK, 0, 0) > 0;
						g_plugin_SquareTargeting->Set_Vision_Settings(eAimingText, bChecked);
						CurrentSettings.vsAimingText = bChecked;
						break;
					case IDC_ShowBounds:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_ShowBounds, BM_GETCHECK, 0, 0) > 0;
						g_plugin_SquareTargeting->Set_Vision_Settings(eBoundsText, bChecked);
						CurrentSettings.vsBoundsText = bChecked;
						break;
					case IDC_3PT:	//TODO-CJT replace these two with select for Red/Blue Ball
						bChecked = SendDlgItemMessage(m_hDlg, IDC_3PT, BM_GETCHECK, 0, 0) > 0;
						g_plugin_SquareTargeting->Set_Vision_Settings(e3PtGoal, bChecked);
						CurrentSettings.vs3ptGoal = bChecked;
						break;
					case IDC_2PT:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_2PT, BM_GETCHECK, 0, 0) > 0;
						g_plugin_SquareTargeting->Set_Vision_Settings(e3PtGoal, !bChecked);
						CurrentSettings.vs3ptGoal = !bChecked;
						break;
					case IDC_ThresholdRGB:
						g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdMode, eThreshRGB);
						GetVisionSettings();
						UpdateControls();
						EnableGBSVSliders(true);
						break;
					case IDC_ThresholdHSV:
						g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdMode, eThreshHSV);
						GetVisionSettings();
						UpdateControls();
						EnableGBSVSliders(true);
						break;
					case IDC_ThresholdLuma:
						g_plugin_SquareTargeting->Set_Vision_Settings(eThresholdMode, eThreshLuma);
						GetVisionSettings();
						UpdateControls();
						EnableGBSVSliders(false);
						break;
					case IDC_ResetThreshold:
						g_plugin_SquareTargeting->ResetThresholds();
						GetVisionSettings();
						UpdateControls();
					}
				}
				else if (notifycode==EN_CHANGE) 
				{
					for (size_t i = 0; i < e_no_procamp_items; i++)
					{
						if ((!m_UpdatingEdit[i]) && (buttonid == s_ThresholdResourceTable_Edit[i]))
						{
							unsigned TextLength = GetWindowTextLength(hWndEdit);
							if (TextLength < 128)
							{
								wchar_t Buffer[128];
								GetWindowText(hWndEdit, Buffer, 128);
								int value = _wtoi(Buffer);
								if( value < 0) value = 0;
								if( value > 255) value = 255;

								CurrentSettings.ThresholdValues[i] = value;
								//DebugOutput("Edit[%d]= %.3f\n",i,value);
								g_plugin_SquareTargeting->Set_Vision_Settings((VisionSetting_enum)(eThresholdPlane1Min + i), value);
								UpdateSlider((VisionSetting_enum)(eThresholdPlane1Min + i)); //implicitly checks for flood control
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
				HWND hWndScroller = (HWND)lParam;
				for ( size_t i = 0; i < eNumThresholdSettings; i++)
				{
					if ((!m_UpdatingSlider[i]) && (hWndScroller == GetDlgItem(m_hDlg, s_ThresholdResourceTable_TrackerBar[i])))
					{
						CurrentSettings.ThresholdValues[i] = SendMessage(hWndScroller, TBM_GETPOS, 0, 0);
						g_plugin_SquareTargeting->Set_Vision_Settings((VisionSetting_enum)(eThresholdPlane1Min + i), CurrentSettings.ThresholdValues[i]);
						UpdateText((VisionSetting_enum)(eThresholdPlane1Min + i));		//implicitly checks for flood control
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
