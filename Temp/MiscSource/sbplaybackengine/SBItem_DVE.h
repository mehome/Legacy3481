#ifndef __SBItem_DVE__
#define __SBItem_DVE__

#include <newtek/newtekrtme_protos.h>

class SBDDLL SBD_Item_DVE : public SBD_Item_Info
{	private:	// ****
				char *m_FileName;

	public:		// **** Multithreaded reading stuff ****
				virtual bool Render_ReadData(SBD_Item_Render_Buffer *Items);
					
				// **** Async Rendering Support ****
				virtual HandleCache_Item *Render_Async(SBD_Item_Render_Buffer *Items);

				// **** Transitions ****
				virtual bool DoRender_Transition(SBD_Item_Info_From_To *Item,int LocalFieldNum);			// Single entry always for transitions !
				virtual bool Render_Transition(SBD_Item_Info_From_To *Items);								// Linked list

				// **** Constructor ****
				SBD_Item_DVE(SBD_Item_Info *Parent,char *FileName);
				~SBD_Item_DVE(void);
};

#endif