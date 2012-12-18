#include "stdafx.h"
#include "Resource.h"

StoryBoard_ViewMode::StoryBoard_ViewMode() {
	m_InOutMode=false;
	m_AudioRange=1.0;

	m_TimeCodeIn.SetColor(255,255,255);
	m_TimeCodeIn.SetFont("Arial");
	m_TimeCodeIn.SetFontSize(-0.75);
	m_TimeCodeIn.SetAlignment(TextItem_CentreHAlign|TextItem_CentreVAlign);
	m_TimeCodeOut.SetColor(255,255,255);
	m_TimeCodeOut.SetFont("Arial");
	m_TimeCodeOut.SetFontSize(-0.75);
	m_TimeCodeOut.SetAlignment(TextItem_CentreHAlign|TextItem_CentreVAlign);

	m_TimeCodeFloatIn.SetItemToLockTo(&m_TimeCodeIn);
	m_TimeCodeFloatOut.SetItemToLockTo(&m_TimeCodeOut);
	m_TimeCodeFloatOut.SetOutPointMode(true);
	}

/*
StoryBoard2_Crouton::~StoryBoard2_Crouton() {
	RemoveAllDependantsToThis();
	}
*/

void StoryBoard_ViewMode::PaintWindow(HWND hWnd,HDC hdc,StoryBoard2_Crouton *sc) {
//	InOut_Wrapper *wrapper=&(s_CroutonListener.m_InOutWrapper);
	InOut_Wrapper *wrapper=sc->s_CroutonListener.GetWrapper(sc->m_contentInstance);
	unsigned l_SubRegionFocus;
	if (sc->Clicked) l_SubRegionFocus=m_SubRegionFocus;
	else l_SubRegionFocus=GetSubRegionFocus(sc);
	//We have to set the in and out points here before calling the predecessor so that the GetFileIcon() will have the
	//correct value

	if (sc->m_IsAltKeyPressed) {
		m_TimeCodeFloatIn.SetFrameRate(wrapper->InOut_GetFrameRate());
		m_TimeCodeFloatOut.SetFrameRate(wrapper->InOut_GetFrameRate());
		sc->ReceiveMessagesOff();
		if ((sc->m_IsCtrlKeyPressed)&&(m_AudioStream)) {
			m_TimeCodeFloatIn.Set(GetInPoint(wrapper,m_AudioStream));
			m_TimeCodeFloatOut.Set(GetOutPoint(wrapper,m_AudioStream));
			}
		else {
			m_TimeCodeFloatIn.Set(GetInPoint(wrapper,InOuts2_All));
			m_TimeCodeFloatOut.Set(GetOutPoint(wrapper,InOuts2_All));
			}
		sc->ReceiveMessagesOn();
		}

	// Draw the standard file button stuff
	sc->VideoEditor_Crouton::PaintWindow(hWnd,hdc);
	if (sc->m_contentInstance) {
		// Get the window size
		RECT rect;
		GetClientRect(hWnd,&rect);

		//bring in the edges to save the selection outline
		rect.left+=1;
		rect.right-=1;
		rect.top+=1;
		rect.bottom-=1;
		unsigned YPosn=rect.top;

		// We are about to draw the Discreet style bar
		ContentInstance *ci=sc->m_contentInstance;
		char *VideoStream,*AudioStream,*TransStream;
		double in,out,cliplength;
		float inpoint,outpoint;
		unsigned NumberofStreams,ReservedStreams=0;

		{//set the menuitems for streams
			VideoStream=AudioStream=TransStream=NULL;
			tList<char *> streamlist;ci->InOut_GetStreamTypes(&streamlist);

				unsigned i;
				NumberofStreams=streamlist.NoItems;
				for (i=0;i<NumberofStreams;i++) {
					char *stream=streamlist[i];
					if (!TransStream) if (!strnicmp(stream,"trans",5)) {
						TransStream=stream;
						ReservedStreams++;
						}
					if (!VideoStream) if (!strnicmp(stream,"video",5)) {
						VideoStream=stream;
						ReservedStreams++;
						//TODO we may want to get to have transition and video mutually exclusive
						}
					if (!AudioStream) if (!strnicmp(stream,"audio",5)) {
						AudioStream=stream;
						ReservedStreams++;
						}
					}
			}

		YPosn=1;

		unsigned streamindex=0;
		//Croutons only paint the reserved streams!
		for (unsigned i=0;i<ReservedStreams;i++) {
			char *currentstream;
			unsigned long medcolor;

			switch (streamindex) {
				case 0:
					if (TransStream) {
						currentstream=TransStream;
						medcolor=StoryBoard2_Crouton_Med_Trans;
						streamindex++;
						break;
						}
					else streamindex++;
				case 1:
					if (VideoStream) {
						currentstream=VideoStream;
						medcolor=StoryBoard2_Crouton_Med_Video;
						streamindex++;
						break;
						}
					else streamindex++;
				case 2:
					if (AudioStream) {
						currentstream=AudioStream;
						medcolor=StoryBoard2_Crouton_Med_Audio;
						streamindex++;
						break;
						}
					else streamindex++;
				case 3:
					return;  //this should never happen
				}

			if (sc->m_IsAltKeyPressed) {
				in=wrapper->InOut_GetInPointStyle(currentstream,eInOut_OutsideTransitions);
				out=wrapper->InOut_GetOutPointStyle(currentstream,eInOut_OutsideTransitions);
				}
			else {
				in=ci->InOut_GetInPointStyle(currentstream,eInOut_OutsideTransitions);
				out=ci->InOut_GetOutPointStyle(currentstream,eInOut_OutsideTransitions);
				}

			cliplength=ci->InOut_GetDefaultLength();
			if (cliplength) {
				if (cliplength < 0.0) cliplength *= -1.0;

				inpoint=max((float)((rect.right*(in/cliplength))+rect.left),rect.left);
				outpoint=min((float)(rect.right*(out/cliplength)),(float)(rect.right));
				// Is the clip reversed ?
				if (out<in) medcolor=StoryBoard2_Crouton_Reversed;


				NewTek_DrawRectangle(hdc,inpoint,YPosn,outpoint,StoryBoard2_Crouton_BarHeight+YPosn,medcolor);

				YPosn+=StoryBoard2_Crouton_AudioOffset;
				}
			}
		//This will print the TimeCode... we'll take advantage of the above local variables
		if (sc->m_IsAltKeyPressed) {
			rect.left+=4;
			rect.right-=4;
			rect.bottom-=15;
			rect.top=rect.bottom-10;

			TextItem *l_TimeCode;
			if (l_SubRegionFocus==1) {
				rect.right=rect.left+m_TimeCodeIn.ScreenObject_GetPreferedXSize();
				l_TimeCode=&m_TimeCodeIn;
				}
			else {
				rect.left=rect.right-m_TimeCodeOut.ScreenObject_GetPreferedXSize();
				l_TimeCode=&m_TimeCodeOut;
				}
			int OldRop2=SetROP2(hdc,R2_MASKPEN);
			NewTek_DrawRectangle(hdc,rect.left-2,rect.top-2,rect.right+2,rect.bottom,RGB(127,127,127));
			SetROP2(hdc,OldRop2);
			l_TimeCode->ScreenObject_DrawItem(&rect,hdc);
			}

		} //end if ci
	} //end PaintWindow()


