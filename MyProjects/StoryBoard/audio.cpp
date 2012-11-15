//TODO take out wavs (much later) disposenode(nodeobject,(struct memlist *)wavobj);
//Use this for close project as well as manual

#include "HandleMain.h"
#include "audio.h"


void audioclass::closeaudio(struct imagelist *media) {
	if (media) {
		struct audionode *index,*indexnext;
		//for now we only support the uncompressed PCM wav format so when we add more we'll 
		//have to decide whethed to switch an audiotype or virtually close a generic close
		if (index=media->audio) {
			do {
				indexnext=index->next;
				audioguiobj.closeaudio((struct wavinfo *)index);
				disposenode(nodeobject,(struct memlist *)index);
				} while (index=indexnext);
			}
		}
	}


/**/
void audioclass::updateimagelist() {
	ULONG count,windowlistsize;
	struct imagelist *streamptr;
	audionode *audioindex;
	struct filterswindowlist *audiowindowprev;
	struct filterswindowlist *audiowindowindex;
	ULONG mediatimer;

	if (IsWindowVisible(window)) {
		//init
		count=0;
		streamptr=controls->streamptr;
		audioindex=NULL;
		audiowindowprev=NULL;
		audiowindowindex=windowlist;
		mediatimer=controls->mediatimer;

		if (streamptr) {
			wsprintf(string,"Audio - %s",streamptr->text);
			SetWindowText(window,string);
			miniscrubobj->initminiscrub();
			audioindex=streamptr->audio;
			}
		if (audioindex) {
			do {
				//TODO switch audiotype here
				//case wav
				//Grab full windowsize
				windowlistsize=audioguiobj.getwindowlistsize();
				//See if we have any windows created already
				if (windowlist) {
					if (audiowindowindex) {
						ShowWindow(audiowindowindex->window,SW_SHOW);
						audiowindowprev=audiowindowindex;
						audiowindowindex=audiowindowindex->next;
						}
					else {
						audiowindowindex=(struct filterswindowlist *)newnode(nodeobject,windowlistsize);
						audiowindowindex->window=audioguiobj.getnewgui(audiowindowindex,window,count);
						//Now link to previous if exist
						audiowindowindex->prev=audiowindowprev;
						audiowindowindex->next=NULL;
						if (audiowindowprev) {
							audiowindowprev->next=audiowindowindex;
							}
						audiowindowprev=audiowindowindex;
						audiowindowindex=NULL;
						}
					}
				else {//first node
					audiowindowprev=windowlist=(struct filterswindowlist *)newnode(nodeobject,windowlistsize);
					windowlist->next=windowlist->prev=NULL;
					windowlist->window=audioguiobj.getnewgui(windowlist,window,count);
					}
				audioguiobj.initcontrols(audiowindowprev,audioindex);
				//end case CG
				//End switch audiotype
				//Set controls to window
				count++;
				} while (audioindex=audioindex->next);
			}
		//Finally Hide all remaining windows for each audiotype created
		if (audiowindowindex) {
			do ShowWindow(audiowindowindex->window,SW_HIDE); while (audiowindowindex=audiowindowindex->next);
			}
		} //end if window is visible
	} //end updateimagelist
/**/

int audioclass::Callback(HWND w_ptr,
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

		case WM_LBUTTONUP: {
			if (w_ptr==dragthis->dragwindow) {
				if (controls->streamptr) {
					//printc("Filters WM_LBUTTONUP");
					//ensure that were dragging something in
					if (debug) {
						wsprintf(string,"Stream=%lx %s",controls->streamptr,controls->streamptr->text);
						printc(string);
						}
					//TODO parse filter type here and call the appropriate filter
					addwavtomedia(controls->streamptr,dragthis->dragimage->filesource);
					//update our gui
					updateimagelist();
					} // end if streamptr
				} //end if we are dragging a filter in
			return (0);
			}  // end LButton Up

		case WM_TIMER:
			player.streamwav();
			return(0);

		case WM_CLOSE:
			windowtoggle(screen,window,IDM_AUDIO);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}


audioclass::audioclass() {
	window=NULL;
	windowlist=NULL;
	playqueuehead=playqueuetail=NULL;
	}


audioclass::~audioclass() {
	player.shutdown();
	}


void audioclass::createwindow(HWND w_ptr) {
	window=CreateWindowEx(0,"OBJWIN","Audio",
		WS_POPUP|WS_OVERLAPPEDWINDOW,235,270,300,300,w_ptr,
		NULL,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(long)this);
	SetTimer(window,IDC_AUDIO,1000,0);
	}

void audioclass::startup () {
	struct miniscrubinput miniparms={0,0,window,hInst};
	miniscrubobj=new miniscrub(&miniparms);
	player.startup();
	}

void audioclass::shutdown() {
	delete miniscrubobj;
	player.shutdown();
	}

void audioclass::resize() {
	size100(screen,window,55,20,92,80);
	}

struct wavinfo *audioclass::addwavtomedia (struct imagelist *streamptr,char *filename) {
	struct audionode *wavobj,*tail;
	int pathoffset,temp,length;
	//First allocate our filter node
	wavobj=(struct audionode *)newnode(nodeobject,sizeof(struct wavinfo));

	//Link in
	if (streamptr->audio) {
		//always insert at tail pretty simple
		tail=streamptr->audio;
		while (tail->next) tail=tail->next;
		tail->next=wavobj;
		wavobj->prev=tail;
		}
	else {
		streamptr->audio=wavobj;
		wavobj->prev=NULL;
		}
	wavobj->next=NULL;

	//Fill in node
	length=strlen(filename);
	strcpy(((struct wavinfo *)wavobj)->filesource=(char *)newnode(nodeobject,length+1),filename);
	//calculate pathoffset
	//char *text; use string
	pathoffset=0;
	for (temp=0;temp<length;temp++) {
		if (filename[temp]=='\\') pathoffset=temp;
		}
	if (temp) pathoffset++;
	strcpy(((struct wavinfo *)wavobj)->name=(char *)newnode(nodeobject,strlen(filename+pathoffset)+1),filename+pathoffset);
	((struct wavinfo *)wavobj)->frameoffset=0;
	return ((struct wavinfo *)wavobj);
	}


void audioclass::addvoicetoplay(struct wavinfo *wavobj,struct imagelist *media) {
	struct wavvoicelist *voiceindex,*voice;
	//link new point node
	if (wavobj) {
		voice=(struct wavvoicelist *)newnode(nodeobject,sizeof(struct wavvoicelist));
		voice->voice=wavobj;
		voice->media=media;
		EnterCriticalSection(&csglobal);
		if (voiceindex=playqueuetail) {
			voice->prev=voiceindex;
			voice->next=NULL;
			voiceindex->next=voice;
			playqueuetail=voice;
			}
		else { //firstnode
			playqueuetail=playqueuehead=voice;
			voice->prev=voice->next=NULL;
			}
		LeaveCriticalSection(&csglobal);
		}
	}


void audioclass::remvoicefromplay(struct wavvoicelist *voice) {
	EnterCriticalSection(&csglobal);
	if (voice->prev) voice->prev->next=voice->next;
	else playqueuehead=voice->next;
	if (voice->next) voice->next->prev=voice->prev;
	else playqueuetail=voice->prev;
	LeaveCriticalSection(&csglobal);
	disposenode(nodeobject,(struct memlist *)voice);
	}
