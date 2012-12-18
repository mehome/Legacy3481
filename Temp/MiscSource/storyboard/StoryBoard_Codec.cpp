#include "StdAfx.h"

//**********************************************************************************************
CVideoElement *CVideoElement_Get(StoryBoard_Codec *SC)
{	// Try casting to a project ...
	CProject		*CP=GetInterface<CProject>(SC);
	if (CP)	return CP->pRoot;
	
	// Try casting the parent to a videoelement
	CVideoElement	*VE=GetInterface<CVideoElement>(SC);
	return VE;
}

// **************** Project ************
StoryBoard_Codec_Project::StoryBoard_Codec_Project(void)
{}

// **************** RTV ****************
StoryBoard_Codec_RTV::StoryBoard_Codec_RTV(StoryBoard_Codec *Parent)
:CSrcRtv(CVideoElement_Get(Parent))
{};

bool StoryBoard_Codec_RTV::SetFileName(char *Fn)
{	return Open(Fn);
}

void StoryBoard_Codec_RTV::SetInOutPoints_Video(unsigned p_InPoint,unsigned p_OutPoint)
{	InPoint=p_InPoint;
	OutPoint=p_OutPoint;
}

unsigned StoryBoard_Codec_RTV::GetNoFrames_Video(void)
{	return dwFrameCount;
}

unsigned StoryBoard_Codec_RTV::GetFrameRate_Video(void) 
{	return PhyFPS; 
}

void StoryBoard_Codec_RTV::SetNoFramesStretch_Video(unsigned NoFrames)
{	FPS=(0.5 + (float)GetFrameRate_Video()*(float)GetNoFrames_Video()/(float)NoFrames);
}

unsigned StoryBoard_Codec_RTV::GetPlayBackFrameRate_Video(void)
{	return FPS;
}

void StoryBoard_Codec_RTV::SetOffset_Video(unsigned NoFrames)
{	// TODO !
}

// **************** AVI ****************
StoryBoard_Codec_AVI::StoryBoard_Codec_AVI(StoryBoard_Codec *Parent)
:CSrcAvi(CVideoElement_Get(Parent))
{}

bool StoryBoard_Codec_AVI::SetFileName(char *Fn)
{	return Open(Fn);
}

void StoryBoard_Codec_AVI::SetInOutPoints_Video(unsigned p_InPoint,unsigned p_OutPoint)
{	InPoint=p_InPoint;
	OutPoint=p_OutPoint;
}

unsigned StoryBoard_Codec_AVI::GetNoFrames_Video(void)
{	return dwFrameCount;
}

unsigned StoryBoard_Codec_AVI::GetFrameRate_Video(void) 
{	return PhyFPS; 
}

void StoryBoard_Codec_AVI::SetNoFramesStretch_Video(unsigned NoFrames)
{	FPS=(0.5 + 1000.0f*(float)GetFrameRate_Video()*(float)GetNoFrames_Video()/(float)NoFrames);
}

unsigned StoryBoard_Codec_AVI::GetPlayBackFrameRate_Video(void)
{	return FPS;
}

void StoryBoard_Codec_AVI::SetOffset_Video(unsigned NoFrames)
{	// TODO !
}

// **************** WAV ****************
StoryBoard_Codec_WAV::StoryBoard_Codec_WAV(StoryBoard_Codec *Parent)
:CSrcWav(CVideoElement_Get(Parent))
{}

bool StoryBoard_Codec_WAV::SetFileName(char *Fn)
{	return Open(Fn);
}

void StoryBoard_Codec_WAV::SetInOutPoints_Audio(unsigned p_InPoint,unsigned p_OutPoint)
{	InPoint=p_InPoint;
	OutPoint=p_OutPoint;
}

unsigned StoryBoard_Codec_WAV::GetNoFrames_Audio(void)
{	return dwFrameCount;
}

unsigned StoryBoard_Codec_WAV::GetFrameRate_Audio(void) 
{	return 29970; // TODO !
}