ScreenObject* StoryBoard_ViewMode::GetFileIcon(StoryBoard2_Crouton *sc) {
	ScreenObject *ret=NULL;
	if (sc->m_IsAltKeyPressed) {
		unsigned l_SubRegionFocus;
		if (sc->Clicked) l_SubRegionFocus=m_SubRegionFocus;
		else l_SubRegionFocus=GetSubRegionFocus(sc);

		if (l_SubRegionFocus==1) {
			sc->m_InScreenObject->SetPreviewOnProgramOutWhenChanging(true, false);
			sc->m_OutScreenObject->SetPreviewOnProgramOutWhenChanging(false, true);
			ret=sc->m_InScreenObject;
			}
		else {
			sc->m_InScreenObject->SetPreviewOnProgramOutWhenChanging(false, false);
			sc->m_OutScreenObject->SetPreviewOnProgramOutWhenChanging(true, true);
			ret=sc->m_OutScreenObject;
			}
		}

	ContentInstance *ci=sc->VideoEditor_Crouton_GetContentInstance();
	if ((!ret)&&(ci)) if (ci->ContentInstance_GetTimeLineElement())
		ret=sc->VideoEditor_Crouton_GetContentInstance()->ContentInstance_GetMoneyShotPreview();

	return (ret);
	}

