#ifndef STRUCTGO_H
#define STRUCTGO_H

class messagebase {
	public:
	virtual int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
	};

#include "FXobject.h"

//filters structs
struct listofptrs {
	struct listofptrs *next;
	struct listofptrs *prev;
	void *ptr;
	};

enum filtertypes{ft_none,ft_CG,ft_alphasat};

struct filternode {
	struct filternode *next;
	struct filternode *prev;
	struct nodevars *nodeobject;
	filtertypes filtertype;
	void *vfilterptr;
	};

struct CGpoints {
	struct CGpoints *next;
	struct CGpoints *prev;
	ULONG mediatime;
	UBYTE type;  // ease in=4 + ease out=2 + (curve=1 either or linear=0)
	char area1; //ease in area
	char area2; //ease out area
	UBYTE trans; //might as well use this in the base
	};

struct CGxypoints {
	struct CGxypoints *next;
	struct CGxypoints *prev;
	ULONG mediatime;
	short x,y;
	UBYTE type;  // ease in=4 + ease out=2 + (curve=1 either or linear=0)
	char area1; //ease in area
	char area2; //ease out area
	UBYTE pad;
	};

struct titlerCG {
	char *title;
	int buffersize;
	DWORD dwColor;
	LOGFONT lf;
	};

struct filterCG {
	struct filternode node;
	char *filesource;
	union {
		char *name;
		struct titlerCG *titleptr;
		};
	UBYTE *alpha;
	ULONG *yuv;
	short *xrange;
	short *yrange;
	char *xytext;
	UBYTE *thickrange;
	char *thicktext;
	struct CGpoints *thickpointstail,*thickpointshead;
	struct CGxypoints *xypointstail,*xypointshead;
	long in,out;
	short x,y;
	UWORD width,height;
	UBYTE thickness;
	UBYTE pad;
	};

union allfilters {
	struct filternode node;
	struct filterCG CG;
	};

struct filterswindowlist {
	struct filterswindowlist *next;
	struct filterswindowlist *prev;
	HWND window;
	int y;
	};

//end filters structs

//TextEdit
//I may need to have a nodeobject as a parameter
struct textinput {
	class textedit **reftextobj;
	char **reftext;
	//Window stuff
	char *name;
	int x,y,width,height;
	HWND parent;
	HINSTANCE hinst;
	//Other
	UINT usermessage;  //NULL will default to WM_APP
	};

struct miniscrubinput {
	//Window stuff
	int x,y;
	HWND parent;
	HINSTANCE hinst;
	};

//Media Loaders
/*
struct avihandle {
	//Some other AVI variables
	PAVIFILE avifile;
	PAVISTREAM audiostream;
	PAVISTREAM videostream;
	PGETFRAME videoframe;
	WAVEFORMATEX *waveinfo;
	LPDIRECTSOUNDBUFFER lpdsb;
	DWORD biCompression;
	char *filesource;
	double framerate;
	long lastframe;			//figure out if we hit a new frame
	LONG nextaudiobeg;
	//LONG nextaudiobeghalf;
	DWORD lastoffset;			//needed to put next stream in right place
	DWORD audiobuffersize;	//pulled suggested buffer size from avi
	DWORD totalaudiosize;
	UWORD x,y;
	UBYTE scalex,scaley,upsidedown;
	char pad;
	};
*/

struct rtvhandle {
	ULONG rtvcacheid;
	union {
		ULONG *mediartvstill;
		long cacheslot;
		};
	};

//Security
struct keycontents {
	char piiiserial[32];
	char name[32];
	};

//Audio
enum wavcursorstat {on1,on2,off1,off2,offc1,offc2};

enum audiotypes{at_none,at_wav};

struct audionode {
	struct audionode *next;
	struct audionode *prev;
	audiotypes audiotype;
	};

struct wavvoicelist {
	struct wavvoicelist *next;
	struct wavvoicelist *prev;
	struct wavinfo *voice;
	struct imagelist *media; //for playingqueue
	};

struct wavinfo {
	audionode node;
	LPDIRECTSOUNDBUFFER lpdsb;
	ULONG currentbytesread;
	ULONG totalsize;
	ULONG outbytes;
	ULONG halfbuffersize; //for performance calculate one time
	int hfile;
	ULONG size;
	char *filesource;
	char *name;
	WAVEFORMATEX pcm;
	long in,out;
	short frameoffset;
	wavcursorstat cursorstatus;
	};

struct toastercapvars {
	char videofilename[MAX_PATH];
	UINT frameindex;
	long lastfield;
	BOOL frameshigh;
	};

struct indeximage {
	struct indeximage *next,*prev;
	struct imagelist *imageptr;
	};

#endif /* STRUCTGO_H */
