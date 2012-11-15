#include "handlemain.h"
#include "selectimage.h"

selectimageclass::selectimageclass() {
	indexhead=indextail=indexptr=NULL;
	blocka=blockb=NULL;
	}

selectimageclass::~selectimageclass() {
	}

void selectimageclass::selectimage(struct imagelist *imageptr) {
	blocka=imageptr;
	imageptr->selected^=1;
	//now to add or remove to index list
	if (imageptr->selected) {
		indexptr=(struct indeximage *)newnode(nodeobject,sizeof(struct indeximage));
		memset(indexptr,0,sizeof(struct indeximage));
		indexptr->imageptr=imageptr;
		if (indextail) { //add to end
			indextail->next=indexptr;
			indexptr->prev=indextail;
			indextail=indexptr;
			}
		else { //new node
			indexhead=indextail=indexptr;
			}
		} //end if the image is now selected
	else { //image is now not selected
		//find image and remove it from list
		indexptr=indextail;
		//scan in reverse for match
		do {
			if (indexptr->imageptr==imageptr) break;
			 } while (indexptr=indexptr->prev);
		if (indexptr) {
			//now to remove the node
			if (indexptr->prev) indexptr->prev->next=indexptr->next;
			if (indexptr->next) indexptr->next->prev=indexptr->prev;
			if (indexptr==indexhead) {
				indexhead=indexptr->next;
				}
			if (indexptr==indextail) {
				indextail=indexptr->prev;
				}
			disposenode(nodeobject,(struct memlist *)indexptr);
			}
		else printc("Warning: unable to find node %lx in indeximage list",imageptr);
		}
	}


void selectimageclass::resetlist(BOOL unselect) {
	struct indeximage *indexnext;
	indexptr=indexhead;
	if (indexptr) {
		if (unselect) {
			do {
				indexnext=indexptr->next;
				indexptr->imageptr->selected^=1;
				//printc("%s",indexptr->imageptr->text);
				disposenode(nodeobject,(struct memlist *)indexptr);
				} while(indexptr=indexnext);
			}
		else {
			do {
				indexnext=indexptr->next;
				//printc("%s",indexptr->imageptr->text);
				disposenode(nodeobject,(struct memlist *)indexptr);
				} while(indexptr=indexnext);
			}
		}
	indexhead=indextail=indexptr=NULL;
	blocka=blockb=NULL;
	}


void selectimageclass::shiftselect(imagelist *imageptr) {
	struct imagelist *blockindex;
	BOOL down=FALSE;

	//For Windows feel we just assign blockb to image ptr and take this out
	if (blockb) {
		blocka=imageptr;
		blockb=NULL;
		selectimage(imageptr);
		goto dontshiftsel;
		}
	else blockb=imageptr;
	//For Windows feel we just assign blockb to image ptr and take this out

	if ((blockindex=blocka)&&(blockb)) {
		//ok this algorithm is lame, but simple and effective and most likely won't have any
		//performance issues... we'll guess that a is on top and check till the end or a match
		//to determine if b is above or below
		do {
			if (blockindex==blockb) {down=TRUE;break;}
			} while(blockindex=blockindex->next);
		blockindex=blocka;
		selectimage(blocka);

		if (down) {
			//going down
			do {
				selectimage(blockindex);
				if (blockindex==blockb) break;
				} while(blockindex=blockindex->next);
			}
		else {
			//going up
			do {
				selectimage(blockindex);
				if (blockindex==blockb) break;
				} while(blockindex=blockindex->prev);
			}
		}
dontshiftsel:;
	}

void selectimageclass::listselection() {
	int t=1;
	indexptr=indexhead;
	if (indexptr) {
		do {
			printc("%d. %s",t,indexptr->imageptr->text);
			t++;
			} while(indexptr=indexptr->next);
		}
	}