void StoryBoard_ViewMode::MouseLButtonClick(long Flags,long x,long y,StoryBoard2_Crouton *sc) {
	bool IsAltKeyPressed=(bool)(GetAsyncKeyState(VK_MENU)&((short)(1<<15)));

	if (IsAltKeyPressed) {
		//Set some stuff up for MouseMoved()
		{//set the menuitems for streams
			m_VideoStream=m_AudioStream=NULL;
			tList<char *> streamlist;sc->m_contentInstance->InOut_GetStreamTypes(&streamlist);

				unsigned i,eoi=streamlist.NoItems;
				for (i=0;i<eoi;i++) {
					char *stream=streamlist[i];
					if (!m_VideoStream) if (!strnicmp(stream,"video",5)) m_VideoStream=stream;
					if (!m_VideoStream) if (!strnicmp(stream,"trans",5)) m_VideoStream=stream; //this is temporary
					if (!m_AudioStream) if (!strnicmp(stream,"audio",5)) m_AudioStream=stream;
					}
			}
		m_SubRegionFocus=GetSubRegionFocus(sc);
		m_FrameRate=sc->m_contentInstance->InOut_GetFrameRate();

		// Capture mouse input
		sc->Clicked=true;
		m_InOutMode=true;
		sc->ExclusiveInput(true);

		// Get the mouse position
		GetCursorPos(&pt);
		// Hide the mouse cursor
		ShowCursor(false);
		}
	// Call the base Class
	sc->VideoEditor_Crouton::MouseLButtonClick(Flags,x,y);
	}

void StoryBoard_ViewMode::MouseLButtonRelease(long Flags,long x,long y,StoryBoard2_Crouton *sc) {
	if (sc->Clicked) {
		// Set the mouse position
		// Move back to the original position
		SetCursorPos(pt.x,pt.y);	
		
		// Capture mouse input
		sc->Clicked=false;
		m_InOutMode=false;
		sc->ExclusiveInput(false);

		// Show the mouse cursor
		ShowCursor(true);
		//Update Ripple for inout change
		StoryBoard_Layout *sbl=GetWindowInterface<StoryBoard_Layout>(sc->GetParent());
		if (sbl) {
			InOut_Wrapper *wrapper=sc->s_CroutonListener.GetWrapper(sc->m_contentInstance);
			sbl->InOut_Editor_CompleteChanges(wrapper);
			}
		}
	//This resets the cursor to a pointer
	sc->VideoEditor_Crouton::MouseMoved(Flags,x,y);
	sc->VideoEditor_Crouton::MouseLButtonRelease(Flags,x,y);
	}

