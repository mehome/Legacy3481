#include "HandleMain.h"  
#include "selectimage.h"

/* * *       StorySource implementation * * */
int storysourcebase::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	if ((w_ptr==window)&&(IsWindowEnabled(window)))	
		return (handlestorysource(w_ptr,uMsg,wParam,lParam));

	if ((w_ptr==toolwindow)&&(IsWindowEnabled(window))) 
		return (handletools(w_ptr,uMsg,wParam,lParam));

	return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
	}

int storysourcebase::handlestorysource(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{
	class storyboardclass *storyboard=getstoryboard();
	
	switch(uMsg) {

		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(window,&ps);
			smartrefresh(ps.hdc,smartrefreshclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(window,&ps);
			return (0);
			}

		case WM_LBUTTONUP:
			if (!(dragthis->dragtoggle)) {
				if (dragthis->dragimage) {
					disposenode(nodeobject,(struct memlist *)dragthis->dragimage);
					dragthis->dragimage=NULL;
					}
				}

			if (imageindex) {
				if (!((wParam&MK_CONTROL)||(wParam&MK_SHIFT))) {
					selectimageobj->resetlist(TRUE);
					if (imageindex->id!=id_dir) {
						selectimageobj->selectimage(imageindex);
						updateimagelist();
						}
					}
				}
			break;

		case WM_LBUTTONDOWN: {
			if (!(dragthis->dragtoggle)) {
				//Draw our Focus insert here if the drag is on
				struct tagPOINT mouseloc;
				RECT frameglow;
				short newitem=-1;
				UWORD xcoord,y,ycoord;
				UWORD t=0;

				GetCursorPos(&mouseloc);
				GetWindowRect(window,&frameglow);
				// x=4 y=22 if windowframe is on
				xcoord=mouseloc.x-(short)frameglow.left;
				ycoord=mouseloc.y-(short)frameglow.top;
		
				newitem=(short)(y=(ycoord-MD_OFFSETY)/(MD_YSIZE));
				//Check to see if there is an image in here
				imageindex=scrollpos;
				while ((imageindex)&&(t<newitem)) {
					t++;
					imageindex=imageindex->next;
					}

				if ((wParam&MK_CONTROL)&&(imageindex)) {
					if (imageindex->id!=id_dir) {
						selectimageobj->selectimage(imageindex);
						updateimagelist();
						struct imagelist *dupimage=duplicateimage(imageindex);
						dupimage->selected=TRUE;
						dragthis->updateimage(dupimage,IDC_MEDIAWINDOW);
						}
					}

				else if ((wParam&MK_SHIFT)&&(imageindex)) {
					if (imageindex->id!=id_dir) {
						selectimageobj->shiftselect(imageindex);
						updateimagelist();
						struct imagelist *dupimage=duplicateimage(imageindex);
						dupimage->selected=TRUE;
						dragthis->updateimage(dupimage,IDC_MEDIAWINDOW);
						}
					}

				else {
					if (
						(y>=numrows)||(ycoord<MD_OFFSETY)||(!imageindex)||
						(xcoord<MD_OFFSETX)||
						(xcoord>(MD_XSIZE)+MD_OFFSETX)
						) newitem=-1;
					if (debug==2) {wsprintf(string,"i%d,oi%d,y%d,yc%d",newitem,olditem,y,ycoord);printc(string);}
					if (!(newitem==olditem)) {
						if (!(olditem==-1)) makeglow(oldy);
						if (!(newitem==-1)) makeglow (y);
						}
					if (!(newitem==-1)) {
						if ((imageindex->id!=id_dir)&&(imageindex->id!=id_error)) 
							dragthis->updateimage(duplicateimage(imageindex),IDC_MEDIAWINDOW);
						}
					olditem=newitem;oldy=y;
					} // end lbutton down only no control or shift
				} // end if not dragtoggle
			return (0);
			}  // end LButton Down

		case WM_LBUTTONDBLCLK: {
			if (imageindex->id==id_dir) {
				SetCurrentDirectory(currentdir);
				if (SetCurrentDirectory(imageindex->text)) {
/*
					changedir(storyboard,imageindex->text);
					updateimagelist();
*/
					IdleChangeDirParm1=this;
					idlesignals|=IDLE_CHANGEDIR;
					strcpy(IdleChangeDirParm2,imageindex->text);
					SetEvent(arrayofevents[EVENT_IDLE]);
					}
				else printc("Invalid Directory.");
				}
			return (0);
			}

		case WM_NCHITTEST: {
			if (!(storyboard->pointermode==dragdrop)) {
				storyboard->pointermode=dragdrop;
				InvalidateRect(storyboard->tools.toolwindow,NULL,FALSE);
				}
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
			} //end WM_NCHITTEST:

		case WM_CLOSE:
			windowtoggle(screen,window,0);
			return(0L);

		case WM_VSCROLL: {
			SCROLLINFO si;
			int oldpos,difference;

			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_POS|SIF_RANGE;
			GetScrollInfo(window,SB_VERT,&si);
			oldpos=si.nPos;
			switch (LOWORD(wParam)) {
				case SB_LINEDOWN: //Scrolls one line down. 
					if (si.nPos+(numrows-1)<si.nMax) si.nPos+=1;
					break;
				case SB_LINEUP: //Scrolls one line up. 
					if (si.nPos>0) si.nPos-=1;
					break;
				case SB_PAGEDOWN: //Scrolls one page down. 
					if (numrows+si.nPos<si.nMax-(numrows-1))
						si.nPos+=numrows;
					else
						si.nPos=si.nMax-(numrows-1);
					break;
				case SB_PAGEUP: //Scrolls one page up. 
					if (numrows<si.nPos)
						si.nPos-=numrows;
					else
						si.nPos=0;
					break;
				case SB_THUMBTRACK: //The user is dragging the scroll box. This message is sent repeatedly until the user releases the mouse button. The nPos parameter indicates the position that the scroll box has been dragged to. 
					si.nPos=HIWORD(wParam);
					break;
				} //End switch scrollcode
			SetScrollInfo(window,SB_VERT,&si,TRUE);
				difference=si.nPos-oldpos;
			if (difference>0) {
				//wsprintf(string,"difference=%ld",difference);printc(string);
				while (difference--) {
					if (scrollpos->next) scrollpos=scrollpos->next;
					}
				}
			else if (difference<0) {
				//wsprintf(string,"difference=%ld",difference);printc(string);
				while (difference++) {
					if (scrollpos->prev) scrollpos=scrollpos->prev;
					}
				}
			updateimagelist();
			return(0L);
			} //end WM_VSCROLL

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


int storysourcebase::handletools(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
	{
	class storyboardclass *storyboard=getstoryboard();

	switch(uMsg) {

	case WM_COMMAND: { //All messages of this type return 0
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {

						case IDC_MEDIAROOT:
							SetCurrentDirectory(currentdir);
							if (SetCurrentDirectory("\\")) {
/*
								changedir(storyboard,string);
								updateimagelist();
*/
								IdleChangeDirParm1=this;
								idlesignals|=IDLE_CHANGEDIR;
								IdleChangeDirParm2[0]=NULL;
								SetEvent(arrayofevents[EVENT_IDLE]);

								}
							//SetFocus(toolwindow);
							break;
						case IDC_MEDIAPARENT:
							SetCurrentDirectory(currentdir);
							if (SetCurrentDirectory("..")) {
/*
								changedir(storyboard,string);
								updateimagelist();
*/
								IdleChangeDirParm1=this;
								idlesignals|=IDLE_CHANGEDIR;
								IdleChangeDirParm2[0]=NULL;
								SetEvent(arrayofevents[EVENT_IDLE]);
								}
							//SetFocus(toolwindow);
							break;
						default: goto skip1;
						} // end switch buttonid
					InvalidateRect(toolwindow,NULL,FALSE);
					return(0);
					} //end case bn clicked
				case CBN_SELCHANGE:
					DWORD selindex;

					selindex=SendMessage(drvspeccombo,CB_GETLBTEXT,
						(WPARAM)(SendMessage(drvspeccombo,CB_GETCURSEL ,0,0)),
						(LPARAM)&string);

					if (selindex>=5) {
						//parse the string for the drive letter only
						string[0]=string[2];
						if (string[3]=='-') string[1]=0;
						else {
							string[1]=string[3];
							string[2]=0;
							}
						strcat(string,":\\");
						//print("CBN_EditChange ");printc(string);
						if (SetCurrentDirectory(string)) {
/*
							changedir(storyboard,string);
							updateimagelist();
*/
							IdleChangeDirParm1=this;
							idlesignals|=IDLE_CHANGEDIR;
							strcpy(IdleChangeDirParm2,string);
							SetEvent(arrayofevents[EVENT_IDLE]);
							}
						}
					else printc("Unable to change to this specified drive");
					//Kill focus
					SetFocus(toolwindow);
					return(0);
				} //end switch notifycode
			skip1:
			break;
			} //case WM_COMMAND

		case WM_DRAWITEM: {
			//RECT rc;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;

			lpdis=(LPDRAWITEMSTRUCT) lParam;
			//printc("WM_DRAWITEM");
			if (lpdis->itemID==(UINT)-1) return(TRUE);
			switch (lpdis->itemAction)  {
				case ODA_SELECT:
					//printc("ODA_SELECT");
				case ODA_DRAWENTIRE:

					/* Is the item selected? */ 
 					if (lpdis->itemState & ODS_SELECTED) { 
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
							hdcMem,((lpdis->CtlID)-TB_IDCMEDIAOFFSET)*SKINGRIDX+1,SKINGRIDY*5+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} //end if selected
					else { //Not selected
						//printc("ODA_DRAWENTIRE");
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
 						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
						hdcMem,((lpdis->CtlID)-TB_IDCMEDIAOFFSET)*SKINGRIDX+1,SKINGRIDY*4+1,SRCCOPY);
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} // end not selected
					break; 
				} // end switch item action
			return (TRUE); 
			}
		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}


void storysourcebase::makeglow(UWORD y) {
	HDC hdc,smartdc;
	RECT frameglow;
	hdc=GetDC(window);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,smartrefreshclip);
	frameglow.left = MD_OFFSETX;
	frameglow.top = (y*(MD_YSIZE))+MD_OFFSETY; 
	frameglow.right = MD_OFFSETX+MD_XSIZE;
	frameglow.bottom = (y*(MD_YSIZE))+MD_OFFSETY + HALFYBITMAP; 
	DrawFocusRect(smartdc,&frameglow);
	InvalidateRect(window,&frameglow,FALSE);
	DeleteDC(smartdc);
	ReleaseDC(window,hdc);
	}


storysourcebase::storysourcebase() {
	medianodeobject=createnode(&pmem,16384,0);
	currentdir=new char[MAX_PATH];
	imagelisthead=scrollpos=imageindex=NULL;
	smartrefreshclip=NULL;
	olditem=-1;
	selectimageobj=new selectimageclass();
	}


void storysourcebase::resize(HWND parent) {
	sizetabchild(window,parent);
	configuresize();
	olditem=-1;
	if (imagelisthead) updateimagelist();
	}


void storysourcebase::configuresize() {
	RECT rc;
	SCROLLINFO si;

	GetClientRect(window,&rc);
	numcolumns=2;
	//Set the pagesize of the scrollbar
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_PAGE;
	GetScrollInfo(window,SB_VERT,&si);
	si.nPage=(UINT)(numrows=(rc.bottom-rc.top-MD_OFFSETY+16)/(MD_YSIZE));
	SetScrollInfo(window,SB_VERT,&si,TRUE);

	md_txt_xsize=rc.right-rc.left-MD_OFFSETX-HALFXBITMAP-MD_TXTOFFSET;
	if (debug) {wsprintf(string,"x->%d,y->%d",MD_XSIZE,numrows);printc(string);}
	}


void storysourcebase::updateimagelist() {
	struct imagelist *index;
	HDC hdc,hmem,smartdc;
	HFONT oldfont;
	UWORD yoffset;
	RECT rc;
	UBYTE y;

	y=0;
	hdc=GetDC(window);
	hmem=CreateCompatibleDC(hdc);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,smartrefreshclip);
	//Clear Area for fragmented endrow
	GetClientRect(window,&rc);
	FillRect(smartdc,&rc,(HBRUSH)(COLOR_MENU+1));
	oldfont=(HFONT)SelectObject(smartdc,mediafont);
	if (index=scrollpos) { //Testing for NULL
		while ((y<numrows)&&(index)) {
			yoffset=(MD_YSIZE)*y+MD_OFFSETY;
			SelectObject(hmem,index->halfimage);
			//wsprintf(string,"y->%d",yoffset);printc(string);
			BitBlt(smartdc,MD_OFFSETX+1,yoffset+1,HALFXBITMAP,HALFYBITMAP,hmem,0,0,index->selected?MERGEPAINT:SRCCOPY);
			//Now to print text
			//TODO Check if there is any text
			if (index->selected) {
				SetTextColor(smartdc,GetSysColor(COLOR_MENU));
				SetBkColor(smartdc,GetSysColor(COLOR_WINDOWTEXT));
				}
			else {
				SetTextColor(smartdc,GetSysColor(COLOR_WINDOWTEXT));
				SetBkColor(smartdc,GetSysColor(COLOR_MENU));
				}
			TextOut (smartdc,HALFXBITMAP+MD_OFFSETX+MD_TXTOFFSET,yoffset+1,index->text,strlen(index->text));
			index=index->next;
			olditem=-1;
			y++;
			}
		}
	else {
		switch (id) {
			case id_media:
				 TextOut (smartdc,MD_OFFSETX,MD_OFFSETY,"<No Media Detected>",19);
				break;
			case id_dve:
				TextOut (smartdc,MD_OFFSETX,MD_OFFSETY,"<No DVE Detected>",17);
				break;
			case id_filter:
				TextOut (smartdc,MD_OFFSETX,MD_OFFSETY,"<No Filters Detected>",21);
				break;
			case id_audio:
				TextOut (smartdc,MD_OFFSETX,MD_OFFSETY,"<No Audio Files Detected>",25);
				break;
			}
		}
	SelectObject(smartdc,oldfont);
	DeleteDC(smartdc);
	DeleteDC(hmem);
	ReleaseDC(window,hdc);
	InvalidateRgn(window,NULL,FALSE);
	}


