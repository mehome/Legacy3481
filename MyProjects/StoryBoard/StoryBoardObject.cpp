#include "HandleMain.h"  


/* Yes the Ultimate Story Board Class!! */

LRESULT CALLBACK handleerrorfix(HWND req_ptr,UINT message,WPARAM wParam,LPARAM lParam) {
	class storyboardclass *storyboard=getstoryboard();
	struct imagelist *imageindex=controls->streamptr;
	BITMAPINFOHEADER *bitmapinfo;
	HBITMAP hbm;
	char filenamepath[MAX_PATH];
	static imageidentifier id=id_error;
	UWORD totalframescheck;

	switch (message) {
		case WM_INITDIALOG:
			SetWindowText(GetDlgItem(req_ptr,IDC_EDIT1),imageindex->filesource);
			return (TRUE);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_EFasl:
					//TODO figure out 2nd parm since I added filename for compatiblity with newer upgrade
					if (getopenfile(filenamepath,NULL,NULL,NULL,"Please select correct filename.",NULL,1))
						SetWindowText(GetDlgItem(req_ptr,IDC_EDIT1),filenamepath);
					break;
				case IDC_EFmedia:
					//printc("IDC_EFmedia");
					id=id_media;
					break;
				case IDC_EFdve:
					//printc("IDC_EFdve");
					id=id_dve;
					break;
				case IDOK:
					//Change the filesource
					{
					int index=0;
					int temp,length;

					GetWindowText(GetDlgItem(req_ptr,IDC_EDIT1),string,255);
					if (imageindex->text) disposenode(nodeobject,(struct memlist *)imageindex->text);
					if (imageindex->filesource) disposenode(nodeobject,(struct memlist *)imageindex->filesource);
					imageindex->filesource=(char *)newnode(nodeobject,length=(strlen(string)+1));
					strcpy(imageindex->filesource,string);
					//char *text; use string

					for (temp=0;temp<length;temp++) {
						if (string[temp]=='\\') index=temp;
						}
					if (temp) index++;
					imageindex->text=(char *)newnode(nodeobject,strlen(string+index)+1);
					strcpy(imageindex->text,string+index);
					}

					hbm=medialoaders->openthumb(imageindex,&totalframescheck,&bitmapinfo);

					if (hbm==NULL) printc("Warning unable to open %s",imageindex->text);
					else {
						project->copythumbtoimage(imageindex,hbm,bitmapinfo);
						imageindex->id=id;
						}

					//Close Media even if it was error
					medialoaders->closethumb(imageindex,bitmapinfo);
					if (hbm) storyboard->premediafx(imageindex);
					storyboard->updateimagelist();
					storyboard->resetframecounter();

				case IDCANCEL:
					//DestroyWindow(About);
					EndDialog(req_ptr, TRUE);
					return (TRUE);
				}
			break;
		}
   return (0L); 
	}

