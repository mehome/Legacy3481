#include "HandleMain.h"
#include "textedit.h"

char *filtersclass::CGclass::gettoken(char token,char *textindex,char *textmax) {
	while ((!(*textindex==token))&&(textindex<textmax)) textindex++;
	textindex++;
	if (!(textindex<textmax)) textindex=NULL;
	return(textindex);
	}

char *filtersclass::CGclass::getnumber(char *textindex,char *textmax,int *number) {
	//check for sign
	char *startpos=textindex;
	int t=0;

	*number=0;
	if (*textindex=='-') {
		string[0]='-';
		t++;textindex++;
		}
	while ((*textindex>='0')&&(*textindex<='9')) {
		string[t]=*textindex;
		t++;textindex++;
		}
	if (!(textindex<textmax)) return(0);
	string[t]=0;
	*number=atol(string);
	return(textindex);
	}

void filtersclass::CGclass::depthtext2points(struct filterCG *filter,struct imagelist *media) {
	char *text=filter->thicktext;
	char *textindex=text;
	int size,number;
	char *textmax;
	long mediatime;
	UBYTE trans;
	//No appending text clear any points
	cleardepthpoints(filter);
	if (text) {
		size=strlen(text);
		textmax=text+size;

		do {
			//expect (
			if (!(textindex=gettoken('(',textindex,textmax))) {
				wsprintf(string,"Error in Text; Expecting (");
				goto syntaxerror;
				}
			//Get media time
			textindex=getnumber(textindex,textmax,&number);
			mediatime=(UWORD)number;
			textindex++; //,
			//Get trans
			textindex=getnumber(textindex,textmax,&number);
			trans=(UBYTE)number;
			textindex++; //)
			//check for linefeeds
			while (*textindex==13||*textindex==10) textindex++;
			//Make sure values are within ranges
			if (mediatime>media->totalframes) mediatime=media->totalframes;
			if (mediatime<1) mediatime=1;
			adddepthpoint(filter,mediatime,trans);
			} while(textindex<textmax);
		}
	goto noerror;
syntaxerror:
	printc(string);
noerror:;
	}


void filtersclass::CGclass::adddepthpoint(struct filterCG *filter,ULONG mediatime,UBYTE trans) {
	struct CGpoints *pointindex,*depthpoint;
	//link new point node
	depthpoint=(struct CGpoints *)newnode(filter->node.nodeobject,sizeof(struct CGpoints));
	depthpoint->trans=trans;
	depthpoint->mediatime=mediatime;
	//TODO figure out type
	depthpoint->type=0; //default linear
	if (pointindex=filter->thickpointstail) {
		depthpoint->prev=pointindex;
		depthpoint->next=NULL;
		pointindex->next=depthpoint;
		filter->thickpointstail=depthpoint;
		}
	else { //firstnode
		filter->thickpointstail=filter->thickpointshead=depthpoint;
		depthpoint->prev=depthpoint->next=NULL;
		}
	}


void filtersclass::CGclass::xytext2points(struct filterCG *filter,struct imagelist *media) {
	char *text=filter->xytext;
	char *textindex=text;
	int size,number;
	char *textmax;
	long mediatime;
	short x,y;
	//No appending text clear any points
	clearxypoints(filter);
	if (text) {
		size=strlen(text);
		textmax=text+size;

		do {
			//expect (
			if (!(textindex=gettoken('(',textindex,textmax))) {
				wsprintf(string,"Error in Text; Expecting (");
				goto syntaxerror;
				}
			//Get media time
			textindex=getnumber(textindex,textmax,&number);
			mediatime=(UWORD)number;
			textindex++; //,
			//Get x
			textindex=getnumber(textindex,textmax,&number);
			x=(short)number;
			textindex++; //,
			//Get y
			textindex=getnumber(textindex,textmax,&number);
			y=(short)number;
			textindex++; //)
			//check for linefeed
			while (*textindex==13||*textindex==10) textindex++;
			//Make sure values are within ranges
			if (mediatime>media->totalframes) mediatime=media->totalframes;
			if (mediatime<1) mediatime=1;
			if (x<-358) x=-358;
			else if (x>358) x=358;
			x<<=2;
			if (y<-239) y=-239;
			else if (y>240) y=240;
			addxypoint(filter,mediatime,x,y);
			} while(textindex<textmax);
		}
	goto noerror;
syntaxerror:
	printc(string);
noerror:;
	}


