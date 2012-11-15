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
		HWND window;
		HWND controlbuttons[7];
		long frameindex,oldframeindex;

		long mycontrolis;

		controlsclass();
		LONG resize();
		void createwindow(HWND w_ptr);
		BOOL startup();
		BOOL controlsfail();
		void shutdown();
		~controlsclass();

	private:
		LARGE_INTEGER flc;
		ULONG mlc;
		UWORD cbtb_offsetx,cbtb_widthx;
		long fiadjustment;
		BOOL playdelay;

		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		BOOL configuresize();
	};

#endif /* CONTROLS_H */
