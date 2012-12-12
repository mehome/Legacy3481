#ifndef CBASIC_H
#define CBASIC_H

#include "system.h"
/*Keep Amiga Varible Storage types!*/
typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef long LONG;
typedef HANDLE APTR;

#define UNDOSIZE 10

/*All Global struct defines here*/
struct memlist
{
struct memlist *memnext;
struct memlist *memprev;
ULONG memsize;
};

struct nodevars
{
struct memlist *pmem;
ULONG heap;
char *root_mem;
ULONG root_ind;
struct memlist *undohead;
struct memlist *disposehead;
struct memlist *disposetail;
UBYTE undosize;
UBYTE undocurrentsize;
UWORD fchip;
};

struct savevars
{
HANDLE hfile;
ULONG heap;  //Note for NT max size is 65535
char *root_mem;
ULONG root_ind;
};

/* End Global struct defines*/


/* Here are the function prototypes */
int error(char msgtype,char *sz,...);
UBYTE save (char *filename,long size,char *location);
APTR load (char *filename,long *size,struct memlist **pmem);
APTR mynew (struct memlist **prev_mem,ULONG size);
void dispose (struct memlist *mem,struct memlist **prev_mem);
void disposeall (struct memlist **prev_mem);
void transmembyte(register char *source,register char *dest,register ULONG size);
void transmem(register ULONG *source,register ULONG *dest,register long size);
void transmemrev(register char *source,register char *dest,register ULONG size);
void clrmem(register char *dest,register ULONG size);
void fill(register char *dest,register ULONG size,register UBYTE fillchar);
void reverse (register char *s);
//Here is the memory node group
struct nodevars* createnode (register struct memlist **pmem,register ULONG heap,register UWORD fchip);
void killnode(register struct nodevars *object,register struct memlist **pmem);
APTR newmemnode (register struct nodevars *object,register ULONG size);
void removenode (register struct memlist *node,register struct memlist *head,register struct memlist *tail);
APTR newnode (register struct nodevars *object,register ULONG size);
void disposenode (register struct nodevars *object,register struct memlist *mem);
//Here is the buffered save group
struct savevars *createsave(register struct memlist **pmem,register char *filename,register ULONG heap);
void transmemsave(register struct savevars *object,char *source,long size);
void killsave(register struct savevars *object,register struct memlist **pmem);
//end buffered save group
ULONG linput(register char *mem,register char *string);
ULONG winput(register char *mem,register char *string);
ULONG fsqrt(register ULONG r);
/* End Global function prototypes */

#endif /* CBASIC_H */
