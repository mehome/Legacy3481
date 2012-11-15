#include "handlemain.h"
#include "textedit.h"

void textedit::updatetext() {
	int length;
	if (*reftext) disposenode(nodeobject,(struct memlist *)*reftext);
	if (length=GetWindowTextLength(editwindow)) {
		length++;
		*reftext=(char *)newnode(nodeobject,length);
		GetWindowText(editwindow,*reftext,length);
		}
	else *reftext=NULL;
	}

void textedit::setwindowtext(char *text) {
	SetWindowText(editwindow,text);
	}

int textedit::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);

			switch (notifycode) {
				case BN_CLICKED:
					switch (buttonid) {
						case EDIT_APPLY:
							updatetext();
							EnableWindow(applybt,FALSE);
							applyon=0;
							SendMessage(parentwindow,usermessage,EDIT_APPLY,(LONG)this);
							break;
						case EDIT_OK:
							updatetext();
							SendMessage(parentwindow,usermessage,EDIT_OK,(LONG)this);
							//No Break!
						case EDIT_CANCEL:
							DestroyWindow(window);
							break;
						} //end switch buttonid
					break;
				case EN_UPDATE:
					if (applyon==0) {
						EnableWindow(applybt,TRUE);
						applyon=TRUE;
						}
					break;
				}// end switch notifycode

			break;
		  } //end switch WM_COMMAND
		case WM_SIZE:
			//TODO configure size
			printc("TextEdit size");
			return (0);
		case WM_DESTROY:
			PostMessage(parentwindow,usermessage,EDIT_CLOSE,(LONG)this);
			break;
		default: return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return (0);
	}

textedit::textedit(textinput *textparms) {
	HINSTANCE hInst=textparms->hinst;
	RECT rc;
	int toolx=0;
	int tooly=0;

	reftextobj=textparms->reftextobj;
	reftext=textparms->reftext;
	parentwindow=textparms->parent;
	if (!(usermessage=textparms->usermessage)) usermessage=WM_APP;
	window=CreateWindowEx(0,"OBJGRAYWIN",textparms->name,
	WS_VISIBLE|WS_POPUP|WS_OVERLAPPEDWINDOW,textparms->x,textparms->y,
	textparms->width,textparms->height,parentwindow,NULL,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(LONG)this);
	GetClientRect(window,&rc);

	editwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_VISIBLE|
	WS_CHILD|ES_AUTOVSCROLL|WS_VSCROLL|
	ES_MULTILINE|ES_WANTRETURN|ES_LEFT,GetSystemMetrics(SM_CXFRAME),0,textparms->width-
	(GetSystemMetrics(SM_CXFRAME)<<1)-
	(GetSystemMetrics(SM_CXBORDER)<<2),textparms->height-GetSystemMetrics(SM_CYCAPTION)-
	(GetSystemMetrics(SM_CYFRAME)<<2)-28,window,(HMENU)EDIT_ED,hInst,NULL);
	tooly=max(rc.bottom-28,rc.top);
	toolx=max(rc.right-206,rc.left);

	okbt=CreateWindowEx(0,"BUTTON","OK",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,64,24,window,(HMENU)EDIT_OK,hInst,NULL);
	toolx+=70;

	cancelbt=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,64,24,window,(HMENU)EDIT_CANCEL,hInst,NULL);
	toolx+=70;

	applybt=CreateWindowEx(0,"BUTTON","Apply",WS_VISIBLE|WS_DISABLED|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,64,24,window,(HMENU)EDIT_APPLY,hInst,NULL);
	applyon=0;
	}

textedit::~textedit() {
	DestroyWindow(window);
	}
