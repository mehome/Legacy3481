		//You shouldn't need to use these unless you are manually inserting buffers into the stream... in which case:
		//1. Do not insert frames if m_BufferCount>m_MaxTBCQueueDepth
		//2. Update m_BufferCount for any frames you add or remove
		size_t GetBufferCount() const {return m_BufferCount;}
		void SetBufferCount(size_t BufferCount) {m_BufferCount=BufferCount;}


					//Fill the TBC queue with the queue depth for silence... this is not the best solution for a timeout, but it will
					//get it closer to where it needs to be
					for (size_t i=0;i<m_LatencyQueueDepth;i++)
					{
						AudioBuffer *pBufferToAdd = new AudioBuffer(&m_InputStreamer,m_InputStreamer.GetAudioDevice());
						//Now to fill buffer with silence
						size_t MemorySize;
						void *mem=pBufferToAdd->pGetBufferData(&MemorySize);
						memset(mem,0,MemorySize); //nice to know we are only using the float format ;)
						m_InputStreamer.AddBufferToTBCList(pBufferToAdd,NULL);
					}



	else if (ItemChanging==&m_PathName)
	{
		ReceiveMessagesOff();
		char Buffer[MAX_PATH];
		strcpy(Buffer,m_PathName.GetText());
		NewTek_StrCatSlashIfNeeded(Buffer);
		strcat(Buffer,m_FileName.GetText());
		m_FullPath.SetText(Buffer);
		ReceiveMessagesOn();
	}
	else if ((ItemChanging==&m_FileName)&&(!strcmp(String,UtilLib_Edit_LoseFocusChanged)))
	{
		ReceiveMessagesOff();
		char Buffer[MAX_PATH];
		strcpy(Buffer,m_PathName.GetText());
		NewTek_StrCatSlashIfNeeded(Buffer);
		strcat(Buffer,m_FileName.GetText());
		m_FullPath.SetText(Buffer);
		ReceiveMessagesOn();
	}
	else if (ItemChanging==&m_FullPath)
	{
		ReceiveMessagesOff();

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		_splitpath(m_FullPath.GetText(),drive,dir,fname,NULL);
		char Buffer[MAX_PATH];
		//Now to construct the new path
		strcpy(Buffer,drive);
		strcat(Buffer,dir);
		NewTek_StrCatSlashIfNeeded(Buffer);
		m_PathName.SetText(Buffer);

		//Now to construct the new filename with extension
		m_FileName.SetText(fname);

		ReceiveMessagesOn();
	}


			pcmpeqw mm4,mm0		//All areas that are padded
			movq mm2,mm4		//Back up mm4 for areas that need padded
			pand mm1,mm4		//mm1 = The replacement samples
			pandn mm4,mm0		//mm0 = padded samples stripped clean

			por mm1,mm4			//mm1 = new replaced samples

			//Make sure the stereo trick worked!

			movq mm0,mm1		//copy the first good canidate
			pshufw mm1,mm0,1<<6|0<<4|3<<2|2  //mm1 =samples 1 and 2 switched
			pand mm1,mm2       //mm1= The replacement samples
			pandn mm2,mm0	  //mm0 = padded samples stripped clean	

			por mm1,mm0			//mm1 = new replaced samples




								else
								{
									BufferInfo *InflateBufferToUse;
									//Try to find any other buffer which could be in range (to avoid starting over)
									GetBufferListBlock()->Block();
									size_t eoi=GetBufferAllocator()->GetNoBuffers();
									for (size_t i=0;i<eoi;i++)
									{
										size_t Frame=((* GetBufferList())[i]).Frame;
										if (!(((* GetBufferList())[i]).ReadyStatus))
											continue;
	
										//Note: since these are unsigned... -1 will never produce a successful condition
										if ((Frame<ReadBufferInfo.Frame) && (Frame>=KeyFrame))
										{
											//We'll now use this buffer as the last one
											InflateBufferToUse=&((* GetBufferList())[i]);
											#ifdef __VTRM_ShowDebugOutput__
											DebugOutput("#### %x Using inflate buf %d with Frame %d to construct %d\n",this,i,InflateBufferToUse->Frame,ReadBufferInfo.Frame);
											#endif __VTRM_ShowDebugOutput__
											memcpy(InflateBufferInfo.Memory,InflateBufferToUse->Memory,m_Parent->GetInflateVideoBufferSize());
											memcpy(InflateBufferInfo.Alpha,InflateBufferToUse->Alpha,m_Parent->GetInflateAlphaBufferSize());
											StartingFrame=InflateBufferToUse->Frame+1;
											//When we go out of this block... protect this buffer from being purged
											InflateBufferToUse->NumberofReads++;
											//assign this so that when this loop is finished it will decrement the number of reads
											//for this buffer
											m_CachedIndexOfPreviousFrame=i;
											//Now to free the last cached one since it is not going to be used
											LastInflateBufferInfo.NumberofReads--;
											break;
										}
									}
									GetBufferListBlock()->UnBlock();
								}




						//I'm most likely going to delete this -1 condition after I have tested the assertion to hold true
						if (m_CachedIndexOfPreviousFrame==-1)
						{
							size_t KeyFrameBufferIndex;
							//in this case we'll want to find the KeyFrame (if it exists)
							if (!(m_Parent->GetReadBufferAllocator()->FindNextReadBuffer(KeyFrameBufferIndex,KeyFrame)))
								assert(false); //TODO
							//Note the read buffer is safe for read access since the buffer status is set to not available
							BufferInfo &KFReadBufferInfo=(* m_Parent->GetReadBufferList())[KeyFrameBufferIndex];
							if (KFReadBufferInfo.Frame!=KeyFrame)
								assert(false); //TODO

							#ifdef __VTRM_ShowDebugOutput__
							DebugOutput("#### %x Need Keyframe %d from read %d ####\n",this,KeyFrame,KFReadBufferInfo.Frame);
							#endif __VTRM_ShowDebugOutput__

							//We now have the keyframe buffer... decompress into the scrub buffer
							if (!(m_Parent->GetSyncReaderInterface()->SRI_InflateFrame(m_Instance,KeyFrame,KFReadBufferInfo.Memory+KFReadBufferInfo.AlignOffset,m_Parent->m_ScrubInflateBuffer,m_Parent->m_ScrubInflateAlphaBuffer,true)))
								goto InflateError;
							CurrentFrameInScrub=KeyFrame;

							//Set read buffer as empty
							#ifdef __VTRM_ShowDebugOutput__
							DebugOutput("**** %x Freeing KeyFrame Read Buffer %d in Buffer %d\n",this,KeyFrame,KeyFrameBufferIndex);
							#endif __VTRM_ShowDebugOutput__

							KFReadBufferInfo.Frame=-1;
							KFReadBufferInfo.ReadyStatus=true;

						}
						else


	#ifdef __Optimize2__

	if (FrameNumber==m_LastFrame+1)
	{
		size_t NoInflateBufs=m_AsyncBufferInflator[0]->GetBufferAllocator()->GetNoBuffers();
		size_t NoInflateThreads=c_MultipleInflateBuffers?m_NoInflateThreads:1;
		size_t NoTotalInflateBufs=NoInflateBufs * NoInflateThreads;

		for (size_t i=0;i<NoInflateThreads;i++)
		{
			tList<BufferInfo> &l_BuffInflateList=(* m_AsyncBufferInflator[i]->GetBufferList());
			for (size_t j=0;j<NoInflateBufs;j++)
			{
				size_t InflateFrame;

				m_AsyncBufferInflator[i]->GetBlockFrameRead()->Block();
				InflateFrame=l_BuffInflateList[j].Frame;
				m_AsyncBufferInflator[i]->GetBlockFrameRead()->UnBlock();

				if (InflateFrame==NativeFrameNumber)
				{
					BufferMatch=j;
					InflateInstanceMatch=i;
					//Now to determine whether to purge... 
					//The trick is to avoid purging minimize the amount of block and encourage the number of inflate threads
					//to be active simultaneously... this will hopefully schedule the threads to run evenly on each processor
					//if (FrameNumber % m_NoInflateThreads != 0)
					//	goto MatchFound;
				}
				else //if (FrameNumber % m_NoInflateThreads == 0)//Purge any inflate frames which are out of buffer range
				{
					if  (!ScrubbingRecommendation)
					{
						if	(
							((InflateFrame<FrameNumber) || (((int)InflateFrame-(int)FrameNumber) >= (int)NoTotalInflateBufs)) &&
							(l_BuffInflateList[j].ReadyStatus==true) // can't purge if the inflate buffer is using it
							)

						{
							if (l_BuffInflateList[j].NumberofReads==0)
							{
	#ifdef __VTRM_ShowDebugOutput__
								DebugOutput("**** Purging inflate frame %d in %d[%d]-------------------------------------------\n",l_BuffInflateList[j].Frame,i,j);
	#endif __VTRM_ShowDebugOutput__

								PurgedInflateBuffers=true;
								l_BuffInflateList[j].Error=false;
								m_AsyncBufferInflator[i]->GetBlockFrameRead()->Block();
								l_BuffInflateList[j].Frame=-1;
								m_AsyncBufferInflator[i]->GetBlockFrameRead()->UnBlock();
								l_BuffInflateList[j].ReadyStatus=true;
							}
						}
					}
				}
			}
		}
	}
	else if ((FrameNumber<=m_LastFrameScrubCounts)&&(!GetKeyFramesNeeded()))
	{
		m_LastFrameScrubCounts=FrameNumber;
		return Scrub_ReadFromFile(NativeFrameNumber,Field0,Field1,Pitch0,Pitch1,Alpha0,Alpha1);
	}
	#endif __Optimize2__





//*************************************************************************************************************
int CCompressUYVY::CountCPUs( void )
{ HKEY hKey_;
 int nCpu_;
 
 for (nCpu_ = 0; nCpu_ < 8; nCpu_++)
 { char szKey_ [80];
 
  wsprintf( szKey_, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%1d", nCpu_ );
 
  if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, szKey_, 0, 0, &hKey_ ) != ERROR_SUCCESS)
   break;
 
  RegCloseKey( hKey_ );
 }
 return( nCpu_ );
}



//It is no different than the Shared Pointer with the exception
//of how it deletes the pointer... when you do a new Object[x];
//e.g.  tSharedPointerArray<char> *Test=new tSharedPointerArray<char>(new char[64]);

template <class L>
class tSharedPointerArray
{	public:
		tSharedPointerArray(L *Pointer=NULL) : m_Pointer(Pointer) , m_Ref(1)
			{	
			}		

		void AddRef(void) const
			{	InterlockedIncrement(&m_Ref);
			}

		//The return value will let you know if this instance has been deleted
		bool Release(void) const
			{	if (InterlockedDecrement(&m_Ref)==0) 
				{
					delete this;
					return true;
				}
				return false;
			}

		// Get access to the variables
			L *GetPointer(void)				
				{	return m_Pointer;
				}

			const L *GetPointer(void) const
				{	return m_Pointer;
				}

		// Operator overloading
			operator L* ( void ) const
				{	return GetPointer();
				}

	private:	
			//okay I can delete myself
			~tSharedPointerArray(void)		
				{	assert(!m_Ref);
					if (m_Pointer)
						delete [] m_Pointer;
				}

			// The reference count
				mutable long m_Ref;

