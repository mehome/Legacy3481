#include "StdAfx.h"

//**********************************************************************************************
int StoryBoard_Item::GetFlags(void)
{	return Flags;
}

void StoryBoard_Item::SetFlags(int Val)
{	Flags=Val;
}

//**********************************************************************************************
float StoryBoard_Item::GetOffSet(unsigned ID)
{	return OffSet[ID];
}

void StoryBoard_Item::SetOffSet(float Val,unsigned ID)
{	OffSet[ID]=Val;
}

//**********************************************************************************************
int StoryBoard_Item::GetCroutonFrame(unsigned ID)
{	return CroutonFrame[ID];
}

void StoryBoard_Item::SetCroutonFrame(int Val,unsigned ID)
{	CroutonFrame[ID]=Val;
}

//**********************************************************************************************
float StoryBoard_Item::GetInPoint(unsigned ID)
{	return InPoint[ID];
}

void StoryBoard_Item::SetInPoint(float Val,unsigned ID)
{	InPoint[ID]=Val;	
}

//**********************************************************************************************
float StoryBoard_Item::GetOutPoint(unsigned ID)
{	return OutPoint[ID];
}

void StoryBoard_Item::SetOutPoint(float Val,unsigned ID)
{	OutPoint[ID]=Val;
}

//**********************************************************************************************
int StoryBoard_Item::GetClipDuration(unsigned ID)
{	return ClipDuration[ID];
}

void StoryBoard_Item::SetClipDuration(int Val,unsigned ID)
{	ClipDuration[ID]=Val;	
}

//**********************************************************************************************
float StoryBoard_Item::GetStretchDuration(unsigned ID)
{	return StretchDuration[ID];
}

void StoryBoard_Item::SetStretchDuration(float Val,unsigned ID,bool RecomputeCoeffs)
{	StretchDuration[ID]=Val;

	if (RecomputeCoeffs)
	{	// Compute the stretch coefficient
		if (ClipDuration[ID])	StretchCoeff[ID]=StretchDuration[ID]/fabs(OutPoint[ID]-InPoint[ID]);
		else					StretchCoeff[ID]=1.0;
	}	
}

//**********************************************************************************************
float StoryBoard_Item::GetStretchCoeff(unsigned ID)
{	return StretchCoeff[ID];
}

void StoryBoard_Item::SetStretchCoeff(float Val,unsigned ID)
{	StretchCoeff[ID]=Val;
}

//**********************************************************************************************
float StoryBoard_Item::GetOriginalFrameRate(unsigned ID)
{	return OriginalFrameRate[ID];
}

void StoryBoard_Item::SetOriginalFrameRate(float Val,unsigned ID)
{	OriginalFrameRate[ID]=Val;
}

//**********************************************************************************************
GUID StoryBoard_Item::GetSerialNunber(void)
{	return Serial;
}

//**********************************************************************************************
StoryBoard_Item* StoryBoard_Item::FindItem(GUID SerialNo)
{	if (Serial==SerialNo) return this;
	for(unsigned i=0;i<MyChildren.NoItems;i++)
	{	StoryBoard_Item *Ret=MyChildren[i]->FindItem(SerialNo);
		if (Ret) return Ret;
	}
	return NULL;
}

//**********************************************************************************************
void StoryBoard_Item::Serialise(bool SerialiseChildren)
{	// Serialise myself
	if (FAILED(CoCreateGuid(&Serial))) _throw("Could not create Unique serial number for the Storyboard item.");

	// move on down to my children
	if (SerialiseChildren)
	{	for(unsigned i=0;i<MyChildren.NoItems;i++)
			MyChildren[i]->Serialise(SerialiseChildren);
	}
}

//**********************************************************************************************
char *StoryBoard_Item::GetFirstValidFileName(void)
{	if (FileName) return FileName;
	for(unsigned i=0;i<MyChildren.NoItems;i++)
	{	char *Temp=MyChildren[i]->GetFirstValidFileName();
		if (Temp) return Temp;
	}
	return NULL;
}