void StoryBoard_Codec_WAV::SetNoFramesStretch_Audio(unsigned NoFrames)
{	FPS=(0.5 + 1000.0f*(float)GetFrameRate_Audio()*(float)GetNoFrames_Audio()/(float)NoFrames);
}

unsigned StoryBoard_Codec_WAV::GetPlayBackFrameRate_Audio(void)
{	return FPS;
}

void StoryBoard_Codec_WAV::SetOffset_Audio(unsigned NoFrames)
{	InPointOffset=NoFrames;	// Is this correct ?
}

// **************** DVE ****************
StoryBoard_Codec_DVE::StoryBoard_Codec_DVE(StoryBoard_Codec *Parent)
:CTransitionDVE(CVideoElement_Get(Parent))
{}

bool StoryBoard_Codec_DVE::SetFileName(char *Fn)
{	// Get the DVE filename ready
	strcpy(szFileName,Fn);
	dwPhyWidth=720;
	dwPhyHeight=480;

	// Succes
	return true;
}

void StoryBoard_Codec_DVE::SetInOutPoints_Video(unsigned p_InPoint,unsigned p_OutPoint)
{	InPoint=p_InPoint;
	OutPoint=p_OutPoint;
}

unsigned StoryBoard_Codec_DVE::GetNoFrames_Video(void)
{	return 0;		// TODO !
}

unsigned StoryBoard_Codec_DVE::GetFrameRate_Video(void) 
{	return 29970;	// TODO !
}

void StoryBoard_Codec_DVE::SetNoFramesStretch_Video(unsigned NoFrames)
{	//FPS=(0.5 + 1000.0f*(float)GetFrameRate_Audio()*(float)GetNoFrames_Audio()/(float)NoFrames);
}

unsigned StoryBoard_Codec_DVE::GetPlayBackFrameRate_Video(void)
{	return 29970;	// TODO
}

void StoryBoard_Codec_DVE::SetOffset_Video(unsigned NoFrames)
{	// TODO !
}

// ************ Folder ************
StoryBoard_Codec_Folder::StoryBoard_Codec_Folder(StoryBoard_Codec *Parent)
:CFolder(CVideoElement_Get(Parent))
{}

void StoryBoard_Codec_Folder::SetInOutPoints_Audio(unsigned p_InPoint,unsigned p_OutPoint)
{}					// TODO !

void StoryBoard_Codec_Folder::SetInOutPoints_Video(unsigned p_InPoint,unsigned p_OutPoint)
{}					// TODO !

unsigned StoryBoard_Codec_Folder::GetNoFrames_Audio(void)
{	return 0;		// TODO !
}

unsigned StoryBoard_Codec_Folder::GetNoFrames_Video(void)
{	return 0;		// TODO !
}

unsigned StoryBoard_Codec_Folder::GetFrameRate_Video(void)
{	return 29970;	// TODO !
}

unsigned StoryBoard_Codec_Folder::GetFrameRate_Audio(void)
{	return 29970;	// TODO !
}

void StoryBoard_Codec_Folder::SetNoFramesStretch_Video(unsigned NoFrames)
{	//FPS=(0.5 + 1000.0f*(float)GetFrameRate_Audio()*(float)GetNoFrames_Audio()/(float)NoFrames);
	// TODO !
}

void StoryBoard_Codec_Folder::SetNoFramesStretch_Audio(unsigned NoFrames)
{	//FPS=(0.5 + 1000.0f*(float)GetFrameRate_Audio()*(float)GetNoFrames_Audio()/(float)NoFrames);
	// TODO !
}

unsigned StoryBoard_Codec_Folder::GetPlayBackFrameRate_Video(void)
{	return 29970;	// TODO
}

unsigned StoryBoard_Codec_Folder::GetPlayBackFrameRate_Audio(void)
{	return 29970;	// TODO
}

void StoryBoard_Codec_Folder::SetOffset_Video(unsigned NoFrames)
{	// TODO !
}

void StoryBoard_Codec_Folder::SetOffset_Audio(unsigned NoFrames)
{	// TODO !
}

// ************ CrossFade ************
StoryBoard_Codec_CrossFade::StoryBoard_Codec_CrossFade(StoryBoard_Codec *Parent)
:CTransitionXFade(CVideoElement_Get(Parent))
{}

