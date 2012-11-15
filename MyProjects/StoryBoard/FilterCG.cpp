#include "HandleMain.h"
#include "textedit.h"

typedef filtersclass::CGclass CGfilter;
const long RGB1[2]={0xb5817062,0x0000da1d};
const long RGB2[2]={0x7fff1917,0x000041cb};
const long RGB3[2]={0xa1bcedd3,0x00007062};
const long addoffsets[2]={0x00100080,0x00100080};

int filtersclass::CGclass::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	static RECT clipregion={0,0,720,483};
	static POINT oldmouseloc;
	char buffer[24];
	filterCG *filter;
	struct filterswindowlist *CGwindowptr;

	switch(uMsg) {

		case WM_MOUSEMOVE: {
			POINT mouseloc;
			if (CGtoggle) {
				filter=(struct filterCG *)filters->getnode(w_ptr,&CGwindowptr);
				GetCursorPos(&mouseloc);
				itoa(mouseloc.x-360,string,10);
				SetWindowText((((struct CGgui *)CGwindowptr)->x),string);
				itoa(mouseloc.y-242,string,10);
				SetWindowText((((struct CGgui *)CGwindowptr)->y),string);
				}
			break;
			}

		case WM_HSCROLL:
			nostaticpreview=0;
			filter=(struct filterCG *)filters->getnode(w_ptr,&CGwindowptr);
			switch LOWORD(wParam) {
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_THUMBTRACK:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
					itoa(SendMessage((((struct CGgui *)CGwindowptr)->depthscrub),TBM_GETPOS,0,0),string,10);
					SetWindowText((((struct CGgui *)CGwindowptr)->depthed),string);
					return(0);
				case TB_BOTTOM:
					itoa(255,string,10);
					SetWindowText((((struct CGgui *)CGwindowptr)->depthed),string);
					return(0);
				case TB_TOP:
					itoa(0,string,10);
					SetWindowText((((struct CGgui *)CGwindowptr)->depthed),string);
					return(0);
				default: goto noprocessHSCROLL;
				}
			return(0);
noprocessHSCROLL:
			break;

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			struct CGgui *CGwindowindex;
			if ((filter=(struct filterCG *)filters->getnode(w_ptr,&CGwindowptr))==0) {
				printc("Internal Link Error: filter controls will not work now until reset");
				break;
				}
			CGwindowindex=(struct CGgui *)CGwindowptr;
			struct imagelist *streamptr=controls->streamptr;

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case CG_CLOSE:
							goto closefilter;
						case CG_XY:
							//printc("CG mousecapture enabled");
							nostaticxypreview=0;
							GetCursorPos(&oldmouseloc);
							clipregion.left=360-(filter->width>>1);
							clipregion.top=242-(filter->height>>1);
							ClipCursor(&clipregion);
							SetCursorPos(360+(filter->x>>2),242+filter->y);
							CGtoggle=TRUE;
							SetCapture(w_ptr);
							ShowCursor(FALSE);
							if (toaster->rtmeoutput) toaster->resync();
							break;
						case CG_XP:
							GetWindowText(filters->miniscrubobj->mediatimeedit,buffer,6);
							SetWindowText(CGwindowindex->inwindow,buffer);
							break;
						case CG_YP:
							GetWindowText(filters->miniscrubobj->mediatimeedit,buffer,6);
							SetWindowText(CGwindowindex->outwindow,buffer);
							break;
						case CG_DEPTHRCHK: {
							initdepthrange(filter,CGwindowindex);
							goto updatevideodisplay;
							}
						case CG_DEPTHRBT: {
							if (!(CGwindowindex->depthred)) {
								struct textinput textparms={&CGwindowindex->depthred,&filter->thicktext,filter->name,10,80,250,250,w_ptr,hInst,NULL};
								CGwindowindex->depthred=new textedit(&textparms);
								}
							CGwindowindex->depthred->setwindowtext(makedepthtext(filter));
							break;
							}
						case CG_DEPTHPLOT: {
							if (filter->thickrange==0) initdepthrange(filter,CGwindowindex);
							adddepthpoint(filter,(UWORD)controls->mediatimer,filter->thickness);
							updatedepthrange(filter,streamptr);
							if (CGwindowindex->depthred) {
								if (IsWindowVisible(CGwindowindex->depthred->window)) CGwindowindex->depthred->setwindowtext(makedepthtext(filter));
								}
							goto updatevideodisplay;
							}
						case CG_XYRCHK: {
							initxyrange(filter,CGwindowindex);
							goto updatevideodisplay;
							}
						case CG_XYRBT: {
							if (!(CGwindowindex->xyred)) {
								struct textinput textparms={&CGwindowindex->xyred,&filter->xytext,filter->name,10,80,250,250,w_ptr,hInst,NULL};
								CGwindowindex->xyred=new textedit(&textparms);
								}
							CGwindowindex->xyred->setwindowtext(makexytext(filter));
							break;
							}
						case CG_XYPLOT: {
							if ((SendMessage(CGwindowindex->xyrchk,BM_GETCHECK,0,0))==BST_UNCHECKED) initxyrange(filter,CGwindowindex);
							addxypoint(filter,(UWORD)controls->mediatimer,filter->x,filter->y);
							updatexyrange(filter,streamptr);
							if (CGwindowindex->xyred) {
								if (IsWindowVisible(CGwindowindex->xyred->window)) CGwindowindex->xyred->setwindowtext(makexytext(filter));
								}
							goto updatevideodisplay;
							}
						default: goto noprocessCOMMAND;
						}
					return(0);
					} //end id bn clicked

				case EN_UPDATE: {
					long source=0;
					long dest=0;

					if (GetWindowText((HWND)lParam,buffer,6)) {
						source=atol(buffer);
						if (debug==2) {
							wsprintf(string,"EN_UPDATE %d",source);
							printc(string);
							}
						switch (buttonid) {
							case CG_IN:	
								dest=filter->in;
								if ((source>=0)&&(source!=dest)&&
									(source<streamptr->totalframes)) {
									if (debug) printc("cropin Changed");
									filter->in=(UWORD)source;
									//Move up cropout if it is smaller
									if (filter->out<=source) {
										filter->out=min(streamptr->actualframes+source,streamptr->totalframes);
										itoa(filter->out,string,10);
										SetWindowText(CGwindowindex->outwindow,string);
										}
									itoa(filter->out-source,string,10);
									SetWindowText(CGwindowindex->durwindow,string);
									}
								break;	
							case CG_OUT:
								dest=filter->out;
								if ((source>0)&&(source!=dest)&&
									(source>filter->in)) {
									if (debug) printc("cropout Changed");
									if	(source>streamptr->totalframes) {
										source=streamptr->totalframes;
										wsprintf(string,"The maximum frames for this media is %ld",source);
										printc(string);
										}
									filter->out=(UWORD)source;
									itoa(source-filter->in,string,10);
									SetWindowText(CGwindowindex->durwindow,string);
									//Manually set the out to the highest value
									if (source==streamptr->totalframes) {
										itoa(source,string,10);
										SetWindowText((HWND)lParam,string);
										}
									}
								break;
							case CG_DUR:
								//Setting actualframes and cropout
								dest=filter->out-filter->in;
								if ((source>0)&&(source!=dest)) {
									if (debug) printc("actualframes Changed");
									if	(source+filter->in>streamptr->totalframes) {
										source=streamptr->totalframes-filter->in;
										wsprintf(string,"With \"In:\" set to %ld, the maximum duration for this media is %ld",filter->in,source);
										printc(string);
										}
									filter->out=(UWORD)(source+filter->in);
									itoa(source+filter->in,string,10);
									SetWindowText(CGwindowindex->outwindow,string);
									//Manually set the out to the highest value
									if (source==streamptr->totalframes-filter->in) {
										itoa(source,string,10);
										SetWindowText((HWND)lParam,string);
										}
									}
								break;
							case CG_X:
								if ((source<359)&&(source-1>-(filter->width>>1))) filter->x=(UWORD)source<<2;
								break;
							case CG_Y:
								if ((source<=240)&&(source>-(filter->height>>1))) filter->y=(UWORD)source;
								break;
							case CG_DEPTHED:
								if (source<256) filter->thickness=(UBYTE)source;
								break;
							default: goto noprocessCOMMAND;
							} //end switch button id
						} //end if able to get text
					else {
						if (debug) printc("Update error");
						goto noprocessCOMMAND;
						}
					break;
					} // end EN_UPDATE
				default: goto noprocessCOMMAND;
				} //End switch notifycode
			goto updatevideodisplay;
