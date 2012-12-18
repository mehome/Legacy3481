#include "StdAfx.h"
#include <newtek/nt_tools_protos.h>

bool SBD_Item_Fade::DoRender_Transition(SBD_Item_Info_From_To *Item,int LocalFieldNum)				// Single entry always for transitions !
{	// Compute the fade ammount ...
	double XFadeAmmount = 256.0*(Item->GetCentreTime() / m_OriginalLength);

	unsigned u_XFadeAmmount  = int(0.5+XFadeAmmount);
	unsigned u_iXFadeAmmount = 256-u_XFadeAmmount;

#ifdef _DISPLAYINFO
	DebugOutput("%s : CrossFade Field Number : %d : [%d,%d]\n",Description,LocalFieldNum,u_XFadeAmmount,u_iXFadeAmmount);
#endif

	// Handle the special cases ...
	if (u_XFadeAmmount==0) 
	{	// Do nothing
	}
	else if (u_iXFadeAmmount==0)
	{	// We can just switch the destinations !
		// NO CPU time !
		Swap(Item->From->Memory,Item->To->Memory);
	}
	else
	{	interpolate_QWORDs_422(	(byte*)Item->From->Memory,u_XFadeAmmount,
								(byte*)Item->To  ->Memory,u_iXFadeAmmount,
								(byte*)Item->To  ->Memory,
								min(Item->To->MemorySize,Item->From->MemorySize)/2);
	}	

	// Success !!
	return true;
}

SBD_Item_Fade::SBD_Item_Fade(SBD_Item_Info *Parent) 
:SBD_Item_Info(Parent) 
{	// Setup the default settings
	SetDescription("CrossFade");
	SetOriginalLength((double)NewTek_fRound(m_OriginalFrameRate)/m_OriginalFrameRate);
	SetOriginalFrameRate(30000.0/1001.0);
	SetInPoint(0.0);
	SetOutPoint(m_OriginalLength);
	SetDuration(m_OriginalLength);
	SetTransition(true);
}

bool SBD_Item_Fade::Render_Transition(SBD_Item_Info_From_To *Items)
{	// We need to stop threads fighting for CPU time ...
	bool Ret=SBD_Item_Info::Render_Transition(Items);
	return Ret;
}