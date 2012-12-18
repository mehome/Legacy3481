#ifndef __StoryBoard_ITEM__
#define __StoryBoard_ITEM__

//********************************************************************************************************************
// These are the defines that allow me to access both audio and video croutons
#define StoryBoard_Video	0
#define StoryBoard_Audio	1

#define StoryBoardFlag_Audio		1
#define StoryBoardFlag_Video		2
#define StoryBoardFlag_MuteAudioL	4
#define StoryBoardFlag_MuteAudioR	8

//********************************************************************************************************************
class SBDLL StoryBoard_Item //: public LoadSave
{	protected:		//**********************************************************************************************
					// Do we have children ?
					tList<StoryBoard_Item*>		MyChildren;

					//**********************************************************************************************
					StoryBoard_Item*			MyParent;

					//**********************************************************************************************
					// This is my filename
					char		*FileName;

					//**********************************************************************************************
					// This is the alias
					char		*Alias;

					//**********************************************************************************************
					// Am I currently selected
					bool		Selected;

					//**********************************************************************************************
					// Get the number of selected children
					unsigned	GetNoSelectedChildren(void);

					//**********************************************************************************************
					// Each storyboard item is identified uniquely by a GUID
					GUID		Serial;										

					//**********************************************************************************************
						// The in and out points
						int		Flags;
						float	InPoint[2],OutPoint[2];	// The in and out points
						float	ClipDuration[2];		// The number of frames in the clip
						float	CroutonFrame[2];		// The frame number that is used for the icon
						float	OffSet[2];				// The offset ...

						float	StretchDuration[2];		// Used for time-stretching
						float	StretchCoeff[2];		// This is to ensure that we do not loose to much inaccuracy
														// with UI interaction. It is the ratio of the lengths

						float	OriginalFrameRate[2];	// The original framerate of the clip

	public:			//**********************************************************************************************
					// Get and set in and out points
					int GetFlags(void);
					void SetFlags(int Val);

					int GetCroutonFrame(unsigned ID=StoryBoard_Video);
					void SetCroutonFrame(int Val,unsigned ID=StoryBoard_Video);

					float GetOffSet(unsigned ID=StoryBoard_Video);
					void SetOffSet(float Val,unsigned ID=StoryBoard_Video);

					float GetInPoint(unsigned ID=StoryBoard_Video);
					void SetInPoint(float Val,unsigned ID=StoryBoard_Video);

					float GetOutPoint(unsigned ID=StoryBoard_Video);
					void SetOutPoint(float Val,unsigned ID=StoryBoard_Video);

					int	GetClipDuration(unsigned ID=StoryBoard_Video);
					void SetClipDuration(int Val,unsigned ID=StoryBoard_Video);

					float GetStretchDuration(unsigned ID=StoryBoard_Video);
					void SetStretchDuration(float Val,unsigned ID=StoryBoard_Video,bool RecomputeCoeffs=true);

					float GetStretchCoeff(unsigned ID=StoryBoard_Video);
					void SetStretchCoeff(float Val,unsigned ID=StoryBoard_Video);

					float GetOriginalFrameRate(unsigned ID=StoryBoard_Video);
					void SetOriginalFrameRate(float Val,unsigned ID=StoryBoard_Video);
		
					//**********************************************************************************************
					// Constructor and destructor
					StoryBoard_Item(void);
					~StoryBoard_Item(void);

					//**********************************************************************************************
					// Setup the filename for this item
					void SetFileName(char *FN);
					char *GetFileName(void);

					//**********************************************************************************************
					// This recurses to find the first valid filename in the children
					char *GetFirstValidFileName(void);

					//**********************************************************************************************
					// Handle tha liases
					void SetAlias(char *FN);
					char *GetAlias(void);

					//**********************************************************************************************
					// The loading and saving interface
					virtual bool LoadSave_SaveAllData(Stream_Out *OutputStream);
					virtual bool LoadSave_SaveAllData_Selected(Stream_Out *OutputStream,bool SelectedOnly=false);
					virtual bool LoadSave_LoadAllData(Stream_In *InputStream,DWORD Version);

					//**********************************************************************************************
					// Get the children					
					StoryBoard_Item* &operator [] (unsigned long i)		{ return MyChildren[i]; }
					unsigned GetNoChildren(void)						{ return MyChildren.NoItems; }

					//**********************************************************************************************
					// Change my parent
					void ChangeParent(StoryBoard_Item *To);

					//**********************************************************************************************
					// Get the top parent
					StoryBoard_Item *GetTopParent(void);
					StoryBoard_Item *GetParent(void);

					//**********************************************************************************************
					// Get a new child
					StoryBoard_Item *NewStoryboardChild(void);

					//**********************************************************************************************
					// Am I selected or not ?
					void SetSelected(bool Flag=true);
					bool GetSelected(void);

					//**********************************************************************************************
					// Deselect all my children
					void SelectAllChildren(bool Flag=true);
					void DeselectAllChildren(void)
					{ SelectAllChildren(false); }

					//**********************************************************************************************
					// Copy another item
					bool CopyFrom(StoryBoard_Item *From,bool SelectedOnly=false,bool CopySerial=false,bool CopyCodec=false);

					//**********************************************************************************************
					// Get the position of the last selected item
					// returns 0xffffffff if there is no selection
					unsigned GetLastSelected(void);

					//**********************************************************************************************
					// Delete all selected children
					void DeleteSelectedChildren(void);
					void DeleteChild(StoryBoard_Item *Chld);

					// Kill everything that I currently own
					void KillAll(void);
					void DeleteAllChildren(void) { KillAll(); }

					//**********************************************************************************************
					// Merge this with another tree. The TreeToMerge children are actually moved into
					// the current tree to avoid excess memory copies.
					// The index represents the first position that the new tree will fill
					void MergeTree(StoryBoard_Item *TreeToMerge,unsigned Index=0xffffffff);

					//**********************************************************************************************
					// Serialise this child, and optionally all children below me
					void Serialise(bool SerialiseChildren=false);
					StoryBoard_Item* FindItem(GUID SerialNo);
					GUID GetSerialNunber(void);

					//**********************************************************************************************
					// This will build a tree for playback
					SBD_Item_Info *BuildTreeForPlayback(SBD_Item_Info *Parent);
};


#endif