noprocessCOMMAND:
			break;
			} //end WM_COMMAND

		case WM_APP:
			if (lParam) {
				struct CGgui *CGwindowindex;
				filter=(struct filterCG *)filters->getnode(w_ptr,&CGwindowptr);
				CGwindowindex=(struct CGgui *)CGwindowptr;
				switch (wParam) {
					case EDIT_CLOSE:
						*(((class textedit *)lParam)->reftextobj)=NULL;
						delete (class messagebase *)lParam;
						
						break;
					case EDIT_OK:
					case EDIT_APPLY:
						//figure out which member got applied
						if (((class textedit *)lParam)->reftext==&filter->xytext) {
							if (filter->xytext) {
								xytext2points(filter,controls->streamptr);
								if ((SendMessage(CGwindowindex->xyrchk,BM_GETCHECK,0,0))==BST_UNCHECKED) initxyrange(filter,CGwindowindex);
								updatexyrange(filter,controls->streamptr);
								goto updatevideodisplay;
								}
							}
						else if (((class textedit *)lParam)->reftext==&filter->thicktext) {
							if (filter->thicktext) {
								depthtext2points(filter,controls->streamptr);
								if (filter->thickrange==0) initdepthrange(filter,CGwindowindex);
								updatedepthrange(filter,controls->streamptr);
								goto updatevideodisplay;
								}
							}
						break;
					}
				}
			break;

		case WM_LBUTTONUP: {
			//printc("Filters WM_LBUTTONUP");
			if (CGtoggle) {
				//printc("CG mousecapture off");
				ReleaseCapture();
				ClipCursor(NULL);
				CGtoggle=FALSE;
				SetCursorPos(oldmouseloc.x,oldmouseloc.y);
				ShowCursor(TRUE);
				}
			return (0);
			}  // end LButton Up

		case WM_LBUTTONDOWN: {
			//printc("Filters WM_LBUTTONDOWN");
			return (0);
			}  // end LButton Down

		case WM_SIZE:
			//TODO configure size
			printc("CGsize");
			return (0);

		case WM_CLOSE: {
closefilter:
			struct filternode *filterprev,*filternext;
			if (debug) printc("CloseCG");
			filter=(struct filterCG *)filters->getnode(w_ptr,&CGwindowptr);
			closefilter((struct filternode *)filter);
			//remove filter from link
			filterprev=filter->node.prev;
			filternext=filter->node.next;
			if (filterprev) {
				filterprev->next=filternext;
				if (filterprev->prev==NULL) controls->streamptr->mediafilter=filterprev;
				}
			else controls->streamptr->mediafilter=filternext;
			if (filternext) filternext->prev=filterprev;
			disposenode(nodeobject,(struct memlist *)filter);
			//updategui
			filters->updateimagelist();
			//updatedisplay
			controls->updatevideoparm=controls->streamptr;
			controls->updatevideorequest++;
			SetEvent(arrayofevents[EVENT_VIDEO]);
			return(0L);
			} //end WM_CLOSE 

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
							hdcMem,((lpdis->CtlID)-203)*SKINGRIDX+1,SKINGRIDY*3+1,SRCCOPY); 
	
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
						hdcMem,((lpdis->CtlID)-203)*SKINGRIDX+1,SKINGRIDY*2+1,SRCCOPY); 
	
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

