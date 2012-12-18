#ifndef __SBFolder__
#define __SBFolder__

//****************************************************************************************************************************
class SBDDLL SBD_Item_Folder_StartStopTimes
{	public:		// Rather a simple class really ...
					double Start,End;

				// Is this on the upper or lower row 
				// of a 'timeline' ... for transitions, this specifies
				// which is the row the transition is from
					bool UpperRow;

				// Since there is no necessarily a 1->1 relationship between items actually used
				// and those in the project !
					SBD_Item_Info *Item;
};

//****************************************************************************************************************************
class SBDDLL SBD_Item_Folder : public SBD_Item_Info
{	protected:	// My list of starting and ending points
					tList<SBD_Item_Folder_StartStopTimes> Timing;

				// Get a timing position
					unsigned GetTimingIndex(double Time,bool First);

	public:		// Constructor
					SBD_Item_Folder(SBD_Item_Info *Parent);

				// This is the most important funcion
					virtual bool DoRender(unsigned NoItems,SBD_Item_Render_Buffer **Items);

				// Notificatoin that 
					virtual double Calculate(void);
};

//****************************************************************************************************************************
class SBDDLL SBD_Item_Project : public SBD_Item_Folder
{	public:		// Constructor
					SBD_Item_Project(void);
};

#endif