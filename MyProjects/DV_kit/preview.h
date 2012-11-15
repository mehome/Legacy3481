#ifndef PREVIEW_H
#define PREVIEW_H

class previewclass : public messagebase {
	public:
		HWND prevwindow;
		long previewoff;
		BOOL overlaysupport;

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

		previewclass();
		void resize();
		BOOL startup();
		void shutdown();
		~previewclass();
		void createwindow(HWND w_ptr);
		BOOL initFail(char *reason);
		DWORD updatepreview(/*ULONG *videobuf*/);
		void showoverlay();
		void hideoverlay();
	private:
		ULONG prevx,prevy,prevwidth,prevheight;
		int metricsx,metricsy;

		int Callback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		BOOL configuresize();
		BOOL setuppreview();
		};

#endif /* PREVIEW_H */