			// The pointer we are referring too
				L * const m_Pointer;
};

	size_t AlreadyBeenFound[9]=
	{
				//									  L R	 L  R
				//                                    UUUL CRLD L
		0x01B,	//UpperLeft		(UL,U ,L ,C       ) = 1101 1000 0
		0x03F,	//Up			(UL,U ,UR,L ,C ,R ) = 1111 1100 0
		0x036,	//UpperRight	(U ,UR,C ,R       ) = 0110 1100 0
		0x0DB,	//Left			(UL,U ,L ,C ,LL,D ) = 1101 1011 0
		0x000,	//Center-       (Not used)          = 0000 0000 0
		0x1B6,	//Right			(U ,UR,C ,R ,D ,LR) = 0110 1101 1
		0x0D8,	//LowerLeft		(L ,C ,LL,D		  ) = 0001 1011 0
		0x1F8,	//Down			(L ,C ,R ,LL,D ,LR) = 0001 1111 1
		0x1B0,	//LowerRight	(C ,R ,D ,LR	  ) = 0000 1101 1
	};


	if (cpuSupportsSSE2())
	{
		CACHE_ALIGN __m128i PixelByteA,PixelByteB;
		size_t x,y;
		//clip off any overlapping for a 16 byte boundary
		int TotalLength=XLength-XStart;
		int Remainder=TotalLength&0x0f;
		int LengthToUse=XLength-Remainder;
		
		for(y=YStart;y<YLength;y++)
		{
			for (x=XStart;x<LengthToUse;x+=16)
			{
				PixelByteA=_mm_loadu_si128((__m128i *)(BufferA+x+XWidthInBytes*(y)));
				PixelByteB=_mm_loadu_si128((__m128i *)(BufferB+x+BufferB_XOffset+(XWidthInBytes * (y+BufferB_YOffset))));
				PixelByteA=_mm_sad_epu8(PixelByteA,PixelByteB);
				Result+=_mm_cvtsi128_si32(PixelByteA);
				PixelByteA=_mm_srli_si128(PixelByteA,8); //now to access the upper 64 bits
				Result+=_mm_cvtsi128_si32(PixelByteA);
			}
		}
	}
	else



bool Delay4f::Process(const float *input, float *output, long numsamples) {

	//all preconditions check here
	if ((m_Error)||(!input)||(!output)) return false;

	//Set up buffers if not ready
	if (!m_DelayBuffer)
		AddStart();

	//find the delay offsets...
	CACHE_ALIGN __m128 DelayOffset=_mm_min_ps(
				_mm_mul_ps(
					_mm_max_ps(_mm_setzero_ps(),m_Delay),
					_mm_set1_ps(m_SampleRate)
					),
				_mm_set1_ps((float)m_BufferSampleSize)
				);

	__m128i iBufferReadOffset;
	const __m128i iBufferSizeMask=_mm_set1_epi32((m_BufferSampleSize<<2)-1);
	const __m128i Four_epi32=_mm_set1_epi32(4);
	const long BufferSizeMask=m_BufferSampleSize-1; //(since it is power of 2.. the subtracting one will get the mask

	size_t BufferWriteOffset=m_WritePointer;

	iBufferReadOffset=  //((BufferWriteOffset + BufferSampleSize - DelayOffset)<<2) & BufferSizeMask
		_mm_and_si128(
			_mm_slli_epi32(
				_mm_sub_epi32(
					_mm_add_epi32(_mm_set1_epi32(BufferWriteOffset),_mm_set1_epi32(m_BufferSampleSize)),
					_mm_cvttps_epi32(DelayOffset)
					),2
				),iBufferSizeMask
			);

	for (size_t SampleIndex=(size_t)numsamples;SampleIndex;SampleIndex--) {

		//Store the input samples
		_mm_store_ps(&m_DelayBuffer[m_WritePointer<<2],*((__m128 *)input));

		_mm_store_ps(
			output,
			_mm_add_ps(
				_mm_mul_ps(m_Dry,*((__m128 *)input)),
				_mm_mul_ps(
					m_Wet,_mm_set_ps(
						m_DelayBuffer[(((int *)&iBufferReadOffset)[3])+3],
						m_DelayBuffer[(((int *)&iBufferReadOffset)[2])+2],
						m_DelayBuffer[(((int *)&iBufferReadOffset)[1])+1],
						m_DelayBuffer[(((int *)&iBufferReadOffset)[0])+0]
						)
					)
				)
			);
		input+=4;
		output+=4;


		iBufferReadOffset=_mm_and_si128(_mm_add_epi32(iBufferReadOffset,Four_epi32),iBufferSizeMask);
		++m_WritePointer&=BufferSizeMask; //will reset when max is reached
		}
	return true;
	}


/*
bool Delay4f::Process(const float *input, float *output, long numsamples) {

	//all preconditions check here
	if ((m_Error)||(!input)||(!output)) return false;

	//Set up buffers if not ready
	if (!m_DelayBuffer)
		AddStart();

	//find the delay offsets...
	CACHE_ALIGN __m128 DelayOffset=_mm_min_ps(
				_mm_mul_ps(
					_mm_max_ps(_mm_setzero_ps(),m_Delay),
					_mm_set1_ps(m_SampleRate)
					),
				_mm_set1_ps((float)m_BufferSampleSize)
				);

	const long BufferSizeMask=m_BufferSampleSize-1; //(since it is power of 2.. the subtracting one will get the mask
	const long BufferSizeMaskx4=(m_BufferSampleSize<<2)-1;

	size_t BufferWriteOffset=m_WritePointer;
	size_t BufferReadOffset0=((BufferWriteOffset + m_BufferSampleSize - ((size_t)((float *)&DelayOffset)[0]))&BufferSizeMask)<<2;
	size_t BufferReadOffset1=((BufferWriteOffset + m_BufferSampleSize - ((size_t)((float *)&DelayOffset)[1]))&BufferSizeMask)<<2;
	size_t BufferReadOffset2=((BufferWriteOffset + m_BufferSampleSize - ((size_t)((float *)&DelayOffset)[2]))&BufferSizeMask)<<2;
	size_t BufferReadOffset3=((BufferWriteOffset + m_BufferSampleSize - ((size_t)((float *)&DelayOffset)[3]))&BufferSizeMask)<<2;

	for (size_t SampleIndex=(size_t)numsamples;SampleIndex;SampleIndex--) {

		//Store the input samples
		_mm_store_ps(&m_DelayBuffer[m_WritePointer<<2],*((__m128 *)input));

		_mm_store_ps(
			output,
			_mm_add_ps(
				_mm_mul_ps(m_Dry,*((__m128 *)input)),
				_mm_mul_ps(
					m_Wet,_mm_set_ps(
						m_DelayBuffer[(BufferReadOffset3)+3],
						m_DelayBuffer[(BufferReadOffset2)+2],
						m_DelayBuffer[(BufferReadOffset1)+1],
						m_DelayBuffer[(BufferReadOffset0)+0]
						)
					)
				)
			);
		input+=4;
		output+=4;

		BufferReadOffset0=(BufferReadOffset0+4)&BufferSizeMaskx4;
		BufferReadOffset1=(BufferReadOffset1+4)&BufferSizeMaskx4;
		BufferReadOffset2=(BufferReadOffset2+4)&BufferSizeMaskx4;
		BufferReadOffset3=(BufferReadOffset3+4)&BufferSizeMaskx4;
		++m_WritePointer&=BufferSizeMask; //will reset when max is reached
		}
	return true;
	}
*/






	//This detects duplicate streams and moves these out of the stream list and into the tle list
	//used to effeciently move the entire bundle of streams for a tle.
	void MoveStreamBundleToTLEList(tList<TimeLineElement *> &TLEList,tList<VideoEditor_InOutStream *> &StreamList);


void TimeLine::MoveStreamBundleToTLEList(tList<TimeLineElement *> &TLEList,tList<VideoEditor_InOutStream *> &StreamList) {
	//This algorithm is searching for the following conditions:
	// 1. a ci match
	// 2. tracks are consecutive

	//Note this is an N2 operation but the typical size of items to work with are minimal (5-10)
	for (unsigned i=0;i<StreamList.NoItems;i++) {
		for (unsigned j=0;j<StreamList.NoItems;j++) {
			if (StreamList[j]->m_contentInstance==StreamList[i]->m_contentInstance) {
				if (i==j) continue;
				if ((abs(StreamList[j]->InOutStream_GetTrackNumber()-StreamList[i]->InOutStream_GetTrackNumber()))==2) {
					TimeLineElement *tle=StreamList[j]->m_contentInstance->ContentInstance_GetTimeLineElement();
					TLEList.Add(tle);
					//now comes the trick part
					assert (i<j);
					StreamList.DeleteEntryInOrder(j);
					StreamList.DeleteEntryInOrder(i);
					//back up our pointers
					j=min(0,j-2);
					i=min(0,i-2);
					}
				}
			}
		}
	}


	tList<TimeLineElement *> tleList;
	MoveStreamBundleToTLEList(tleList,overlapList);
	// Iterate through this list, pushing all of the other elements out of the way
	for (unsigned i = 0; i < tleList.NoItems; i++) {
		int thisTH = tleList[i]->TLE_GetTotalTrackHeight();
		int thisTrack = tleList[i]->GetTrackNumber();
		if ((thisTrack > -1) && (thisTrack < (trackNumber+trackHeight)) && (trackNumber < (thisTH+thisTrack))) {
			// Push this stream onto the next available track
			TimeLine_PushOntoTrack(tleList[i], trackNumber + trackHeight);
			}
		}


		union __M128i {
			__m128i			m_SSE2;
			unsigned		m_U32[4];
			int				m_I32[4];
			unsigned short	m_U16[8];
			short			m_I16[8];
			unsigned char	m_U8[16];
			char			m_I8[16];

			__M128i(void) {};
			__M128i(__m128i a) { m_SSE2=a; }
			operator __m128i& (void)		{ return m_SSE2; }
			operator __m128i  (void) const	{ return m_SSE2; }
			};

	long no4samples=numsamples>>2; //how many 4 samples iterations can we do


		__m128 combinput1=_mm_load_ps(input); //R0 = p[0] .. R3=p[3]
		__m128 combinput2=_mm_load_ps(input+4);
		__m128 combinput3=_mm_load_ps(input+8);
		__m128 combinput4=_mm_load_ps(input+12);

		_MM_TRANSPOSE4_PS(combinput1,combinput2,combinput3,combinput4);
		__m128 sum=_mm_add_ps(combinput1,combinput2);
		__m128 sum2=_mm_add_ps(combinput3,sum);
		sum=_mm_add_ps(combinput4,sum2); //sum of all channels (for 4 samples)

		__m128 combinput=_mm_mul_ps(sum,m_Gain);	//combinput= input1 , input2, input3, input4

		combinput1=_mm_shuffle_ps(combinput,combinput,_MM_SHUFFLE(0,0,0,0));
		combinput2=_mm_shuffle_ps(combinput,combinput,_MM_SHUFFLE(1,1,1,1));
		combinput3=_mm_shuffle_ps(combinput,combinput,_MM_SHUFFLE(2,2,2,2));
		combinput4=_mm_shuffle_ps(combinput,combinput,_MM_SHUFFLE(3,3,3,3));



		//All these types render a button requestor if the box is selected
		_CodeBlockStart;
			if (!(fileButton->DragAndDrop_AmISelected())) _CodeBlockExit;
			char *ColumnSelection=fileButton->GetDetailsColumnSelection();
			if (!ColumnSelection) _CodeBlockExit;
			if	(stricmp(this->DVE_GetColumnName(),ColumnSelection))
				_CodeBlockExit;

			//Tweek a bit with this screenobject
			int state=buttonState;
			if (state==1) state=2;
			if (state==3) state=2;

			ScreenObject_BitmapFile *so=ButtonRequestorBitmapResource::GetBitMaps(state);
			assert(so);
			RECT buttonrect;
			unsigned width=18;
			unsigned height=16;
			buttonrect.top=inRect.top+4;
			buttonrect.bottom=buttonrect.top+height;
			buttonrect.left=inRect.right-width-6;
			buttonrect.right=buttonrect.left+width;
			
			so->ScreenObject_DrawItem(&buttonrect,hdc);
			//Tweek a bit with this screenobject
			buttonrect.left+=1;
			ExcludeClipRect(hdc,buttonrect.left,buttonrect.top,buttonrect.right,buttonrect.bottom);

		_CodeBlockEnd;

