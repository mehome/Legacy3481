#ifndef HANDLEMAIN_H
#define HANDLEMAIN_H
#define _WIN32_WINNT 0x0400
//video x and y are suppose to be 704 x 480
//TODO make a stretchblt thats looks decent
//define VIDEOX 172
//define VIDEOY 129
#define VIDEOX 704
#define VIDEOY 480
#define XBITMAP 80
#define YBITMAP 60
#define HALFXBITMAP 40
#define HALFYBITMAP 30
#define TOOLBUTTONX 20
#define TOOLBUTTONY 20
#define SKINGRIDX 21
#define SKINGRIDY 21

#define EVENT_PLAY 0
#define EVENT_STREAM 1
#define EVENT_VIDEO 2
#define EVENT_PREVIEW 3
#define EVENT_IDLE 4
#define EVENT_DVE1 5
#define EVENT_DVE2 6
#define EVENT_DVE1F 7
#define EVENT_DVE2F 8
#define EVENT_MAX 9

#define IDLE_CHANGEDIR 1 //Idle signal bits
#define IDLE_RENDERFX 2
#define IDLE_FTLEFT 4
#define IDLE_FTRIGHT 8
#define IDLE_TOOLBAR 16

#define MMDRAFTMODE 1
#define MMSCRUBMODE 2
#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#define NO_OVERLAY_HARDWARE     "Sorry, In order to run this application you must "\
				"have a display adapter and driver which support overlays."

enum imageidentifier {id_error,id_media,id_dve,id_filter,id_audio,id_dir};
enum mediatypes {media_unknown,rtv,rtv_still,avi,mpg,mov,iff,bmp,pcx,tif,jpg,gif,tga,dveplugin,filterplugin};
enum audiosourcetypes {audio_unknown,wav};
/* All objects headers */
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h> //added to compile for NT
#include <commctrl.h>
// DirectX includes
#include <ddraw.h>
#include <dsound.h>
#include <dvp.h>
// DirectX media for DV cam
#include <streams.h>
#include <mmreg.h>
#include <msacm.h>
// other multimedia
#include <vfw.h>

#include "CBasic.h"
#include "GBasic.h"
#include "structgo.h"
#include "structstay.h"
#include "StorySourceClass.h"
#include "StoryBoardObject.h"
#include "StorySourceBase.h"
#include "RGBvideorender.h"
#include "loaders.h"
#include "FXobject.h"
#include "filters.h"
#include "DragObject.h"
#include "preview.h"
#include "controls.h"
#include "console.h"
#include "resource.h"
#include "toaster.h"
#include "security.h"
#include "project.h"
#include "audio.h"
#include "capture.h"
/* end include object headers*/

/*Global Vars*/
extern HMENU m_ptr;
extern char username[32];
extern char defaultpath[MAX_PATH]; //default path upon execution
extern toasterclass *toaster;
extern loadersclass *medialoaders;
extern filtersclass *filters;
extern controlsclass *controls;
extern previewclass *preview;
extern dragthisclass *dragthis;
extern securityclass *security;
extern projectclass *project;
extern audioclass *audio;
extern captureclass *capture;
extern storysourceclass *tabs1;
extern HFONT mediafont;
extern HBITMAP skinbmp;
extern HDRAWDIB drawdib;
extern HANDLE arrayofevents[EVENT_MAX];
extern HWND About;
extern int debug;
extern BOOL killplaythread;
extern BOOL killstreamthread;
extern BOOL killvideothread;
extern BOOL killpreviewthread;
extern UWORD idlesignals;
extern UWORD streamthreadmode;
extern class storysourcebase *IdleChangeDirParm1;
extern class miniscrub *idleminiscrub;
extern char IdleChangeDirParm2[MAX_PATH];
extern int nostaticpreview;
extern int nostaticxypreview;
extern int test;
/* End Globals */

/* Here are the function prototypes */
LRESULT CALLBACK handlemain(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK handleabout(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK gObjCallback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
int gtexteditcallback (ULONG userdata,int messagecode);
DWORD WINAPI playthreadfunc(LPVOID parm);
DWORD WINAPI streamthreadfunc(LPVOID parm);
DWORD WINAPI videothreadfunc(LPVOID parm);
DWORD WINAPI previewthreadfunc(LPVOID parm);
DWORD WINAPI videothreadNoCard(LPVOID parm);
DWORD WINAPI idlethreadfunc(LPVOID parm);
DWORD WINAPI dve1threadfunc(LPVOID parm);
DWORD WINAPI dve2threadfunc(LPVOID parm);

void resizewindows();
void initchildren();
void print(char *sz,...);
void printc(char *sz,...);
struct imagelist *duplicateimage(struct imagelist *source);
void g_updateframedisplay();
class fxclass *getfxobj();
class storyboardclass *getstoryboard();
void toastercallback(void *userdata,long frames_behind);
void	toasterdcallback(void *userdata,ULONG source_time);
char *getdirectory(char *inputprompt,char *pathholder);
BOOL getopenfile(char *dest,char *defpath,char *defext,char *inputprompt,char *filter,BOOL musthave);
ULONG fliplong(ULONG x);
/* End Global function prototypes */
/*External Global function prototypes*/
/*
LARGE_INTEGER divclocksbyframerate(LARGE_INTEGER x,ULONG y);
ULONG divu64byu32(LARGE_INTEGER x,ULONG y);
LARGE_INTEGER subu64byu32(LARGE_INTEGER x,ULONG y);
*/
/* End Global function prototypes */


#endif /* HANDLEMAIN_H */