void filtersclass::CGclass::addxypoint(struct filterCG *filter,ULONG mediatime,short x,short y) {
	struct CGxypoints *pointindex,*xypoint;
	//link new point node
	xypoint=(struct CGxypoints *)newnode(filter->node.nodeobject,sizeof(struct CGxypoints));
	xypoint->x=x;
	xypoint->y=y;
	xypoint->mediatime=mediatime;
	//TODO figure out type
	xypoint->type=0; //default linear
	if (pointindex=filter->xypointstail) {
		xypoint->prev=pointindex;
		xypoint->next=NULL;
		pointindex->next=xypoint;
		filter->xypointstail=xypoint;
		}
	else { //firstnode
		filter->xypointstail=filter->xypointshead=xypoint;
		xypoint->prev=xypoint->next=NULL;
		}
	}


void filtersclass::CGclass::cleardepthpoints(struct filterCG *filter) {
	struct CGpoints *pointindex,*pointindexnext;
	if (pointindex=filter->thickpointshead) {
		do {
			pointindexnext=pointindex->next;
			disposenode(filter->node.nodeobject,(struct memlist *)pointindex);
			pointindex=pointindexnext;
			} while(pointindexnext);
		filter->thickpointshead=filter->thickpointstail=NULL;
		}
	}


void filtersclass::CGclass::clearxypoints(struct filterCG *filter) {
	struct CGxypoints *pointindex,*pointindexnext;
	if (pointindex=filter->xypointshead) {
		do {
			pointindexnext=pointindex->next;
			disposenode(filter->node.nodeobject,(struct memlist *)pointindex);
			pointindex=pointindexnext;
			} while(pointindexnext);
		filter->xypointshead=filter->xypointstail=NULL;
		}
	}


char *filtersclass::CGclass::makedepthtext (struct filterCG *filter) {
	int size=0;
	struct CGpoints *pointindex=filter->thickpointshead;
	char *textindex=NULL;
	//gather the size
	if (pointindex) {
		//(5,3)2=14
		do size+=14; while(pointindex=pointindex->next);
		if (filter->thicktext) disposenode(nodeobject,(struct memlist *)filter->thicktext);
		filter->thicktext=textindex=(char *)newnode(nodeobject,size);
		pointindex=filter->thickpointshead;
		do {
			wsprintf(string,"(%d,%d)",pointindex->mediatime,pointindex->trans);
			strcpy(textindex,string);
			textindex+=strlen(string);
			*textindex++=13;
			*textindex++='\n';
			} while(pointindex=pointindex->next);
		}
	else { //no points... make sure there is no text as well
		if (filter->thicktext) {
			disposenode(nodeobject,(struct memlist *)filter->thicktext);
			filter->thicktext=NULL;
			}
		}
	if (textindex) *textindex=0;
	return(filter->thicktext);
	}


char *filtersclass::CGclass::makexytext (struct filterCG *filter) {
	int size=0;
	struct CGxypoints *pointindex=filter->xypointshead;
	char *textindex=NULL;
	//gather the size
	if (pointindex) {
		//(5,3,3)2=18
		do size+=18; while(pointindex=pointindex->next);
		if (filter->xytext) disposenode(nodeobject,(struct memlist *)filter->xytext);
		filter->xytext=textindex=(char *)newnode(nodeobject,size);
		pointindex=filter->xypointshead;
		do {
			wsprintf(string,"(%d,%d,%d)",pointindex->mediatime,pointindex->x>>2,pointindex->y);
			strcpy(textindex,string);
			textindex+=strlen(string);
			*textindex++=13;
			*textindex++='\n';
			} while(pointindex=pointindex->next);
		}
	else { //no points... make sure there is no text as well
		if (filter->xytext) {
			disposenode(nodeobject,(struct memlist *)filter->xytext);
			filter->xytext=NULL;
			}
		}
	if (textindex) *textindex=0;
	return(filter->xytext);
	}