updatevideodisplay:
	//updatedisplay
	if (toaster->rtmeoutput) controls->updatevideonow();
	else {
		controls->updatevideoparm=controls->streamptr;
		controls->updatevideorequest++;
		SetEvent(arrayofevents[EVENT_VIDEO]);
		}
	return(0L);
	}


filtersclass::CGclass::CGclass() {
	CGtoggle=FALSE;
	windowlistsize=sizeof(struct CGgui);
	}
filtersclass::CGclass::~CGclass() {
	}


struct filternode *filtersclass::getnode(HWND w_ptr,struct filterswindowlist **CGwindowindex) {
	struct imagelist *streamptr=controls->streamptr;
	filternode *filterindex=NULL;
	*CGwindowindex=filters->windowlist;

	if ((streamptr)&&(*CGwindowindex)) {
		filterindex=streamptr->mediafilter;
		}
	else return(0);

	//Take w_ptr compare against CGwindowlist to get our node

	if (filterindex) {
		do {
			//See if we have any windows created already
			if (*CGwindowindex) {
				if ((*CGwindowindex)->window==w_ptr) break;
				else *CGwindowindex=(*CGwindowindex)->next;
				}
			} while (filterindex=filterindex->next);
		}

	return (filterindex);
	}


void filtersclass::CGclass::closefilter(struct filternode *filter) {
	struct filterCG *filterindex=(struct filterCG *)filter;
	if (filterindex) {
		if (filterindex->alpha) {
			dispose((struct memlist *)filterindex->alpha,&pmem);
			filterindex->alpha=NULL;
			}
		if (filterindex->yuv) {
			dispose((struct memlist *)filterindex->yuv,&pmem);
			filterindex->yuv=NULL;
			}
		if (filterindex->node.nodeobject) {
			killnode(filterindex->node.nodeobject,&pmem);
			filterindex->node.nodeobject=NULL;
			}
		filterindex->filesource=NULL;
		filterindex->name=NULL;
		filterindex->thickpointstail=filterindex->thickpointshead=NULL;
		filterindex->thickrange=NULL;
		filterindex->thicktext=NULL;
		filterindex->xypointshead=filterindex->xypointshead=NULL;
		filterindex->xrange=filterindex->yrange=NULL;
		filterindex->xytext=NULL;
		}
	}


