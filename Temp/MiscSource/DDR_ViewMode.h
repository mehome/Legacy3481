/*!	\file		DDR_ViewMode.h
 *	\ingroup	VideoEditor_StoryBoard
 */

#ifndef __DDR_ViewModeH_
#define __DDR_ViewModeH_

/*!	\class		DDR_ViewMode
 *	\ingroup	VideoEditor_StoryBoard
 *	\brief		A UtilLib_FileButton_ViewMode that can draw a StoryBoard2_Crouton in its own special way
 *
 *	DDR_ViewMode is the Large Icon croutons which have the special alt shortcuts for manipulating in/out points
 */
class StoryBoard2_Crouton;

class VideoEditor_StoryBoardDLL DDR_ViewMode : public StoryBoard_ViewMode_Interface {
	public:
		DDR_ViewMode();
//		virtual ~DDR_ViewMode();

		//These are from StoryBoardView Interface
		virtual void PaintWindow(HWND hWnd,HDC hdc,StoryBoard2_Crouton *sc);
		virtual ScreenObject *GetFileIcon(StoryBoard2_Crouton *sc);
		virtual void MouseLButtonClick(long Flags,long x,long y,StoryBoard2_Crouton *sc);
		virtual void MouseLButtonRelease(long Flags,long x,long y,StoryBoard2_Crouton *sc);
		virtual void MouseMoved(long Flags,long x,long y,StoryBoard2_Crouton *sc);
		virtual void MouseEnter(StoryBoard2_Crouton *sc);
		virtual void MouseExit(StoryBoard2_Crouton *sc);
		virtual void OnTimer(unsigned long TimerID,StoryBoard2_Crouton *sc);

	private:
		double m_FrameRate; // Used from mousedown to mouseup for performance
		char *m_VideoStream;
		char *m_AudioStream;

		// These are used for mouse move on in out keyshort cut editing
		POINT pt;
		bool m_InOutMode; //Triggered by pressing lmouse w/ alt key... released on lbuttonup
		unsigned char m_SubRegionFocus;
		//TimeCode display on crouton
		TextItem m_TimeCodeIn,m_TimeCodeOut;
		TimecodeFloat m_TimeCodeFloatIn,m_TimeCodeFloatOut;
		Dynamic<double> m_AudioRange;

		unsigned GetSubRegionFocus(StoryBoard2_Crouton *sc);
		char *FindAStream(InOut_Wrapper *wrapper);
		double SetInPoint(InOut_Wrapper *wrapper,double adjustment,char *stream);
		double GetInPoint(InOut_Wrapper *wrapper,char *stream);
		double SetOutPoint(InOut_Wrapper *wrapper,double adjustment,char *stream);
		double GetOutPoint(InOut_Wrapper *wrapper,char *stream);
	};

#endif	// #ifndef __DDR_ViewModeH_