//**********************************************************************************************
void StoryBoard_Item::DeleteSelectedChildren(void)
{	// Only copy the selected items
	for(unsigned i=0;i<MyChildren.NoItems;i++)
	if (MyChildren[i]->GetSelected())
	{	// Delete the child
		delete MyChildren[i];
		MyChildren.DeleteEntryInOrder(i);

		// We need to move back one item since there are one fewer items in the list now
		i--;
	}
}

//**********************************************************************************************
unsigned StoryBoard_Item::GetLastSelected(void)
{	for(int i=MyChildren.NoItems-1;i>=0;i--)
	if (MyChildren[i]->GetSelected()) return i;
	return 0xffffffff;
}

//**********************************************************************************************
unsigned StoryBoard_Item::GetNoSelectedChildren(void)
{	unsigned Ret=0;
	for(unsigned i=0;i<MyChildren.NoItems;i++)
	if (MyChildren[i]->GetSelected()) Ret++;
	return Ret;
}

//**********************************************************************************************
bool StoryBoard_Item::CopyFrom(StoryBoard_Item *From,bool SelectedOnly,bool CopySerial,bool CopyCodec)
{	// Remove any existing children
	KillAll();
	
	// Set my properties
	SetFileName(From->GetFileName());
	SetAlias(From->GetAlias());
	SetSelected(From->GetSelected());	

	// Setup all the in/out parameters
	SetFlags(From->GetFlags());

	SetCroutonFrame(From->GetCroutonFrame(StoryBoard_Video),StoryBoard_Video);
	SetOffSet(From->GetOffSet(StoryBoard_Video),StoryBoard_Video);
	SetInPoint(From->GetInPoint(StoryBoard_Video),StoryBoard_Video);
	SetOutPoint(From->GetOutPoint(StoryBoard_Video),StoryBoard_Video);
	SetClipDuration(From->GetClipDuration(StoryBoard_Video),StoryBoard_Video);
	SetStretchDuration(From->GetStretchDuration(StoryBoard_Video),StoryBoard_Video,false);
	SetStretchCoeff(From->GetStretchCoeff(StoryBoard_Video),StoryBoard_Video);
	SetOriginalFrameRate(From->GetOriginalFrameRate(StoryBoard_Video),StoryBoard_Video);
	SetCroutonFrame(From->GetCroutonFrame(StoryBoard_Audio),StoryBoard_Audio);
	SetOffSet(From->GetOffSet(StoryBoard_Audio),StoryBoard_Audio);
	SetInPoint(From->GetInPoint(StoryBoard_Audio),StoryBoard_Audio);
	SetOutPoint(From->GetOutPoint(StoryBoard_Audio),StoryBoard_Audio);
	SetClipDuration(From->GetClipDuration(StoryBoard_Audio),StoryBoard_Audio);
	SetStretchDuration(From->GetStretchDuration(StoryBoard_Audio),StoryBoard_Audio,false);
	SetStretchCoeff(From->GetStretchCoeff(StoryBoard_Audio),StoryBoard_Audio);
	SetOriginalFrameRate(From->GetOriginalFrameRate(StoryBoard_Audio),StoryBoard_Audio);

	// Copy the serial number
	if (CopySerial) Serial=From->Serial;

	// Add all th children
	if (!SelectedOnly)
	{	// Now copy them all across, irrespextive of selection status
		for(unsigned i=0;i<From->GetNoChildren();i++)
		{	StoryBoard_Item *Child=NewStoryboardChild();
			if (!Child) return false;
			Child->CopyFrom((*From)[i],false,CopySerial,CopyCodec);
		}
	}
	else
	{	// Only copy the selected items
		for(unsigned i=0;i<From->GetNoChildren();i++)
		if ((*From)[i]->GetSelected())
		{	StoryBoard_Item *Child=NewStoryboardChild();
			if (!Child) return false;
			Child->CopyFrom((*From)[i],false,CopySerial);			
		}
	}

	// Success
	return true;
}

