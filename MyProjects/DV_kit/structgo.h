#ifndef STRUCTGO_H
#define STRUCTGO_H

class messagebase {
	public:
	virtual int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
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

#endif /* STRUCTGO_H */
