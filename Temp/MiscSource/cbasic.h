#ifndef CBASIC_H
#define CBASIC_H

#ifndef CBASIC_PROTOS_H
#include "CBasic_protos.h"
#endif

#ifndef GF_PROTOS_H
#include "gf_protos.h"
#endif

#define joyidle ((Peek(12574720)&128)!=0)
#define MAX(a,b)	 ((a)>(b)?(a):(b))
#define UNDOSIZE 10

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
ULONG root_mem;  /*This has to be ULONG because APTR can't add to index*/
ULONG root_ind;
struct memlist *undohead;
struct memlist *disposehead;
struct memlist *disposetail;
UBYTE undosize;
UBYTE undocurrentsize;
UWORD fchip;
};

struct stackvars
{
struct memlist **pmem;
ULONG heap;
APTR  head_mem;
ULONG head_ind;
APTR  tail_mem;
ULONG tail_ind;
UWORD hunkcounter;
UBYTE unitsize;
UBYTE fchip;
};

#endif /* CBASIC_H */