void filtersclass::CGclass::updatexyrange (struct filterCG *filter,struct imagelist *media) {
	struct CGxypoints *pointA,*pointB;
	struct CGxypoints *pointindex=filter->xypointshead;
	short *xrange=filter->xrange;
	short *yrange=filter->yrange;
	short *xrangeindex=xrange;
	short *yrangeindex=yrange;
	int framerangebegin,framerangeend,t,total;
	int xsource,ysource,xdest,ydest;
	int mediatime=pointindex->mediatime<<1;
	int maxrangeend=mediatime;
	short x=pointindex->x;
	short y=pointindex->y;
	if (pointindex) {
		//fill in area from frame 1 to pointindex frame with pointindex value
		for (t=0;t<mediatime;t++) {
			*xrangeindex++=x;
			*yrangeindex++=y;
			}
		do {
			pointA=pointindex;
			pointB=pointindex->next;
			//TODO parse math type and do formula
			if (pointB) {
				framerangebegin=pointA->mediatime<<1;
				framerangeend=pointB->mediatime<<1;
				xsource=pointA->x;
				ysource=pointA->y;
				xdest=pointB->x;
				ydest=pointB->y;
				total=framerangeend-framerangebegin;
				maxrangeend=max(maxrangeend,framerangeend);
				for (t=framerangebegin;t<framerangeend;t++) {
					xrange[t]=(xsource-((xsource-xdest)*(t-framerangebegin)/total))&0xFFFC;
					yrange[t]=ysource-((ysource-ydest)*(t-framerangebegin)/total);
					}
				}
			} while (pointindex=pointindex->next);
		//fill in area from last point to end of array
		//We can assume this value is valid since head is valid
		xrangeindex=xrange+maxrangeend;
		yrangeindex=yrange+maxrangeend;
		x=filter->xypointstail->x;
		y=filter->xypointstail->y;
		framerangeend=media->totalframes<<1;
		for (t=maxrangeend;t<=framerangeend;t++) {
			*xrangeindex++=x;
			*yrangeindex++=y;
			}
		}// end if pointindex
	}


void filtersclass::CGclass::initxyrange (struct filterCG *filter,struct CGgui *CGwindowindex) {
	struct imagelist *streamptr=controls->streamptr;
	int t,totalfields=(UWORD)(streamptr->totalframes<<1);
	short *xindex,*yindex;
	short x=filter->x;
	short y=filter->y;
	LRESULT chkstate=SendMessage(CGwindowindex->xyrchk,BM_GETCHECK,0,0);
	if (chkstate==BST_CHECKED) {
		SendMessage(CGwindowindex->xyrchk,BM_SETCHECK,BST_UNCHECKED,0);
		//clear x and y field
		if (filter->xrange) {
			disposenode(filter->node.nodeobject,(struct memlist *)filter->xrange);
			filter->xrange=NULL;
			}
		if (filter->yrange) {
			disposenode(filter->node.nodeobject,(struct memlist *)filter->yrange);
			filter->yrange=NULL;
			}
		//clear points
		clearxypoints(filter);
		//clear text points too
		if (filter->xytext) {
			disposenode(nodeobject,(struct memlist *)filter->xytext);
			filter->xytext=NULL;
			if (CGwindowindex->xyred) {
				if (IsWindowVisible(CGwindowindex->xyred->window)) CGwindowindex->xyred->setwindowtext(makexytext(filter));
				}
			}
		}
	else {
		SendMessage(CGwindowindex->xyrchk,BM_SETCHECK,BST_CHECKED,0);
		//allocate thickrange field
		filter->xrange=xindex=(short *)newnode(filter->node.nodeobject,totalfields<<1);
		filter->yrange=yindex=(short *)newnode(filter->node.nodeobject,totalfields<<1);
		//initialize array with current static settings
		//TODO if there is no text in range window
		for (t=0;t<=totalfields;t++) {
			*xindex++=x;
			*yindex++=y;
			}
		}
	}