//**********************************************************************************************
StoryBoard_Item *StoryBoard_Item::GetParent(void)
{	return MyParent;
}

//**********************************************************************************************
StoryBoard_Item *StoryBoard_Item::GetTopParent(void)
{	StoryBoard_Item *Ret=this;
	while(Ret->MyParent) Ret=Ret->MyParent;
	return Ret;
}

//**********************************************************************************************
void StoryBoard_Item::ChangeParent(StoryBoard_Item *To)
{	if (MyParent) MyParent->MyChildren.DeleteInOrder(this);
	MyParent=To;
	if (MyParent) MyParent->MyChildren.Add(this);
}

//**********************************************************************************************
void StoryBoard_Item::SetFileName(char *FN)
{	if (!FN)
	{	NewTek_free(FileName);
		FileName=NULL;		
	}
	else
	{	FileName=(char*)NewTek_realloc(FileName,strlen(FN)+1);
		strcpy(FileName,FN);
	}
}

//**********************************************************************************************
char *StoryBoard_Item::GetFileName(void)
{	return FileName;
}

//**********************************************************************************************
StoryBoard_Item *StoryBoard_Item::NewStoryboardChild(void)
{	StoryBoard_Item *Ret=new StoryBoard_Item;
	if (!Ret) return NULL;
	Ret->MyParent=this;
	MyChildren.Add(Ret);
	return Ret;
}

//**********************************************************************************************
void StoryBoard_Item::KillAll(void)
{	// Delete all my children
	for(unsigned i=0;i<MyChildren.NoItems;i++)
		delete MyChildren[i];
	MyChildren.DeleteAll();

	if (FileName)	NewTek_free(FileName);
	if (Alias)		NewTek_free(Alias);

	FileName=NULL;
	Alias=NULL;
}

//**********************************************************************************************
bool StoryBoard_Item::LoadSave_SaveAllData(Stream_Out *OutputStream)
{	return LoadSave_SaveAllData_Selected(OutputStream);
}

//**********************************************************************************************
bool StoryBoard_Item::LoadSave_SaveAllData_Selected(Stream_Out *OutputStream,bool SelectedOnly)
{	// Save my string
	if (!OutputStream->PutString(FileName)) return false;
	if (!OutputStream->PutString(Alias)) return false;

	// Save my serial number
	if (!OutputStream->PutData(&Serial)) return false;	

	// Save all the in/out points
	if (!OutputStream->PutData(&Flags)) return false;

	if (!OutputStream->PutData(&CroutonFrame[StoryBoard_Video])) return false;
	if (!OutputStream->PutData(&InPoint[StoryBoard_Video])) return false;
	if (!OutputStream->PutData(&OutPoint[StoryBoard_Video])) return false;
	if (!OutputStream->PutData(&ClipDuration[StoryBoard_Video])) return false;
	if (!OutputStream->PutData(&StretchDuration[StoryBoard_Video])) return false;
	if (!OutputStream->PutData(&StretchCoeff[StoryBoard_Video])) return false;
	if (!OutputStream->PutData(&OriginalFrameRate[StoryBoard_Video])) return false;	
	if (!OutputStream->PutData(&OffSet[StoryBoard_Video])) return false;	
	if (!OutputStream->PutData(&CroutonFrame[StoryBoard_Audio])) return false;
	if (!OutputStream->PutData(&InPoint[StoryBoard_Audio])) return false;
	if (!OutputStream->PutData(&OutPoint[StoryBoard_Audio])) return false;
	if (!OutputStream->PutData(&ClipDuration[StoryBoard_Audio])) return false;
	if (!OutputStream->PutData(&StretchDuration[StoryBoard_Audio])) return false;
	if (!OutputStream->PutData(&StretchCoeff[StoryBoard_Audio])) return false;
	if (!OutputStream->PutData(&OriginalFrameRate[StoryBoard_Audio])) return false;	
	if (!OutputStream->PutData(&OffSet[StoryBoard_Audio])) return false;	

	// Get the number of children that need to be saved
	unsigned NoItems=MyChildren.NoItems;
	if (SelectedOnly) NoItems=GetNoSelectedChildren();

	// Save all my children
	if (!OutputStream->PutData(&NoItems)) return false;
	if (SelectedOnly)
	{	for(unsigned i=0;i<MyChildren.NoItems;i++)
		if (MyChildren[i]->GetSelected())
		{	if (!MyChildren[i]->SavePlugin(OutputStream)) return false;	}
	}
	else
	{	for(unsigned i=0;i<MyChildren.NoItems;i++)
		if (!MyChildren[i]->SavePlugin(OutputStream)) return false;	
	}

	// Am I selected ?
	if (!OutputStream->PutData(&Selected)) return false;

	// Success
	return true;
}

