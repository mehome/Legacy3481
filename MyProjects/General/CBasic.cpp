#include "system.h"

UBYTE save(char *filename,long size,char *location) {
	HANDLE hfile;
	DWORD written;
	hfile=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (!((int)hfile<1)) {
		WriteFile(hfile,location,size,&written,NULL);
		//wsprintf(string,"%s @ %lx size %ld",filename,location,size);printc(string);
		//wsprintf(string,"Number of bytes written %ld",written);printc(string);
		CloseHandle(hfile);
		}
	if (written) return(TRUE);
	else return(FALSE);
	}


APTR load(char *filename,long *size,struct memlist **pmem) {
	HANDLE hfile;
	DWORD bogus;
	APTR mem=NULL;
	//wsprintf(string,"Loading %s...",filename);printc(string);
	hfile=CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hfile==(HANDLE)-1) return(NULL);
	*size=GetFileSize(hfile,NULL);
	mem=mynew(pmem,*size);
	if (!(ReadFile(hfile,mem,*size,&bogus,NULL))) {
		//wsprintf(string,"Unable to load %s",filename);printc(string);
		dispose((struct memlist *)mem,pmem);
		mem=NULL;
		}
	//wsprintf(string,"%ld bytes read from %lx to %lx",*size,mem,(ULONG)mem+(*size));printc(string);
	CloseHandle(hfile);
	return(mem);
	}


APTR mynew(struct memlist **prev_mem,ULONG size) {
	struct memlist *mem;
	HGLOBAL hmem;
	size +=sizeof(struct memlist);
	if ((hmem=GlobalAlloc(GPTR|GMEM_NOCOMPACT,(DWORD)size))==NULL) {
		//print("Memory Request Denied");
		return (0);
		}
	/* This is the only way to successfully cast? Oh Well...*/
	mem=(struct memlist *)hmem; 
	//wsprintf(string,"Allocated from %lx to %lx",mem,(ULONG)mem+size);printc(string);
	//if (csglobal.LockCount) EnterCriticalSection(&csglobal);
	mem->memsize=size;
	mem->memprev=*prev_mem;
	mem->memnext=NULL;
	if (*prev_mem!=0) {
		(*prev_mem)->memnext=mem;
		}
	*prev_mem=mem;
	//if (csglobal.LockCount) LeaveCriticalSection(&csglobal);
	return ((APTR)(mem+1));
	}


void dispose(struct memlist *mem,struct memlist **prev_mem) {
	struct memlist *prevv,*nextt;
	HGLOBAL hmem,test;

	mem-=1;
	prevv=mem->memprev;
	nextt=mem->memnext;
	/*
	wsprintf(string,"contents of mem %lx",mem);printc(string);
	wsprintf(string,"1. %lx now points to %lx",prevv,nextt);printc(string);
	*/
	//EnterCriticalSection(&csglobal);
	if (prevv) prevv->memnext=nextt;
	if (nextt) nextt->memprev=prevv;
	else *prev_mem=prevv;
	//LeaveCriticalSection(&csglobal);
	hmem=(HGLOBAL)mem;
	if (test=GlobalFree(hmem)) {
		error(0,"Failed Attempt to free memory at address %lx",mem);
		}
	}


void disposeall(struct memlist **prev_mem) {
	struct memlist *mem;
	HGLOBAL hmem,test;
	mem=*prev_mem;
	while (mem) {
		//wsprintf(string,"disposed mem @ %lx",mem);printc(string);
		*prev_mem=mem->memprev;
		hmem=(HGLOBAL)mem;
		if (test=GlobalFree(hmem)) {
			wsprintf(string,"Failed Attempt to free memory at address %lx",mem);
			error (0,string);
			}
		mem=*prev_mem;
		}
	}


void transmembyte(
	register char *source,
	register char *dest,
	register ULONG size)
	{

	do {*dest++=*source++;} while (--size);
	}


void transmem(
	register ULONG *source,
	register ULONG *dest,
	register long size)
	{
	register ULONG remainder=size-((size>>2)<<2);
	/*printf("remainder=%d\n",remainder);*/
	size-=4;
	do {*dest++=*source++;size-=4;} while (size>0);
	if (remainder) transmembyte((char *)source,(char *)dest,remainder);
	}
	

void transmemrev(
	register char *source,
	register char *dest,
	register ULONG size)
	{

	source += size;
	dest += size;
	do {*dest--=*source--;} while (--size);
	}


void clrmem(register char *dest,register ULONG size)
	{
	register ULONG count;
	for (count=0;count<size;count++) *dest++=0;
	}

	
void fill(register char *dest,register ULONG size,register UBYTE fillchar)
	{
	register ULONG count;
	for (count=0;count<size;count++) *dest++=fillchar;
	}


void reverse(register char *s) {
	register WORD c,i,j;
	for (i=0,j=strlen(s)-1;i<j;i++,j--)  {
		c=s[i];
		s[i]=s[j];
		s[j]=(UBYTE)c;
		}
	}

//Here is the memory node handler area

struct nodevars* createnode(
	register struct memlist **pmem,
	register ULONG heap,
	register UWORD fchip)
	{
	/*root_ind must be initialized to heap! */
	struct nodevars* mem;
	
	mem=(struct nodevars*)mynew(pmem,sizeof(struct nodevars));
	mem->pmem=NULL;
	mem->heap=heap;
	mem->root_mem=NULL;
	mem->root_ind=heap;
	mem->undohead=NULL;
	mem->disposehead=NULL;
	mem->disposetail=NULL;
	mem->undosize=UNDOSIZE;
	mem->undocurrentsize=0;
	mem->fchip=fchip;
	return(mem);
	}

	
