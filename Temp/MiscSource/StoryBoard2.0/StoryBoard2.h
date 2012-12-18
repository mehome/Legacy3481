#ifndef __STORYBOARD2__
#define __STORYBOARD2__

class StoryBoard2_Crouton;

class SB2DLL StoryBoard2 : public UtilLib_SimpleGrid {
	public:
		StoryBoard2();
		~StoryBoard2();
		// Get the window ready (from BaseWindowClass)
		virtual void InitialiseWindow(void);

		virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
		WindowLayout_Item *DragNDrop_DropItemsHere
		   ( int Width,int Height,    // Window size
			int MousePosnX,int MousePosnY,  // Mouse posn
			WindowLayout_Item *ObjectsInWindow, // The objects already in the window
			WindowLayout_Item *ObjectsDropped, // The objects that have been dropped in the window
			bool Resizing,bool DroppedHere
		   );

		virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);
		virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

		void Ripple(double StartTime,double Adjustment);
		void DeleteSelectedButtons();

		void SetTimeLineToEdit(TimeLine* timeLine);
	protected:
		enum TLEtype { //Yes these are local to StoryBoard only
			//Ok I'm cheating here... I'm sorting these by priority
			//TODO we may wish to add OverlayAudio OverlayVideo and Utility
			TLE_Trans=1,
			TLE_Clip=2,
			TLE_Over=4,
			TLE_Audio=8,
			TLE_OverAudio=16,
			TLE_Utility=32
			};

		TLEtype GetTLEtype(ContentSource *source);
		TimeLineElement *GetTLE(WindowLayout_Item *icon);

		TimeLine m_DefaultTimeLine;
		TimeLine *m_TimeLine;  //This is the whole storyboard project

	private:
		//These functions deal with insertion
		int NumOrderPrev(tList<StoryBoard2_Crouton *> *SClist,int DefaultValue);
		TimeLineElement *FindLastEvent(tList<StoryBoard2_Crouton *> *SClist,int flags,unsigned *index=NULL);
		int GetPrevAcceptableEvent(TLEtype EventType);
		bool UseNewEventSPFromPrev(ContentSource *PrevSource,ContentSource *Source);
		double ComputeStartingPoint(TimeLineElement *PrevElement,ContentInstance *ci);
		//These functions deal with deletion
		void DeleteTLE(TimeLineElement *tle);
		//These functions deal with refreshing the storyboard
		void SortTLEs();
		bool SortCondition(TimeLineElement *compvar,TimeLineElement *scanvar);
	};

#endif //__STORYBOARD2__