// Handle the mouse functions
void StoryBoard_ViewMode::MouseMoved(long Flags,long x,long y,StoryBoard2_Crouton *sc) {
	
	if (!sc->Clicked) {
		//bool IsAltKeyPressed=(bool)(GetAsyncKeyState(VK_MENU)&((short)(1<<15)));
		if (sc->m_IsAltKeyPressed) {
			if (GetSubRegionFocus(sc)==1) {
				SetCursor(LoadCursor(g_VideoEditor_StoryBoard,MAKEINTRESOURCE(IDC_INCURSOR)));
				sc->RePaint();
				}
			else {
				SetCursor(LoadCursor(g_VideoEditor_StoryBoard,MAKEINTRESOURCE(IDC_OUTCURSOR)));
				sc->RePaint();
				}
			}
		else sc->VideoEditor_Crouton::MouseMoved(Flags,x,y);
		return;
		}


	if (m_InOutMode) {
		POINT PtNew;
		int dValue;
		InOut_Wrapper *wrapper=sc->s_CroutonListener.GetWrapper(sc->m_contentInstance);

		// Get the current cursor position
		GetCursorPos(&PtNew);	

		dValue=((PtNew.x-pt.x)>>1);

		// If we have not moved, do nothing
		if (!dValue) return;

		//AdustInOut(double Frames)
		{ //TODO move this to its own function
			ContentInstance *ci=sc->m_contentInstance;
			double framerate=m_FrameRate;
			double Adjustment=((double)dValue)/framerate;
			bool IsShiftKeyPressed=(bool)(GetAsyncKeyState(VK_SHIFT)&((short)(1<<15)));
			bool IsCtrlKeyPressed=(bool)(GetAsyncKeyState(VK_CONTROL)&((short)(1<<15)));
			char *stream=InOuts2_All;
			//If Ctrl is pressed we select audio stream only
			if (IsCtrlKeyPressed) stream=m_AudioStream;  //If Null nothing happens ;)
			//If Shift is Pressed we perform slip and slide
			//If no key is pressed we increment the in or out depending upon the SubRegionFocus
			if (stream) {
				if ((m_SubRegionFocus==1)||(IsShiftKeyPressed)) {
					//double in=ci->InOut_GetInPointStyle(stream,eInOut_OutsideTransitions);
					double in=GetInPoint(wrapper,stream);
					//Modify In Point
					SetInPoint(wrapper,Adjustment,stream);
					//ci->InOut_SetInPointStyle(stream,in+Adjustment,eInOut_OutsideTransitions);					
					}
				if ((m_SubRegionFocus==2)||(IsShiftKeyPressed)) {
					double out=GetOutPoint(wrapper,stream);
					SetOutPoint(wrapper,Adjustment,stream);
					}
				sc->RePaint();
				}
			}

		// Set the cursor position back to where it was
		SetCursorPos(pt.x,pt.y);

		}
	else sc->VideoEditor_Crouton::MouseMoved(Flags,x,y);
	}


void StoryBoard_ViewMode::MouseEnter(StoryBoard2_Crouton *sc) {
	sc->StartTimer((unsigned)sc,100); sc->OnTimer((unsigned)sc);
	sc->VideoEditor_Crouton::MouseEnter();
	}

void StoryBoard_ViewMode::MouseExit(StoryBoard2_Crouton *sc)  {
	sc->StopTimer((unsigned)sc); 
	sc->m_IsAltKeyPressed=false;
	sc->m_IsCtrlKeyPressed=false;
	if (sc->m_InScreenObject) {
		NewTek_Delete(sc->m_InScreenObject);
		sc->m_InScreenObject=NULL;
		}
	if (sc->m_OutScreenObject) {
		NewTek_Delete(sc->m_OutScreenObject);
		sc->m_OutScreenObject=NULL;
		}
	sc->RePaint();
	sc->VideoEditor_Crouton::MouseExit();
	}

