#include "HandleMain.h"


consoleclass::handleconsole(HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{

	switch(uMsg) {

		case WM_COMMAND:
			/*
			switch(LOWORD(wParam)) {  
				case LBN_SELCHANGE:
					printc("wtf!!");
					break;
				}
			*/
			break;
		/*
		case WM_SIZE:
				SetWindowPos(consolewindow,NULL,0,0,LOWORD(lParam),HIWORD(lParam),
								SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
			break;
		*/
		case WM_CLOSE:
			windowtoggle(screen,consolewindow,IDM_CON);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}


consoleclass::consoleclass() {
	printbuf=new char[64000];
	printbuf[0]=0;
	consolewindow=NULL;
	}

consoleclass::~consoleclass() {
	delete [] printbuf;
	}

void consoleclass::createwindow(HWND w_ptr) {
	consolewindow=CreateWindowEx(0,"EDIT","",WS_VISIBLE|
		WS_THICKFRAME|WS_CHILD|ES_AUTOVSCROLL|WS_VSCROLL|
		ES_MULTILINE|ES_WANTRETURN|ES_LEFT|WS_BORDER,235,270,300,300,w_ptr,
		(HMENU)IDC_CONSOLE,hInst,NULL);
	}

void consoleclass::resize(LONG y) {
	size100(screen,consolewindow,0,0,100,100);
	}


void consoleclass::startup() {
	}


void consoleclass::clearconsole() {
	printbuf[0]=0;
	if (IsWindowVisible(consolewindow)) SetWindowText(consolewindow,printbuf);
	}


void consoleclass::print(char *string) {
	short length;
	strcat(printbuf,string);
	length=strlen(printbuf);
	if (length>1000) {
		//error("Overflow need to reset buffer",1);
		clearconsole();
		}
	if (IsWindowVisible(consolewindow)) {
		SetWindowText(consolewindow,printbuf);
		scrolldown();
		}
	}


void consoleclass::printc(char *string) {
	short length;
	strcat(printbuf,string);
	length=strlen(printbuf);
	if (length>1000) {
		//error("Overflow need to reset buffer",1);
		clearconsole();
		}
	printbuf[length]=13;
	length++;
	printbuf[length]=10;
	length++;
	printbuf[length]=0;
	if (IsWindowVisible(consolewindow)) {
		SetWindowText(consolewindow,printbuf);
		scrolldown();
		}
	}


void consoleclass::scrolldown() {
	WPARAM wParam;
	wParam=MAKELONG(SB_THUMBPOSITION,32767);
	SendMessage(consolewindow,WM_VSCROLL,wParam,0);
	}