/*
		//----------------------------------------Format Specification Section------------------------------------------
		//Please Note that if you do not support certain selection types for Audio and Video... then you can ignore the 
		//functions that apply to each bool selection.

		// *	*	*	*	*	*	*	*	*** Video ***	*	*	*	*	*	*	*	*	*	*
		virtual bool HasVideo()=0;

		//if the index is in range this will return the valid connection name... otherwise will return NULL
		//i.e.  Composite 1, Composite 2, Component1, Component2, DV-FireWire...  etc...
		virtual bool HasVideoConnectionSelection()=0;
		virtual char *GetVideoConnectionName(unsigned index)=0;
		virtual bool SetVideoConnectionName(char *VideoConnection)=0;

		//if the index is in range this will return the valid video codec name... otherwise will return NULL
		//i.e.	RTV, AVI Uncompessed(UYVY), DivX, IntelIndeo, MJPG
		virtual bool HasVideoCodecSelection()=0;
		virtual char *GetVideoCodec(unsigned index)=0;
		virtual bool SetVideoCodec(char *VideoCodec)=0;

		virtual bool HasVideoResolutionSelection()=0;
		virtual bool HasVideoFrameRateSelection()=0;
		virtual bool HasVideoProgression()=0;
		//This allows you to define a range of acceptable resolutions and framerates... for each video codec supported
		//Note: You may wish to implement this function even if the user can not specify these format vars
		//The default implementation must return true!
		virtual bool IsVideoFormatValid(char *VideoCodec,unsigned XRes,unsigned YRes,double FrameRate,bool Progressive)=0;

		//This is a nice combined summary of video formats available for any given video codec
		virtual bool HasVideoFormatSelection()=0;
		virtual char *GetVideoFormat(char *VideoCodec,unsigned index)=0;
		virtual bool SetVideoFormat(char *VideoFormat)=0;	//This should implicitly set Resolution and framerate and check if this is valid from IsVideoFormatValid

		// *	*	*	*	*	*	*	*	*** Audio ***	*	*	*	*	*	*	*	*	*	*
		virtual bool HasAudio()=0;

		virtual bool HasAudioConnectionSelection()=0;
		virtual char *GetAudioConnectionName(unsigned index)=0;
		virtual bool SetAudioConnectionName(char *AudioConnection);

		virtual bool HasAudioCodecSelection()=0;
		virtual char *GetAudioCodec(unsigned index)=0;
		virtual bool SetAudioCodec(char *AudioCodec);

		virtual bool HasAudioSampleRateSelection()=0;
		//This allows you to define a range of acceptable samples per second... for each audio codec supported
		//Note: You may wish to implement this function even if the user can not specify a sample rate change
		//The default implementation must return true!
		virtual bool IsAudioFormatValid(char *AudioCodec,unsigned long SamplesPerSecond)=0;

		virtual bool HasAudioFormatSelection()=0;
		//This is a nice combined summary of audio formats available for any given audio codec
		virtual char *GetAudioFormat(char *AudioFormat,unsigned index)=0;
		virtual bool SetAudioFormat(char *AudioFormat)=0;	//This should implicitly set SampleRate and check if this is valid from IsAudioFormatValid

*/

bool ExternalSource::VCR_IsRewinding() {
	long mode;
	mode=GetTransportMode();
	return (mode==ED_MODE_REW)||(mode==ED_MODE_PLAY_FASTEST_REV));
	}
bool ExternalSource::VCR_IsFastForwarding() {
	long mode;
	mode=GetTransportMode();
	return (mode==ED_MODE_FF)||(mode==ED_MODE_PLAY_FASTEST_FWD));
	}
bool ExternalSource::VCR_IsStepFF() {
	long mode;
	mode=GetTransportMode();
	return (mode==ED_MODE_STEP_FWD)||(mode==ED_MODE_PLAY_SLOWEST_FWD));
	}
bool ExternalSource::VCR_IsStepREW() {
	long mode;
	mode=GetTransportMode();
	return (mode==ED_MODE_STEP_REV)||(mode==ED_MODE_PLAY_SLOWEST_REV));
	}

bool TargaReader::OpenTarga(char *filename,TargaInfo *TGAHandle) {
	bool Success=false;
	do {
		//Open the File
		HANDLE hfile;
		if ((hfile=OpenReadSeq(filename))==(void *)-1) break;
		TGAHandle->FileHandle=hfile;
		unsigned long highpart;
		TGAHandle->FileSize=myGetFileSize(hfile,&highpart);
		if (highpart) break; //file should never be more than 2 gig!
		//Now to parse the TGA
		TGAHeader *lpTGAheader=&(TGAHandle->HeaderInfo);
		myRead(hfile,lpTGAheader,sizeof(TGAHeader));

		unsigned x,y,depth;
		
		x=TGAHandle->width=lpTGAheader->imagespecs.width;
		y=TGAHandle->height=lpTGAheader->imagespecs.height;
		depth=lpTGAheader->imagespecs.depth;
		//printf("Diminsions %d x %d x %d\n",x,y,depth);

		if (x<8) {
			printf("width must be greater than 8\n");
			break;
			}

		//Now to see if there is an Image ID field
		DWORD advance=lpTGAheader->idlength;
		advance+=sizeof(TGAHeader);
		//Now to see if there is a Colormap field
		if (lpTGAheader->colormaptype) {
			advance+= //Heres that stupid non word aligned member
				(lpTGAheader->cmapspecs.cmaplength[0]+lpTGAheader->cmapspecs.cmaplength[1]*256) *
				lpTGAheader->cmapspecs.bitsperentry;
			}
		TGAHandle->SeekStartPos=advance;
		TGAHandle->BytesPerPixel=((lpTGAheader->imagespecs.depth+(lpTGAheader->imagespecs.imagedesc&tgaID_ALPHA))>>3);
		//printf("Alpha Depth=%ld\n",lpTGAheader->imagespecs.imagedesc&tgaID_ALPHA);


		bool error=false;
		bool rle=false;
		switch ((tgaimagetypes)lpTGAheader->imagetype) {
			case tga_noimage: 
				printf("No image data included\n");
				error=true;
				break;
			case tga_nocomcmp: 
				printf("Uncompressed, color-mapped images\n");
				//colormode=cmap;
				printf("Not Supported at this time\n");
				error=true;
				break;
			case tga_nocomtc:
				printf("Uncompressed, RGB images\n");  //----------------Supported
				//colormode=rgb;
				if ((depth==24)||(depth==32)) {
					printf("Only 24 or 32bit Uncompressed RGB is Supported at this time\n");
					error=true;
					}
				break;
			case tga_nocombw:
				printf("Uncompressed, black and white images\n");
				//colormode=bw;
				printf("Not Supported at this time\n");
				error=true;
				break;
			case tga_rlecmp:
				printf("Runlength encoded color-mapped images\n");
				rle=true;
				//colormode=cmap;
				printf("Not Supported at this time\n");
				error=true;
				break;
			case tga_rlergb:
				//printf("Runlength encoded RGB images\n"); //----------------Supported 32bit only
				rle=true;
				//colormode=rgb;
				if (depth!=32) {
					printf("Only 32bit RLE is Supported at this time\n");
					error=true;
					}
				break;
			case tga_rlebw:
				printf("Compressed, black and white images\n");
				rle=true;
				//colormode=rgb;
				printf("Not Supported at this time\n");
				error=true;
				break;
			case tga_rlehdcmp:
				printf("Compressed color-mapped data, using Huffman, Delta, and runlength encoding.\n");
				printf("TGA Huffman Delta compression not supported\n");
				error=true;
				break;
			case tga_rlehd2cmp:
				printf("Compressed color-mapped data, using Huffman, Delta,	and runlength encoding.  4-pass quadtree-type process.\n");
				printf("TGA Huffman Delta compression not supported\n");
				error=true;
				break;
			default: 
				printf("Unknown image type\n");
				error=true;
				break;
			}
		if (error) break;

		TGAHandle->RLECompression=rle;
		// Top to Bottom? 
		TGAHandle->TopToBottom=((lpTGAheader->imagespecs.imagedesc&tgaID_TOP)!=false);
	
		m_SeriesWidth=max(m_SeriesWidth,x);
		m_SeriesHeight=max(m_SeriesHeight,y);
		m_SeriesFileSize=max(m_SeriesFileSize,TGAHandle->FileSize);

		myClose(hfile); //All done with the parse phase
		TGAHandle->FileHandle=NULL;
		//Success
		Success=true;
		} while(false);
	if (!Success) {
		TGAHandle->Error=true;
		CloseTarga(TGAHandle);
		}
	return Success;
	}


push eax
push esi
sub esi,ebx
mov ebx,esi
shl eax,2
cmp eax,ebx
je testskip
nop
testskip:
pop esi
pop eax

bool TargaReader::GetFrame(
			TargaInfo *TGAHandle,	//handle to Targa (or use frame number (see below))
			byte **FieldEvenZero,	//Top Row Even Field... or interleaved (if FieldOddOne is NULL)
			byte **FieldOddOne,		//2nd Row Odd Field
			byte **AlphaEvenZero,	//Top Row Even Field... or interleaved (if AlphaOneOdd is NULL) no alpha sent if NULL
			byte **AlphaOneOdd) {	//2nd Row Odd Field.		 Alphas=NULL for 24bit Targa Reads.

	bool Success=false;
	do {
		if (TGAHandle->Error) break;
		//Do all common stuff here...
		//Alocate Memory for Video and Alpha Buffers....
		bool InterleavedVideo=(FieldOddOne==false);
		unsigned FrameSize=TGAHandle->height*TGAHandle->width*2;
		unsigned VideoSize=InterleavedVideo?FrameSize:FrameSize>>1;
		if (FieldEvenZero) if (*FieldEvenZero==NULL)
			*FieldEvenZero=(byte *)malloc(VideoSize);
		if (FieldOddOne) if (*FieldOddOne==NULL)
			*FieldOddOne=(byte *)malloc(VideoSize);

		if (TGAHandle->BytesPerPixel==4) {
			InterleavedVideo=(AlphaOneOdd==false);
			FrameSize=TGAHandle->height*TGAHandle->width;
			VideoSize=InterleavedVideo?FrameSize:FrameSize>>1;
			if(AlphaEvenZero) if (*AlphaEvenZero==NULL)
				*AlphaEvenZero=(byte *)malloc(VideoSize);
			if (AlphaOneOdd) if (*AlphaOneOdd==NULL)
				*AlphaOneOdd=(byte *)malloc(VideoSize);
			}
		else {
			if (AlphaEvenZero) *AlphaEvenZero=NULL;
			if (AlphaOneOdd) *AlphaOneOdd=NULL;
			}

		GetFrame32 (TGAHandle,FieldEvenZero,FieldOddOne,AlphaEvenZero,AlphaOneOdd);
		//success
		Success=true;
		} while(false);
	return Success;
	}