void filtersclass::CGclass::updatedepthrange (struct filterCG *filter,struct imagelist *media) {
	struct CGpoints *pointA,*pointB;
	struct CGpoints *pointindex=filter->thickpointshead;
	UBYTE *depthrange=filter->thickrange;
	UBYTE *depthrangeindex=depthrange;
	int framerangebegin,framerangeend,t,total;
	int thicksource,thickdest;
	int mediatime=pointindex->mediatime<<1;
	int maxrangeend=mediatime;
	UBYTE thickness=pointindex->trans;
	if (pointindex) {
		//fill in area from frame 1 to pointindex frame with pointindex value
		for (t=0;t<mediatime;t++) *depthrangeindex++=thickness;
		do {
			pointA=pointindex;
			pointB=pointindex->next;
			//TODO parse math type and do formula
			if (pointB) {
				framerangebegin=pointA->mediatime<<1;
				framerangeend=pointB->mediatime<<1;
				thicksource=pointA->trans;
				thickdest=pointB->trans;
				total=framerangeend-framerangebegin;
				maxrangeend=max(maxrangeend,framerangeend);
				for (t=framerangebegin;t<framerangeend;t++) {
					depthrange[t]=thicksource-((thicksource-thickdest)*(t-framerangebegin)/total);
					}
				}
			} while (pointindex=pointindex->next);
		//fill in area from last point to end of array
		//We can assume this value is valid since head is valid
		depthrangeindex=depthrange+maxrangeend;
		thickness=filter->thickpointstail->trans;
		framerangeend=media->totalframes<<1;
		for (t=maxrangeend;t<=framerangeend;t++) *depthrangeindex++=thickness;
		}// end if pointindex
	}


void filtersclass::CGclass::initdepthrange (struct filterCG *filter,struct CGgui *CGwindowindex) {
	struct imagelist *streamptr=controls->streamptr;
	int t,totalfields=(UWORD)(streamptr->totalframes<<1);
	UBYTE *thickindex;
	UBYTE thickness=filter->thickness;
	LRESULT chkstate=SendMessage(CGwindowindex->depthrchk,BM_GETCHECK,0,0);
	if (chkstate==BST_CHECKED) {
		SendMessage(CGwindowindex->depthrchk,BM_SETCHECK,BST_UNCHECKED,0);
		//clear thickrange field
		if (filter->thickrange) {
			disposenode(filter->node.nodeobject,(struct memlist *)filter->thickrange);
			filter->thickrange=NULL;
			}
		//clear points
		cleardepthpoints(filter);
		//clear text points too
		if (filter->thicktext) {
			disposenode(nodeobject,(struct memlist *)filter->thicktext);
			filter->thicktext=NULL;
			if (CGwindowindex->depthred) {
				if (IsWindowVisible(CGwindowindex->depthred->window)) CGwindowindex->depthred->setwindowtext(makedepthtext(filter));
				}
			}
		}
	else {
		SendMessage(CGwindowindex->depthrchk,BM_SETCHECK,BST_CHECKED,0);
		//allocate thickrange field
		filter->thickrange=thickindex=(UBYTE *)newnode(filter->node.nodeobject,totalfields);
		//initialize array with current static settings
		//TODO if there is no text in range window
		for (t=0;t<=totalfields;t++) *thickindex++=thickness;
		}
	}

