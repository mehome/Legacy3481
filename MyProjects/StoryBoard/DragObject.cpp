#include "HandleMain.h"  

int dragthisclass::Callback(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
	{
	storyboardclass *storyboard=getstoryboard();
	switch(uMsg) {
		case WM_MEASUREITEM:
			//measureitem(lParam);
			return (TRUE);

		case WM_MOUSEMOVE: {
			struct tagPOINT mouseloc;
			GetCursorPos(&mouseloc);
			SendMessage(storyboard->storywindow,WM_NCHITTEST,NULL,MAKELONG(mouseloc.x,mouseloc.y));
			SetWindowPos(dragwindow,NULL,mouseloc.x-HALFXBITMAP,mouseloc.y-HALFYBITMAP,0,0,SWP_NOSIZE|SWP_NOZORDER);
			break;
			}

		case WM_LBUTTONUP: {
			RECT rc;
			struct tagPOINT mouseloc;

			if (dragtoggle) {
				dragtoggle=FALSE;
				ShowWindow(dragwindow,SW_HIDE);
				ReleaseCapture();
				GetCursorPos(&mouseloc);

				switch (dragimage->id) {
					case id_filter:
						if (IsWindowVisible(filters->window)) {
							GetWindowRect(filters->window,&rc);
							if (!((mouseloc.x<rc.left)||(mouseloc.x>rc.right)||(mouseloc.y<rc.top)||(mouseloc.y>rc.bottom))) {
								disposenode(nodeobject,(struct memlist *)dragimage);
								return (filters->Callback(w_ptr,uMsg,wParam,lParam));
								}
							}
						else printc("Drag filter media inside the Media Filters Window");
						break;
					case id_audio:
						if (IsWindowVisible(audio->window)) {
							GetWindowRect(audio->window,&rc);
							if (!((mouseloc.x<rc.left)||(mouseloc.x>rc.right)||(mouseloc.y<rc.top)||(mouseloc.y>rc.bottom))) {
								disposenode(nodeobject,(struct memlist *)dragimage);
								return (audio->Callback(w_ptr,uMsg,wParam,lParam));
								}
							}
						else printc("Drag audio file inside the Audio Window");
						break;
					case id_dve:
					case id_media:
						if (IsWindowVisible(storyboard->storywindow)) {
							GetWindowRect(storyboard->storywindow,&rc);
							if ((!((mouseloc.x<rc.left)||(mouseloc.x>rc.right)||(mouseloc.y<rc.top)||(mouseloc.y>rc.bottom)))||(dragorigin==IDC_STORYBOARD))
								return (storyboard->Callback(w_ptr,uMsg,wParam,lParam));
							else return (dragimage->idobj->Callback(dragimage->idobj->window,uMsg,wParam,lParam));
							}
						break;
					}
				disposenode(nodeobject,(struct memlist *)dragimage);
				}
			break;
			}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(dragwindow,&ps);
			smartrefresh(ps.hdc,smartrefreshclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(dragwindow,&ps);
			return (0);
			}

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


void dragthisclass::updateimage(struct imagelist *drag,UWORD originator) {
	HDC hdc,hmem,smartdc;
	struct tagPOINT mouseloc;
	dragimage=drag;
	hdc=GetDC(dragwindow);
	hmem=CreateCompatibleDC(hdc);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,smartrefreshclip);
	if (drag->selected) 	SelectObject(hmem,selectimageclass::multiselectimage);
	else SelectObject(hmem,drag->image);

 	BitBlt(smartdc,0,0,XBITMAP,YBITMAP,hmem,0,0,SRCCOPY);
	DeleteDC(smartdc);
	DeleteDC(hmem);
	ReleaseDC(dragwindow,hdc);
	InvalidateRgn(dragwindow,NULL,FALSE);
	dragtoggle=TRUE;
	dragorigin=originator;
	GetCursorPos(&mouseloc);
	SetWindowPos(dragwindow,NULL,mouseloc.x-HALFXBITMAP,mouseloc.y-HALFYBITMAP,0,0,SWP_NOSIZE|SWP_NOZORDER);
	ShowWindow(dragwindow,SW_SHOW);
	SetCapture(dragwindow);
	}


dragthisclass::dragthisclass() {
	dragimage=NULL;
	dragtoggle=NULL;
	smartrefreshclip=NULL;
	}

void dragthisclass::createwindow(HWND w_ptr) {
	dragwindow=CreateWindowEx(0,"OBJWIN",NULL,WS_POPUP
		/*Temporary WS_OVERLAPPEDWINDOW */
		,50,50,XBITMAP-2,YBITMAP-2,w_ptr,
		NULL,hInst,NULL);
	smartrefreshclip=createclipwin(dragwindow,(HBRUSH)(COLOR_WINDOW+1));
	SetWindowLong(dragwindow,GWL_USERDATA,(long)this);
	}

void dragthisclass::shutdown() {
	if (smartrefreshclip) DeleteObject(smartrefreshclip);
	}


dragthisclass::~dragthisclass() {
	//nothing to clean yet...
	}