void storysourcebase::closedirectory(struct imagelist *index) {
	if (index) {
		 do {
			if ((!(index->image==audioimage))&&(index->image)) DeleteObject(index->image);
			if (!((index->halfimage==folder)||(index->halfimage==errorimage)||
				(index->halfimage==audiohalfimage))) DeleteObject(index->halfimage);
			} while (index=index->next);
		}
	imagelisthead=scrollpos=imageindex=NULL;
	}


void storysourcebase::startup() {
	RECT rc;
	DWORD drvindex;

	GetClientRect(window,&rc);
	toolwindow=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|
		WS_DLGFRAME|WS_CHILD,0,0,rc.right-rc.left,TOOLBUTTONY+6,window,
		(HMENU)IDC_MEDIATOOLS,hInst,NULL);

	SetWindowLong(toolwindow,GWL_USERDATA,(long)this);

	//Create the Drive Specifier
   drvspeccombo=CreateWindow("COMBOBOX", "",WS_VISIBLE|
		WS_CHILD|CBS_DROPDOWNLIST|CBS_HASSTRINGS,0,0,50,200,toolwindow,NULL,hInst,NULL);
	drvindex=SendMessage(drvspeccombo,CB_DIR,(WPARAM)DDL_DRIVES|DDL_EXCLUSIVE,(LPARAM)"*");

	//Create the Parent and Root Buttons
	CreateWindowEx(0,"BUTTON","\\",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		50,0,TOOLBUTTONX,TOOLBUTTONY,toolwindow,
		(HMENU)IDC_MEDIAROOT,hInst,NULL);
	CreateWindowEx(0,"BUTTON","..",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		TOOLBUTTONX+50,0,TOOLBUTTONX,TOOLBUTTONY,toolwindow,
		(HMENU)IDC_MEDIAPARENT,hInst,NULL);

	smartrefreshclip=createclipwin(window,(HBRUSH)(COLOR_MENU+1));
	InvalidateRgn(window,NULL,FALSE);
	updateimagelist();
	}


