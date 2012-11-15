#include "handlemain.h"

//General Globals
struct memlist *pmem;
struct nodevars *nodeobject;
char string[256];
HINSTANCE hInst=NULL;
HWND screen=NULL;

//Globals for showdown
unsigned *elements,compares,swaps;
unsigned numofitems;

int error(char msgtype,char *sz,...) {
	printf(sz);
	return(FALSE);
	}

void printc(char *sz,...) {
	static TCHAR ach[256];
	va_list va;

	va_start(va, sz);
	wvsprintf (ach,sz, va);
	va_end(va);

	printf("%s\n",ach);
	}


void init() {
	nodeobject=createnode(&pmem,65536,0);
   unsigned seed,rndnum;
	unsigned i;
   float x;

   printf("\nSort showdown by James Killian\n\n");
/*
   printf("Please pick a number between 1-100000?");
   scanf ("%ld",&seed);
   srand(seed);
*/
	seed=1000;
   srand(seed);
   rndnum=rand();
   x=((((float)(rndnum&65535))/65535)*100)+10;
   numofitems=(unsigned)(x);
	elements=(unsigned *)mynew(&pmem,numofitems*sizeof(int));
	for (i=0;i<numofitems;i++) {
	   rndnum=rand();
		x=((((float)(rndnum&65535))/65535)*26)+65;
		elements[i]=(unsigned)(x);
		}
	}


void cleanup() {
	if (pmem) killnode(nodeobject,&pmem);
	if (pmem) disposeall(&pmem);
	exit(TRUE);
	}

void bubblesort(int *list,unsigned numofitems) {
	unsigned i,j;
	for (i=0;i<numofitems;i++) {
		for (j=i+1;j<numofitems;j++) {
			}
		}
	}

void main() {
	init();
	}