void TargaReader::GetFrame32 (TargaInfo *TGAHandle,byte **FieldEvenZero,byte **FieldOddOne,byte **AlphaEvenZero,byte **AlphaOneOdd) {
	unsigned scanwidthbytes=TGAHandle->width*TGAHandle->BytesPerPixel;
	unsigned LineBufferSize=AlignBPS((scanwidthbytes));
	byte *LineBuffer1=(byte *)mallocAligned(LineBufferSize);
	byte *LineBuffer2=(byte *)mallocAligned(LineBufferSize);
	HANDLE hfile=TGAHandle->FileHandle;
	//Get the init seek position
	mySeek(hfile,0,FILE_BEGIN);

	unsigned height=TGAHandle->height;

	unsigned top;
	long direction;
	//Loop through rows*
	if (TGAHandle->TopToBottom) {
		top=0;
		direction=1;
		}
	else {
		top=height-1;
		direction=-1;
		}
	//Prime the pump... Note: no matter what we'll read "everything" sequentally
	unsigned BytesOverLapped=LineBufferSize-scanwidthbytes;
	unsigned Alignment; //Note: this should always be zero!
	unsigned BytesRead=LineBufferSize;
	unsigned seekposition=LineBufferSize;
	unsigned BothLineBuffers=LineBufferSize<<1;

	unsigned remainder=TGAHandle->FileSize%BothLineBuffers;
	unsigned eofaligned=TGAHandle->FileSize-remainder;
	myRead(hfile,LineBuffer1,LineBufferSize,&Alignment);
	//unsigned row,ycount;
	//for (row=top,ycount=0;ycount<height;row+=direction,ycount++) 
	do {
		//Prefetch the next line
		myRead(hfile,LineBuffer2,LineBufferSize,&Alignment,true);
		//Decode LineBuffer1
		seekposition+=LineBufferSize;
		if (seekposition>=eofaligned) break;
		myRead(hfile,LineBuffer1,LineBufferSize,&Alignment,true);
		//Decode LineBuffer2
		seekposition+=LineBufferSize;
		} while(seekposition<eofaligned);
	//free up resources
	freeAligned(LineBuffer1);
	freeAligned(LineBuffer2);
	}

/*
		{		//Now to make a sample table for each frame
			DWORDLONG qwAudioBase;
			byte AUXSourcePack[8];
			//FrameRate (0-NTSC,1-PAL).... SampleRate (48,41.1,32)
			static const unsigned int SampleSizeTable[2][3]= {{1580,1452,1053},{1896,1742,1264}};
			unsigned AddToSampleSize=SampleSizeTable[r_dv_aux.frame_rate][r_dv_aux.aaux[0].sample_rate];
			DWORD dwAudioOffset,dwAudioSize;
			DWORD audiomax=m_TotalAudioChunks=m_TotalFrames;
			AudioSampleInfo *l_SampleTable;
			unsigned AccumulatingSample=0;
			unsigned aindex;
			unsigned bytespersample=1;
			unsigned NoSamples;

			m_SampleTable=l_SampleTable=(AudioSampleInfo *)malloc(audiomax*sizeof(AudioSampleInfo));

			for (aindex=0;aindex<audiomax;aindex++) {
				getframeoffset(aindex,AVIstreamhead,&qwAudioBase,&dwAudioOffset,&dwAudioSize);
				//Now to add the magic number offset to the AUX source pack
				dwAudioOffset+=0x10E0;
				mySeek64(hfile,qwAudioBase+dwAudioOffset,SEEK_SET);
				myRead(hfile,AUXSourcePack,8);
				//Parity check
				if (!(AUXSourcePack[3]=0x50)) {
					free(l_SampleTable);
					m_SampleTable=NULL;
					break;
					}
				l_SampleTable[aindex].SampleStartCount=AccumulatingSample;
				NoSamples=(AUXSourcePack[4]&0x3f)+AddToSampleSize;
				l_SampleTable[aindex].SampleSize=NoSamples;
				AccumulatingSample+=NoSamples;
				}
			m_TotalAudioSamples=AccumulatingSample;
			}
*/


		//Use the storyboard method to find the transition
		TimeLineElement *tle=this->InOut_GetTimeLineElement();
		TimeLine *tl=tle?tle->GetTimeLine():NULL;
		if (tl) {
			StoryBoard_Operations *sbops=tl->GetStoryBoardOperations();
			unsigned clipindex=sbops->GetChildIndex(tle);
			if ((clipindex!=-1)&&(sbops->GetTLEtype(tle)==TLE_Clip)) {
				unsigned nextindex=sbops->FindLastEvent(clipindex,TLE_Clip|TLE_Trans,clipindex);
				if (nextindex!=-1) {
					TimeLineElement *transtle=sbops->GetChildItem_TLE(nextindex);
					if (sbops->GetTLEtype(transtle)==TLE_Trans)
						tranLength=transtle->TimeLineElement_GetLength();
					}
				}
			}

/* Delete Rules
	//If we are deleting a transition in insertion mode then we need not perform any ripple effects but do need to
	//re-adjust the clip length for both A and B
	if ((GetTLEtype(tle)==TLE_Trans)&&(m_InsertionMode)) {
		//Find the clips
		TimeLineElement *AClip,*BClip;
		unsigned TransIndex,AIndex,BIndex;
		double NewBStart;
		bool BClipMultRanged=false;

		TransIndex=GetChildIndex(tle);
		AIndex=FindLastEvent(TransIndex,TLE_Clip,TransIndex);
		BIndex=FindNextEvent(TransIndex,TLE_Clip,TransIndex);
		//Both ClipA and ClipB must exist otherwise we do NOP which is the same as non-insertion mode
		if (!(BIndex==-1)) {
			BClip=m_childItems[BIndex]->GetTimeLineElement();
			ContentInstance *Bci;
			Bci=BClip->GetContentInstance();
			double TransLength=tle->GetContentInstance()->ContentInstance_GetClipLength();
			double firsthalf=TransLength/2;
			NewBStart=BClip->GetGlobalStartPoint()+firsthalf;
			double BInpoint=GetInPoint(Bci,InOuts2_All);
			double Bsf=Bci->InOut_GetStretchFactor();
			if (!(AIndex==-1)) BClip->SetGlobalStartPoint(NewBStart);
			firsthalf/=Bsf;
			SetInPoint(Bci,firsthalf,InOuts2_All);
			}
		if (!(AIndex==-1)) {
			AClip=m_childItems[AIndex]->GetTimeLineElement();
			ContentInstance *Aci;
			Aci=AClip->GetContentInstance();
			double ALength=Aci->ContentInstance_GetClipLength();
			double TransLength=tle->GetContentInstance()->ContentInstance_GetClipLength();
			double firsthalf=TransLength/2;
			double AStart=AClip->GetGlobalStartPoint();
			double secondhalf;
			if ((BIndex==-1)||(BClipMultRanged)) secondhalf=TransLength-firsthalf;
			else secondhalf=ALength-(NewBStart-AStart);
			double AOutpoint=GetOutPoint(Aci,InOuts2_All);
			double Asf=Aci->InOut_GetStretchFactor();

			secondhalf/=Asf;
			SetOutPoint(Aci,-secondhalf,InOuts2_All);
			}
		}
	else {
		//If we have a clip next to a transition and a clip... the other clip needs to extend to half the duration of the
		//transition

		if ((GetTLEtype(tle)==TLE_Clip)&&(m_InsertionMode)) {
			ContentInstance *ci=tle->GetContentInstance();
			unsigned Index,AIndex,BIndex;
			TimeLineElement *AClip=NULL,*BClip=NULL;
			TLEtype Atype,Btype;

			Index=GetChildIndex(tle);
			AIndex=FindLastEvent(Index,TLE_Clip|TLE_Trans,Index);
			BIndex=FindNextEvent(Index,TLE_Clip|TLE_Trans,Index);
			if (!(AIndex==-1)) AClip=m_childItems[AIndex]->GetTimeLineElement();
			if (!(BIndex==-1)) BClip=m_childItems[BIndex]->GetTimeLineElement();
			Atype=GetTLEtype(AClip);
			Btype=GetTLEtype(BClip);

//CCT
			if ((Atype==TLE_Clip)&&(Btype==TLE_Trans)) {
				//Extend New A Clip out point
				double TransLength=BClip->GetContentInstance()->ContentInstance_GetClipLength();
				double firsthalf,scaledfirsthalf;
				firsthalf=TransLength/2;
				ContentInstance *Aci;
				Aci=AClip->GetContentInstance();
				double Asf=Aci->InOut_GetStretchFactor();
				scaledfirsthalf=firsthalf/Asf;
				SetOutPoint(Aci,scaledfirsthalf,InOuts2_All);
				//Now restore the deleted clip back
				Asf=ci->InOut_GetStretchFactor();
				scaledfirsthalf=firsthalf/Asf;
				SetOutPoint(ci,-scaledfirsthalf,InOuts2_All);
				//explictly Assign the transition to the new clip
				TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(BClip->GetContentInstance());
				if (tci) tci->Transition_SetClipA(AClip);
				}
//TCC
			else if ((Btype==TLE_Clip)&&(Atype==TLE_Trans)) {
				//Extend New B Clip in point
				double TransLength=AClip->GetContentInstance()->ContentInstance_GetClipLength();
				double firsthalf,scaledfirsthalf;
				firsthalf=TransLength/2;
				ContentInstance *Bci;
				Bci=BClip->GetContentInstance();
				double Bsf=Bci->InOut_GetStretchFactor();
				scaledfirsthalf=firsthalf/Bsf;
				SetInPoint(Bci,-scaledfirsthalf,InOuts2_All);
				//Now restore the deleted clip back
				Bsf=ci->InOut_GetStretchFactor();
				scaledfirsthalf=firsthalf/Bsf;
				SetInPoint(ci,scaledfirsthalf,InOuts2_All);
				//explictly Assign the transition to the new clip
				TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(AClip->GetContentInstance());
				if (tci) tci->Transition_SetClipB(BClip);
				}
			}
*/



