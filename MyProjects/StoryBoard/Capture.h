#ifndef CAPTURE_H
#define CAPTURE_H

#include "../include/DVCam.h"

class captureclass {
	public:
		class capcommon : public messagebase {
			public:
			HWND window;
			HMENU menu;

			virtual BOOL startup()=0;
			virtual void shutdown()=0;
			virtual void createwindow(HWND w_ptr)=0;
			virtual BOOL startcapture()=0;
			virtual void stopcapture()=0;
			virtual void showdisplay()=0;
			virtual void hidedisplay()=0;
			};

		class DVCamclass : public capcommon {
			public:
			HWND prevwindow;
			HWND startstopbt;
			class DVCamlib *dvcamlib;

			int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
			DVCamclass();
			~DVCamclass();
			BOOL startup();
			void shutdown();
			void createwindow(HWND w_ptr);
			BOOL startcapture();
			void stopcapture();
			void showdisplay();
			void hidedisplay();

			private:
			HWND videochk,videopathedit,videoasl,clipnameedit;
			char videopath[MAX_PATH];
			char clipname[32];
			char fullname[32];
			int filenumber,hfile,totalbyteswritten;
			};

		class captoaster : public capcommon {
			public:
			HWND startstopbt;

			int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
			captoaster();
			~captoaster();
			BOOL startup();
			void shutdown();
			void createwindow(HWND w_ptr);
			BOOL startcapture();
			BOOL startaudiocapture();
			void stopcapture();
			void showdisplay();
			void hidedisplay();

			private:
			HWND audiochk,videochk,audiopathedit,videopathedit,audioasl,videoasl,clipnameedit;
			char audiopath[MAX_PATH];
			char videopath[MAX_PATH];
			char audiofilename[MAX_PATH];
			char clipname[32];
			int filenumber,hfile,totalbyteswritten;
			LPDIRECTSOUNDCAPTURE lpDSC;
			DSCBUFFERDESC dscbd;
			LPDIRECTSOUNDCAPTUREBUFFER pDSCB;
			wavcursorstat cursorstatus;

			void streamcapture();
			BOOL initFail(char *reason);
			};

		class capcommon *vcaptureptr;

		captureclass();
		BOOL choosedevice(int capdevice);
		~captureclass();
		BOOL startup();
		void shutdown();
		void createwindow(HWND w_ptr);
		BOOL startcapture();
		void stopcapture();
		void showdisplay();
		void hidedisplay();
	private:
		BOOL capturing;
	};

#endif /* CAPTURE_H */
