/*!	\file		StoryBoard_ViewMode.h
 *	\ingroup	VideoEditor_StoryBoard
 */

#ifndef __StoryBoard_ViewModeH_
#define __StoryBoard_ViewModeH_

/*!	\class		StoryBoard_ViewMode
 *	\ingroup	VideoEditor_StoryBoard
 *	\brief		A UtilLib_FileButton_ViewMode that can draw a StoryBoard2_Crouton in its own special way
 *
 *	StoryBoard_ViewMode is the Large Icon croutons which have the special alt shortcuts for manipulating in/out points
 */
class StoryBoard2_Crouton;

class StoryBoard_ViewMode_Interface : public HeightWidth_ViewMode {
	public:
		virtual	void PaintWindow(HWND hWnd,HDC hdc,StoryBoard2_Crouton *sc)=0;
		//We want to draw our own inout frame during edit operations; Return NULL for filebutton icon default
		virtual ScreenObject *GetFileIcon(StoryBoard2_Crouton *sc)=0;
		virtual void MouseLButtonClick(long Flags,long x,long y,StoryBoard2_Crouton *sc)=0;
		virtual void MouseLButtonRelease(long Flags,long x,long y,StoryBoard2_Crouton *sc)=0;
		virtual void MouseMoved(long Flags,long x,long y,StoryBoard2_Crouton *sc)=0;
		virtual void MouseEnter(StoryBoard2_Crouton *sc)=0;
		virtual void MouseExit(StoryBoard2_Crouton *sc)=0;
		virtual void OnTimer(unsigned long TimerID,StoryBoard2_Crouton *sc)=0;
	};

class VideoEditor_StoryBoardDLL StoryBoard_ViewMode : public StoryBoard_ViewMode_Interface {
	public:
		StoryBoard_ViewMode();
//		virtual ~StoryBoard_ViewMode();

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

#endif	// #ifndef __StoryBoard_ViewModeH_
