#ifndef __WM_DECKCONTROLGUIBASE_H__
#define __WM_DECKCONTROLGUIBASE_H__

class DECK_CONTROL_DLL DeckControlGUIBase : public DynamicListener,
											public DeckControlGUI_Interface {
	public:
		DeckControlGUIBase(DeckControlConfigData *config);
		virtual ~DeckControlGUIBase();
		//The parent GUI will call this to specify which deck he is currently working with
		//This should update all the BobDots/BWC's to match the new devices current channel setting
		virtual void SetCurrentDevice(int device);
		//This should call ...AddNewBobDotToList() if creating BobControl_Video windows
		virtual BaseWindowClass *CreateNewBobDot(BaseWindowClass *Parent,RECT rect);

		virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);
	protected:
		//This should only called once from CreateNewBobDot() if your using BobControl_Video windows
		void AddNewBobDotToList(BobControl_Video *bcv);
	private:
		//collect all of the bob dots created here
		tList2<BobControl_Video *> m_OpenedBobDots;
		DeckControlConfigData *m_ConfigData;
	};

#endif __WM_DECKCONTROLGUIBASE_H__

/*
Note to Check(Possible TODO): 
There are several functions in here which need BobControl_Video to Get and Set the InputChannel...
If Users are going to typically inherit from this class to create new kinds of BobDots then this should
be fine;  However if they are not... then we should have a new interface that each BobDot should use so
that this Base Class will be more flexible.
*/
