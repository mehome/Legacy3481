
#include "StdAfx.h"

//***************************************************************************************************************************************************
long ExternalSource::AdditionalCallback( HWND hWnd, UINT msg, UINT wParam, UINT lParam ) {
	long Ret=0;
	switch (msg) {
		case WM_GRAPHNOTIFY: {
			long evCode=0;
			//long param1, param2;
/*
			if (pMediaEvent) {
				pMediaEvent->GetEvent( &evCode, &param1, &param2, 0L );
				pMediaEvent->FreeEventParams( evCode, param1, param2 );
				}
*/
			if ((evCode==EC_DEVICE_LOST)) {
				//OutputDebugString("ABANDON SHIP...ABANDON SHIP\n");
				if (LEDTimer->GetRunning()) {
					//OutputDebugString("*+*+*+*+*+*+*+*+*+* AC Thread ABOUT TO STOP *+*+*+*+*+*+*+*+*+*+*+*+*+\n");
					LEDTimer->SetThreadMustExit( true );
					//LEDTimer->StopThread();
					//OutputDebugString("*+*+*+*+*+*+*+*+*+* AC Thread STOPPED *+*+*+*+*+*+*+*+*+*+*+*+*+\n");
					}	
				SelectDeckControlSource();
				//m_DeckControlSource.Set (0);

				BuildAvailableSourcesList();
//TODO: may need a method to notify a source list update here...
/*
				if(ComboPtr) {
					ComboPtr->DeleteAllItems();
					for(unsigned i=0;i<m_AvailableSourceNames.NoItems;i++)
						ComboPtr->AddItem(m_AvailableSourceNames[i],i);
						}
*/
					}
				//else {
					//char tmps[128]="";
					//sprintf( tmps, "Recieving an odd pattern, Enterprise... %d\n", evCode );
					//OutputDebugString( tmps );
					//}
			Ret=S_OK;
			break;
			}
		default: {
			printf ("Recieving an odd pattern, Enterprise...\n");
			}
		}
	return Ret;
	}

unsigned ExternalSource::GetSelectedDevice() { 
	return m_DeckControlSource;
	}

//***************************************************************************************************************************************************
void ExternalSource::UpdateTimeCode(void)
{
//	if( true )
//		return;
	// Initialise the timecode sample
	TIMECODE_SAMPLE TS;
//	long lMode;
	char TimeCode[24]="";
	memset( TimeCode, 0, 24 );
	TS.timecode.dwFrames=0;
	TS.dwFlags=ED_DEVCAP_TIMECODE_READ;
	// Get the timecode
	if( m_TimeCode!=NULL )
	{
//		if( pMediaEvent )
		{
//			long EventCode, lParam1, lParam2;
//			pMediaEvent->GetEvent(&EventCode, &lParam1, &lParam2, 70L);
//			if( EC_TIMECODE_AVAILABLE==EventCode )
			{
//				m_TimeCode->GetTCRMode( ED_TCR_SOURCE, &lMode );
				m_TimeCode->SetTCRMode( ED_TCR_SOURCE, ED_TCR_LAST_VALUE );
				m_TimeCode->GetTimecode( &TS );
//				m_TimeCode->SetTCRMode( ED_TCR_SOURCE, lMode );
				
				m_LEDs[0]=( (TS.timecode.dwFrames & 0xF0000000)>>28 );
				m_LEDs[1]=( (TS.timecode.dwFrames & 0x0F000000)>>24 );
				m_LEDs[2]=( (TS.timecode.dwFrames & 0x00F00000)>>20 );
				m_LEDs[3]=( (TS.timecode.dwFrames & 0x000F0000)>>16 );
				m_LEDs[4]=( (TS.timecode.dwFrames & 0x0000F000)>>12 );
				m_LEDs[5]=( (TS.timecode.dwFrames & 0x00000F00)>>8 );
				m_LEDs[6]=( (TS.timecode.dwFrames & 0x000000F0)>>4 );
				m_LEDs[7]=(  TS.timecode.dwFrames & 0x0000000F );
			}
		}
//		else
		{
//			m_TimeCode->GetTCRMode(ED_TCR_SOURCE, &lMode);
			m_TimeCode->SetTCRMode(ED_TCR_SOURCE, ED_TCR_LTC);
			//
			// Read the timecode off the deck
//			TS.timecode.dwFrames = 32768;
			if (SUCCEEDED(m_TimeCode->GetTimecode(&TS)))
			{
				if (TS.timecode.wFrameRate==AM_TIMECODE_30DROP)
							ConvertTimecodeToString(TimeCode,30000.0/1001.0,TS.timecode.dwFrames);
				else if (TS.timecode.wFrameRate==AM_TIMECODE_30NONDROP)
							ConvertTimecodeToString(TimeCode,30000.0/1000.0,TS.timecode.dwFrames);
				else if (TS.timecode.wFrameRate==AM_TIMECODE_25)
							ConvertTimecodeToString(TimeCode,25000.0/1000.0,TS.timecode.dwFrames);
				else if (TS.timecode.wFrameRate==AM_TIMECODE_24)
							ConvertTimecodeToString(TimeCode,24000.0/1000.0,TS.timecode.dwFrames);
				else		ConvertTimecodeToString(TimeCode,30000.0/1000.0,TS.timecode.dwFrames);
				m_LEDs[0]=(TimeCode[0] -'0');
				m_LEDs[1]=(TimeCode[1] -'0');
				m_LEDs[2]=(TimeCode[3] -'0');
				m_LEDs[3]=(TimeCode[4] -'0');
				m_LEDs[4]=(TimeCode[6] -'0');
				m_LEDs[5]=(TimeCode[7] -'0');
				m_LEDs[6]=(TimeCode[9] -'0');
				m_LEDs[7]=(TimeCode[10]-'0');
//char tmp[256]="";
//sprintf( tmp, " UpdateTimeCode- %d %s\n", TS.timecode.dwFrames, TimeCode );
//OutputDebugString( tmp );
			}
//			else
//OutputDebugString( "GetTimeCode failed\n\n");
//			m_TimeCode->SetTCRMode(ED_TCR_SOURCE, lMode);
		}
	}
	else
	{	// Set all the LEDs into '0' state
		for(unsigned i=0;i<8;i++)
			m_LEDs[i]=(0);
	}
}