unsigned StoryBoard_Codec_CrossFade::GetFrameRate_Video(void)
{	return 29970;	// TODO !
}

unsigned StoryBoard_Codec_CrossFade::GetPlayBackFrameRate_Video(void)
{	return 29970;	// TODO
}

void StoryBoard_Codec_CrossFade::SetOffset_Video(unsigned NoFrames)
{	// TODO !
}

//**********************************************************************************************
int StoryBoard_Codec_GetType(StoryBoard_Codec *Item)
{	if (GetInterface<StoryBoard_Codec_Project>(Item)) return CodecType_Project;
	if (GetInterface<StoryBoard_Codec_RTV>(Item)) return CodecType_RTVFile;
	if (GetInterface<StoryBoard_Codec_AVI>(Item)) return CodecType_AVIFile;
	if (GetInterface<StoryBoard_Codec_WAV>(Item)) return CodecType_WAVFile;
	if (GetInterface<StoryBoard_Codec_DVE>(Item)) return CodecType_DVEFile;
	if (GetInterface<StoryBoard_Codec_Folder>(Item)) return CodecType_Folder;
	if (GetInterface<StoryBoard_Codec_CrossFade>(Item)) return CodecType_CrossFade;
	return CodecType_Error;
}

StoryBoard_Codec *StoryBoard_Codec_NewType(int Type,StoryBoard_Codec *Parent)
{	switch(Type)
	{	case CodecType_RTVFile:		return new StoryBoard_Codec_RTV(Parent);
		case CodecType_AVIFile:		return new StoryBoard_Codec_AVI(Parent);
		case CodecType_DVEFile:		return new StoryBoard_Codec_DVE(Parent);
		case CodecType_WAVFile:		return new StoryBoard_Codec_WAV(Parent);
		case CodecType_CrossFade:	return new StoryBoard_Codec_CrossFade(Parent);
		case CodecType_Folder:		return new StoryBoard_Codec_Folder(Parent);
		case CodecType_Project:		return new StoryBoard_Codec_Project();
	}

	// Error,unknown type !
	return NULL;
}

//**********************************************************************************************
int StoryBoard_Item::GetEntryType(void)
{	// If we do not have a filename, then we must be a folder
	if (!FileName) return CodecType_Folder;

	// Get the file extension
	char *Extension=strrchr(FileName,'.');

	// Otherwise we need to look at what kind of file this is
	if (!_stricmp(Extension,".rtv")) return CodecType_RTVFile;

	// Are we an AVI ?
	if (!_stricmp(Extension,".avi")) return CodecType_AVIFile;

	// Are we a DVE ?
	if (!_stricmp(Extension,".dve")) return CodecType_DVEFile;

	// Are we a wave ?
	if (!_stricmp(Extension,".wav")) return CodecType_WAVFile;	

	// This item is an error !
	return CodecType_Error;
}

//**********************************************************************************************
StoryBoard_Codec *StoryBoard_Item::BuildCodecTree(void)
{	// If the parent does not have an entry, we need to sort things out
	if (!GetParent())
	{	if (!MyCodecElement) return NULL;
		return MyCodecElement;
	}
	
	// What kind of file are we
	return BuildCodecTree(GetEntryType());
}

StoryBoard_Codec *StoryBoard_Item::BuildCodecTree(int MyType)
{	// Get the current type	...
	if (MyType!=StoryBoard_Codec_GetType(MyCodecElement))
	{	if (MyCodecElement) delete MyCodecElement;
		MyCodecElement=StoryBoard_Codec_NewType(MyType,GetParent()?GetParent()->MyCodecElement:NULL);
		if (!MyCodecElement) return NULL;
	}

	// Debug info
	DebugOutput("StoryBoard_Item::Building Codec, File=%s Type=%s\n",FileName,GetTypeName(MyCodecElement));

	// Get all my children ready
	for(unsigned i=0;i<MyChildren.NoItems;i++)
		MyChildren[i]->BuildCodecTree();

	// return the result !
	return MyCodecElement;
}