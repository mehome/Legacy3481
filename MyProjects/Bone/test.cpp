#include "HandleMain.h"


testclass::Callback(HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{

	switch(uMsg) {

		case WM_COMMAND:
			break;
		/*
		case WM_SIZE:
				SetWindowPos(consolewindow,NULL,0,0,LOWORD(lParam),HIWORD(lParam),
								SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
			break;
		*/
		case WM_CLOSE:
			windowtoggle(screen,window,IDM_TEST);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}


testclass::testclass() {
	window=NULL;
	}

testclass::~testclass() {
	}

void testclass::createwindow(HWND w_ptr) {
	window=CreateWindowEx(0,"OBJWIN","Test",WS_VISIBLE|WS_THICKFRAME|
		WS_CHILD|WS_OVERLAPPEDWINDOW,235,270,300,300,w_ptr,
		(HMENU)IDC_TEST,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(LONG)this);
	}

void testclass::resize() {
	size100(window,0,10,40,90);
	}


void testclass::startup() {
	}

void testclass::shutdown() {
	}

