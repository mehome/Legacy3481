/*
**************************TextEdit Class**********************
This will display a simple text edit box... and modify a string
reference pointer... The size of the string will be a multiple of 256
to reduce the number of re-allocations of memory

* * * Dependancies * * *

There has to be a window registry called "OBJWIN" here's an example:

	wc.style         = CS_HREDRAW|CS_VREDRAW|CS_PARENTDC; 
   wc.lpfnWndProc   = (WNDPROC)gObjCallback;        
	wc.cbClsExtra    = sizeof(WORD);
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInst;
	wc.hIcon         = LoadIcon(hInst, "MYAPP");
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = "OBJWIN";                   
	wc.lpszClassName = "OBJWIN";  
	if (!RegisterClass(&wc)) return(FALSE);


Copy this to a root header file where it can be referenced by the gObjCallback:

class messagebase {
	public:
	virtual Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
	};

Finally there needs to be a global callback handler which looks like so:

LRESULT CALLBACK gObjCallback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	class messagebase *messageobj=(class messagebase *)GetWindowLong(w_ptr,GWL_USERDATA);
	if (messageobj) return (messageobj->Callback(w_ptr,uMsg,wParam,lParam));
	else return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
	}
*/

//Change this to a header which includes windows and the messagebase
#include "handlemain.h"

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define EDIT_CLOSE	100
#define EDIT_ED		200
#define EDIT_OK		201
#define EDIT_CANCEL	202
#define EDIT_APPLY	203

class textedit : public messagebase {
	public:
	class textedit **reftextobj;
	char **reftext;
	HWND window;
	textedit(textinput *textparms);
	~textedit();

	void setwindowtext(char *text);
	private:
	HWND editwindow,parentwindow,okbt,applybt,cancelbt;
	UINT usermessage;
	int applyon;

	int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	void updatetext();
	};

class miniscrub : public messagebase {
	public:
	HWND window,mediatimeedit,mediatimescrub;

	void initminiscrub();
	miniscrub(miniscrubinput *miniparms);
	~miniscrub();
	void adjustscrub(int amount);

	private:
	ULONG mediatimeroffset;
	HWND parentwindow;

	int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	};

#endif /* TEXTEDIT_H */
