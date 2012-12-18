#ifndef CONTROLS_H
#define CONTROLS_H

#define CONTROLBUTTONX 20
#define CONTROLBUTTONY 20
#define CB_NUMOFBUTTONS 7
#define CB_IDCOFFSET 1000

#define REWINDING 1
#define PLAYING 2 //Playing and Mm's Scrubmode should stay equal
#define FASTFORWARDING 4
#define RECORDING 8

class controlsclass : public messagebase {
	public:
		HWND window,frameswindow,prevwindow,scrub;
		HWND controlbuttons[7];
		long frameindex,oldframeindex;
		long mediatimer;
		UWORD glowpos;
		struct imagelist *streamptr;
		struct imagelist *updatevideoparm;
		UWORD updatevideorequest;
		unsigned long source_time;
		long frames_behind;
		long previewoff;

		//DirectX draw
		LPDIRECTDRAW				lpDD;					// DirectDraw object
		LPDIRECTDRAW2				lpDD4;				// DirectDraw object version 4
		LPDIRECTDRAWSURFACE		lpDDSPrimary;		// DirectDraw primary surface
		LPDIRECTDRAWSURFACE		lpDDSOverlay;		// DirectDraw overlay surface
		LPDIRECTDRAWCLIPPER		lpClipper;			// clipper for primary
		LPDIRECTDRAWPALETTE		lpDDPal;				// DirectDraw palette
	   DDOVERLAYFX ovfx;
		DWORD dwupdateflags;
		RECT rs,rd;
		//end DirectX stuff
		long mycontrolis;

		controlsclass();
		int handlecontrols(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		int handleframes(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		DWORD controlsclass::playfunc(class storyboardclass *storyboard,LPVOID parm);
		void initframecounter(class storyboardclass *storyboard);
		void adjustframecounter(class storyboardclass *storyboard,float linenumber,long framenum,char add);
		void updateframecounter(class storyboardclass *storyboard);
		void adjustglow(class storyboardclass *storyboard,short item);
		long computeactualframes(struct imagelist *streamptr);
		BOOL updateframesdisplay(class storyboardclass *storyboard);
		DWORD updatevideo(void);
		void updatevideonow(void);
		DWORD updatevideoNoCard(void);
		DWORD updatepreview(ULONG *videobuf);
		DWORD updatepreviewthread(void);
		LONG resize();
		BOOL configuresize();
		BOOL setuppreview();
		void createwindow(HWND w_ptr);
		BOOL startup();
		void clearframeclip();
		BOOL initFail(char *reason);
		BOOL controlsfail();
		void shutdown();
		~controlsclass();

	private:
		LARGE_INTEGER performancefrequency;
		LARGE_INTEGER flc;
		ULONG mlc;
		HBITMAP framesclip;
		UWORD cbtb_offsetx,cbtb_widthx;
		ULONG prevx,prevy,prevwidth,prevheight;
		BOOL updatingstreamptr;
		long oldframecounter;
		long fiadjustment;
		BOOL playdelay;

		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	};

#endif /* CONTROLS_H */
