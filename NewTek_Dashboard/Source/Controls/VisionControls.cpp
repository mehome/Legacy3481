#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

extern DialogBase *g_pVisionControls;
extern Dashboard_Controller_Interface *g_Controller;
extern Plugin_Controller_Interface *g_plugin;

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

		std::string m_BLS_Filename; //keep note of the formed name for exit
};

DialogBase *CreateVisionControlsDialog() {return new VisionControls;}

const char * const csz_Filename="Vision_";
static void GetVisionFilename(HWND pParent,std::string &Output)
{
	Output = csz_Filename;
	wchar_t Buffer[128];
	GetWindowText(pParent, Buffer, 128);
	{
		wchar2char(Buffer);
		Output += wchar2char_pchar;
		Output += ".ini";
	}
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

	for( int i = 0; i < eNumThresholdSettings; i++ )
	{
		SendDlgItemMessage(m_hDlg, s_ThresholdResourceTable_TrackerBar[i], TBM_SETRANGE, 1, (LPARAM) MAKELONG(0, 255));
		SendDlgItemMessage(m_hDlg, s_ThresholdResourceTable_TrackerBar[i], TBM_SETTICFREQ, 21, 0);
	}

	using namespace std;
	string InFile;
	GetVisionFilename(GetParent(m_hDlg),InFile);
	m_BLS_Filename=InFile;  //keep copy for exit

	GetVisionSettings();

	std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		string StringEntry;
		in >> StringEntry; in >> StringEntry;
		if( CurrentSettings.vsTrackerType != (TrackerType)atoi(StringEntry.c_str()) )
		{
			CurrentSettings.vsTrackerType = (TrackerType)atoi(StringEntry.c_str());
			g_plugin->Set_Vision_Settings(eTrackerType, (double)CurrentSettings.vsTrackerType);
			GetVisionSettings();	// need to update the rest.
		}
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsDisplayType = (DisplayType)atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eDisplayType, (double)CurrentSettings.vsDisplayType);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsSolidMask = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin->Set_Vision_Settings(eSolidMask, (double)CurrentSettings.vsSolidMask);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsOverlays = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin->Set_Vision_Settings(eOverlays, (double)CurrentSettings.vsOverlays);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsAimingText = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin->Set_Vision_Settings(eAimingText, (double)CurrentSettings.vsAimingText);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsBoundsText = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin->Set_Vision_Settings(eBoundsText, (double)CurrentSettings.vsBoundsText);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vs3ptGoal = (bool)(atoi(StringEntry.c_str()) > 0);
		g_plugin->Set_Vision_Settings(e3PtGoal, (double)CurrentSettings.vs3ptGoal);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.vsThresholdMode = (ThresholdColorSpace)atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdMode, (double)CurrentSettings.vsThresholdMode);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[0] = atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdPlane1Min, (double)CurrentSettings.ThresholdValues[0]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[1] = atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdPlane2Min, (double)CurrentSettings.ThresholdValues[1]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[2] = atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdPlane3Min, (double)CurrentSettings.ThresholdValues[2]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[3] = atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdPlane1Max, (double)CurrentSettings.ThresholdValues[3]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[4] = atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdPlane2Max, (double)CurrentSettings.ThresholdValues[4]);
		in >> StringEntry; in >> StringEntry;
		CurrentSettings.ThresholdValues[5] = atoi(StringEntry.c_str());
		g_plugin->Set_Vision_Settings(eThresholdPlane3Max, (double)CurrentSettings.ThresholdValues[5]);
		in.close();
	}

	UpdateControls();

	return ret;
}

VisionControls::~VisionControls()
{
	using namespace std;
	g_pVisionControls=NULL;
	string OutFile=m_BLS_Filename;
	ofstream out(OutFile.c_str(), std::ios::out );
	//this is unrolled to look pretty... :)
	out << "TrackerType= " << CurrentSettings.vsTrackerType << endl;
	out << "DisplayType= " << CurrentSettings.vsDisplayType << endl;
	out << "SolidMask= " << CurrentSettings.vsSolidMask << endl;
	out << "Overlays= " << CurrentSettings.vsOverlays << endl;
	out << "Aiming= " << CurrentSettings.vsAimingText << endl;
	out << "Bounds= " << CurrentSettings.vsBoundsText << endl;
	out << "3ptgoal= " << CurrentSettings.vs3ptGoal << endl;
	out << "ThresholdType= " << CurrentSettings.vsThresholdMode << endl;
	out << "Plane1Min= " << CurrentSettings.ThresholdValues[0] << endl;
	out << "Plane2Min= " << CurrentSettings.ThresholdValues[1] << endl;
	out << "Plane3Min= " << CurrentSettings.ThresholdValues[2] << endl;
	out << "Plane1Max= " << CurrentSettings.ThresholdValues[3] << endl;
	out << "Plane2Max= " << CurrentSettings.ThresholdValues[4] << endl;
	out << "Plane3Max= " << CurrentSettings.ThresholdValues[5] << endl;
	out.close();

}