void storysourcebase::shutdown() {
	closedirectory(imagelisthead);
	if (smartrefreshclip) DeleteObject(smartrefreshclip);
	}


storysourcebase::~storysourcebase() {
	delete selectimageobj;
	if (pmem) killnode(medianodeobject,&pmem);
	delete [] currentdir;
	}


void storysourcebase::initdirlist(char *initpath) {
	dirlisthead=(struct cachedir *)newnode(medianodeobject,sizeof(struct cachedir));
	dirlisthead->dirpath="Imports";
	dirlisthead->next=dirlisthead->prev=NULL;
	//Set our currentdir var to the initial path
	//TODO have a prefs file remember last path
	strcpy(string,defaultpath);
	strcat(string,initpath);
	SetCurrentDirectory(string);
	GetCurrentDirectory(MAX_PATH,currentdir);
	}


void storysourcebase::updatefilelist(struct cachedir *dirlist) {
	WIN32_FIND_DATA fi;
	HANDLE lock;
	struct cachefile *fi3;
	struct cachefile *filescan,*xhead,*xtail;
	struct cachefile *filehead,*filetail;
	struct cachefile *dirhead,*dirtail;
	int filecomp;
	char *filename;
	mediatypes media_sniff,filter_sniff,dve_sniff;
	audiosourcetypes audio_sniff;

	//This checks a filelist with the actual directories and updates them
	//by adding and removing the nodes to match
	strcpy(string,dirlist->dirpath);
	strcat(string,"\\*.*");
	lock=FindFirstFile(string,&fi);
	fi3=filehead=filetail=dirhead=dirtail=NULL;
	if ((!lock)||(lock==INVALID_HANDLE_VALUE)) {
		lock=0; //This is stupid should be NULL to begin with
		printc ("Unable to find the specified path");
		}
	else {//Directory path is Valid
		//Stan this is for you, losing the parent & root :)
		if (!(strcmpi(fi.cFileName,"."))) if(!(FindNextFile(lock,&fi))) goto skip_nofilesdetected;
		if (!(strcmpi(fi.cFileName,".."))) if(!(FindNextFile(lock,&fi))) goto skip_nofilesdetected;
		//Like in TermGDCC do a tail insert sort
		do { //iterate through the directory
			if (!(fi.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				switch (id) {
					case id_media:
						if ((media_sniff=getmediatype(fi.cFileName))==media_unknown) continue;
						break;
					case id_dve:
						if ((dve_sniff=getdvetype(fi.cFileName))==media_unknown) continue;
						break;
					case id_filter:
						if ((filter_sniff=getfiltertype(fi.cFileName))==media_unknown) continue;
						break;
					case id_audio:
						if ((audio_sniff=getaudiotype(fi.cFileName))==audio_unknown) continue;
						break;
					}
			//lets make a new node and fill it up
			fi3=(struct cachefile *)newnode(medianodeobject,sizeof(struct cachefile));
			switch (id) {
				case id_media:
					fi3->mediatype=media_sniff;
					break;
				case id_dve:
					fi3->mediatype=dve_sniff;
					break;
				case id_filter:
					fi3->mediatype=filter_sniff;
					break;
				case id_audio:
					fi3->audiotype=audio_sniff;
					break;
				}
			filename=(char *)newnode(medianodeobject,strlen(fi.cFileName)+1);
			strcpy(filename,fi.cFileName);
			fi3->filename=filename;
			fi3->date.dwLowDateTime=fi.ftLastWriteTime.dwLowDateTime;
			fi3->date.dwHighDateTime=fi.ftLastWriteTime.dwHighDateTime;
			fi3->sizeh=fi.nFileSizeHigh;
			fi3->sizel=fi.nFileSizeLow;
			fi3->filenumber=0;
			fi3->filetype=fi.dwFileAttributes;
			//Now lets link it to the existing list
			//start the bottom of the dir or file list
			if (fi3->filetype&FILE_ATTRIBUTE_DIRECTORY) {
				xtail=dirtail;
				xhead=dirhead;
				}
			else {
				xtail=filetail;
				xhead=filehead;
				}
			filescan=xtail;
			if (xtail) { //searching backwards
				//start it to search
				while ((filecomp=strcmpi(fi3->filename,filescan->filename))<1) {
					if (filescan->prev) filescan=filescan->prev;
					else { //fi3 has reached the headnode
						if (filecomp<1) xhead=fi3; //now the new node is the head
						goto breaksearch; //shut down the compare
						}
					} //end the insertsort search
breaksearch:
				//Link the node where filescan stopped
				if ((filescan==xtail)&&(!(xhead==fi3))) { //fi3 is the tail AND NOT THE HEAD!
					xtail=fi3;
					fi3->next=NULL;
					fi3->prev=filescan;
					filescan->next=fi3;
					}
				else {
					if (xhead==fi3) { //fi3 is the head
						fi3->prev=NULL;
						fi3->next=filescan;
						filescan->prev=fi3;
						}
					else { //we can assume fi3 is in the middle
						//so we insert behind filescan
						//this will be hairy :)
						fi3->prev=filescan;
						fi3->next=filescan->next;
						filescan->next->prev=fi3; //Wee Haw
						filescan->next=fi3;
						}
					}
				} //end if xtail
			else { //assign this first node head and tail
				xtail=fi3;
				if (fi3->prev) error(0,"An internal updatefilelist() linking error has been detected.\n  Please call Technical Support");
				else xhead=fi3; //put node at head too
				} //end else filetail				
			//Now reassign the head tail to the proper list
			if (fi3->filetype&FILE_ATTRIBUTE_DIRECTORY) {
				dirtail=xtail;
				dirhead=xhead;
				}
			else {
				filetail=xtail;
				filehead=xhead;
				}

			} while (FindNextFile(lock,&fi));
		//The Directory Examine is now complete
skip_nofilesdetected:
		FindClose(lock);
		//TODO match all cached numbers to list and remove that list
		//link dirs to dirlist
		dirlist->dirlist=dirhead;
		dirlist->filelist=filehead;
		} //end Valid directory path
	}


struct imagelist *storysourcebase::cachemanager(class storyboardclass *storyboard,char *path) {
	struct cachedir *finddir;
	struct cachefile *fi;
	struct imagelist *index;
	struct imagelist *indexhead=NULL;
	struct imagelist *previndex=NULL;
	BITMAPINFOHEADER *bitmapinfo;
	HBITMAP hbm;
	HDC hdc,shmem,dhmem;
	SCROLLINFO si;
	int count=0;
	UWORD totalframes;

	dirlisthead->dirpath=path;
	finddir=dirlisthead; 
	updatefilelist(finddir);

	//and finally make an image list from our finddir files
	if (fi=finddir->dirlist) {
		do {
			index=(struct imagelist *)newnode(medianodeobject,sizeof(struct imagelist));
			clrmem((char *)index,sizeof(struct imagelist));
			index->halfimage=folder;
			index->image=NULL;
			index->filesource=NULL;
			index->text=(char *)newnode(medianodeobject,strlen(fi->filename)+1);
			strcpy(index->text,fi->filename);
			index->totalframes=index->actualframes=0;
			index->id=id_dir;
			index->next=NULL;
			index->prev=previndex;
			if (previndex) previndex->next=index;
			else indexhead=index;
			previndex=index;
			count++;
			} while (fi=fi->next);
		}
	scrollpos=indexhead;

	//put scrollbar at the top
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_POS;
	GetScrollInfo(window,SB_VERT,&si);
	si.nPos=0;
	SetScrollInfo(window,SB_VERT,&si,TRUE);

	if (fi=finddir->filelist) {
		do {
			index=(struct imagelist *)newnode(medianodeobject,sizeof(struct imagelist));
			clrmem((char *)index,sizeof(struct imagelist));
			index->filesource=(char *)newnode(medianodeobject,strlen(fi->filename)+strlen(finddir->dirpath)+2);
			strcpy(index->filesource,finddir->dirpath);
			strcat(index->filesource,"\\");
			strcat(index->filesource,fi->filename);
			index->text=(char *)newnode(medianodeobject,strlen(fi->filename)+1);
			strcpy(index->text,fi->filename);
			index->id=id;
			index->idobj=this;
			index->mediatype=fi->mediatype;
			bitmapinfo=NULL;

			switch (id) {
				case id_media:
				case id_filter:
					//Now for Media use the grabaframe
					hbm=medialoaders->openthumb(index,&totalframes,&bitmapinfo);
					break;
				case id_dve:
					//and for DVE use loadimage for bmp
					//TODO implement sbt image parsing here
					//hbm=(HBITMAP)LoadImage(hInst,index->filesource,IMAGE_BITMAP,VIDEOX,VIDEOY,LR_LOADFROMFILE);
					hbm=medialoaders->openthumb(index,&totalframes,&bitmapinfo);
					//hbm=medialoaders->bmpobj.getthumbbmp(index,&bitmapinfo);
					totalframes=60;
					break;
				case id_audio:
					index->halfimage=audiohalfimage;
					index->image=audioimage;
					totalframes=0;
					goto skipimageblt;
					break;
				default:
					hbm=NULL;
				}

			if (hbm==NULL) {
				index->halfimage=errorimage;
				index->image=NULL;
				index->id=id_error;
				totalframes=60;
				}
			else {
				HPALETTE halftonepalette;
				hdc=GetDC(screen);
				SetBrushOrgEx(hdc,0,0,NULL);
				halftonepalette=CreateHalftonePalette(hdc);
				SelectPalette(hdc,halftonepalette,TRUE);
				RealizePalette(hdc);
				index->halfimage=CreateCompatibleBitmap(hdc,HALFXBITMAP-2,HALFYBITMAP-2);
				index->image=CreateCompatibleBitmap(hdc,XBITMAP-2,YBITMAP-2);
				shmem=CreateCompatibleDC(NULL);
				SelectObject(shmem,hbm);
				dhmem=CreateCompatibleDC(NULL);
				SetStretchBltMode(dhmem,HALFTONE);
				SelectObject(dhmem,index->halfimage);
				StretchBlt(dhmem,0,0,HALFXBITMAP-2,HALFYBITMAP-2,shmem,0,0,bitmapinfo->biWidth,bitmapinfo->biHeight,SRCCOPY);
				SelectObject(dhmem,index->image);
				StretchBlt(dhmem,0,0,XBITMAP-2,YBITMAP-2,shmem,0,0,bitmapinfo->biWidth,bitmapinfo->biHeight,SRCCOPY);
				DeleteDC(dhmem);
				DeleteDC(shmem);
				DeleteObject(halftonepalette);
				ReleaseDC(screen,hdc);
				}

skipimageblt:

			//Close Media even if it was error
			//for those medias that are partially open
			medialoaders->closethumb(index,bitmapinfo);
			/*
			if ((id==id_media)||(id==id_filter)) medialoaders->closethumb(index,bitmapinfo);
			else if ((id==id_dve)&&(hbm)) DeleteObject(hbm);
			*/
			index->totalframes=index->actualframes=totalframes;
			index->next=NULL;
			index->prev=previndex;
			if (previndex) previndex->next=index;
			else indexhead=index;
			previndex=index;
			imagelisthead=scrollpos=indexhead;
			//Set the maximum items in the scrollbar
			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_RANGE|SIF_PAGE;
			GetScrollInfo(window,SB_VERT,&si);
			si.nPage=(UINT)(numrows);
			si.nMax=count++;
			si.nMin=0;
			SetScrollInfo(window,SB_VERT,&si,TRUE);
			updateimagelist();
			//Sleep(500);
			} while (fi=fi->next);
		}
	else {
		//say all of this again to update if there is no media
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_RANGE|SIF_PAGE;
		GetScrollInfo(window,SB_VERT,&si);
		si.nPage=(UINT)(numrows);
		si.nMax=--count;
		si.nMin=0;
		SetScrollInfo(window,SB_VERT,&si,TRUE);
		updateimagelist();
		}
	return (indexhead);
	}


void storysourcebase::changedir(class storyboardclass *storyboard,char *newdir) {
	//Reset multiselect
	selectimageobj->resetlist(FALSE);
	//Change to the new Directory
	if (GetCurrentDirectory(MAX_PATH,currentdir)) {
		//Take out slash if its the last char
		if (currentdir[strlen(currentdir)-1]=='\\') currentdir[strlen(currentdir)-1]=0;
		//First part dispose list
		closedirectory(imagelisthead);
		if (pmem) killnode(medianodeobject,&pmem);
		medianodeobject=createnode(&pmem,16384,0);
		//This is out of cache design but since our heap now disposes
		//the linklist that is part of the cache the cache dirlist
		//will have to be re-initialized
		dirlisthead=(struct cachedir *)newnode(medianodeobject,sizeof(struct cachedir));
		dirlisthead->next=dirlisthead->prev=NULL;

		//no go and read the new directory
		imagelisthead=scrollpos=cachemanager(storyboard,currentdir);
		if (strlen(currentdir)<4) strcat(currentdir,"\\");
		}
	else printc("Unable to lock onto new Directory");
	}

mediatypes storysourcebase::getmediatype(char *filename) {
	transmembyte(filename+strlen(filename)-3,string,3);
	string[3]=0;
	if (!strcmpi(string,"avi")) return(avi);
	if (!strcmpi(string,"bmp")) return(bmp);
	if (!strcmpi(string,"pcx")) return(pcx);
	if (!strcmpi(string,"tif")) return(tif);
	if (!strcmpi(string,"jpg")) return(jpg);
	if (!strcmpi(string,"iff")) return(iff);
	if (!strcmpi(string,"rtv")) return(rtv);
	if (!strcmpi(string,"tga")) return(tga);
	return (media_unknown);
	}//end get media type

mediatypes storysourcebase::getdvetype(char *filename) {
	transmembyte(filename+strlen(filename)-3,string,3);
	string[3]=0;
	if (!strcmpi(string,"sbt")) return(dveplugin);
	if (!strcmpi(string,"bmp")) return(bmp);
	if (!strcmpi(string,"pcx")) return(pcx);
	if (!strcmpi(string,"tif")) return(tif);
	if (!strcmpi(string,"jpg")) return(jpg);
	if (!strcmpi(string,"iff")) return(iff);
	if (!strcmpi(string,"tga")) return(tga);
	return (media_unknown);
	}//end get dve type

//It appears this has to be media to work with the loaders
mediatypes storysourcebase::getfiltertype(char *filename) {
	transmembyte(filename+strlen(filename)-3,string,3);
	string[3]=0;
	if (!strcmpi(string,"sbf")) return(filterplugin);
	if (!strcmpi(string,"tga")) return(tga);
	return (media_unknown);
	}//end get dve type

audiosourcetypes storysourcebase::getaudiotype(char *filename) {
	transmembyte(filename+strlen(filename)-3,string,3);
	string[3]=0;
	if (!strcmpi(string,"wav")) return(wav);
	return (audio_unknown);
	}//end get dve type