void filtersclass::CGclass::initcontrols(struct filterswindowlist *CGwindowptr,struct filternode *filterptr) {
	struct CGgui *CGwindowindex=(struct CGgui *)CGwindowptr;
	struct filterCG  *filterindex=(struct filterCG  *)filterptr;
	//struct imagelist *streamptr=controls->streamptr;
	if (filterindex->name) SetWindowText(((struct CGgui *)CGwindowptr)->caption,filterindex->name);

	wsprintf(string,"%d",filterindex->in);
	SetWindowText(CGwindowindex->inwindow,string);
	wsprintf(string,"%d",filterindex->out);
	SetWindowText(CGwindowindex->outwindow,string);
	wsprintf(string,"%d",filterindex->out-filterindex->in);
	SetWindowText(CGwindowindex->durwindow,string);
	wsprintf(string,"%d",filterindex->x/4);
	SetWindowText(CGwindowindex->x,string);
	wsprintf(string,"%d",filterindex->y);
	SetWindowText(CGwindowindex->y,string);
	wsprintf(string,"%d",filterindex->thickness);
	SetWindowText(CGwindowindex->depthed,string);
	SendMessage(CGwindowindex->depthscrub,TBM_SETPOS,TRUE,filterindex->thickness);
	if (filterindex->thickrange) SendMessage(CGwindowindex->depthrchk,BM_SETCHECK,BST_CHECKED,0);
	else SendMessage(CGwindowindex->depthrchk,BM_SETCHECK,BST_UNCHECKED,0);
	if ((filterindex->xrange)||(filterindex->yrange)) SendMessage(CGwindowindex->xyrchk,BM_SETCHECK,BST_CHECKED,0);
	else SendMessage(CGwindowindex->xyrchk,BM_SETCHECK,BST_UNCHECKED,0);
	}


