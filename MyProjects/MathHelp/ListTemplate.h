#ifndef __LIST_TEMPLATE__
#define __LIST_TEMPLATE__

template<class L>
void Swap(L &a,L &b)
{ L c=a; a=b; b=c; }


/*!	\class		tList
 *	\ingroup	ControlDLL
 *	\brief		A Dynamic list template that manages simple arrayed lists
 *
 *	Warning: Do not instantiate classes that have virtual functions.  It is possible to instantiate structs or classes
 *  which do not contain virtual functions.  Use pointers for classes which contain virtual functions.
 */
template<class L>
class tList
{	public:		unsigned		NoItems;
				L*				Memory;				

				virtual void Resize(void)
				{	if (!Memory) 
					{	if (NoItems) Memory=(L*)NewTek_malloc((NoItems)*sizeof(L));
					} else {
						if (!NoItems) { NewTek_free(Memory); Memory=NULL; }
						else Memory=(L*)NewTek_realloc(Memory,(NoItems)*sizeof(L));
					}
				}				

				//! Indexing
				L& operator [] (unsigned i) const
				{	assert(i<NoItems);
					return Memory[i]; 
				}

				//! Swap two items
				void SwapItems(int a,int b)
				{	if ((a>=NoItems)||(b>=NoItems)) return;
					L Temp=Memory[a];
					Memory[a]=Memory[b];
					Memory[b]=Temp;
				}

				/*!	Swap all information with another entry 
					(this is a dangerous call unless you know exactly what you are doing.
					 Using dlls in win32 it is extremely likely to cause a crash/memory leak)*/
				void ExchangeData(tList<L> *a)
				{	Swap(a->NoItems,NoItems);
					Swap(a->Memory,Memory);
				}

				void CopyFrom(tList<L> *a)
				{	NoItems=a->NoItems;
					Resize();
					if (NoItems) memcpy(Memory,a->Memory,sizeof(L)*NoItems);
				}

				//! Stuff
				void Add(L Value)
				{	NoItems++; Resize();					
					Memory[NoItems-1]=Value;
				}

				L* New(void)
				{	NoItems++; Resize();
					return &Memory[NoItems-1];
				}

				void Add(L Value,int Position)
				{	NoItems++; Resize();
					if (Position>=(int)NoItems-1) Memory[NoItems-1]=Value;
					else
					{	for(int i=NoItems-1;i>Position;i--)
							Memory[i]=Memory[i-1];
						Memory[Position]=Value;
					}
				}				

				bool Delete(L Value)
				{	for(unsigned i=0;i<NoItems;i++)
					if (Memory[i]==Value)	
					{	Memory[i]=Memory[NoItems-1];
						NoItems--; Resize();					
						return true;
					}
					return false;
				}				

				bool DeleteInOrder(L Value)
				{	for(unsigned i=0;i<NoItems;i++)
					if (Memory[i]==Value)	
					{	for(;i<NoItems-1;i++)
							Memory[i]=Memory[i+1];
						NoItems--; Resize();
						return true;
					}
					return false;
				}				

				bool DeleteEntry(unsigned i)
				{	memcpy(&Memory[i],&Memory[NoItems-1],sizeof(L));
					NoItems--; Resize();					
					return true;
				}

				bool DeleteEntryInOrder(unsigned i)
				{	if (!NoItems) return false;
					for(;i<NoItems-1;i++) 
						memcpy(&Memory[i],&Memory[i+1],sizeof(L));
					NoItems--; Resize();
					return true;
				}

				bool Exists(L Value) const
				{	for(unsigned i=0;i<NoItems;i++)
					if (Memory[i]==Value) return true; 
					return false;
				}

				int GetPosition(L Value) const
				{	for(unsigned i=0;i<NoItems;i++)
					if (Memory[i]==Value) return (int)i;
					return -1;
				}

				void DeleteAll(void)
				{	SetSize(0);					
				}

				//! Set the size of the list
				void SetSize(unsigned Size)
				{	NoItems=Size; 
					Resize();
				}

				//! Build the list of a given size
				tList(unsigned Size)
				{	NoItems=Size; Memory=NULL;
					Resize();
				}

				//! Constructor
				tList(void) 
				{	NoItems=0; Memory=NULL;
				}

				//! Destructor
				~tList(void)
				{	if (Memory) 
						NewTek_free(Memory);
				}
};

template<class L,int tBlockSize=16>
class tListExpandOnly : public tList<L>
{	private:	unsigned CurrentlyAllocated;

	public:		
				virtual void Resize(void)
				{	//! Expansion, always allocate to much
					if (NoItems>CurrentlyAllocated)
					{	CurrentlyAllocated=((NoItems+tBlockSize-1)/tBlockSize)*tBlockSize;
						Memory=(L*)NewTek_realloc(Memory,sizeof(L)*CurrentlyAllocated);
					}
				}

				void ExchangeData(tListExpandOnly<L> *a)
				{	Swap(a->CurrentlyAllocated,CurrentlyAllocated);
					tList<L>::ExchangeData(a);
				}

				void ExchangeData(tList<L> *a)
				{	CurrentlyAllocated=a->NoItems;
					tList<L>::ExchangeData(a);
				}

				void DeleteAll(void)
				{	tList<L>::DeleteAll();
					//CurrentlyAllocated=0;	// mwatkins was here
				}

				tListExpandOnly(void)
				{	CurrentlyAllocated=0;
				}
};

#endif __LIST_TEMPLATE__