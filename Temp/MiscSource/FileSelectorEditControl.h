#ifndef	__FileSelectorEditControl__
#define	__FileSelectorEditControl__

class FileSelectorDLL ButtonRequestorBitmapResource {
	public:
		static void AddRef() {InterlockedIncrement(&s_Ref);}
		static void Release();
		static ScreenObject_BitmapFile *GetBitMaps(unsigned ButtonState);
	private:
		static long s_Ref;
		static ButtonRequestorBitmapResource *s_Instance;
		ScreenObject_BitmapFile m_RequestorBitmaps[4];
	};


//This is a kind of UtilLib_Edit that includes a button to bring up a file requestor (when in focus) to assist in string input
class FileSelectorDLL UtilLib_Edit_FileSelector : public UtilLib_Edit {
	public:
		UtilLib_Edit_FileSelector();
		virtual ~UtilLib_Edit_FileSelector();

		virtual void InitialiseWindow(void);
		virtual void DestroyWindow(void);

		//set up listening to this text item
		virtual void Controls_Edit_SetItem(TextItem *ItemToEdit);

		virtual const char *GetFileSelectorPluginName() {return "FileSelector";}
		virtual bool ClickToSelectFolders() {return false;}
		void OpenFileSelector();
	protected:
		//For a cleaner separation from utilLib edit we'll make an independant listener for focus
		class FSEC_Listener : public DynamicListener {
			public:
				FSEC_Listener(UtilLib_Edit_FileSelector *parent) : m_Parent(parent) {}
				virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);
			private:
				UtilLib_Edit_FileSelector * const m_Parent;
			} *m_FSEC_Listener;
		friend FSEC_Listener;

		UtilLib_Button *m_RequestorButton;
		FileSelector *m_FileSelector;

		DynamicInt m_ButtonToUse; //listen for button changes

		void OpenRequestButton();
		void CloseRequestButton();
	};

class FileSelectorDLL UtilLib_Edit_DirectorySelector : public UtilLib_Edit_FileSelector {
	public:
		virtual const char *GetFileSelectorPluginName() {return "DirectorySelector";}
		virtual bool ClickToSelectFolders() {return true;}
	};


//! A small class for ensuring that the width stays correct
class FileSelectorDLL AssetEditDirectory : public UtilLib_Edit_DirectorySelector
{
public:
	virtual long LayoutHints_GetPreferedXSize(void) {return -1;}
	virtual long LayoutHints_GetPreferedYSize(void) {return -1;}
};

#endif	__FileSelectorEditControl__