#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/dos.h>
#include <stat.h>
#include <stdlib.h>
//#include <proto/all.h> Taken care of by gst

#include "cbasic/gf_protos.h"
#include "cbasic.h"
#include "cbasic_protos.h"


void __asm transmem(
	register __a0 char *source,
	register __a1 char *dest,
	register __d1 ULONG size)
	{
	
	for (;size;size--) *dest++=*source++;
	}

	
void __asm transmemrev(
	register __a0 char *source,
	register __a1 char *dest,
	register __d1 ULONG size)
	{
	
	source += size;
	dest += size;
	for (;size;size--) *dest--=*source--;
	}


void __asm clrmem(
	register __a0 char *dest,
	register __d1 ULONG size)
	{
	register ULONG count;
	
	for (count=0;count<size;count++) *dest++=0;
	}

	
void __asm fill(
	register __a0 char *dest,
	register __d1 ULONG size,
	register __d2 UBYTE fillchar)
	{
	register ULONG count;
	
	for (count=0;count<size;count++) *dest++=fillchar;
	}


APTR __asm mynew (
	register __a0 struct memlist **prev_mem,
	register __d1 ULONG size,
	register __d2 UBYTE fchip)
	{
	/* if chip=1 then allocate chip memory */
	struct memlist *mem;
	
	size +=sizeof(struct memlist);
	if (!(fchip)) {
		if ((mem=AllocMem((ULONG)(size),0L))==NULL)
			{/*printf("need more memory\n");*/return(0);}
		}
	else {
		if ((mem=AllocMem((ULONG)(size),MEMF_CHIP|MEMF_CLEAR|MEMF_PUBLIC))==NULL)
			{/*printf("need more chip memory\n");*/return(0);}
		}
	/* printf("Allocated from %lx to %lx\n",mem,(ULONG)mem+size);*/
	mem->memsize=size;
	mem->memprev=*prev_mem;
	mem->memnext=NULL;
	if (*prev_mem!=0) {
		(*prev_mem)->memnext=mem;
		}
	*prev_mem=mem;
	return ((APTR)(mem+1));
	}


void __asm dispose(
	register __a0 struct memlist *mem,
	register __a1 struct memlist **prev_mem)
	{
	struct memlist *prevv,*nextt;
	
	mem-=1;
	prevv=mem->memprev;
	nextt=mem->memnext;
	/* printf("contents of mem %lx\n",mem);
	printf("1. %lx now points to %lx\n",prevv,nextt);*/
	if (prevv) {prevv->memnext=nextt;}
	if (nextt) {nextt->memprev=prevv;}
		else {*prev_mem=prevv;}
	FreeMem((APTR)(mem),mem->memsize);
	}


void __asm disposeall(register __a0 struct memlist **prev_mem)
	{
	struct memlist *mem;
	mem=*prev_mem;
	while (mem) {
		/*printf("disposed mem @ %lx\n",mem);*/
		*prev_mem=mem->memprev;
		FreeMem((APTR)(mem),mem->memsize);
		mem=*prev_mem;
		}
	}


struct stackvars* __asm createstack (
	register __a0 struct memlist **pmem,
	register __d1 ULONG heap,
	register __d0 ULONG chipsize)
	{
	/*note chipsize is two bytes combined into word*/
	struct stackvars* mem;
	
	mem=mynew(pmem,sizeof(struct stackvars),0);
	clrmem((char *)mem,sizeof(struct stackvars));
	mem->pmem=pmem;
	mem->heap=heap;
	mem->head_ind=heap;
	mem->unitsize=(UBYTE)(chipsize&255);
	mem->fchip=(UBYTE)(chipsize>>16);
	return(mem);
	}


void __asm killstack(
	register __a0 struct stackvars *stack)
	{
	if (stack) dispose((struct memlist *)stack,stack->pmem);
	}


UBYTE __asm pushstack(
	register __a0 struct stackvars *stack,
	register __a1 char *source)
	{
	return (0);
	}
	
	
UBYTE __asm pullstack(
	register __a0 struct stackvars *stack,
	register __a1 char *source)
	{
	return (0);
	}	


