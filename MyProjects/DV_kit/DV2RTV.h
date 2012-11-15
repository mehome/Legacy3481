#ifndef DV2RTV_H
#define DV2RTV_H

#include "handlemain.h"
#include "AVIparser.h"
#include "progress.h"

struct imagelist {
	struct imagelist *next;
	struct imagelist *prev;
	char *filesource;
	class avihandle *mediaavi;
	};

class dv2rtvclass : public messagebase {

	public:
		HWND window;
		class DVCamlib *dvcamlib;
		BOOL playing;

		dv2rtvclass();
		~dv2rtvclass();
		void createwindow(HWND w_ptr);
		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		void dv2rtvmain();

	private:
		HIC hic;
		BITMAPINFOHEADER *outbmi;
		char *invideobuf,*outvideobuf;
		BYTE *audiopcm;
		BYTE *audiopcm2;
		class progressclass *progressobj;
		HBITMAP smartrefreshclip;
		char audiopath[MAX_PATH];
		char videopath[MAX_PATH];
		char sourceavi[MAX_PATH];
		char clipname[MAX_PATH];
		HWND audiochk,videochk,audiopathedit,videopathedit,audioasl,videoasl,clipnameedit,startstopbt;
		//extras
		HWND minbt,closebt,optionsbt;
		HWND sourcefileedit,sourceasl;
		char *source;
		long size;
		int pcmsize;
		int xcoord,ycoord;
		BOOL dragtoggle;
		BOOL audioon,videoon;
		
		BOOL openavi (struct imagelist *media);
		BOOL getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum);
		void closeavi (struct imagelist *media);
		BOOL beginavi (struct imagelist *media);
		void endavi(struct imagelist *media);
		struct savevars *BuildWavFile (char *filename,WAVEFORMATEX *pcmdata,ULONG heap);
		BOOL WriteWavFile (struct savevars *saveobj,char *dest,int size);
		void CloseWavFile (struct savevars *saveobj,char *filename,ULONG datasize);
		void writertv(BOOL frameshigh,char *destvideo);
		BOOL renderframe (ULONG *videobuf,class avihandle *avivar);
		};

#endif /* DV2RTV_H */