/* 3rd
								if ((ElementType==TLE_Clip)&&(PrevElementType==TLE_Trans)) {
//-TC
									unsigned previndex=FindLastEvent(insertbeforehere,TLE_Clip|TLE_Trans,insertbeforehere);
									//are we inserting a transition between 2 clips or T t C  or / t C?
									if ((!(previndex==-1))) {
										TimeLineElement *oldtle=m_childItems[previndex]->GetTimeLineElement();
										if ((oldtle)&&(GetTLEtype(oldtle)==TLE_Clip)) {
//CTC
											double cilength=prevci->ContentInstance_GetClipLength();
											double firsthalf=cilength/2;
											double sf=ci->InOut_GetStretchFactor();
											double expandamount=firsthalf/sf;
											//Extend the In point back by the length of the first half
											double inpoint=GetInPoint(ci,InOuts2_All);
											if (doSBnumbering) SetStoryBoardPosition(i,tle,sbPos);
											SetInPoint(ci,-expandamount,InOuts2_All);
											//only disable ripple if we are between 2 clips
											ExtraInsertionModeAdjustment=TransitionException;
											}
										}
//CTC and TTC
									//Let the transition know who the B-Clip is
									TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(prevci);
									if (tci) tci->Transition_SetClipB(tle);
									}
								//If we are inserting a AClip that proceeds a transition we extend the new a and trim the old
								else if ((ElementType==TLE_Trans)&&(PrevElementType==TLE_Clip)) {
//-CT
									double TransLength=ci->ContentInstance_GetClipLength();
									double firsthalf,scaledfirsthalf;
									double Asf;
									firsthalf=TransLength/2;
									unsigned previndex=FindLastEvent(insertbeforehere,TLE_Clip|TLE_Trans,insertbeforehere);
									if ((!(previndex==-1))) {
										TimeLineElement *oldtle=m_childItems[previndex]->GetTimeLineElement();
										ContentInstance *oldci;
										if ((oldtle)&&(GetTLEtype(oldtle)==TLE_Clip)) {
//CCT
											oldci=oldtle->GetContentInstance();
											//trim the old A clip back
											Asf=oldci->InOut_GetStretchFactor();
											scaledfirsthalf=firsthalf/Asf;
											SetOutPoint(oldci,-scaledfirsthalf,InOuts2_All);
											PreviousElement->SetGlobalStartPoint(PreviousElement->GetGlobalStartPoint()-firsthalf);
											}
										}
//TCT and CCT
									//Now extend the new A clip out
									Asf=prevci->InOut_GetStretchFactor();
									scaledfirsthalf=firsthalf/Asf;
									SetOutPoint(prevci,scaledfirsthalf,InOuts2_All);
									//explictly Assign the transition to the new clip
									TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(ci);
									if (tci) tci->Transition_SetClipA(PreviousElement);
									ExtraInsertionModeAdjustment=ClipAException;
									}
*/

/* 2nd Rules
					if ((ElementType==TLE_Trans)&&(PrevElementType==TLE_Clip)) {
//CT-
						//Only extend A if we have a b clip
						ContentInstance *nextci=FindNextEventofDragItem(insertbeforehere,insertbeforehere+1,i,TLE_Clip|TLE_Trans);
						if (nextci) {
							TimeLineElement *oldtle=nextci->ContentInstance_GetTimeLineElement();
							if (oldtle) {
								if (GetTLEtype(oldtle)==TLE_Clip) {
//CTC
									double cilength=ci->ContentInstance_GetClipLength();
									double firsthalf=cilength/2;
									double outpoint=GetOutPoint(prevci,InOuts2_All);
									double inpoint=GetInPoint(prevci,InOuts2_All);
									double sf=prevci->InOut_GetStretchFactor();
									double Duration=prevci->ContentInstance_GetClipLength();
									double StartingPoint=(Duration-firsthalf);
									double endpointoftrans=StartingPoint+cilength;
									double expandamount=(endpointoftrans-Duration)/sf;
									SetOutPoint(prevci,expandamount,InOuts2_All);
									}
								else if (GetTLEtype(oldtle)==TLE_Trans) {
//CTT
									//we need to have the transition endpoint to match clip a (not halfway)
									double clipaend=PreviousElement->GetGlobalStartPoint();
									clipaend+=prevci->ContentInstance_GetClipLength();
									tle->SetGlobalStartPoint(clipaend-ci->ContentInstance_GetClipLength());
									}
								}
							}
						//Let the transition know who the A-Clip is
						TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(ci);
						if (tci) tci->Transition_SetClipA(PreviousElement);
						}
					//If we are inserting a BClip followed by a transition we extend the new b and trim the old
					else if ((ElementType==TLE_Clip)&&(PrevElementType==TLE_Trans)) {
//TC-
						double TransLength=prevci->ContentInstance_GetClipLength();
						double firsthalf,scaledfirsthalf;
						double Bsf;
						firsthalf=TransLength/2;
						if ((FirstLoop==true)&&(insertbeforehere<m_childItems.NoItems)) {
							unsigned oldbindex=FindLastEvent(insertbeforehere+1,TLE_Clip|TLE_Trans);
							if (!(oldbindex==-1)) {
								TimeLineElement *oldtle=m_childItems[oldbindex]->GetTimeLineElement();
								ContentInstance *oldci;
								if ((oldtle)&&(GetTLEtype(oldtle)==TLE_Clip)) {
//TCC
									oldci=oldtle->GetContentInstance();
									//trim the old B clip back
									Bsf=oldci->InOut_GetStretchFactor();
									scaledfirsthalf=firsthalf/Bsf;
									SetInPoint(oldci,scaledfirsthalf,InOuts2_All);
									oldtle->SetGlobalStartPoint(oldtle->GetGlobalStartPoint()+firsthalf);
									}
								}
							}
//TCT & TCC
						//Now extend the new B clip out
						Bsf=ci->InOut_GetStretchFactor();
						scaledfirsthalf=firsthalf/Bsf;
						SetInPoint(ci,-scaledfirsthalf,InOuts2_All);
						//explictly Assign the transition to the new clip
						TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(prevci);
						if (tci) tci->Transition_SetClipB(tle);
						}
					}  //end if insertion mode
				if ((ElementType==TLE_Trans)&&(PrevElementType==TLE_Trans)) {
//TT-
					if ((FirstLoop==true)&&(insertbeforehere<m_childItems.NoItems)) {
						unsigned oldbindex=FindLastEvent(insertbeforehere+1,TLE_Clip|TLE_Trans);
						if (!(oldbindex==-1)) {
							TimeLineElement *oldtle=m_childItems[oldbindex]->GetTimeLineElement();
							if (oldtle) {
								TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(prevci);
								if (tci) {
									if (GetTLEtype(oldtle)==TLE_Clip)
//TTC								
										tci->Transition_SetClipB(oldtle);
									else
//TTT
										tci->Transition_SetClipB(NULL);
									tci->Transition_SetClipA(NULL);
									}
								}
							}
						}

					}
*/


/*
				//check all streams for changes
				tList<char *> *streamlist=m_InOut_Interface->InOut_GetStreamTypes();
				char *l_stream;
				double newin,newout;

				if (streamlist) {
					unsigned i,eoi=streamlist->NoItems;
					for (i=0;i<eoi;i++) {
						l_stream=(*streamlist)[i];
						newin=GetInPoint(l_stream);
						newout=GetOutPoint(l_stream);
						if (!(newin==m_In.Get())) m_In.Set(newin);
						if (!(newout==m_Out.Get())) m_Out.Set(newout);
						}
					}
*/

void AssetCombo::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	//This does not work! the m_Combo is not being properly deleted... if you open prefs change a combo item
	//close prefs then open again it will crash... I'm going to have to fix this correctly to where the combo
	//listens for AssetCombo changes

	//TODO even though this seems to be working, its really not the correct way to handle the 
	//changes... we should probably have the new AssetComboButton listen for when the combo and bool setter functions
	//change As of right now any time the value changes this will get called.
/*
	if (ItemChanging==this) {
		if (IsChanged(String)) if (m_Combo) {
			unsigned i,eoi;
			char *string;
			//We'll start clean
			 m_Combo->DeleteAllItems();
			 //m_Combo->Button_UseVariable(this);
			//Set some default attributes
			//All of the resources for ButtonLabel have defaults already 
			// Set the properties for the button we just created
			//See if we can add the tList to the combobutton
			
			if (eoi=m_AssetItems.NoItems)
				for (i=0;i<eoi;i++)
					m_Combo->AddItem(m_AssetItems[i].text,m_AssetItems[i].id);

			m_Combo->GetSelection(&string);
			m_Combo->ButtonLabel_SetTextSelection(string);
			}
		}
*/
	}

/*
		if ((!(AIndex==-1))&&(!(BIndex==-1))) {
			AClip=m_childItems[AIndex]->GetTimeLineElement();
			BClip=m_childItems[BIndex]->GetTimeLineElement();
			//make sure Clip A is not multiranged
			if (!(IsClipMultiRanged(AClip))) {
				ContentInstance *Aci,*Bci;
				Aci=AClip->GetContentInstance();
				Bci=BClip->GetContentInstance();
				double ALength=Aci->ContentInstance_GetClipLength();
				double TransLength=tle->GetContentInstance()->ContentInstance_GetClipLength();
				double firsthalf=TransLength/2;
				double NewBStart=BClip->GetGlobalStartPoint()+firsthalf;
				double AStart=AClip->GetGlobalStartPoint();
				double secondhalf=ALength-(NewBStart-AStart);
				double AOutpoint=GetOutPoint(Aci,InOuts2_All);
				double BInpoint=GetInPoint(Bci,InOuts2_All);
				double Asf=Aci->InOut_GetStretchFactor();
				double Bsf=Bci->InOut_GetStretchFactor();

				BClip->SetGlobalStartPoint(NewBStart);
				firsthalf/=Bsf;secondhalf/=Asf;
				SetOutPoint(Aci,AOutpoint-secondhalf,InOuts2_All);
				SetInPoint(Bci,BInpoint+firsthalf,InOuts2_All);
				}
			}
*/


	/*
	double oldcliplength,newcliplength;
	TimeLineElement *tle=wrapper->InOut_GetTimeLineElement();
	DebugOutput("SB complete changes\n");
	//for better performance check to see if the length has been modified
	oldcliplength=tle->GetContentInstance()->ContentInstance_GetClipLength();
	wrapper->PerformAllChanges();
	newcliplength=tle->GetContentInstance()->ContentInstance_GetClipLength();
	if (!(newcliplength==oldcliplength)) {
		m_timeLine->ContentRegionEditor_StartRegionChanges();
		//The elements must be in order (We may want to see if we can keep in order without the sort)
		SortTLEs();
		InOut_SmartRipple(tle);
		m_timeLine->ContentRegionEditor_CompleteRegionChanges();
		}
	*/
			/*
			ContentInstance *ci;
			TimeLineElement *tle;
			TimeLine *timeline=NULL;
			if (ci=vc->VideoEditor_Crouton_GetContentInstance()) {
				tle=ci->ContentInstance_GetTimeLineElement();
				if (tle) timeline=tle->GetTimeLine();
				//If our project hasn't been set yet or if the ci is not associated to any project
				//finally if the projects match
				if ((!tle)||(!m_TimeLine)||(timeline==m_TimeLine)) SetContentInstance(ci);
				}
				*/

		unsigned SmartRippleForClipTrans(unsigned index);