void VisionControls::GetVisionSettings()
{
	CurrentSettings.vsTrackerType = (TrackerType)(int)g_plugin->Get_Vision_Settings(eTrackerType);
	CurrentSettings.vsDisplayType = (DisplayType)(int)g_plugin->Get_Vision_Settings(eDisplayType);
	CurrentSettings.vsSolidMask = (bool)((int)g_plugin->Get_Vision_Settings(eSolidMask) > 0);
	CurrentSettings.vsOverlays = (bool)((int)g_plugin->Get_Vision_Settings(eOverlays) > 0);
	CurrentSettings.vsAimingText = (bool)((int)g_plugin->Get_Vision_Settings(eAimingText) > 0);
	CurrentSettings.vsBoundsText = (bool)((int)g_plugin->Get_Vision_Settings(eBoundsText) > 0);
	CurrentSettings.vs3ptGoal = (bool)((int)g_plugin->Get_Vision_Settings(e3PtGoal) > 0);
	CurrentSettings.vsThresholdMode = (ThresholdColorSpace)(int)g_plugin->Get_Vision_Settings(eThresholdMode);
	for( int i = 0; i < eNumThresholdSettings; i++ )
	{
		CurrentSettings.ThresholdValues[i] = (int)g_plugin->Get_Vision_Settings((VisionSetting_enum)(eThresholdPlane1Min + i));
	}
}

void VisionControls::UpdateControls()
{
	SendDlgItemMessage(m_hDlg, IDC_TargetFrisbe, BM_SETCHECK, ( CurrentSettings.vsTrackerType == eFrisbeTracker ), 0);
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
		m_UpdatingSlider[setting] = true;
		HWND hWndScroller = GetDlgItem(m_hDlg, s_ThresholdResourceTable_TrackerBar[setting - eThresholdStart]);
		SendMessage(hWndScroller, TBM_SETPOS, TRUE, CurrentSettings.ThresholdValues[setting - eThresholdStart]);
		m_OldSlider_ThresholdValues[setting - eThresholdStart] = CurrentSettings.ThresholdValues[setting - eThresholdStart];  //keep note of last value written
		m_UpdatingSlider[setting] = false;
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
	assert(g_plugin);

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
						g_plugin->Set_Vision_Settings(eTrackerType, eGoalTracker);
						GetVisionSettings();
						UpdateControls();
						break;
					case IDC_TargetFrisbe:
						g_plugin->Set_Vision_Settings(eTrackerType, eFrisbeTracker);
						GetVisionSettings();
						UpdateControls();
						break;
					case IDC_DisplayNormal:
						g_plugin->Set_Vision_Settings(eDisplayType, eNormal);
						CurrentSettings.vsDisplayType = eNormal;
						break;
					case IDC_DisplayThreshold:
						g_plugin->Set_Vision_Settings(eDisplayType, eThreshold);
						CurrentSettings.vsDisplayType = eThreshold;
						break;
					case IDC_DisplayMask:
						g_plugin->Set_Vision_Settings(eDisplayType, eMasked);
						CurrentSettings.vsDisplayType = eMasked;
						break;
					case IDC_SOLID:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_SOLID, BM_GETCHECK, 0, 0) > 0;
						g_plugin->Set_Vision_Settings(eSolidMask, bChecked);
						CurrentSettings.vsSolidMask = bChecked;
						break;
					case IDC_ShowOverlay:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_ShowOverlay, BM_GETCHECK, 0, 0) > 0;
						g_plugin->Set_Vision_Settings(eOverlays, bChecked);
						CurrentSettings.vsOverlays = bChecked;
						break;
					case IDC_ShowAiming:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_ShowAiming, BM_GETCHECK, 0, 0) > 0;
						g_plugin->Set_Vision_Settings(eAimingText, bChecked);
						CurrentSettings.vsAimingText = bChecked;
						break;
					case IDC_ShowBounds:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_ShowBounds, BM_GETCHECK, 0, 0) > 0;
						g_plugin->Set_Vision_Settings(eBoundsText, bChecked);
						CurrentSettings.vsBoundsText = bChecked;
						break;
					case IDC_3PT:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_3PT, BM_GETCHECK, 0, 0) > 0;
						g_plugin->Set_Vision_Settings(e3PtGoal, bChecked);
						CurrentSettings.vs3ptGoal = bChecked;
						break;
					case IDC_2PT:
						bChecked = SendDlgItemMessage(m_hDlg, IDC_2PT, BM_GETCHECK, 0, 0) > 0;
						g_plugin->Set_Vision_Settings(e3PtGoal, !bChecked);
						CurrentSettings.vs3ptGoal = !bChecked;
						break;
					case IDC_ThresholdRGB:
						g_plugin->Set_Vision_Settings(eThresholdMode, eThreshRGB);
						GetVisionSettings();
						UpdateControls();
						EnableGBSVSliders(true);
						break;
					case IDC_ThresholdHSV:
						g_plugin->Set_Vision_Settings(eThresholdMode, eThreshHSV);
						GetVisionSettings();
						UpdateControls();
						EnableGBSVSliders(true);
						break;
					case IDC_ThresholdLuma:
						g_plugin->Set_Vision_Settings(eThresholdMode, eThreshLuma);
						GetVisionSettings();
						UpdateControls();
						EnableGBSVSliders(false);
						break;
					case IDC_ResetThreshold:
						g_plugin->ResetThresholds();
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
								g_plugin->Set_Vision_Settings((VisionSetting_enum)(eThresholdPlane1Min + i), value);
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
						g_plugin->Set_Vision_Settings((VisionSetting_enum)(eThresholdPlane1Min + i), CurrentSettings.ThresholdValues[i]);
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