//**********************************************************************************************
bool StoryBoard_Item::LoadSave_LoadAllData(Stream_In *InputStream,DWORD Version)
{	// We cannot have any children at this stage
	KillAll();

	// Load my string
	char Temp[MAX_PATH];
	long StrLen;
	if (!InputStream->GetString(Temp,MAX_PATH,StrLen)) return false;
	if (!StrLen)	SetFileName(NULL);
	else			SetFileName(Temp);

	if (!InputStream->GetString(Temp,MAX_PATH,StrLen)) return false;
	if (!StrLen)	SetAlias(NULL);
	else			SetAlias(Temp);	

	// Get my serial number
	if (!InputStream->GetData(&Serial)) return false;

	// Save all the in/out points
	if (!InputStream->GetData(&Flags)) return false;

	if (!InputStream->GetData(&CroutonFrame[StoryBoard_Video])) return false;
	if (!InputStream->GetData(&InPoint[StoryBoard_Video])) return false;
	if (!InputStream->GetData(&OutPoint[StoryBoard_Video])) return false;
	if (!InputStream->GetData(&ClipDuration[StoryBoard_Video])) return false;
	if (!InputStream->GetData(&StretchDuration[StoryBoard_Video])) return false;
	if (!InputStream->GetData(&StretchCoeff[StoryBoard_Video])) return false;
	if (!InputStream->GetData(&OriginalFrameRate[StoryBoard_Video])) return false;	
	if (!InputStream->GetData(&OffSet[StoryBoard_Video])) return false;	
	if (!InputStream->GetData(&CroutonFrame[StoryBoard_Audio])) return false;
	if (!InputStream->GetData(&InPoint[StoryBoard_Audio])) return false;
	if (!InputStream->GetData(&OutPoint[StoryBoard_Audio])) return false;
	if (!InputStream->GetData(&ClipDuration[StoryBoard_Audio])) return false;
	if (!InputStream->GetData(&StretchDuration[StoryBoard_Audio])) return false;
	if (!InputStream->GetData(&StretchCoeff[StoryBoard_Audio])) return false;
	if (!InputStream->GetData(&OriginalFrameRate[StoryBoard_Audio])) return false;	
	if (!InputStream->GetData(&OffSet[StoryBoard_Audio])) return false;	

	// Load all my children
	unsigned NoChildren;
	if (!InputStream->GetData(&NoChildren)) return false;

	// Add all the children
	for(unsigned i=0;i<NoChildren;i++)
	{	// Create a new child
		StoryBoard_Item *NewItem=NewStoryboardChild();
		if (!NewItem) 
		{	DebugOutput("StoryBoard_Item::LoadSave_LoadAllData Error, cannot allocate memory for child !");
			return false;
		}

		// Load data into it
		if (!NewItem->LoadPlugin(InputStream)) return false;
	}

	// Am I selected ?
	if (!InputStream->GetData(&Selected)) return false;

	// Success
	return true;
}