HWND filtersclass::CGclass::getnewgui(struct filterswindowlist *CGwindowptr,HWND filterwindow,ULONG count) {
	//Gui stuff
	struct CGgui *CGwindowindex=(struct CGgui *)CGwindowptr;
	HWND maincontrol;
	RECT rc;
	int toolx=0;
	int tooly=0;

	GetClientRect(filterwindow,&rc);

	//Init all windows that will be opened later
	maincontrol=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_DLGFRAME|
		WS_CHILD,0,0,300,124,filterwindow,NULL,hInst,NULL);

	//TODO move resize to separate function for handler to call
	//size100notyorh(filterwindow,maincontrol,0,124*count+CONTROLBUTTONY,100,124);
	SetWindowLong(maincontrol,GWL_USERDATA,(long)this);

	CGwindowindex->windownode.y=124;
	CGwindowindex->caption=CreateWindowEx(0,"STATIC",NULL,WS_VISIBLE|WS_CHILD,
		toolx+8,tooly,rc.right-40,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);

	getcommonCGgui(CGwindowindex,filterwindow,maincontrol,0);

	return (maincontrol);
	}


ULONG *filtersclass::CGclass::opentga(char *filesource,UBYTE **alpha,UWORD *width,UWORD *height) {
	char *decodedbits=NULL;
	char *source=NULL;
	long size=0;
	int x=0;
	int y=0;
	if (!(source=(char *)(load(filesource,&size,&pmem)))) goto error;
	if (!(decodedbits=medialoaders->tgaobj.TGA2raw(source,size,(UWORD *)(&x),(UWORD *)(&y)))) goto error;
	dispose((struct memlist *)source,&pmem);
	return(alphayuv(decodedbits,x,y,FALSE,alpha,width,height));

error:
	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	if (source) dispose((struct memlist *)source,&pmem);
	return (NULL);
	}


struct filterCG  *filtersclass::CGclass::initfilterCG(struct imagelist *dragimage,struct imagelist *streamptr) {
	struct filterCG  *mediafilterCG;
	struct filternode *tail;
	UINT actualrange;
	//First allocate our filter node
	mediafilterCG=(struct filterCG  *)newnode(nodeobject,sizeof(struct filterCG ));

	mediafilterCG->node.nodeobject=createnode(&pmem,1024,0);
	//Link in
	if (streamptr->mediafilter) {
		//always insert at tail pretty simple
		tail=streamptr->mediafilter;
		while (tail->next) tail=tail->next;
		tail->next=(struct filternode *)mediafilterCG;
		mediafilterCG->node.prev=tail;
		}
	else {
		streamptr->mediafilter=(struct filternode *)mediafilterCG;
		mediafilterCG->node.prev=NULL;
		}
	mediafilterCG->node.next=NULL;

	//Fill in node
	mediafilterCG->node.filtertype=ft_CG;
	mediafilterCG->filesource=(char *)newnode(mediafilterCG->node.nodeobject,strlen(dragimage->filesource)+1);
	strcpy(mediafilterCG->filesource,dragimage->filesource);
	mediafilterCG->name=(char *)newnode(mediafilterCG->node.nodeobject,strlen(dragimage->text)+1);
	strcpy(mediafilterCG->name,dragimage->text);
	//set defaults
	mediafilterCG->x=0;
	mediafilterCG->y=0;
	mediafilterCG->in=0;
	actualrange=streamptr->actualframes;
	if (streamptr->id==id_media) {
		if (streamptr->prev) if (streamptr->prev->id==id_dve)
		actualrange-=streamptr->prev->actualframes;
		if (streamptr->next) if (streamptr->next->id==id_dve)
		actualrange-=streamptr->next->actualframes;
		}
	mediafilterCG->out=actualrange;
	mediafilterCG->thickness=128;
	mediafilterCG->thickrange=NULL;
	mediafilterCG->thickpointshead=mediafilterCG->thickpointstail=NULL;
	mediafilterCG->thicktext=NULL;
	mediafilterCG->xrange=mediafilterCG->yrange=NULL;
	mediafilterCG->xypointshead=mediafilterCG->xypointstail=NULL;
	mediafilterCG->xytext=NULL;
	mediafilterCG->yuv=opentga(dragimage->filesource,&mediafilterCG->alpha,&mediafilterCG->width,&mediafilterCG->height);
	return(mediafilterCG);
	}