void killnode(
	register struct nodevars *object,
	register struct memlist **pmem)
	{
	if (object) {
		if (object->pmem) disposeall(&object->pmem);
		dispose((struct memlist *)object,pmem);
		}
	}
	
	
APTR newmemnode(
	register struct nodevars *object,
	register ULONG size)
	{
	APTR mem=(APTR)(object->root_mem+object->root_ind);
	if (object->root_ind+size>object->heap) {
		//make sure size is less than heap
		if (object->heap>size) mem=mynew(&object->pmem,object->heap);
		else mem=mynew(&object->pmem,size);
		//next time this gets called it should immediately
		//get a new heap if the size was larger
		object->root_mem=(char *)mem;
		object->root_ind=0;
		}
	object->root_ind+=size;
	return(mem);
	}


void removenode(
	register struct memlist *node,
	register struct memlist **head,
	register struct memlist **tail)
	{
	if (node==*head) {
		*head=node->memnext;
		}
	if (node==*tail) {
		*tail=node->memprev;
		}
	if (node->memprev) {
		node->memprev->memnext=node->memnext;
		}
	if (node->memnext) {
		node->memnext->memprev=node->memprev;
		}
	}


APTR newnode(
	register struct nodevars *object,
	register ULONG size)
	{
	struct memlist *mem;
	struct memlist *search;
	UBYTE newmemflag=FALSE;
	
	//EnterCriticalSection(&csglobal);
	size+=sizeof(struct memlist);
	/* Search for existing memory in used list */
	search=object->disposehead;
	if (search) {
		do {
			if (search->memsize>=size) {
				mem=search;
				/*Take node out of link list*/
				removenode(search,&object->disposehead,&object->disposetail);
				break;
				}
			else newmemflag=TRUE;
			} while (search=object->disposehead->memnext);
		} /* end if search */
	else newmemflag=TRUE;
	if (newmemflag) {
		mem=(struct memlist*)newmemnode(object,size);
		mem->memsize=size;
		}
	//LeaveCriticalSection(&csglobal);
	return((APTR)(mem+1));
	}

	
void disposenode(
	register struct nodevars *object,
	register struct memlist *mem)
	{
	//EnterCriticalSection(&csglobal);

	mem-=1;
	if (object->disposetail) {
		object->disposetail->memnext=mem;
		mem->memprev=object->disposetail;
		}
	else {
		object->undohead=mem;
		mem->memprev=NULL;
		}
	mem->memnext=NULL;
	object->disposetail=mem;
	/* set head once buffersize is full, and advance it if not*/
	if (object->undocurrentsize<object->undosize) {
		object->undocurrentsize++;
		if (object->disposehead) object->disposehead=object->disposehead->memnext;
		}
	else {
		object->disposehead=mem;
		}
	//LeaveCriticalSection(&csglobal);
	}


//Here is the buffered save area * * *


struct savevars *createsave(
	register struct memlist **pmem,
	register char *filename,
	register ULONG heap)
	{
	struct savevars* mem;

	mem=(struct savevars*)mynew(pmem,sizeof(struct savevars));
	mem->hfile=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	mem->heap=heap;
	mem->root_mem=(char *)mynew(pmem,heap);
	mem->root_ind=0;
	return(mem);
	}


void transmemsave(
	register struct savevars *object,
	char *source,
	long size)
	{
	DWORD written;
	if (object->root_ind+size>object->heap) {
		WriteFile(object->hfile,object->root_mem,object->root_ind,&written,NULL);
		object->root_ind=0;
		}
/*
	if (size>=4) transmem((ULONG *)source,(ULONG *)(object->root_mem)+object->root_ind),size);
	else transmembyte(source,object->root_mem+object->root_ind,size);
*/
	//make sure the actual request fits.. or write that in as well
	if ((ULONG)size>object->heap) {
		WriteFile(object->hfile,source,size,&written,NULL);
		}
	else memcpy(object->root_mem+object->root_ind,source,size);
	object->root_ind+=size;
	} //end transmem save

void killsave(
	register struct savevars *object,
	register struct memlist **pmem)
	{
	DWORD written;
	//writing the remaining data from the buffer
	if (object->root_ind)
		WriteFile(object->hfile,object->root_mem,object->root_ind,&written,NULL);
	CloseHandle(object->hfile);
	dispose((struct memlist *)object->root_mem,pmem);
	dispose((struct memlist *)object,pmem);
	}


//end buffer save group


ULONG linput(
	register char *mem,
	register char *string)
	{
	UBYTE count=0; /* take out any extra spaces */

	while (*mem==32 && count<30) {mem++;count++;}
	while ((*mem!=10)&&(*mem!=13)&&(count<255)) {
		*string++=*mem++;
		count++;
		}
	string--; /* take out line feed and cr*/
	while (*string==' ') string--; /* take out extra spaces */
	string++;
	*string='\0';
	//Check for lf/ff
	if ((mem[0]==13)&&(mem[1]==10)||(mem[0]==10)&&(mem[1]==13)) count++;
	return((ULONG)count+1); /* count is NOT strlen, rather the index counter */
	}


ULONG winput(
	register char *mem,
	register char *string)
	{
	UBYTE count=0; /* take out any extra spaces */

	while (*mem==32 && count<30) {mem++;count++;}
	while ((*mem!=32 || *(mem+1)!=32) && *mem!=0 && *mem!=10 && count<100) {
		*string++=*mem++;
		count++;
		}
	if (*mem==10) count--;
	*string='\0';
	return((ULONG)count+1);
	}


ULONG fsqrt(register ULONG r)
	{
	register ULONG t,b,c=0;
	
	for (b=0x10000000;b!=0;b>>=2) {
		t=c+b;
		c>>=1;
		if (t<=r){
			r-=t;
			c+=b;
			}
		}
	return(c);
	} /*end fsqrt*/