struct nodevars* __asm createnode (
	register __a0 struct memlist **pmem,
	register __d1 ULONG heap,
	register __d2 UWORD fchip)
	{
	/*root_ind must be initialized to heap! */
	struct nodevars* mem;
	
	mem=mynew(pmem,sizeof(struct nodevars),0);
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
	
void __asm killnode(
	register __a0 struct nodevars *object,
	register __a1 struct memlist **pmem)
	{
	if (object) {
		if (object->pmem) disposeall(&object->pmem);
		dispose((struct memlist *)object,pmem);
		}
	}
	
	
APTR __asm newmemnode (
	register __a0 struct nodevars *object,
	register __d1 ULONG size)
	{
	APTR mem=(APTR)(object->root_mem+object->root_ind);
	if (object->root_ind+size>object->heap) {
		mem=mynew(&object->pmem,(UBYTE)object->heap,object->fchip);
		object->root_mem=(ULONG)mem;
		object->root_ind=0;
		}
	object->root_ind+=size;
	return(mem);
	}


APTR __asm newnode (
	register __a0 struct nodevars *object,
	register __d1 ULONG size)
	{
	struct memlist *mem;
	struct memlist *search;
	UBYTE newmemflag=FALSE;
	
	size+=sizeof(struct memlist);
	/* Search for existing memory in used list */
	search=object->disposehead;
	if (search) {
		while (search=object->disposehead->memnext) {
			if (search->memsize>=size) {
				mem=search;
				/*Take node out of link list*/
				removenode(search,object->disposehead,object->disposetail);
				}
			else newmemflag=TRUE;
			} /* end while search*/
		} /* end if search */
	else newmemflag=TRUE;
	if (newmemflag) {
		mem=newmemnode(object,size);
		mem->memsize=size;
		}
	return((APTR)(mem+1));
	}


void __asm removenode (
	register __a0 struct memlist *node,
	register __a1 struct memlist *head,
	register __a2 struct memlist *tail)
	{
	if (node==head) {
		head=node->memnext;
		}
	if (node==tail) {
		tail=node->memprev;
		}
	if (node->memprev) {
		node->memprev->memnext=node->memnext;
		}
	if (node->memnext) {
		node->memnext->memprev=node->memprev;
		}
	}
	
	
void __asm disposenode (
	register __a0 struct nodevars *object,
	register __a1 struct memlist *mem)
	{
	
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
	}


UBYTE __asm save (
	register __a0 char *filename,
	register __d1 long size,
	register __a1 char *location)
	{
	BPTR handle;
	
	if((handle=Open(filename,MODE_NEWFILE))==NULL)
		{/*printf("couldn't open file for save.\n");*/return(0);}
	if(Write(handle,location,size)==-1)
		{/*printf("couldn't write file.\n");*/return(0);}
	Close(handle);	
	return(1);	
	}

	
APTR __asm load (
	register __a1 struct memlist **pmem,
	register __a0 char *filename,
	register __d1 long *size,
	register __d2 UBYTE fchip)
	{
	int rc;
	struct stat st;
	BPTR handle;
	APTR mem=0;
	
	/*printf("Loading %s...\n",filename);*/
		if((handle=Open(filename,MODE_OLDFILE))==NULL)
		{/*printf("couldn't open file.\n");*/return(0);}
	/*printf("handle=%lx\n",handle);*/
	if(rc=stat(filename,&st)!=NULL)
		{/*printf("couldn't obtain st info.\n");*/return(0);}
	mem=mynew(pmem,st.st_size,fchip);
	if(Read(handle,mem,st.st_size)==-1) {
		/*printf("couldn't read file.\n");*/
		dispose((struct memlist *)mem,pmem);
		return(0);
		}
	*size=st.st_size;
	/* printf("%ld bytes read from %lx to %lx\n",st.st_size,mem,((ULONG)mem+st.st_size));*/
	Close(handle);
	return (mem);
	}

void __asm itoa (register __d1 WORD n,
					  register __a0 char *s)
	{
	register WORD i=0;
	register WORD sign=FALSE;
	if (n<0)  {
		sign = TRUE;
		n = -n;
		}
	do s[i++] = n%10+'0'; while (n/=10);
	if (sign) s[i++] = '-';
	s[i]=0;
	reverse(s);
	}

void __asm reverse (register __a0 char *s)
	{
	register WORD c,i,j;
	for (i=0,j=stlen(s)-1;i<j;i++,j--)  {
		c=s[i];
		s[i]=s[j];
		s[j]=c;
		}
	}

ULONG __asm linput(register __a0 char *mem,
						 register __a1 char *string)
	{
	UBYTE count=0; /* take out any extra spaces */
	while (*mem==32 && count<30) {mem++;count++;}
	while (*mem!=10 && count<255) {
		*string++=*mem++;
		count++;
		}
	string--; /* take out line feed and cr*/
	while (*string==' ') string--; /* take out extra spaces */
	string++;
	*string='\0';
	return((ULONG)count+1); /* count is NOT strlen, rather the index counter */
	}

ULONG __asm winput(register __a0 char *mem,
						 register __a1 char *string)
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


ULONG __asm fsqrt(register __d1 ULONG r)
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

