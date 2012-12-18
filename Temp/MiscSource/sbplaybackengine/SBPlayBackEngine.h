#ifndef __SBPLAYBACKENGINE__
#define __SBPLAYBACKENGINE__

//********************************************************************************************************************
#define StoryBoard_PlayBack_Unknown	1
#define StoryBoard_PlayBack_Pause	2
#define StoryBoard_PlayBack_Play	3

//********************************************************************************************************************
class SBDDLL StoryBoard_PlayBack : public WorkerThread
{	protected:		// The Video Toaster Output
					RTMEOutput				*m_OutputEngine;

					// The item being rendered					
					SBD_Item_Info			*m_ItemToRender;
					SBD_Item_Info			*m_ItemToRender_Req;

					// The current deck state
					unsigned				m_DeckState;
					unsigned				m_DeckState_Req;

					double					m_ValueToChange;
					double					m_EndPosition;

					void RequestDeckState(unsigned NewState,bool WaitFor=false);
					unsigned GetDeckState(void);

					// Change the position and playback positions
					bool					m_FP_Req;
					double					m_FrameNo_Req;
					double					m_PlaybackSpeed_Req;

					// Image information
					unsigned				m_XRes;
					unsigned				m_YRes;
					double					m_FrameRate;
					double					m_FieldTime;
					double					m_FieldStart;

					// These are the ID values that are used for rendering dispatch
					LONGLONG				m_RenderID;		// The nos of the frames ready to be rendered
					LONGLONG				m_PlayID;		// The nos of the frames ready to be played

					// The rendering threads
					unsigned				m_NoThreads;
					SBD_Renderer			**RenderThreads;

					// The current time information
					double					m_FrameNo;
					double					m_PlaybackSpeed;

					// This is the information about which buffer is ready
					unsigned				m_MaximumLeadFields;

					// This is the list of buffers that are ready to be sent out of the Toaster ...
					class OutputBufferDesc
					{	public:		void		*Memory;
									LONGLONG	ID;
					};

					tListExpandOnly<OutputBufferDesc>	OutputBuffers;

					// The thread entry point
					virtual void ThreadProcessor(void);
					virtual void ThreadProcessor_Start(void);

	public:			StoryBoard_PlayBack(SBD_Item_Info *Project,
										unsigned	NoThreads=4,
										unsigned	XRes=720,
										unsigned	YRes=240,
										double		FrameRate=30000.0/1001.0,
										unsigned	MaximumLeadFields=65);	// This MUST be at least NoThreads*NoThreadBuffers
																			// In practice it should be slightly higher to accomodate slightly
																			// more lead fields. Note that in practice generally at least twice as much
																			// memory is used in playback.

					~StoryBoard_PlayBack(void);

					// Deck control on the super renderer
					void Pause(bool WaitFor); // There is no stop, just use pause ... It will not purge buffers, infact it will go on thinking 
					void Play (bool WaitFor); // Start sending frames back to the card

					// Change the item being viewed :0
					// It would be very dangerous not to wait for the change since you
					// run the risk of deleting the item before the playback engine has finished with it !
					void ChangeItemBeingViewed(SBD_Item_Info *NewProject,bool WaitFor=true);

					// Set the playback position
					void ChangePlaybackPosition(double NewTime=DBL_MAX,double NewPlaybackRate=DBL_MAX,bool WaitFor=true);
};

#endif