unsigned StoryBoard_Layout::SmartRippleForClipTrans(unsigned index) {
	//Both Transitions and clips have pretty much the same rules applied for this range
	//Return 0 if we need not perform dumb ripple i.e. reached the end of the list
	//From index position to the next defined clip we have to analyze each crouton and compute its new starting point
	double offset=0,startingpoint;
	unsigned eoi=m_childItems.NoItems,deletedindex=index;
	VideoEditor_Layout_ChildItem *scanstart=m_childItems[index];
	VideoEditor_Layout_ChildItem *scan;
	index++;

	for (;index<eoi;index++) {
		//Note I'm exanding all of this out for now to debug---
		scan=m_childItems[index];
		TimeLineElement *tle=scan->GetTimeLineElement();
		TLEtype type=GetTLEtype(tle->GetContentInstance());
		if (type==TLE_Clip) break;
		//Find the correct previous clip to compute starting point
		unsigned previndex=FindLastEvent(index,GetPrevAcceptableEvent(type),deletedindex);
		startingpoint=ComputeStartingPoint(previndex,index,offset);
		m_childItems[index]->SetStartingPoint(startingpoint);
		}

	if (index>=eoi) index=0; //no need to ripple if we reached the end
	return (index);
	}



	/*
	switch (type) {
		case TLE_Clip:
			if (index=SmartRippleForClipTrans(index)) {
				double Adjustment=0,newstartingpoint;
				unsigned prev=FindLastEvent(index,GetPrevAcceptableEvent(TLE_Clip),startindex);
				if (!(prev==-1)) newstartingpoint=ComputeStartingPoint(prev,index); //ToDo add offset
				else newstartingpoint=0;
				Adjustment=newstartingpoint-(m_childItems[index]->GetTimeLineElement()->GetGlobalStartPoint());
				DumbRipple(scan->start,Adjustment,index);
				}
			break;

		case TLE_Trans:
			if (index=SmartRippleForClipTrans(index)) {
				double Adjustment=0,newstartingpoint;
				unsigned prev=FindLastEvent(index,GetPrevAcceptableEvent(TLE_Clip),startindex);
				if (!(prev==-1)) newstartingpoint=ComputeStartingPoint(prev,index); //ToDo add offset
				else newstartingpoint=0;
				Adjustment=newstartingpoint-(m_childItems[index]->GetTimeLineElement()->GetGlobalStartPoint());
				DumbRipple(scan->start,Adjustment,index);
				}
			break;

		case TLE_Audio: {
			double offset=0,startingpoint,delduration;
			unsigned eoi=m_childItems.NoItems,deletedindex=index;
			VideoEditor_Layout_ChildItem *scanstart=m_childItems[index];
			VideoEditor_Layout_ChildItem *scan;
			index++;
			delduration=scanstart->end-scanstart->start;
			for (;index<eoi;index++) {
				//Note I'm exanding all of this out for now to debug---
				scan=m_childItems[index];
				TimeLineElement *tle=scan->GetTimeLineElement();
				TLEtype type=GetTLEtype(tle->GetContentInstance());
				if (type!=TLE_Audio) break;
				//Find the correct previous clip to compute starting point
				unsigned previndex=FindLastEvent(index,GetPrevAcceptableEvent(type),deletedindex);
				//Calculate offset
				startingpoint=ComputeStartingPoint(previndex,index,offset);
				m_childItems[index]->SetStartingPoint(startingpoint);
				}
			break;
			}
		}
		*/

				/*
				double Adjustment=0;
				TLEtype prevtype,nexttype;
				scan=m_childItems[startindex];
				//TODO see if we have ripple mode... 
				unsigned prev=FindLastEvent(index,GetPrevAcceptableEvent(TLE_Clip),startindex);
				if (!(prev==-1)) prevtype=GetTLEtype(m_childItems[prev]->GetTimeLineElement());
				Adjustment=(scan->end-scan->start)*-1;
				if (prevtype==TLE_Trans) {
					unsigned next=FindNextEvent(startindex,GetPrevAcceptableEvent(TLE_Clip),startindex);
					if (!(next==-1)) {
						nexttype=GetTLEtype(m_childItems[next]->GetTimeLineElement());
						if (nexttype==TLE_Trans) {
							VideoEditor_Layout_ChildItem *prevscan=m_childItems[prev];
							Adjustment+=(prevscan->end-prevscan->start);
							VideoEditor_Layout_ChildItem *nextscan=m_childItems[next];
							Adjustment+=(nextscan->end-nextscan->start);
							}
						}
					}
				*/

				/*
				TLEtype prevtype;
				scan=m_childItems[startindex];
				//TODO see if we have ripple mode... 
				//TODO see if we have insert mode enabled
				unsigned prev=FindLastEvent(index,GetPrevAcceptableEvent(TLE_Trans),startindex);
				if (!(prev==-1)) prevtype=GetTLEtype(m_childItems[prev]->GetTimeLineElement());
				switch (prevtype)
					{
					case TLE_Clip:
						DumbRipple(scan->start,scan->end-scan->start,index);
						break;
					case TLE_Trans:
						DumbRipple(scan->start,(scan->end-scan->start)*-1,index);
						break;
					}
				*/



						//Adjustment=ComputeRippleTime(PreviousElement,FindNextEvent(Scan->Next,GetPrevAcceptableEvent(GetTLEtype(newci))),newci,Adjustment);

							//Adjustment=ComputeRippleTime(PreviousElement,FindNextEvent(Scan->Next,GetPrevAcceptableEvent(GetTLEtype(ci))),ci,Adjustment);

double StoryBoard_Layout::ComputeRippleTime(TimeLineElement *PreviousElement,TimeLineElement *NextElement,ContentInstance *ci,double Adjustment) {
	TLEtype type=GetTLEtype(ci);
	switch (type) {
		case TLE_Clip:
			Adjustment+=ci->ContentInstance_GetClipLength();
			//Note these only apply when we are in non-insert mode
			if (GetTLEtype(PreviousElement)==TLE_Trans) {
				Adjustment-=PreviousElement->GetContentInstance()->ContentInstance_GetClipLength();
				//Yes only check the next one if the previous one has been set
				if (GetTLEtype(NextElement)==TLE_Trans)
					Adjustment-=NextElement->GetContentInstance()->ContentInstance_GetClipLength();
				}
			break;
		case TLE_Trans:
			if (GetTLEtype(PreviousElement)==TLE_Trans)
				Adjustment+=PreviousElement->GetContentInstance()->ContentInstance_GetClipLength();
			else 
				Adjustment-=ci->ContentInstance_GetClipLength();
			break;
		}
	return Adjustment;
	}

		double ComputeRippleTime(TimeLineElement *PreviousElement,TimeLineElement *NextElement,ContentInstance *ci,double Adjustment);

//	TimeLine *l_TimeLine=m_timeLine;
//	TimeLineElement *tle;
	//it is probably faster to go through the entire list than to sort the elements
//	unsigned eoi=l_TimeLine->GetNumTimeLineElements();
//	for (unsigned i=0;i<eoi;i++) {
//		tle=l_TimeLine->GetTimeLineElement(i);
//		if (tle->GetGlobalStartPoint()>=StartTime) 
//			tle->SetGlobalStartPoint(tle->GetGlobalStartPoint()+Adjustment);
//		}


	// We are going to use the Brute Force approach of working through all of my children in order,
	// An setting their ContentInstances to those found in the list
	// ASSERT: All Children should be VideoEditor_Croutons by now!!!

	/*
	unsigned timeLineIndex = 0;
	unsigned numElements = this->m_timeLine->GetNumTimeLineElements();
	unsigned numChildren = this->GetNoChildren();
	unsigned numValidChildren = 0;
	for (unsigned childIndex = 0; (childIndex < numChildren) && (timeLineIndex < numElements); childIndex++)
	{
		// Get this Child
		VideoEditor_Crouton* thisCrouton = GetWindowInterface<VideoEditor_Crouton>(this->GetChildhWnd(childIndex));
		VideoEditor_Layout_ChildItem* thisChild = this->VideoEditor_Layout_GetChildItem(thisCrouton->GetWindowHandle());
		if (thisChild)
		{
			numValidChildren++;

			// Get the TimeLineElement we are interested in here
			TimeLineElement* thisElement = this->m_timeLine->GetTimeLineElement(timeLineIndex++);

			// Set the Crouton and the Child to look at this element
			thisChild->timeLineElement = thisElement;
			thisCrouton->VideoEditor_Crouton_SetContentInstance(thisElement->GetContentInstance(), false);
		}
	}

	if (numElements > numChildren)
	{
		// We need to create some new elements and set them up
		for (unsigned i = numChildren; i < numElements; i++)
		{
			// Create a new Crouton
			TimeLineElement* thisElement = this->m_timeLine->GetTimeLineElement(timeLineIndex++);
			VideoEditor_Crouton* newCrouton = GetWindowInterface<VideoEditor_Crouton>(this->OpenChild("VideoEditor_Crouton"));
			newCrouton->VideoEditor_Crouton_SetContentInstance(thisElement->GetContentInstance(), false);
			this->VideoEditor_Layout_AddChildItem(newCrouton->GetWindowHandle(), thisElement);
		}
	}
	*/

	//char string[MAX_PATH];
	//char *path=GetSkinRoot();
	//sprintf(string,"%s\\ProcAmp_AdvancedSettings.txt",path);
	//ChangeAssetList(string);


			if (FIC) {
				//insert sort ContextMenu interface in order of priority order (2000) - 0 - (-2000)
				unsigned insertpos;
				unsigned eoi=interfaceList.NoItems;
				long IndexedItemVal;
				long NewItemVal=FIC->Interface_GetMenuPriority();
				for (insertpos=0;insertpos<eoi;insertpos++) {
					IndexedItemVal=interfaceList[insertpos]->Interface_GetMenuPriority();
					if (NewItemVal>=IndexedItemVal) break;
					}
				//TODO: make sure that tList really inserts instead of append given position
				//It appears to do so... -James
				interfaceList.Add(FIC,insertpos);
				}
/*
AnimatedIcon_Interface* FileIcon::GetAnimatedIconReader(char* fileName)
{
	if (!fileName) return NULL;
	
	// We cycle through all plugins that are of the 
	unsigned NoPluginInfos;
	PluginClassInfo** Infos=NewTek_GetPluginInfo(NoPluginInfos);

	// Look at all the plugins, for the correct type
	for(unsigned i=0;i<NoPluginInfos;i++)
	{	// Get the item
		PluginClassInfo* Nfo=Infos[i];

		// Is the item the correct type ?
		if (!strcmp(Nfo->Category,"AnimatedIcon_Interface"))
		{	// Create the plugin
			PluginClass *PC=Nfo->New();
			AnimatedIcon_Interface *FIC=GetInterface<AnimatedIcon_Interface>(PC);
			if ((FIC)&&(FIC->AnimatedIcon_CreateAnimatedIcon(fileName)))
			{
				return FIC;
			}				
			NewTek_Delete(PC);
		}
	}

	// Error
	return NULL;
}
*/


/*#define ProcAmp_ColorControls_Brightness_Default	100
#define ProcAmp_ColorControls_Contrast_Default		100
#define ProcAmp_ColorControls_Hue_Default			100
#define ProcAmp_ColorControls_Saturation_Default	100
#define ProcAmp_ColorControls_UOffSet_Default		100
#define ProcAmp_ColorControls_VOffSet_Default		100
#define ProcAmp_ColorControls_UGain_Default			100
#define ProcAmp_ColorControls_VGain_Default			100
#define ProcAmp_ColorControls_Pedestal_Default		1*/