int storyboardclass::Callback(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
	{

	//LPDRAWITEMSTRUCT lpdis;
	//struct tagPOINT mouseloc;

	switch(uMsg) {

		case WM_MEASUREITEM:
			if (debug) printc("MeasureItem");
			return (TRUE);

		case WM_NCHITTEST: {
			//Draw our Focus insert here if the drag is on
			struct tagPOINT mouseloc;
			struct imagelist *index,*tail;
			RECT frameglow;
			//short item;
			short x,xcoord,y,ycoord;
			short xdragoffset;
			BOOL DragNotToStory;

			if (dragthis->dragimage) {
				if ((dragthis->dragimage->id==id_media)||(dragthis->dragimage->id==id_dve))
					DragNotToStory=FALSE;
				else DragNotToStory=TRUE;
				}
			else DragNotToStory=FALSE;

			if ((pointermode==dragdrop)&&((!DragNotToStory)||(!dragthis->dragtoggle))) {
				GetCursorPos(&mouseloc);
				GetWindowRect(storywindow,&frameglow);
				// x=4 y=22 if windowframe is on
				xcoord=mouseloc.x-(short)frameglow.left-4;
				ycoord=mouseloc.y-(short)frameglow.top-4;

				if (dragthis->dragtoggle) {
					x=(xcoord-sb_offsetx+HALFXBITMAP)/XBITMAP;
					xdragoffset=sb_offsetx-HALFXBITMAP;
					}
				else {
					x=(xcoord-sb_offsetx)/XBITMAP;
					xdragoffset=sb_offsetx;
					}
				y=(ycoord-sb_offsety)/YBITMAP;

				if ((y>=numrows)||(xcoord<sb_offsetx)||(ycoord<sb_offsety)) {
					newitem=-1;
					if (!(olditem==-1)) makeglow(oldx,oldy,xdragoffset,sb_offsety);
					olditem=-1;
					}
				else {
					newitem=getitem(x,y,&index,&tail);

					if (dragthis->dragtoggle) {
				
						if ((!index)&&(x<=numcolumns)&&(y<numrows)) {
							x=(xcoord-sb_offsetx)/XBITMAP;
							newitem=getitem(x,y,&index,&tail);
							if (newitem==-1) {
								x=(xcoord-sb_offsetx+HALFXBITMAP)/XBITMAP;
								goto itsouttarange;
								}
							else {
								if (index) {x++;newitem++;}
								//Here don't move over if its at the end
								if ((!freezeglow)&&(!(x==numcolumns))) {
									if (!(olditem==-1)) makeglow(oldx,oldy,sb_offsetx-HALFXBITMAP,sb_offsety);
									makeglow(x,y,sb_offsetx,sb_offsety);
									freezeglow=TRUE;
									oldx=x;oldy=y;
									olditem=-1;
									}
								}
							}
						else {
							itsouttarange:
							if (x>numcolumns) newitem=-1;
							if (freezeglow) {
								freezeglow=FALSE;
								makeglow(oldx,oldy,sb_offsetx,sb_offsety);
								//print("<-end");
								}
							}
						}
			
					else { //normal non dragtoggle mode
						if ((x>=numcolumns)||(xcoord<sb_offsetx)||(!index)) newitem=-1;
						freezeglow=FALSE;
						}

					//wsprintf(string,"i%d,oi%d,im%d ",newitem,olditem,freezeglow);printc(string);
					if ((!(newitem==olditem))&&(!freezeglow)) {
						if (!(olditem==-1)) makeglow(oldx,oldy,xdragoffset,sb_offsety);
						if (!(newitem==-1)) makeglow (x,y,xdragoffset,sb_offsety);
						}
					}	//end else it's in the grid range
				if (!freezeglow) {olditem=newitem;oldx=x;oldy=y;}
				} //end if dragdrop mode
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
			} //end WM_NCHITTEST:


		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(storywindow,&ps);
			smartrefresh(ps.hdc,smartrefreshclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(storywindow,&ps);
			return (0);
			}

		case WM_COMMAND:
			if (debug) printc("Storyboard WM_COMMAND");
			switch(LOWORD(wParam)) {
				case IDM_STORYFILTERS:
					if (IsWindowVisible(filters->window)==0) windowtoggle(screen,filters->window,IDM_FILTERS);
					filters->updateimagelist();
					break;
				case IDM_STORYAUDIO:
					if (IsWindowVisible(audio->window)==0) windowtoggle(screen,audio->window,IDM_AUDIO);
					audio->updateimagelist();
					break;
				case IDM_STORYDVE:
					controls->streamptr->DVEprefs->openprefs();
					break;
				case IDM_STORYERROR:
					printc("Fixing Error");
					DialogBox(hInst,(LPCTSTR)IDD_ERRORFIX,w_ptr,DLGPROC(handleerrorfix));
					break;
				case KEY_DELETE: {
					SCROLLINFO si;
					struct indeximage *multiindex,*indexnext;
					if (multiindex=selectimageobj->indexhead) {
						do {
							indexnext=multiindex->next;
							removeimage(multiindex->imageptr,TRUE);
							} while(multiindex=indexnext);
						selectimageobj->resetlist(FALSE);
						resetframecounter();
						adjustscrollbar();
						updateimagelist();
						controls->initframecounter(this);
						si.cbSize=sizeof(SCROLLINFO);
						si.fMask=SIF_POS;
						GetScrollInfo(storywindow,SB_VERT,&si);
						si.nPos=0;
						SetScrollInfo(storywindow,SB_VERT,&si,TRUE);
						}
					break;
					}
				}
			break;

		case WM_LBUTTONUP: {
			struct imagelist *index,*tail,*dragptr;
			short item;
			UWORD t=0;

			if (!(dragthis->dragtoggle)) {
				if (pointermode==dragdrop) {
					if (w_ptr==dragthis->dragwindow) {
						dragptr=dragthis->dragimage;
						//turn off the glow
						if (!(newitem==-1)) {
							makeglow(oldx,oldy,sb_offsetx-HALFXBITMAP,sb_offsety);
							if (debug==2) {wsprintf(string,"MouseUp->%d",olditem);printc(string);}
							//insert dragged image in imagelist
							//Find Image place for insertion
							if (imagelisthead) item=getitem(oldx,oldy,&index,&tail);
							else index=tail=NULL;

linkattail:
							//Here is where all images get linked to Storyboard imagelist
							if (dragptr->selected) {
								struct indeximage *multiindex=dragptr->idobj->selectimageobj->indexhead;
								struct imagelist *dupimage;
								if (multiindex) {
									do {
										dupimage=duplicateimage(multiindex->imageptr);
										insertimage(dupimage,index,tail);
										if (dragptr->id==id_dve) if (index) if (index->next) index=index->next;
										if (index==0) tail=dupimage;
										} while(multiindex=multiindex->next);
									//reset the storysource list
									dragptr->idobj->selectimageobj->resetlist(TRUE);
									dragptr->idobj->updateimagelist();
									//the actual dragptr needs to be disposed
									if (dragthis->dragimage) {
										disposenode(nodeobject,(struct memlist *)dragthis->dragimage);
										dragthis->dragimage=NULL;
										}
									resetframecounter();
									}
								}
							else {
								//The dragimage becomes the linked image have to set to NULL to avoid
								//storysource from erasing the memory
								dragthis->dragimage=NULL;
								if (insertimage(dragptr,index,tail)) return(0);
								}

							//Remove old image now if there is one to remove
							if (removethis) {
								removeimage(removethis,0);
								removethis=NULL;
								}

							} //newitem has been inserted
						//else the object needs to be discarded
						else {
							RECT rc;
							struct tagPOINT mouseloc;

							//Remove old image now if there is one to remove
							if (removethis) {
								removeimage(removethis,0);
								removethis=NULL;
								}
							//all this figuring window region should be removed
							//Once I replace the dragclass with a sprite
							GetCursorPos(&mouseloc);
							GetWindowRect(storywindow,&rc);

							//Note this will delete the object if it is dragged from storyboard
							//As the origin from storysource would result in a add to last
							//These images do not have HalfImages
							//RemoveImage for the dragimage will not close the image (andcloseit=0) 
							//because all those contents were moved to the dragptr, 
							//so at this point we finally close all the internals

							//bug note ||(mouseloc.x<rc.left) need not be in here because this
							//callback should never get called if we select a storysource object
							//and drag it to its same window

							if (dragthis->dragorigin!=IDC_MEDIAWINDOW) closeimage(dragthis->dragimage);

							else {
								// Here's a nice little trick this simulates as if
								// We actually put the image at the last position
								// By explicitly finding the tail then going back
								// to the area which does the linking its nice to
								// know that the else's skip to where we need to return
								// Long live the GOTO! :)

								tail=index=NULL;
								if (imagelisthead) {
									tail=imagelisthead; //TODO set to imagelisttail if we implement it 
									while ((tail->next)) {
										tail=tail->next;
										}
									}
								goto linkattail;
								} //end else if dragorigin from media

							} // end else if newitem==-1
						adjustscrollbar();
						updateimagelist();
						} //if w_ptr == dragwindow
					} //end if dragdrop mode
				else { //For Now Selection mode is the remainder of all other modes
					struct tagPOINT mouseloc;
					RECT frameglow;
					UWORD x,y;

					if (imagelisthead) {
						GetCursorPos(&mouseloc);
						GetWindowRect(storywindow,&frameglow);
						x=(mouseloc.x-(short)frameglow.left-4-sb_offsetx)/XBITMAP;
						y=(mouseloc.y-(short)frameglow.top-4-sb_offsety)/YBITMAP;
						y+=scrolloffset;
						item=x+numcolumns*y;
						controls->adjustglow(this,item);
						tools.updatetoolbar(controls->streamptr);

						//Multiselect
						x=(LOWORD(lParam)-sb_offsetx)/XBITMAP;
						y=(HIWORD(lParam)-sb_offsety)/YBITMAP;
						item=getitem(x,y,&index,&tail);
						if (index) {
							if (!((wParam&MK_CONTROL)||(wParam&MK_SHIFT))) {
								selectimageobj->resetlist(TRUE);
								if (index->id!=id_dir) {
									selectimageobj->selectimage(index);
									updateimagelist();
									}
								}
							}
						//end multiselect
						}
					}// end Selectmode
				}//end dragtoggle on
			return(0L);
			}

		case WM_LBUTTONDOWN: {
			struct imagelist *index,*tail;
			UWORD x=(LOWORD(lParam)-sb_offsetx)/XBITMAP;
			UWORD y=(HIWORD(lParam)-sb_offsety)/YBITMAP;
			short item;
			long frameadjust=0;

			if (x>=numcolumns||y>=numrows) item=-1;
			else {
					item=getitem(x,y,&index,&tail);
				if (pointermode==dragdrop) {
					if (debug==2) {wsprintf(string,"MouseDown->%d",item);printc(string);}
					if (index) {
						//turn off the glow
						if (!(olditem==-1)) {
							makeglow(oldx,oldy,sb_offsetx,sb_offsety);
							olditem=-1;
							}

						//Grab item put on in drag window;
						//From all the other possibilities left, it is safe
						//To assume we grab The image to be pulled
						dragthis->updateimage(duplicateimage(index),IDC_STORYBOARD);
						//Delete old image and turn existing image to black
						if (index->image) DeleteObject(index->image);
						index->image=fillerimage;
						if (dragthis->dragimage->id==id_media) frameadjust-=dragthis->dragimage->actualframes;
						else frameadjust+=dragthis->dragimage->duration;
						controls->adjustframecounter(this,dragthis->dragimage->linenumber,frameadjust,-1);
						removethis=index;
						updateimagelist();
						}
					} //end if dragdrop mode
				else {
					if ((wParam&MK_CONTROL)&&(index)) {
						if (index->id!=id_dir) {
							selectimageobj->selectimage(index);
							updateimagelist();
							}
						}

					else if ((wParam&MK_SHIFT)&&(index)) {
						if (index->id!=id_dir) {
							selectimageobj->shiftselect(index);
							updateimagelist();
							}
						}
					}
				} //end valid item 
			return (0);
			} /* End WM_LBUTTONDOWN*/

		case WM_SIZE:
			if (debug) printc("WM_SIZE");
			break;

		case WM_CLOSE:
			windowtoggle(screen,storywindow,IDM_STORYBOARD);
			return(0L);

		case WM_VSCROLL: {
			SCROLLINFO si;
			int oldpos,difference,t;

			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_POS|SIF_RANGE;
			GetScrollInfo(storywindow,SB_VERT,&si);
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
			SetScrollInfo(storywindow,SB_VERT,&si,TRUE);
				difference=si.nPos-oldpos;
			if (difference>0) {
				//wsprintf(string,"difference=%ld",difference);printc(string);
				while (difference--) {
					t=numcolumns;
					while (t--) {
						if (scrollpos->next) scrollpos=scrollpos->next;
						else break;
						}
					}
				}
			else if (difference<0) {
				//wsprintf(string,"difference=%ld",difference);printc(string);
				while (difference++) {
					t=numcolumns;
					while (t--) {
						if (scrollpos->prev) scrollpos=scrollpos->prev;
						else break;
						}
					}
				}
			scrolloffset=si.nPos;
			updateimagelist();
			return(0L);
			} //end WM_VSCROLL

		case WM_CONTEXTMENU: {
			HMENU hMenu;
			struct tagPOINT mouseloc;
			RECT frameglow;
			struct imagelist *streamptr;
			UWORD x,y;
			short item;

			if (imagelisthead) {
				GetCursorPos(&mouseloc);
				GetWindowRect(storywindow,&frameglow);
				x=(mouseloc.x-(short)frameglow.left-4-sb_offsetx)/XBITMAP;
				y=(mouseloc.y-(short)frameglow.top-4-sb_offsety)/YBITMAP;
				item=x+numcolumns*y;
				if (pointermode==dragdrop) makeglow(x,y,sb_offsetx,sb_offsety);
				controls->adjustglow(this,item);
				if (pointermode==dragdrop) makeglow(x,y,sb_offsetx,sb_offsety);
				}
			if (streamptr=controls->streamptr) {
				hMenu = CreatePopupMenu();

				AppendMenu( hMenu, MFT_STRING, IDM_STORYFILTERS, "&Filters..." );
				AppendMenu( hMenu, MFT_STRING, IDM_STORYAUDIO, "&Audio..." );
				if (streamptr->id==id_dve) AppendMenu( hMenu, MFT_STRING, IDM_STORYDVE, "&DVE..." );
				if (streamptr->id==id_error) AppendMenu (hMenu,MFT_STRING, IDM_STORYERROR, "F&ix DVE/Media...");

				TrackPopupMenu(hMenu,TPM_RIGHTBUTTON|TPM_TOPALIGN|TPM_LEFTALIGN,
					LOWORD(lParam),HIWORD(lParam),0,storywindow,NULL); 
				DestroyMenu(hMenu);
				}
			break;
			}

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}


void storyboardclass::resetframecounter() {
	struct imagelist *index;
	long framecount=0;
	if (index=imagelisthead) {
		do {
			if ((index->id==id_media)||(index->id==id_error)) framecount+=index->actualframes;
			else if (index->id==id_dve) {
				if (index->prev) {
					if (index->prev->id==id_media) framecount-=index->duration;
					else framecount+=index->duration;
					}
				}
			else printc("Warning: resetframecounter detected unknown media");
			} while (index=index->next);
		}
	framecounter=framecount;
	controls->updateframecounter(this);
	}

void storyboardclass::adjustscrollbar() {
	//Set the maximum items in the scrollbar
	SCROLLINFO si;
	int t;

	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_RANGE|SIF_PAGE|SIF_POS;
	GetScrollInfo(storywindow,SB_VERT,&si);
	si.nPage=(UINT)(numrows);
	if ((imagecount-1)/numcolumns<si.nMax) {
		t=numcolumns;
		while (t--) {
			if (scrollpos) {
				if (scrollpos->prev) scrollpos=scrollpos->prev;
				else break;
				}
			else break;
			}
		si.nPos-=1;
		}
	si.nMax=(imagecount-1)/numcolumns;
	si.nMin=0;
	SetScrollInfo(storywindow,SB_VERT,&si,TRUE);
	}

long storyboardclass::clipduration(struct imagelist *index,struct imagelist *tail,struct imagelist *dragptr) {
	//Here is where we'll clip the duration if necessary
	long leftduration,actualdur;
	long oldduration=leftduration=dragptr->duration;
	if (index) { 
		if (index->prev) if (index->prev->id==id_media) {
			actualdur=index->prev->actualframes;
			//Now that duration matches the prev media see if there is yet another DVE locked onto it
			if (index->prev->prev) if (index->prev->prev->id==id_dve)
				actualdur-=index->prev->prev->duration;
			if (!actualdur) {
				printc("The media to the left of this insertion point is saturated by the DVE to the left of it");
				printc("To place a DVE here you must lower the duration of that DVE");
				return(-1);
				}
			if (actualdur<dragptr->duration) leftduration=actualdur;
			}
		if (index->id==id_media) {
			actualdur=index->actualframes;
			if (index->next) if (index->next->id==id_dve)
				actualdur-=index->next->duration;
			if (!actualdur) {
				printc("The media to the right of this insertion point is saturated by the DVE to the right of it");
				printc("To place a DVE here you must lower the duration of that DVE");
				return(-1);
				}
			dragptr->duration=min(leftduration,actualdur);
			}
		}
	else if (tail)	if (tail->id==id_media) {
		actualdur=tail->actualframes;
		if (tail->prev) if (tail->prev->id==id_dve) 
			actualdur-=tail->prev->duration;
		if (!actualdur) {
			printc("The media to the left of this insertion point is saturated by the DVE to the left of it");
			printc("To place a DVE here you must lower the duration of that DVE");
			return(-1);
			}
		if (actualdur<dragptr->duration) dragptr->duration=actualdur;
		}
	return(oldduration-dragptr->duration);
	}

BOOL storyboardclass::insertimage(struct imagelist *dragptr,struct imagelist *index,struct imagelist *tail) {
	struct imagelist *draghead,*dragtail;
	long frameadjust=0;
	float average;
	char add=1; //default if no exceptions pop up, may want to get rid of this

	draghead=dragtail=dragptr;
	imagecount++;
	if (dragthis->dragorigin==IDC_MEDIAWINDOW) premediafx(draghead);

	if (dragptr->id==id_media) frameadjust+=dragptr->actualframes;
	else {
		if (clipduration(index,tail,dragptr)==-1) return(1);
		frameadjust-=dragptr->duration;
		}

	if (imagelisthead) {
		//Link the dragimage to the previous image
		if (index) {
			if (index->prev) {
				index->prev->next=draghead;
				average=index->prev->linenumber;
				}
			else {
				imagelisthead=scrollpos=draghead;
				average=0;
				}
			draghead->prev=index->prev;
			dragtail->next=index;
			index->prev=dragtail;
			average=(average+index->linenumber)/2;
			}
		else { //insert at tail
			index=tail;
			index->next=draghead;
			draghead->prev=index;
			dragtail->next=NULL;
			average=draghead->prev->linenumber+100;
			}
		}

	else { //first node
		controls->streamptr=imagelisthead=imagelisttail=scrollpos=draghead;
		//We should assume scrollpos is imagehead
		//since any scroll offset denies access to the first row
		draghead->prev=NULL;
		dragtail->next=NULL;
		average=100;
		updateglow(0,imagelisthead);
		}

	dragptr->linenumber=average;
	controls->adjustframecounter(this,average,frameadjust,add);
	olditem=-1;
	return(0);
	}

void storyboardclass::closeimage(struct imagelist *index) {
	postmediafx(index);
	if (index->image) DeleteObject(index->image);
	if (index->filesource) disposenode(nodeobject,(struct memlist *)index->filesource);
	if (index->text) disposenode(nodeobject,(struct memlist *)index->text);
	if (index->DVEprefs) delete (index->DVEprefs);
	//filters and audio will check for NULL
	filters->closefilters(index);
	audio->closeaudio(index);
	disposenode(nodeobject,(struct memlist *)index);
	}

void storyboardclass::removeimage(struct imagelist *index,BOOL andcloseit) {
	//RemoveImage only takes image out of node and disposes the node, unless the andclose it
	//is true... this is because the dragimage will preserve all the internals when copying
	//the image... the andcloseit flag is implemented for the multiselect enhancement

	while (controls->updatevideorequest) Sleep(1);
	medialoaders->flushusedmedia(index);

	if (index->prev) index->prev->next=index->next;
	//We should assume scrollpos is imagehead
	//since any scroll offset denies access to the first row
	else {
		imagelisthead=scrollpos=index->next;
		if (imagelisthead) {
			controls->updatevideoparm=imagelisthead;
			controls->updatevideorequest++;
			SetEvent(arrayofevents[EVENT_VIDEO]);
			}
		}
	if (index->next) index->next->prev=index->prev;
	if (controls->streamptr==index) controls->initframecounter(this);
	imagecount--;
	if (andcloseit) closeimage(index);
	else 	disposenode(nodeobject,(struct memlist *)index);
	}


short storyboardclass::getitem(
	UWORD x,UWORD y,struct imagelist **index,struct imagelist **tail)
	{
	UWORD t=0;
	short item=x+numcolumns*y;

	*index=scrollpos;

	while ((t<item)&&(*index)) {
		*tail=*index;
		*index=(*index)->next;
		t++;
		}
	//wsprintf(string,"index=%lx,t=%lx",*index,t);printc(string);
	if (*index) {
		*tail=NULL;
		return (item);
		}
	else {
		if (item==t) return (t);
		else {*tail=NULL; return (-1);}
		}
	}


void storyboardclass::makeglow(UWORD x,UWORD y,short offsetx,UWORD offsety) {
	HDC hdc,smartdc;
	RECT frameglow;
	hdc=GetDC(storywindow);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,smartrefreshclip);
	frameglow.left = x*XBITMAP+offsetx;
	frameglow.top = y*YBITMAP+offsety; 
	frameglow.right = x*XBITMAP + XBITMAP+offsetx;
	frameglow.bottom = y*YBITMAP + YBITMAP+offsety; 
	DrawFocusRect(smartdc,&frameglow);
	InvalidateRect(storywindow,&frameglow,FALSE);
	DeleteDC(smartdc);
	ReleaseDC(storywindow,hdc);
	}


void storyboardclass::closeproject() {
	struct imagelist *index=imagelisthead;
	struct imagelist *indexnext;
	SCROLLINFO si;

	medialoaders->shutdown();
	if (index) {
		do {
			indexnext=index->next;
			closeimage(index);
/*
			if (index->image) DeleteObject(index->image);
			postmediafx(index);
			if (index->filesource) disposenode(nodeobject,(struct memlist *)index->filesource);
			if (index->text) disposenode(nodeobject,(struct memlist *)index->text);
			//From closefilters the function will check for NULL
			filters->closefilters(index);
*/
			} while (index=indexnext);
		imagelisthead=scrollpos=NULL;
		controls->initframecounter(this);
		framecounter=0;
		controls->adjustframecounter(this,100,0,FALSE);
		controls->updateframesdisplay(this);
		controls->clearframeclip();
		imagecount=0;

		//Clear Scrollbar
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_PAGE|SIF_RANGE|SIF_POS;
		GetScrollInfo(storywindow,SB_VERT,&si);
		si.nMax=0;
		si.nMin=0;
		si.nPage=si.nMax;
		si.nPos=si.nMin;
		SetScrollInfo(storywindow,SB_VERT,&si,TRUE);

		updateimagelist();
		}
	}


void storyboardclass::premediafx(struct imagelist *index) {
	//determine whether media or fx and switch the filter ext
	if (index->id==id_dve) {
		/*
		switch(dve->mediatype) {
			}
		*/
		//DVEprefs should already be set in projects
		if (index->DVEprefs==0) index->DVEprefs=(class generalFX *)(new alphaFX(index));
		} //end if id=dve
	else medialoaders->beginmedia(index);
	} //end premediafx


void storyboardclass::postmediafx(struct imagelist *index) {
	if (index->id==id_dve) {
		delete index->DVEprefs;
		index->DVEprefs=NULL;
		}
	else if (index->id==id_media) medialoaders->endmedia(index);
	}


void storyboardclass::updateimagelist() {
	struct imagelist *index;
	HDC hdc,hmem,smartdc;
	UWORD xoffset,yoffset;
	RECT rc;
	UBYTE t=0;

	hdc=GetDC(storywindow);
	hmem=CreateCompatibleDC(hdc);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,smartrefreshclip);
	//Clear Area for fragmented endrow
	GetClientRect(storywindow,&rc);
	FillRect(smartdc,&rc,(HBRUSH)(COLOR_WINDOW+1));

	index=scrollpos;
	
	while ((t<numcolumns*numrows)&&(index)) {
		yoffset=YBITMAP*(t/numcolumns)+sb_offsety;
		xoffset=XBITMAP*(t%numcolumns)+sb_offsetx;
		/*wsprintf(string,"x->%d,y->%d",xoffset,yoffset);printc(string);*/
		if (index->image)	SelectObject(hmem,index->image);
		else SelectObject(hmem,storysourcebase::errorimage);
	 	BitBlt(smartdc,xoffset+1,yoffset+1,XBITMAP,YBITMAP,hmem,0,0,index->selected?MERGEPAINT:SRCCOPY);
		index=index->next;
		t++;
		}

	DeleteDC(smartdc);
	DeleteDC(hmem);
	ReleaseDC(storywindow,hdc);
	InvalidateRgn(storywindow,NULL,FALSE);
	}