void StoryBoard_ViewMode::OnTimer(unsigned long TimerID,StoryBoard2_Crouton *sc) {
	if ((bool)(GetAsyncKeyState(VK_MENU)&((short)(1<<15)))) {
		if (!sc->m_IsAltKeyPressed) {
			ContentInstance *ci=sc->m_contentInstance;
			InOut_Wrapper *wrapper=sc->s_CroutonListener.GetWrapper(ci);
			//Set our timecodefloat view mode
			if (ci) {

				double num,den;
				num=ci->InOut_GetFrameRate();
				den=1.0;
				if (num==29.97) {
					num=30000;
					den=1001;
					}

				eContentInstance_SourceTypes ciType=(eContentInstance_SourceTypes)ci->ContentInstance_GetSourceType();
				if (ciType==ContentInstance_Transition) {
					m_TimeCodeFloatIn.SetAttr(e_DecimalView);
					m_TimeCodeFloatOut.SetAttr(e_DecimalView);
					double lastframe;
					double defaultlength=ci->InOut_GetDefaultLength();

					if (defaultlength<0) {
						lastframe=(-0.5+(defaultlength/(den/num)));
						lastframe*=-1;
						}
					else lastframe=(0.5+(defaultlength/(den/num)));
					lastframe=(float)((int)lastframe); //trunicate the decimal

					m_TimeCodeFloatIn.SetScalingFactor(num/den/lastframe*100);
					m_TimeCodeFloatOut.SetScalingFactor(num/den/lastframe*100);
					}
				else {
					m_TimeCodeFloatIn.SetAttr(e_DefaultTimeCode);
					m_TimeCodeFloatOut.SetAttr(e_DefaultTimeCode);
					m_TimeCodeFloatIn.SetScalingFactor(1);
					m_TimeCodeFloatOut.SetScalingFactor(1);
					}
				}

			sc->m_IsAltKeyPressed=true;
			{//set the menuitems for streams
				m_VideoStream=m_AudioStream=NULL;
				tList<char *> streamlist;ci->InOut_GetStreamTypes(&streamlist);

					unsigned i,eoi=streamlist.NoItems;
					for (i=0;i<eoi;i++) {
						char *stream=streamlist[i];
						if (!m_VideoStream) if (!strnicmp(stream,"video",5)) m_VideoStream=stream;
						if (!m_VideoStream) if (!strnicmp(stream,"trans",5)) m_VideoStream=stream; //this is temporary
						if (!m_AudioStream) if (!strnicmp(stream,"audio",5)) m_AudioStream=stream;
						}
				}

			//set the wrapper to this content instance
			wrapper->SetWrapperVariables(sc->VideoEditor_Crouton_GetInOutEditor(),sc->VideoEditor_Crouton_GetInOutInterface());
			char *stream=NULL;
			if (m_VideoStream) stream=m_VideoStream;
			else if (m_AudioStream) stream=m_AudioStream;
			if (stream) {
				sc->m_InScreenObject=wrapper->InOut_CreatePreviewScreenObject(stream,&m_TimeCodeFloatIn,false);
				sc->m_OutScreenObject=wrapper->InOut_CreatePreviewScreenObject(stream,&m_TimeCodeFloatOut,true);
				}
			else sc->m_InScreenObject=sc->m_OutScreenObject=NULL;
			m_SubRegionFocus=GetSubRegionFocus(sc);
			sc->RePaint();
			}
		}
	else {
		if (sc->m_IsAltKeyPressed) {
			sc->m_IsAltKeyPressed=false;
			if (sc->m_InScreenObject) sc->m_InScreenObject->SetPreviewOnProgramOutWhenChanging(false, false);
			if (sc->m_OutScreenObject) sc->m_OutScreenObject->SetPreviewOnProgramOutWhenChanging(false, true);
			if (sc->m_InScreenObject) {
				NewTek_Delete(sc->m_InScreenObject);
				sc->m_InScreenObject=NULL;
				}
			if (sc->m_OutScreenObject) {
				NewTek_Delete(sc->m_OutScreenObject);
				sc->m_OutScreenObject=NULL;
				}
			sc->RePaint();
			}
		}

	if ((bool)(GetAsyncKeyState(VK_CONTROL)&((short)(1<<15)))) {
		if (!sc->m_IsCtrlKeyPressed) {
			sc->m_IsCtrlKeyPressed=true;
			sc->RePaint();
			}
		}
	else 
		if (sc->m_IsCtrlKeyPressed) {
			sc->m_IsCtrlKeyPressed=false;
			sc->RePaint();
			}
	}