//**********************************************************************************************
StoryBoard_Item::StoryBoard_Item(void)
{	MyParent=NULL;
	FileName=NULL;
	Alias=NULL;
	Selected=false;
	Serialise();

	// In and out point stuff
	Flags=StoryBoardFlag_Audio|StoryBoardFlag_Video;
	CroutonFrame[0]=0.0f;
	CroutonFrame[1]=0.0f;
	OffSet[0]=0.0f;
	OffSet[1]=0.0f;
	InPoint[0]=0.0f;
	InPoint[1]=0.0f;
	OutPoint[0]=600.0f;
	OutPoint[1]=600.0f;
	ClipDuration[0]=600.0f;
	ClipDuration[1]=600.0f;
	StretchDuration[0]=600.0f;
	StretchDuration[1]=600.0f;
	StretchCoeff[0]=1.0f;
	StretchCoeff[1]=1.0f;
	OriginalFrameRate[0]=30000.0f/1001.0f;
	OriginalFrameRate[1]=30000.0f/1001.0f;
}

//**********************************************************************************************
void StoryBoard_Item::DeleteChild(StoryBoard_Item *Chld)
{	if (!MyChildren.Exists(Chld)) return;
	Chld->ChangeParent(NULL);
	delete Chld;
}

//**********************************************************************************************
StoryBoard_Item::~StoryBoard_Item(void)
{	// Kill all my children
	KillAll();

	// Kill the filename
	SetFileName(NULL);
}

//**********************************************************************************************
void StoryBoard_Item::SetSelected(bool Flag)
{	Selected=Flag;
}

//**********************************************************************************************
bool StoryBoard_Item::GetSelected(void)
{	return Selected;
}

//**********************************************************************************************
void StoryBoard_Item::MergeTree(StoryBoard_Item *TreeToMerge,unsigned Index)
{	// We merge the items backwards
	while(TreeToMerge->GetNoChildren())
	{	// Get the pointer to the last item in the tree
		StoryBoard_Item *ChildToMove=(*TreeToMerge)[TreeToMerge->GetNoChildren()-1];

		// We now change the parent of it
		ChildToMove->MyParent=this;

		// We add it to my list
		MyChildren.Add(ChildToMove,Index);

		// Delete it from the list of the tree we are copying from
		TreeToMerge->MyChildren.DeleteEntry(TreeToMerge->GetNoChildren()-1);
	}
}

//**********************************************************************************************
void StoryBoard_Item::SelectAllChildren(bool Flag)
{	for(unsigned i=0;i<MyChildren.NoItems;i++)
		MyChildren[i]->SetSelected(Flag);
}

//**********************************************************************************************
void StoryBoard_Item::SetAlias(char *FN)
{	if (!FN)
	{	NewTek_free(Alias);
		Alias=NULL;
		return;
	}
	Alias=(char*)NewTek_realloc(Alias,strlen(FN)+1);
	strcpy(Alias,FN);
}

//**********************************************************************************************
char *StoryBoard_Item::GetAlias(void)
{	return Alias;
}

//**********************************************************************************************
SBD_Item_Info *StoryBoard_Item::BuildTreeForPlayback(SBD_Item_Info *Parent)
{	// What kind of entity am I ?
	if ((FileName)&&(strlen(FileName)))
	{	if (IsRTV(FileName)!=RTVLIB_UNKNOWN)
		{	return new SBD_Item_RTV(Parent,FileName);
		}
		else if (NewTek_IsDVE(FileName))
		{	int DVF=NewTek_GetDVF(FileName);
			if (DVF==-1) 
			{	return new SBD_Item_DVE(Parent,FileName);
			}
			if (DVF==1)
			{	return new SBD_Item_Fade(Parent);	
			}
		}
		return NULL;
	}

	// I am a folder
	SBD_Item_Info *Ret=new SBD_Item_Folder(Parent);

	// It is a folder :(
	for(unsigned i=0;i<MyChildren.NoItems;i++)
	{	SBD_Item_Info *Answer=MyChildren[i]->BuildTreeForPlayback(Ret);
	}

	return Ret;
}






