#ifndef __SBItem_Fade__
#define __SBItem_Fade__

class SBDDLL SBD_Item_Fade : public SBD_Item_Info
{	public:		// **** Transitions ****
				virtual bool DoRender_Transition(SBD_Item_Info_From_To *Item,int LocalFieldNum);			// Single entry always for transitions !
				virtual bool Render_Transition(SBD_Item_Info_From_To *Items);								// Linked list

				// Constructor
				SBD_Item_Fade(SBD_Item_Info *Parent);
};

#endif