#include "StdAfx.h"

//****************************************************************************************************************************
bool SBD_Item_Info::Render_ReadData(SBD_Item_Render_Buffer *Items)
{	return true;
}

//****************************************************************************************************************************
int SBD_Item_Info::GetLocalFieldNo(double Time)
{	return NewTek_fRound(Time*m_OriginalFrameRate*2.0*m_StretchMultiply - m_FrameTime*0.5);
}

//****************************************************************************************************************************
bool SBD_Item_Info::IsOddField(double FieldTime)
{	int FieldNo=GetLocalFieldNo(FieldTime);
	return FieldNo&1;
}

//****************************************************************************************************************************
void SBD_Item_Info::SetDescription(char *Name)
{	Description=(char*)realloc(Description,strlen(Name)+1);
	strcpy(Description,Name);
}

//****************************************************************************************************************************
SBD_Item_Info::SBD_Item_Info(SBD_Item_Info *p_Parent)
{	// I have no parent
	Parent=p_Parent;
	if (Parent) 
	{	Parent->MyChildren.Add(this);
		Parent->NewChild(this);
	}

	// Defaults
	SetOriginalFrameRate(30000.0/1001.0);
	SetOriginalLength(DBL_MAX);

	SetInPoint(0.0);
	SetOutPoint(DBL_MAX);
	SetDuration(DBL_MAX);	
	SetTransition(false);

	Description=NULL;
	SetDescription("SBItem");
}

//***********************************
SBD_Item_Info::~SBD_Item_Info(void)
{	// Free my description
	if (Description) free(Description);

	// Delete myself from my parent
	if (Parent) Parent->MyChildren.Delete(this);
	
	// Delete all my children
	for(unsigned i=0;i<MyChildren.NoItems;i++)
		delete MyChildren[i];
}

//***********************************
int SBD_Item_Info_Render_Compare( const void *arg1, const void *arg2 )
{	SBD_Item_Render_Buffer **Item1=(SBD_Item_Render_Buffer**)arg1;
	SBD_Item_Render_Buffer **Item2=(SBD_Item_Render_Buffer**)arg2;
	if ((*Item1)->GetCentreTime() > (*Item2)->GetCentreTime()) return  1;
	if ((*Item1)->GetCentreTime() < (*Item2)->GetCentreTime()) return -1;
	return 0;
}

int SBD_FgBg_Info_Render_Compare( const void *arg1, const void *arg2 )
{	SBD_Item_Info_From_To **Item1=(SBD_Item_Info_From_To**)arg1;
	SBD_Item_Info_From_To **Item2=(SBD_Item_Info_From_To**)arg2;
	if ((*Item1)->GetCentreTime() > (*Item2)->GetCentreTime()) return  1;
	if ((*Item1)->GetCentreTime() < (*Item2)->GetCentreTime()) return -1;
	return 0;
}

//***********************************
HandleCache_Item *SBD_Item_Info::Render_Async(SBD_Item_Render_Buffer *Items)
{	return NULL;
}

//***********************************
bool SBD_Item_Info::DoRender(unsigned NoItems,SBD_Item_Render_Buffer **Items)
{	return false;
}

//***********************************
bool SBD_Item_Info::Render(SBD_Item_Render_Buffer *Items)
{	// We sort the items into order
	SBD_Item_Render_Buffer *ItemsToRender[MaximumFieldsProcessedAtOnce];
	SBD_Item_Render_Buffer *Scan=Items;;

	unsigned DstNum=0;
	while(Scan)
	{	// Insert the item
		ItemsToRender[DstNum]=Scan;		

		// Look at the next item
		DstNum++; Scan=Scan->GetNext();

		// To mny items where sent here for rendering
		if ((DstNum>=MaximumFieldsProcessedAtOnce)&&(Scan)) return false;
	}

	// Now quicksort the items into order
	qsort(ItemsToRender,DstNum,sizeof(SBD_Item_Render_Buffer*),SBD_Item_Info_Render_Compare);

	// Render the stuff
	return DoRender(DstNum,ItemsToRender);
}

//*********************************************************************************************************************************
bool SBD_Item_Info::DoRender_Transition(SBD_Item_Info_From_To *Item,int LocalFieldNum)
{	
#ifdef _DISPLAYINFO
	DebugOutput("%s : Do Render Transition field %d, (%fs)\n",Description ,LocalFieldNum,Item->m_RenderingTime);
#endif
	return true;
}

bool SBD_Item_Info::Render_Transition(SBD_Item_Info_From_To *Items)
{	// We sort the items into order
	SBD_Item_Info_From_To *ItemsToRender[MaximumFieldsProcessedAtOnce];
	SBD_Item_Info_From_To *Scan=Items;;

	unsigned DstNum=0;
	while(Scan)
	{	// Insert the item
		ItemsToRender[DstNum]=Scan;		

		// Look at the next item
		DstNum++; Scan=Scan->Next;

		// To mny items where sent here for rendering
		if ((DstNum>=MaximumFieldsProcessedAtOnce)&&(Scan)) return false;
	}

	// Now quicksort the items into order
	qsort(ItemsToRender,DstNum,sizeof(SBD_Item_Info_From_To*),SBD_FgBg_Info_Render_Compare);

	// Now we need to scan through all the images, and we only actually render 
	// fields that have not been see before.
	for(unsigned i=0;i<DstNum;i++)
	{	// Is it actually a new field ?
		int ThisFieldNo=GetLocalFieldNo(ItemsToRender[i]->GetCentreTime());

		// If the item is actually a different field number, then we need to render it 
		DoRender_Transition(ItemsToRender[i],ThisFieldNo);
	}

	// Success
	return true;
}