void storyboardclass::storyboardtools::updatetoolbar(struct imagelist *streamptr) {
	//Now put the contents of the Media into the IOD textboxes
	if (streamptr->id==id_media) {
		wsprintf(string,"%d",streamptr->cropin);
		SetWindowText(inwindow,string);
		EnableWindow(inwindow,TRUE);
		wsprintf(string,"%d",streamptr->cropout);
		SetWindowText(outwindow,string);
		EnableWindow(outwindow,TRUE);
		wsprintf(string,"%d",streamptr->actualframes);
		SetWindowText(durwindow,string);
		EnableWindow(durwindow,TRUE);
		}
	else if (streamptr->id==id_dve) {
		wsprintf(string,"%d",streamptr->duration);
		SetWindowText(durwindow,string);
		EnableWindow(inwindow,FALSE);
		EnableWindow(outwindow,FALSE);
		EnableWindow(durwindow,TRUE);
		}
	}


void storyboardclass::updateglow(UWORD glowpos,struct imagelist *streamptr) {
	struct audionode *audioindex;
	HDC hdc,hmem,smartdc;
	UWORD xoffset,yoffset;
	RECT rc;

	if (!IsIconic(screen)) {
		hdc=GetDC(storywindow);
		hmem=CreateCompatibleDC(hdc);
		smartdc=CreateCompatibleDC(hdc);
		SelectObject(smartdc,smartrefreshclip);

		//Clear Area for fragmented endrow
		if (oldstreamptr) {
			yoffset=YBITMAP*(oldglowpos/numcolumns-scrolloffset)+sb_offsety-1;
			xoffset=XBITMAP*(oldglowpos%numcolumns)+sb_offsetx-1;
			rc.left=xoffset;
			rc.top=yoffset;
			rc.right=rc.left+XBITMAP+2;
			rc.bottom=rc.top+YBITMAP+2;
			FillRect(smartdc,&rc,(HBRUSH)(COLOR_WINDOW+1));
			if (oldstreamptr->image) SelectObject(hmem,oldstreamptr->image);
			else SelectObject(hmem,storysourcebase::errorimage);

			BitBlt(smartdc,xoffset+2,yoffset+2,XBITMAP,YBITMAP,hmem,0,0,oldstreamptr->selected?MERGEPAINT:SRCCOPY);
			InvalidateRect(storywindow,&rc,FALSE);
			}

		yoffset=YBITMAP*(glowpos/numcolumns-scrolloffset)+sb_offsety-1;
		xoffset=XBITMAP*(glowpos%numcolumns)+sb_offsetx-1;
		// Now lets alphablend the index pointer
		SelectObject(hmem, glowbmp);
		//AlphaBlend(smartdc,xoffset,yoffset,XBITMAP+1,YBITMAP+1,hmem,43,85,XBITMAP+1,YBITMAP+1,bf);
		AlphaBlend(smartdc,xoffset,yoffset,XBITMAP+1,YBITMAP+1,hmem,1,1,XBITMAP+1,YBITMAP+1,bf);
		DeleteDC(smartdc);
		DeleteDC(hmem);
		ReleaseDC(storywindow,hdc);
		rc.left=xoffset;
		rc.top=yoffset;
		rc.right=rc.left+XBITMAP+2;
		rc.bottom=rc.top+YBITMAP+2;
		InvalidateRect(storywindow,&rc,FALSE);
		oldglowpos=glowpos;
		oldstreamptr=streamptr;
		//tools.updatetoolbar(streamptr);  
		//This one was commented out to make dual stream faster
		//unfortunately a separate thread does not work either.
		//idlesignals|=IDLE_TOOLBAR;
		//SetEvent(arrayofevents[EVENT_IDLE]);

		filters->updateimagelist();
		audio->updateimagelist();
		} //end is screen window minimized
	//here is where we open all audio layers for this media
	if (streamptr->prev) if (streamptr->prev->id==id_dve) goto alreadyqueuedsound;
	if (audioindex=streamptr->audio) {
		do {
			if (((struct wavinfo *)audioindex)->hfile==0) 
				if (audio->player.openmedia((struct wavinfo *)audioindex))
					audio->addvoicetoplay((struct wavinfo *)audioindex,streamptr);
			} while (audioindex=audioindex->next);
		}
	//Toggle the stream offset if our new media is a DVE
	if (streamptr->id==id_dve) {
		medialoaders->toggleimagestream=!medialoaders->toggleimagestream;
		//while we are at it check next media for audio
		if (streamptr->next) {
			if (audioindex=streamptr->next->audio) {
				do {
					if (((struct wavinfo *)audioindex)->hfile==0) 
						if (audio->player.openmedia((struct wavinfo *)audioindex))
							audio->addvoicetoplay((struct wavinfo *)audioindex,streamptr);
					} while (audioindex=audioindex->next);
				}
			}
		}
alreadyqueuedsound:;
	} //end update glow position

