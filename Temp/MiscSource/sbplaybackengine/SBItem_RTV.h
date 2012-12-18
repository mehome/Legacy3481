#ifndef __SBItem_RTV__
#define __SBItem_RTV__

class SBDDLL SBD_Item_RTV : public SBD_Item_Info
{	private:		// A local copy of my filename
					char *m_FileName;

					// The internal resolution of this file
					unsigned XRes,YRes;

	public:			// **** Multithreaded reading stuff ****
					virtual bool Render_ReadData(SBD_Item_Render_Buffer *Items);
					
					// **** Async Rendering Support ****
					virtual HandleCache_Item *Render_Async(SBD_Item_Render_Buffer *Items);
		
					// Constructor and destructor
					SBD_Item_RTV(SBD_Item_Info *p_Parent,char *FileName);
					~SBD_Item_RTV(void);	
};

#endif