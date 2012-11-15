#include "HandleMain.h"

// Globals
//End Globals

static consoleclass con;
static testclass testobj;

LRESULT CALLBACK handlemain(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HMENU m_ptr;

	switch (uMsg) {
		case WM_CREATE:
			con.createwindow(w_ptr);
			printc("* * * Console Interface * * *");
			printc(" ");
			testobj.createwindow(w_ptr);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDM_TESTTHIS: {
					break;
					}

				case IDM_CON:
					windowtoggle(screen,con.consolewindow,IDM_CON);
		 			break;
				case IDM_TEST:
					windowtoggle(screen,testobj.window,IDM_TEST);
		 			break;
				/* End All Window Toggles */
				case IDM_CLEARCON:
					con.clearconsole();
					break;
				/* * * * HELP * * * */
				case IDM_ABOUT:
					DialogBox(hInst,"AboutBox",w_ptr,DLGPROC(handleabout));
					break;
				} //end switch low parm of command
			break;  //end command

		case WM_MOVE:
		case WM_SIZE:
			resizewindows();
			return(0);

		case WM_DESTROY:
			testobj.shutdown();
			cleanup();
			PostQuitMessage(0);
			break;

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		} /* End switch uMsg */
	return(0L);
	}


void cleanup() {
	if (pmem) killnode(nodeobject,&pmem);
	if (pmem) disposeall(&pmem);
	if (IsWindowVisible(screen)) DestroyWindow(screen);
	}


LRESULT CALLBACK handleabout(HWND req_ptr,UINT message,WPARAM wParam,LPARAM lParam) {
   char params[]="";
   char dr[]="";
	switch (message) {
		case WM_INITDIALOG: 
			return (TRUE);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(req_ptr, TRUE);
					return (TRUE);
				}
			break;
		}
   return (0L); 
	}


/*Handle import window messages here*/


LRESULT CALLBACK gObjCallback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	class messagebase *messageobj=(class messagebase *)GetWindowLong(w_ptr,GWL_USERDATA);
	if (messageobj) return (messageobj->Callback(w_ptr,uMsg,wParam,lParam));
	else return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
	}


void resizewindows() {
	LONG y=10;
	con.resize(y);
	testobj.resize();
	}


void initchildren() {
	con.startup();
	testobj.startup();
	printc("Ready.");
	}


void print(char *string) {
	con.print(string);
	}


void printc(char *string) {
	con.printc(string);
	}