//******************************************************************************************************************
/*
class PROCDLL ProcAmp : public StretchySkinControl
{	private:	// My Skin
				ScreenObject_StretchySkin *m_ScreenObjectSkin;

				// From StretchySkinControl
				virtual void	ReadClassData(FILE* fp);

				// The colors
				Dynamic<float>	ColorControls[ProcAmp_ColorControls_Num];
				Dynamic<int>	Pedestal;

				// My own bitmap item
				BitmapItem		SliderBar_Up;
				BitmapItem		SliderBar_Dn;

				// My access to the actual switcher control
				struct SwitcherClient *sc;

	public:		// Get the window ready
				virtual void InitialiseWindow(void);
				virtual void DestroyWindow(void);
				virtual void DynamicCallback(long ID,char *String,void *args);
};
*/

/*
void ProcAmp::DynamicCallback(long ID,char *String,void *args)
{	if (IsWindowMessage(String)) return;

	else if (ID==(long)&EditValue[ProcAmp_EditValue_Brightness])
	{	long Val=EditValue[ProcAmp_EditValue_Brightness];
		switcherclient_SetVideoInputParam(sc,0,0,"control.brightness",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_Contrast])
	{	long Val=EditValue[ProcAmp_EditValue_Contrast];
		switcherclient_SetVideoInputParam(sc,0,0,"control.contrast",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_Hue])
	{	long Val=EditValue[ProcAmp_EditValue_Hue];
		switcherclient_SetVideoInputParam(sc,0,0,"control.hue",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_Saturation])
	{	long Val=EditValue[ProcAmp_EditValue_Saturation];
		switcherclient_SetVideoInputParam(sc,0,0,"control.saturation",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_UOffSet])
	{	long Val=EditValue[ProcAmp_EditValue_UOffSet];
		switcherclient_SetVideoInputParam(sc,0,0,"control.uoffset",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_VOffSet])
	{	long Val=EditValue[ProcAmp_EditValue_VOffSet];
		switcherclient_SetVideoInputParam(sc,0,0,"control.voffset",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_UGain])
	{	long Val=EditValue[ProcAmp_EditValue_UGain];
		switcherclient_SetVideoInputParam(sc,0,0,"control.ugain",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&EditValue[ProcAmp_EditValue_VGain])
	{	long Val=EditValue[ProcAmp_EditValue_VGain];
		switcherclient_SetVideoInputParam(sc,0,0,"control.vgain",SVT_INTEGER, &Val);
	}
	else if (ID==(long)&Pedestal)
	{	long Val=Pedestal;
		switcherclient_SetVideoOutputParam(sc,0,0,"control.pedestal",SVT_LONG,&Val);
		switcherclient_SetVideoInputParam (sc,0,0,"control.pedestal",SVT_LONG,&Val);
	}
	else StretchySkinControl::DynamicCallback(ID,String,args);
}


void ProcAmp::ReadClassData(FILE* fp)
{	// Load the sliderbar
	SliderBar_Dn.GetBitmap()->ReadBitmapFile(FindFiles_FindFile(FINDFILE_SKINS,"ProcAmp\\Slider_Button_RO.tga"));
	SliderBar_Dn.SetAlignment(BitmapTile_StretchX|BitmapTile_TopAlign);

	SliderBar_Up.GetBitmap()->ReadBitmapFile(FindFiles_FindFile(FINDFILE_SKINS,"ProcAmp\\Slider_Button.tga"));
	SliderBar_Up.SetAlignment(BitmapTile_StretchX|BitmapTile_TopAlign);	
	
	// Setup the sliders
	unsigned SliderColors[]= {	RGBA(231,101,26), RGBA(238,156,0), RGBA(249,244,0), RGBA(125,198,34), RGBA(0,159,98),
								RGBA(0,164,158),  RGBA(0,105,179), RGBA(50,67,149) };
	// Setup the text boxes
	unsigned TextColors[]= {	RGBA(243,158,119), RGBA(246,181,128), RGBA(249,205,138), RGBA(255,249,157), RGBA(199,225,158),
								RGBA(137,202,157), RGBA(141,207,244), RGBA(149,149,198) };

	static Dynamic<float> Min(-127.0);
	static Dynamic<float> Max( 128.0);

	for(unsigned i=0;i<ProcAmp_EditValue_Num;i++)
	{	UtilLib_Slider *US=GetWindowInterface<UtilLib_Slider>(OpenChild(SliderColors[i],"UtilLib_Slider"));
		if (!US) _throw "ProcAmp::ReadClassData Could not ceate slider !";

		US->Button_SetResource(Controls_Button_UnSelected,&SliderBar_Up);
		US->Button_SetResource(Controls_Button_Selected,&SliderBar_Dn);
		US->Button_SetResource(Controls_Button_MouseOver,&SliderBar_Dn);
		US->Button_SetResource(Controls_Button_MouseOverDn,&SliderBar_Dn);
		US->Slider_SetVariable(&EditValue[i]);
		US->Slider_SetMinVariable(&Max);
		US->Slider_SetMaxVariable(&Min);

		// Because the default slider size forces a fixed width, we need to do resize the thing
		// now that we have set the resources. I do not like this
		RECT rect;
		m_ScreenObjectSkin->GetRect(SliderColors[i],rect);
		US->SetWindowPosition(rect.left,rect.top);
		US->SetWindowSize(rect.right-rect.left,rect.bottom-rect.top);

		UtilLib_Edit *UE=GetWindowInterface<UtilLib_Edit>(OpenChild(TextColors[i],"UtilLib_Edit"));
		if (!UE) _throw "ProcAmp::ReadClassData Could not ceate slider !";
		UE->Slider_SetVariable(&EditValue[i]);
		UE->Slider_SetMinVariable(&Min);
		UE->Slider_SetMaxVariable(&Max);
	}	

	// Pedestal
	SkinControl_SubControl					*MyControl=OpenChild_SubControl(RGBA(223,0,41),"SkinControl_SubControl_ToggleButton");
	SkinControl_SubControl_ToggleButton		*But=GetInterface<SkinControl_SubControl_ToggleButton>(MyControl);
	if (!But) _throw "Could not case to SkinControl_SubControl_ToggleButton";
	But->Button_SetResource(0,0); 
	But->Button_SetResource(1,2);
	But->Button_SetResource(2,1);
	But->Button_SetResource(3,3);
	But->Button_UseVariable(&Pedestal);

	// Get the size of the window
	SetWindowSize(	m_ScreenObjectSkin->ScreenObject_GetPreferedXSize(),
					m_ScreenObjectSkin->ScreenObject_GetPreferedYSize());
}

void ProcAmp::InitialiseWindow(void)
{	// Setup the defaults
	EditValue[ProcAmp_EditValue_Brightness] = ProcAmp_EditValue_Brightness_Default;
	EditValue[ProcAmp_EditValue_Contrast]	= ProcAmp_EditValue_Contrast_Default;
	EditValue[ProcAmp_EditValue_Hue]		= ProcAmp_EditValue_Hue_Default;
	EditValue[ProcAmp_EditValue_Saturation]	= ProcAmp_EditValue_Saturation_Default;
	EditValue[ProcAmp_EditValue_UOffSet]	= ProcAmp_EditValue_UOffSet_Default;
	EditValue[ProcAmp_EditValue_VOffSet]	= ProcAmp_EditValue_VOffSet_Default;
	EditValue[ProcAmp_EditValue_UGain]		= ProcAmp_EditValue_UGain_Default;
	EditValue[ProcAmp_EditValue_VGain]		= ProcAmp_EditValue_VGain_Default;
	Pedestal=ProcAmp_EditValue_Pedestal_Default;
	
	// Setup the skin
	m_ScreenObjectSkin=GetInterface<ScreenObject_StretchySkin>(NewTek_New("ScreenObject_StretchySkin"));
	if (!m_ScreenObjectSkin) _throw "KeyerControl::InitialiseWindow Cannot create the stretchy skin !";
	Canvas_SetResource(m_ScreenObjectSkin);
	ReadScriptFile(FindFiles_FindFile(FINDFILE_SKINS,"ProcAmp\\ProcAmp.txt"));
	StretchySkinControl::InitialiseWindow();

	// Add dependants
	for(unsigned i=0;i<ProcAmp_EditValue_Num;i++)
		EditValue[i].AddDependant(this,(long)&EditValue[i]);
	Pedestal.AddDependant(this,(long)&Pedestal);

	// Setup the switcher stuff 
	// We get access to the actual switcher client
	LARGE_INTEGER Timer;  QueryPerformanceCounter(&Timer);
	char Temp[256]; 
	sprintf(Temp,"Apps.Deliverance.ProcAmp.%dI64",Timer);
	
	if (sc=switcherclient_Create(Temp)) {}
	else _throw "SwitcherControl::SwitcherControl, Could not access the switcher.";	
}	

void ProcAmp::DestroyWindow(void)
{	// Close access to the switcher
	if (sc) switcherclient_Delete(sc);
	
	// Remove dependants
	for(unsigned i=0;i<ProcAmp_EditValue_Num;i++)
		EditValue[i].DeleteDependant(this);
	Pedestal.DeleteDependant(this);
	
	// Delete my resource
	if (m_ScreenObjectSkin) NewTek_Delete(m_ScreenObjectSkin);	

	// We must always call my predecssor
	StretchySkinControl::DestroyWindow();
}
*/




				else {
					//infinately fill with silence
					memset (dsbuf1,(BYTE)(avivar->waveinfo.wBitsPerSample == 8 ? 128 : 0),min(dsbuflen1,samplesize));
					if (dsbuf2) {
						if (samplesize>dsbuflen1) memset(dsbuf2,(BYTE)(avivar->waveinfo.wBitsPerSample == 8 ? 128 : 0),min(samplesize-dsbuflen1,dsbuflen2));
						avivar->lastoffset=dsbuflen2;
						}
					else avivar->lastoffset+=dsbuflen1;
					}

								//take care of the end if it is a small fragment to pad with space
				if (samplesize<(avivar->audiobuffersize>>1)) {
					samplesize=avivar->audiobuffersize;
					if FAILED(lpdsb->Lock(avivar->lastoffset,samplesize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)) {
						printc("Unable to lock DS buffer");
						continue;
						}
					//infinately fill with silence
					memset (dsbuf1,(BYTE)(avivar->waveinfo.wBitsPerSample == 8 ? 128 : 0),min(dsbuflen1,samplesize));
					if (dsbuf2) {
						if (samplesize>dsbuflen1) memset(dsbuf2,(BYTE)(avivar->waveinfo.wBitsPerSample == 8 ? 128 : 0),min(samplesize-dsbuflen1,dsbuflen2));
						avivar->lastoffset=dsbuflen2;
						}
					else avivar->lastoffset+=dsbuflen1;
					if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2)) {
						printc("Unable to unlock DS buffer");
						continue;
						}


