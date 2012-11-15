#ifndef DVCAM_H
#define DVCAM_H

#include <streams.h>
#include "Crossbar.h"

#define DVCAMLIBS_EXPORTS

#ifdef DVCAMLIBS_EXPORTS
#define DVCAMLIBS_API __declspec(dllexport)
#else
#define DVCAMLIBS_API __declspec(dllimport)
#endif

#define MENU_VDEVICE0		16
#define MENU_VDEVICE1		17
#define MENU_VDEVICE2		18
#define MENU_VDEVICE3		19
#define MENU_VDEVICE4		20
#define MENU_VDEVICE5		21
#define MENU_VDEVICE6		22
#define MENU_VDEVICE7		23
#define MENU_VDEVICE8		24
#define MENU_VDEVICE9		25
#define MENU_ADEVICE0		26
#define MENU_ADEVICE1		27
#define MENU_ADEVICE2		28
#define MENU_ADEVICE3		29
#define MENU_ADEVICE4		30
#define MENU_ADEVICE5		31
#define MENU_ADEVICE6		32
#define MENU_ADEVICE7		33
#define MENU_ADEVICE8		34
#define MENU_ADEVICE9		35

struct _capstuff {
	char szCaptureFile[_MAX_PATH];
	WORD wCapFileSize;  // size in Meg
	ICaptureGraphBuilder2 *pBuilder;
	IVideoWindow *pVW;
	IMediaEventEx *pME;
	IAMDroppedFrames *pDF;
	IAMVideoCompression *pVC;
	IAMVfwCaptureDialogs *pDlg;
	IAMStreamConfig *pASC;      // for audio cap
	IAMStreamConfig *pVSC;      // for video cap
	IBaseFilter *pRender;
	IBaseFilter *pVCap, *pACap;
	IGraphBuilder *pFg;
	IFileSinkFilter *pSink;
	IConfigAviMux *pConfigAviMux;
	int  iMasterStream;
	BOOL fCaptureGraphBuilt;
	BOOL fPreviewGraphBuilt;
	BOOL fCapturing;
	BOOL fPreviewing;
	BOOL fCapAudio;
	BOOL fCapCC;
	BOOL fCCAvail;
	BOOL fCapAudioIsRelevant;
	int  iVideoDevice;
	int  iAudioDevice;
	double FrameRate;
	BOOL fWantPreview;
	long lCapStartTime;
	long lCapStopTime;
	char achFriendlyName[120];
	BOOL fUseTimeLimit;
	BOOL fUseFrameRate;
	DWORD dwTimeLimit;
	int iFormatDialogPos;
	int iSourceDialogPos;
	int iDisplayDialogPos;
	int iVCapDialogPos;
	int iVCrossbarDialogPos;
	int iTVTunerDialogPos;
	int iACapDialogPos;
	int iACrossbarDialogPos;
	int iTVAudioDialogPos;
	int iVCapCapturePinDialogPos;
	int iVCapPreviewPinDialogPos;
	int iACapCapturePinDialogPos;
	long lDroppedBase;
	long lNotBase;
	BOOL fPreviewFaked;
	CCrossbar *pCrossbar;
	int iVideoInputMenuPos;
	LONG NumberOfVideoInputs;
	HMENU hMenuPopup;
	};


class DVCAMLIBS_API DVCamlib {
	public:
	HWND prevwindow,window,window2;
	char *filepath,*filename;

	struct _capstuff gcap;
	DVCamlib();
	~DVCamlib();
	BOOL startup(HWND prevwindowparm,HWND windowparm,HWND windowparm2,char *filepathparm,char *filenameparm,BOOL preview);
	void shutdown();
	BOOL startcapture();
	void stopcapture();
	LONG PASCAL AppCommand(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam);

	private:
	ErrMsg (LPTSTR sz,...);
	BOOL MakeBuilder();
	BOOL MakeGraph();
	void ResizeWindow(int w,int h);
	void FreeCapFilters();
	void NukeDownstream(IBaseFilter *pf);
	void TearDownGraph();
	BOOL BuildPreviewGraph();
	BOOL StartPreview();
	BOOL StopPreview();
	BOOL InitCapFilters();
	void ChooseDevices(int idV, int idA);
	void AddDevicesToMenu();
	//Section 2 interface functions
	BOOL StartCapture();
	BOOL StopCapture();
	BOOL AllocCaptureFile();
	BOOL SetCaptureFile();
	BOOL BuildCaptureGraph();
	void MakeMenuOptions();
	};

#endif /* DVCAM_H */
