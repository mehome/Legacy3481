#include "HandleMain.h"  


/* * *   storysourceclass for Media FX and VRE implementation * * */

int storysourceclass::Callback(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{

	switch(uMsg) {
		/*
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(mediawindow,&ps);
			smartrefresh(ps.hdc,smartrefreshclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(mediawindow,&ps);
			return (0);
			}
		*/
		case WM_CLOSE:
			return(0L);

		case WM_NOTIFY: {
			SCROLLINFO si;
			switch (((NMHDR*)lParam)->code) {
				case TCN_SELCHANGE: {
					int selection=TabCtrl_GetCurSel(tabwindow);
					switch (selection) {
						//We need to bring new window to bottom
						case 0: //Media
							EnableWindow(media.window,TRUE);
							EnableWindow(fxobject.window,FALSE);
							EnableWindow(filtersourceobj.window,FALSE);
							EnableWindow(audiosourceobj.window,FALSE);
							BringWindowToTop(fxobject.window);
							BringWindowToTop(filtersourceobj.window);
							BringWindowToTop(audiosourceobj.window);
							InvalidateRect(media.window,NULL,TRUE);
							//What a cheesy way to refresh the scrollbar
							ShowScrollBar(media.window,SB_VERT,FALSE);
							si.cbSize=sizeof(SCROLLINFO);
							si.fMask=SIF_PAGE|SIF_RANGE;
							GetScrollInfo(media.window,SB_VERT,&si);
							if ((UINT)si.nMax>=si.nPage) ShowScrollBar(media.window,SB_VERT,TRUE);
							break;
						case 1: //DVE
							EnableWindow(fxobject.window,TRUE);
							EnableWindow(media.window,FALSE);
							EnableWindow(filtersourceobj.window,FALSE);
							EnableWindow(audiosourceobj.window,FALSE);
							BringWindowToTop(media.window);
							BringWindowToTop(filtersourceobj.window);
							BringWindowToTop(audiosourceobj.window);
							InvalidateRect(fxobject.window,NULL,TRUE);
							ShowScrollBar(fxobject.window,SB_VERT,FALSE);
							si.cbSize=sizeof(SCROLLINFO);
							si.fMask=SIF_PAGE|SIF_RANGE;
							GetScrollInfo(fxobject.window,SB_VERT,&si);
							if ((UINT)si.nMax>=si.nPage) ShowScrollBar(fxobject.window,SB_VERT,TRUE);
							break;
						case 2: //Filters
							EnableWindow(filtersourceobj.window,TRUE);
							EnableWindow(audiosourceobj.window,FALSE);
							EnableWindow(fxobject.window,FALSE);
							EnableWindow(media.window,FALSE);
							BringWindowToTop(media.window);
							BringWindowToTop(fxobject.window);
							BringWindowToTop(audiosourceobj.window);
							InvalidateRect(filtersourceobj.window,NULL,TRUE);
							ShowScrollBar(filtersourceobj.window,SB_VERT,FALSE);
							si.cbSize=sizeof(SCROLLINFO);
							si.fMask=SIF_PAGE|SIF_RANGE;
							GetScrollInfo(filtersourceobj.window,SB_VERT,&si);
							if ((UINT)si.nMax>=si.nPage) ShowScrollBar(filtersourceobj.window,SB_VERT,TRUE);
							ShowWindow(filters->window,TRUE);
							filters->updateimagelist();
							break;
						case 3: //Audio
							EnableWindow(audiosourceobj.window,TRUE);
							EnableWindow(filtersourceobj.window,FALSE);
							EnableWindow(fxobject.window,FALSE);
							EnableWindow(media.window,FALSE);
							BringWindowToTop(media.window);
							BringWindowToTop(fxobject.window);
							BringWindowToTop(filtersourceobj.window);
							InvalidateRect(audiosourceobj.window,NULL,TRUE);
							ShowScrollBar(audiosourceobj.window,SB_VERT,FALSE);
							si.cbSize=sizeof(SCROLLINFO);
							si.fMask=SIF_PAGE|SIF_RANGE;
							GetScrollInfo(audiosourceobj.window,SB_VERT,&si);
							if ((UINT)si.nMax>=si.nPage) ShowScrollBar(audiosourceobj.window,SB_VERT,TRUE);
							ShowWindow(audio->window,TRUE);
							audio->updateimagelist();
							break;
						}
					break;
					}
				}
			break; 
			} //end WM_NOTIFY

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


storysourceclass::storysourceclass() {
	smartrefreshclip=NULL;
	}

void storysourceclass::createwindow(class storyboardclass *storyboard,HWND w_ptr) {
	TC_ITEM item;
	window=CreateWindowEx(0,"OBJWIN","",WS_CHILD|
		WS_VISIBLE,200,200,235,260,w_ptr,
		(HMENU)IDC_TABSET1,hInst,NULL);

	tabwindow=CreateWindowEx(0,WC_TABCONTROL,"",WS_CHILD|
		WS_VISIBLE,0,0,235,260,window,
		(HMENU)IDC_TABSET1,hInst,NULL);

	SendMessage(tabwindow,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);
	item.mask=TCIF_TEXT;
	item.pszText="Media";
	TabCtrl_InsertItem(tabwindow,0,&item);
	item.pszText=" DVE";
	TabCtrl_InsertItem(tabwindow,1,&item);
	item.pszText="Filters";
	TabCtrl_InsertItem(tabwindow,2,&item);
	item.pszText="Audio";
	TabCtrl_InsertItem(tabwindow,3,&item);
	SetWindowLong(window,GWL_USERDATA,(long)this);

	media.createwindow(storyboard,tabwindow);
	fxobject.createwindow(storyboard,tabwindow);
	filtersourceobj.createwindow(storyboard,tabwindow);
	audiosourceobj.createwindow(storyboard,tabwindow);
	}


void storysourceclass::resize() {
	RECT rc;
	size100(screen,window,0,0,27,65);
	GetClientRect(window,&rc);
	SetWindowPos(tabwindow,NULL,0,0,
		rc.right,rc.bottom,SWP_NOACTIVATE|SWP_NOZORDER);

	/*
	smartrefreshclip=createclipwin(storywindow,(HBRUSH)(COLOR_WINDOW+1));
	*/
	media.resize(tabwindow);
	fxobject.resize(tabwindow);
	filtersourceobj.resize(tabwindow);
	audiosourceobj.resize(tabwindow);
	}


void storysourceclass::startup() {
	media.startup();
	fxobject.startup();
	filtersourceobj.startup();
	audiosourceobj.startup();
	//Put the proper tab window on the bottom by placing the others on top
	EnableWindow(fxobject.window,FALSE);
	EnableWindow(filtersourceobj.window,FALSE);
	EnableWindow(audiosourceobj.window,FALSE);
	BringWindowToTop(fxobject.window);
	BringWindowToTop(filtersourceobj.window);
	BringWindowToTop(audiosourceobj.window);
	}


void storysourceclass::shutdown() {
	/*
	if (smartrefreshclip) DeleteObject(smartrefreshclip);
	*/
	media.shutdown();
	fxobject.shutdown();
	filtersourceobj.shutdown();
	audiosourceobj.shutdown();
	//Must destroy the tabs window before postquitmessage does
	//Since the instance of the class is deleted
	DestroyWindow(window);
	}


storysourceclass::~storysourceclass() {
	//nothing to clean yet...
	}


//mediawindow implementation
//This class is a type of source to put into the storyboard
//So it uses the base class storysource.



//* * * This group initializes the media class * * *
storysourceclass::mediaclass::mediaclass() : storysourcebase() {
	}

void storysourceclass::mediaclass::createwindow(class storyboardclass *storyboard,HWND w_ptr) {
	//original 100 vals 201 204 235 259
	window=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_VSCROLL|
		WS_CHILD,200,200,200,100,w_ptr,
		(HMENU)IDC_MEDIAWINDOW,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	id=id_media;
	initdirlist("\\Imports");
	/*
	dirlisthead=(struct cachedir *)newnode(medianodeobject,sizeof(struct cachedir));
	dirlisthead->dirpath="Imports";
	dirlisthead->next=dirlisthead->prev=NULL;
	//Set our currentdir var to the initial path
	strcpy(string,"V:\\VideoMedia");
	SetCurrentDirectory(string);
	GetCurrentDirectory(MAX_PATH,currentdir);
	*/
	imagelisthead=scrollpos=cachemanager(storyboard,currentdir);
	}


storysourceclass::mediaclass::~mediaclass() {
	//nothing to clean yet...
	}


storysourceclass::dvesourceclass::dvesourceclass() : storysourcebase() {
	//printc("import construct called");
	}

void storysourceclass::dvesourceclass::createwindow(class storyboardclass *storyboard,HWND w_ptr) {
	//original 100 vals 201 204 235 259
	window=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_VSCROLL|
		WS_CHILD,200,200,200,100,w_ptr,
		(HMENU)IDC_TRANSITIONS,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);

	id=id_dve;
	initdirlist("\\Transitions");
	imagelisthead=scrollpos=cachemanager(storyboard,currentdir);
	}

storysourceclass::dvesourceclass::~dvesourceclass() {
	//nothing to clean yet...
	}

/* * *       FilterSource implementation * * */

storysourceclass::filtersourceclass::filtersourceclass() {
	}


void storysourceclass::filtersourceclass::createwindow(class storyboardclass *storyboard,HWND tabwindow) {
	//Here is the window for our storysource side
	window=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_VSCROLL|
		WS_CHILD,200,200,200,100,tabwindow,
		(HMENU)IDC_FILTERSWINDOW,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	id=id_filter;
	initdirlist("\\Filters");
	imagelisthead=scrollpos=cachemanager(storyboard,currentdir);
	}

storysourceclass::filtersourceclass::~filtersourceclass() {
	}

/* * *       AudioSource implementation * * */

storysourceclass::audiosourceclass::audiosourceclass() {
	}


void storysourceclass::audiosourceclass::createwindow(class storyboardclass *storyboard,HWND tabwindow) {
	//Here is the window for our storysource side
	window=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_VSCROLL|
		WS_CHILD,200,200,200,100,tabwindow,
		(HMENU)IDC_AUDIO,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	id=id_audio;
	initdirlist("\\Audio");
	imagelisthead=scrollpos=cachemanager(storyboard,currentdir);
	}

storysourceclass::audiosourceclass::~audiosourceclass() {
	}