unsigned StoryBoard_ViewMode::GetSubRegionFocus(StoryBoard2_Crouton *sc) {
	struct tagPOINT mouseloc;
	int xcoord;
	int SubRegionFocus;

	GetCursorPos(&mouseloc);  //couldn't find the wrapper for this one

	xcoord=mouseloc.x-sc->GetWindowPosX();
	if (xcoord<sc->GetWindowWidth()>>1) SubRegionFocus=1; //Decrement Focus
	else SubRegionFocus=2; //Increment Focus

	return (SubRegionFocus);
	}


char *StoryBoard_ViewMode::FindAStream(InOut_Wrapper *wrapper) {
	char *A_Stream=NULL;
	tList<char *> streamlist;wrapper->InOut_GetStreamTypes(&streamlist);
	A_Stream=streamlist[0];
	return A_Stream;
	}

double StoryBoard_ViewMode::SetInPoint(InOut_Wrapper *wrapper,double adjustment,char *stream) {
	if (wrapper) {
		if (!strcmp(stream,InOuts2_All)) {
			//set the points of all the streams
			tList<char *> streamlist; wrapper->InOut_GetStreamTypes(&streamlist);
			char *l_stream;
			double lastvalue;

				unsigned i,eoi=streamlist.NoItems;
				for (i=0;i<eoi;i++) {
					l_stream=streamlist[i];
					lastvalue=wrapper->InOut_GetInPointStyle(l_stream,eInOut_OutsideTransitions);
					lastvalue+=adjustment;
					lastvalue=wrapper->InOut_SetInPointStyle(l_stream, lastvalue, eInOut_OutsideTransitions);
					}
				//TODO we may want to prioritize to video of the return value
				return lastvalue;
			}
		else
			adjustment+=wrapper->InOut_GetInPointStyle(stream,eInOut_OutsideTransitions);
			return wrapper->InOut_SetInPointStyle(stream, adjustment, eInOut_OutsideTransitions);
		}
	return 0.0;
	}

double StoryBoard_ViewMode::GetInPoint(InOut_Wrapper *wrapper,char *stream) {
	if (wrapper) {
		if (!strcmp(stream,InOuts2_All)) {
			if (stream=FindAStream(wrapper)) return wrapper->InOut_GetInPointStyle(stream, eInOut_OutsideTransitions);
			}
		else
			return wrapper->InOut_GetInPointStyle(stream, eInOut_OutsideTransitions);
		}
	return 0.0;
	}

double StoryBoard_ViewMode::SetOutPoint(InOut_Wrapper *wrapper,double adjustment,char *stream) {
	if (wrapper) {
		if (!strcmp(stream,InOuts2_All)) {
			//set the points of all the streams
			tList<char *> streamlist;wrapper->InOut_GetStreamTypes(&streamlist);
			char *l_stream;
			double lastvalue;

				unsigned i,eoi=streamlist.NoItems;
				for (i=0;i<eoi;i++) {
					l_stream=streamlist[i];
					lastvalue=wrapper->InOut_GetOutPointStyle(l_stream,eInOut_OutsideTransitions);
					lastvalue+=adjustment;
					lastvalue=wrapper->InOut_SetOutPointStyle(l_stream, lastvalue, eInOut_OutsideTransitions);
					}
				//TODO we may want to prioritize to video of the return value
				return lastvalue;
			}
		else
			adjustment+=wrapper->InOut_GetOutPointStyle(stream,eInOut_OutsideTransitions);
			return wrapper->InOut_SetOutPointStyle(stream, adjustment, eInOut_OutsideTransitions);
		}
	return 0.0;
	}

double StoryBoard_ViewMode::GetOutPoint(InOut_Wrapper *wrapper,char *stream) {
	if (wrapper) {
		if (!strcmp(stream,InOuts2_All)) {
			if (stream=FindAStream(wrapper)) return wrapper->InOut_GetOutPointStyle(stream, eInOut_OutsideTransitions);
			}
		else
			return wrapper->InOut_GetOutPointStyle(stream, eInOut_OutsideTransitions);
		}
	return 0.0;
	}

