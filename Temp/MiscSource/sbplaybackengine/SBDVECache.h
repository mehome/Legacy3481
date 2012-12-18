#ifndef __SBDVECache__
#define __SBDVECache__

// must be 2^N
#define DVECache_Size		16

//***************************************************************************************************************************
class SBDDLL StoryBoard_DVE : public VariableBlock
{	public:		// **** 
				struct FXFileHandle		*fxfh;
				char					*m_FileName;
				unsigned				YRes;
				unsigned				NoFrames;
				double					FrameRate;

				StoryBoard_DVE(char *FN);
				~StoryBoard_DVE(void);
};

//***************************************************************************************************************************
class SBDDLL DVECache : public VariableBlock
{	protected:		StoryBoard_DVE *m_StoryBoard_DVE[DVECache_Size];

	public:			DVECache(void);
					~DVECache(void);

					// Returns a DVE handle, which is currently locked ...
					// To unlock it to allow it to be deleted, etc...
					// do (StoryBoard_DVE *)->UnBlock();
					StoryBoard_DVE *GetDVE(char *Fn);
};

#endif