#ifndef __StoryBoard_Codec__
#define __StoryBoard_Codec__

//********************************************************************************************************************
enum	{	CodecType_Error, 
			CodecType_RTVFile, 
			CodecType_AVIFile, 
			CodecType_DVEFile, 
			CodecType_WAVFile,
			CodecType_CrossFade,
			CodecType_Folder,
			CodecType_Project
		};

class SBDLL StoryBoard_Codec
{	public:		// Set the filename being used
				virtual bool SetFileName(char *FN) { return true; }

				virtual void SetInOutPoints_Audio(unsigned InPoint,unsigned OutPoint) {}
				virtual void SetInOutPoints_Video(unsigned InPoint,unsigned OutPoint) {}

				virtual void SetOffset_Audio(unsigned NoFrames) {}
				virtual void SetOffset_Video(unsigned NoFrames) {}

				virtual unsigned GetNoFrames_Audio(void) { return 0; }
				virtual unsigned GetNoFrames_Video(void) { return 0; }

				virtual unsigned GetFrameRate_Audio(void) { return 29970; }
				virtual unsigned GetFrameRate_Video(void) { return 29970; }

				virtual void SetNoFramesStretch_Audio(unsigned NoFrames) {}
				virtual void SetNoFramesStretch_Video(unsigned NoFrames) {}

				virtual unsigned GetPlayBackFrameRate_Audio(void) { return 29970; }
				virtual unsigned GetPlayBackFrameRate_Video(void) { return 29970; }				
};

class SBDLL StoryBoard_Codec_Project : public StoryBoard_Codec, public CProject
{	public:		// Constructor
				StoryBoard_Codec_Project(void);				
};

class SBDLL StoryBoard_Codec_RTV : public StoryBoard_Codec, public CSrcRtv
{	public:		// Constructor
				StoryBoard_Codec_RTV(StoryBoard_Codec *Parent);

				// Set the filename to use
				virtual bool		SetFileName(char *FN);
				virtual void		SetInOutPoints_Video(unsigned InPoint,unsigned OutPoint);
				virtual unsigned	GetNoFrames_Video(void);
				virtual unsigned	GetFrameRate_Video(void);
				virtual void		SetNoFramesStretch_Video(unsigned NoFrames);
				virtual unsigned	GetPlayBackFrameRate_Video(void);
				virtual void		SetOffset_Video(unsigned NoFrames);
};

class SBDLL StoryBoard_Codec_AVI : public StoryBoard_Codec, public CSrcAvi
{	public:		// Constructor
				StoryBoard_Codec_AVI(StoryBoard_Codec *Parent);

				// Set the filename to use
				virtual bool		SetFileName(char *FN);
				virtual void		SetInOutPoints_Video(unsigned InPoint,unsigned OutPoint);
				virtual unsigned	GetNoFrames_Video(void);
				virtual unsigned	GetFrameRate_Video(void);
				virtual void		SetNoFramesStretch_Video(unsigned NoFrames);
				virtual unsigned	GetPlayBackFrameRate_Video(void);
				virtual void		SetOffset_Video(unsigned NoFrames);
};

class SBDLL StoryBoard_Codec_WAV : public StoryBoard_Codec, public CSrcWav
{	public:		// Constructor
				StoryBoard_Codec_WAV(StoryBoard_Codec *Parent);

				// Set the filename to use
				virtual bool		SetFileName(char *FN);
				virtual void		SetInOutPoints_Audio(unsigned InPoint,unsigned OutPoint);
				virtual unsigned	GetNoFrames_Audio(void);
				virtual unsigned	GetFrameRate_Audio(void);
				virtual void		SetNoFramesStretch_Audio(unsigned NoFrames);
				virtual unsigned	GetPlayBackFrameRate_Audio(void);
				virtual void		SetOffset_Audio(unsigned NoFrames);
};

class SBDLL StoryBoard_Codec_DVE : public StoryBoard_Codec, public CTransitionDVE
{	public:		// Constructor
				StoryBoard_Codec_DVE(StoryBoard_Codec *Parent);

				// Set the filename to use
				virtual bool		SetFileName(char *FN);
				virtual void		SetInOutPoints_Video(unsigned InPoint,unsigned OutPoint);
				virtual unsigned	GetNoFrames_Video(void);
				virtual unsigned	GetFrameRate_Video(void);
				virtual void		SetNoFramesStretch_Video(unsigned NoFrames);
				virtual unsigned	GetPlayBackFrameRate_Video(void);
				virtual void		SetOffset_Video(unsigned NoFrames);
};

class SBDLL StoryBoard_Codec_Folder : public StoryBoard_Codec, public CFolder
{	public:		// Constructor
				StoryBoard_Codec_Folder(StoryBoard_Codec *Parent);
				virtual void		SetInOutPoints_Audio(unsigned InPoint,unsigned OutPoint);
				virtual void		SetInOutPoints_Video(unsigned InPoint,unsigned OutPoint);
				virtual unsigned	GetNoFrames_Audio(void);
				virtual unsigned	GetNoFrames_Video(void);
				virtual unsigned	GetFrameRate_Video(void);
				virtual unsigned	GetFrameRate_Audio(void);
				virtual void		SetNoFramesStretch_Video(unsigned NoFrames);
				virtual void		SetNoFramesStretch_Audio(unsigned NoFrames);
				virtual unsigned	GetPlayBackFrameRate_Audio(void);
				virtual unsigned	GetPlayBackFrameRate_Video(void);
				virtual void		SetOffset_Audio(unsigned NoFrames);
				virtual void		SetOffset_Video(unsigned NoFrames);
};

class SBDLL StoryBoard_Codec_CrossFade : public StoryBoard_Codec, public CTransitionXFade
{	public:		// Constructor
				StoryBoard_Codec_CrossFade(StoryBoard_Codec *Parent);
				virtual unsigned	GetFrameRate_Video(void);
				virtual unsigned	GetPlayBackFrameRate_Video(void);
				virtual void		SetOffset_Video(unsigned NoFrames);
};

int SBDLL StoryBoard_Codec_GetType(StoryBoard_Codec *Item);
StoryBoard_Codec SBDLL *StoryBoard_Codec_NewType(int Type,StoryBoard_Codec *Parent);

#endif