//This one is a perfect one for one scrub mode
BOOL loadersclass::aviclass::queueaudio(struct imagelist *mediaptr,ULONG framenum,ULONG samplesize) {
	class avihandle *avivar=mediaptr->mediaavi;
	void *dsbuf1,*dsbuf2;
	DWORD dsbuflen1,dsbuflen2;
	LPDIRECTSOUNDBUFFER lpdsb=avivar->lpdsb;
	BYTE *audiopcm=avivar->audiopcm;

	if ((avivar->lpdsb)&&(controls->mycontrolis&2)) {

		if FAILED(lpdsb->Lock(avivar->lastoffset,samplesize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)) {
			printc("Unable to lock DS buffer");
			return (TRUE);
			}

		memcpy (dsbuf1,audiopcm,min(dsbuflen1,samplesize));
		if (dsbuf2) {
			if (samplesize>dsbuflen1) memcpy(dsbuf2,audiopcm+dsbuflen1,min(samplesize-dsbuflen1,dsbuflen2));
			avivar->lastoffset=dsbuflen2;
			}
		else avivar->lastoffset+=dsbuflen1;

		if (avivar->lastoffset>=avivar->totalaudiosize) avivar->lastoffset=0;
		//unlock immediately
		if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2)) {
			printc("Unable to unlock DS buffer");
			return (TRUE);
			}

		if (framenum==(ULONG)mediaptr->cropin) {
			avivar->lpdsb->SetCurrentPosition(0);
			avivar->lpdsb->Play(0,0,DSBPLAY_LOOPING);
			}

		if (framenum>=(ULONG)mediaptr->cropout-1) {
			avivar->lpdsb->Play(0,0,0);
			avivar->lastoffset=0;
			}
		} // end if we have sound
	return(0);
	} //end stream audio 


				/*
				For now until we work with other avi types... we'll ignore audio stream of type2
				DV avi's... eventually we'll need to support multiple streams for the older avi's
				
				if (strh->avistreamnode.fccType==ID_auds) {
					lseek(hfile,advance,SEEK_CUR);
					goto ignoreindex;
					}
				*/


//Originally in the else of the buttondown
					//selectimageobj->resetlist(TRUE);
					//updateimagelist();
/*
Originally in the buttonup area
					//if (wParam&MK_SHIFT) selectimageobj->shiftselect(imageindex);

				if ((wParam&MK_CONTROL)&&(imageindex)) {
					if (imageindex->id!=id_dir) {
						selectimageobj->selectimage(imageindex);
						updateimagelist();
						}
					}

				else if ((wParam&MK_SHIFT)&&(imageindex)) {
					if (imageindex->id!=id_dir) {
						selectimageobj->shiftselect(imageindex);
						updateimagelist();
						}
					}
*/

/*
	//if both blocks set then start over
	if (blocka==blockb) {
		if (blocka) selectimage(blocka);
		blockb=imageptr;
		}
	else if ((blocka)&&(blockb)) blocka=blockb=imageptr;
	//see if blocks have been started
	if (blocka==NULL) blocka=imageptr;
	if (blockb==NULL) blockb=imageptr;
*/


/*
					const static ULONG headerdata[18]= {0x52544d46,1,0x0200,2,0xa8c00,0,0x080001,0x054600,0x8724,0x06b4,0x05a0,0xf0,0x080001,0x054600,0x076a10,0x06b4,0x05a0,0xf0};
					char *tempmem=(char *)mynew(&pmem,691712);
					memset(tempmem,0,512);
					memcpy(tempmem,headerdata,18*4);
					memcpy(tempmem+512,toaster->videobuf,691200);
					save("c:\\myprojects\\storyboard\\imports\\test.rtv",691712,tempmem);
*/

//Old Stuff
/*
	StretchDIBits(hmem,0,0,720,VIDEOY,0,0,avivar->x,avivar->y,decodedbits,(LPBITMAPINFO)lpBitmapInfoHeader,DIB_RGB_COLORS,SRCCOPY);
	//if (!(dest=(char *)mynew(&pmem,1382400))) return(0);
	//if (dest) dispose((struct memlist *)dest,&pmem);
*/
//8088 way to convert to RGB
/*
			unsigned long R,G,B,R1,G1,B1,UYVY;
			R=G=B=*((ULONG *)(dest+(xindex<<2)+2880*yindex));	
			R1=G1=B1=*((ULONG *)(dest+((xindex+1)<<2)+2880*yindex));
*/
/*
			B=B&0x0ff;
			G=G>>8&0x0ff;
			R=R>>16&0x0ff;
			B1=B1&0x0ff;
			G1=G1>>8&0x0ff;
			R1=R1>>16&0x0ff;

			UYVY=(((28770*B-19071*G-9699*R)>>16)+128)+\
			 (((16843*R+33030*G+6423*B)>>8&65280)+4096)+\
			 (((28770*R1-24117*G1-4653*B1)&16711680)+8388608)+\
			 (((16843*R1+33030*G1+6423*B1)<<8&4278190080)+268435456);
			*YUVdest++=UYVY;
*/
/*
	{
	ULONG xmin,ymin,xcenter,ycenter;
	ULONG *destindex;

	destindex=(ULONG *)dest;
	xcenter=ycenter=0;
	if (x<672) xcenter=((720-x)>>1)-24;
	if (y<480) ycenter=(480-y)>>1;
	xmin=min(x,720);ymin=min(y,480);
	destindex+=720*ycenter;
	for (yindex=ymin-1;yindex>0;yindex--) {
		destindex+=xcenter;
		for (xindex=0;xindex<xmin;xindex++) {
			*destindex++=*((ULONG *)decodedbits+xindex+x*yindex);
			}
		if (x<720) for (xindex=x+xcenter;xindex<720;xindex++) *destindex++=0;
		}
	}
*/

/*
ULONG *filtersclass::CGclass::opentga(char *filesource,UBYTE **alpha,UWORD *width,UWORD *height) {
	ULONG *YUVdest,*YUVdeststart;
	char *tempbuffer,*source,*dest;
	long size=0;
	ULONG *destindex;
	UBYTE *alphaindex;
	ULONG xindex,yindex;
	ULONG x,y,xmin,ymin;
	UWORD x2,y2;

	if (debug) printc("tga");
	if (!(source=(char *)(load(filesource,&size,&pmem)))) goto error;
	if (!(tempbuffer=medialoaders->tgaobj.TGA2raw(source,size,&x2,&y2))) goto error;
	x=(ULONG)x2;y=(ULONG)y2; //optimise make them long
	//calculate the width to a multiple of 8
	if (x&7) x+=(8-(x%8));
	//This is our final width and height
	dispose((struct memlist *)source,&pmem);
	//Scale the image into 720x480 using crop
	xmin=min(x,720);ymin=min(y,480);
	*width=(UWORD)xmin;*height=(UWORD)ymin;
	if (!(dest=(char *)mynew(&pmem,xmin*ymin*4))) goto error;
	if (!(*alpha=alphaindex=(UBYTE *)mynew(&pmem,xmin*ymin))) goto error;
	destindex=(ULONG *)dest;
	for (yindex=0;yindex<ymin;yindex++) {
		for (xindex=0;xindex<xmin;xindex++) {
			*destindex++=*((ULONG *)tempbuffer+xindex+x2*yindex);
			//*alphaindex++=(UBYTE)(*((ULONG *)tempbuffer+xindex+x*yindex));
			}
		}
	dispose((struct memlist *)tempbuffer,&pmem);
	//Now we have 720x480x32 RGB now we convert to YUV and separate fields
	//Intentionally close these vars in a local scope to pull registers
	if (!(YUVdest=YUVdeststart=(ULONG *)mynew(&pmem,xmin*ymin*2))) goto error;
	for (yindex=0;yindex<ymin;yindex+=2) {
		for (xindex=0;xindex<xmin;xindex+=2) {
			unsigned long R,G,B,A,R1,G1,B1,A1,UYVY;
			R=G=B=A=*((ULONG *)(dest+xindex*4+xmin*4*yindex));	
			R1=G1=B1=A1=*((ULONG *)(dest+(xindex+1)*4+xmin*4*yindex));
			B=B&0x0ff;
			G=G>>8&0x0ff;
			R=R>>16&0x0ff;
			A=A>>24&0x0ff;
			B1=B1&0x0ff;
			G1=G1>>8&0x0ff;
			R1=R1>>16&0x0ff;
			A1=A1>>24&0x0ff;

			UYVY=(((28770*B-19071*G-9699*R)>>16)+128)+\
			 (((16843*R+33030*G+6423*B)>>8&65280)+4096)+\
			 (((28770*R1-24117*G1-4653*B1)&16711680)+8388608)+\
			 (((16843*R1+33030*G1+6423*B1)<<8&4278190080)+268435456);
			*YUVdest++=UYVY;
			*alphaindex++=(UBYTE)A;
			*alphaindex++=(UBYTE)A1;
			} //end YUV conversion scope
		}
	for (yindex=1;yindex<ymin;yindex+=2) {
		for (xindex=0;xindex<xmin;xindex+=2) {
			unsigned long R,G,B,A,R1,G1,B1,A1,UYVY;
			R=G=B=A=*((ULONG *)(dest+xindex*4+xmin*4*yindex));	
			R1=G1=B1=A1=*((ULONG *)(dest+(xindex+1)*4+xmin*4*yindex));
			B=B&0x0ff;
			G=G>>8&0x0ff;
			R=R>>16&0x0ff;
			A=A>>24&0x0ff;
			B1=B1&0x0ff;
			G1=G1>>8&0x0ff;
			R1=R1>>16&0x0ff;
			A1=A1>>24&0x0ff;

			UYVY=(((28770*B-19071*G-9699*R)>>16)+128)+\
			 (((16843*R+33030*G+6423*B)>>8&65280)+4096)+\
			 (((28770*R1-24117*G1-4653*B1)&16711680)+8388608)+\
			 (((16843*R1+33030*G1+6423*B1)<<8&4278190080)+268435456);
			*YUVdest++=UYVY;
			*alphaindex++=(UBYTE)A;
			*alphaindex++=(UBYTE)A1;
			} //end YUV conversion scope
		}
	if (dest) dispose((struct memlist *)dest,&pmem);
	return (YUVdeststart);
error:
	//We are done with source and tempbuffer so dispose them
	if (dest) dispose((struct memlist *)dest,&pmem);
	return (NULL);
	}
*/


/*
					//do all third pixel now 
				fourtimes=scalex>>2;
				do {
				__asm {
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					mov			eax,pixel
					add			eax,4
					movd			mm0,[eax]
					add			eax,4
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Gp3_Bp3_Rp2_Gp2
					psrlq			mm0,32			//mm0 = _________Gp3_Bp3
					movd			mm2,[eax]

					punpcklbw	mm2,mm1			//mm2 = _Rp4_Gp4_Bp4_Rp3

					movq			mm5,mm2			//mm5 = _Rp4_Gp4_Bp4_Rp3
					psllq			mm5,48			//mm5 = _Rp3____________
					psrlq			mm5,16			//mm5 = _____Rp3________
					por			mm0,mm5			//mm0 = _____Rp3_Gp3_Bp3
					movq			mm4,mm0			//mm4 = _____Rp3_Gp3_Bp3
					movq			mm2,mm0			//mm2 = _____Rp3_Gp3_Bp3
					movq			mm5,mm0			//mm5 = _____Rp3_Gp3_Bp3
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					}
				} while (fourtimes--);

					//do all forth pixel now 
				fourtimes=scalex>>2;
				do {
				__asm {
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					mov			eax,pixel
					add			eax,8
					movd			mm2,[eax]
					punpcklbw	mm2,mm1			//mm2 = _Rp4_Gp4_Bp4_Rp3
					psrl			mm2,16			//mm2 = _____Rp4_Gp4_Bp4
					movq			mm5,mm2			//mm5 = _____Rp4_Gp4_Bp4
					movq			mm0,mm2			//mm0 = _____Rp4_Gp4_Bp4
					movq			mm4,mm2			//mm4 = _____Rp4_Gp4_Bp4
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1

					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					emms
					mov	[videobuf],edi
					mov	[scalebufindex],esi
					}
				} while (fourtimes--);
*/
