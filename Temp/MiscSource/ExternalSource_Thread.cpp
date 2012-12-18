#include "stdafx.h"

void DeckControlUITimer::setTimer( unsigned ms ) 
{ 
	timerMS = ms; 
};

void DeckControlUITimer::setInterlock( Dynamic<int> *trigger ) 
{ 
	interlock = trigger; 
};

void DeckControlUITimer::setDeckControlObject( ExternalSource *srcUI ) 
{ 
	sourceUI = srcUI; 
};

void DeckControlUITimer::ThreadProcessor() 
{
	//this->Block();
	//sourceUI->AdditionalCallback( NULL, WM_GRAPHNOTIFY, NULL, NULL );
//	if( sourceUI->GetExtSource()>0 )
//	{
//OutputDebugString("*+*+*+*+*+*+*+*+*+* Thread Run START\n");
		if( lock != false )
		{
			lock = false;
		}
		else
		{
			lock = true;
			if( sourceUI )
				sourceUI->UpdateTimeCode();
//OutputDebugString("*+*+*+*+* UpdateTimeCode *+*+*+*+*+*+*+*\n");
		}

		//if( interlock->Get() > 0 )
		//	interlock->Set( 0 );
		//else
		//	interlock->Set( 1 );
		//this->UnBlock();
//		sourceUI->DeferredMessage( 2, 0 );
		Sleep( timerMS );
//OutputDebugString("*+*+*+*+*+*+*+*+*+* Thread Run END\n");
//	}
//	else
	{
		SetThreadMustExit( true );
//		StopThread();
//OutputDebugString("*&*&*&*&*&*&*&* ThreadProcessor called but deck source is 0....Stopping Thread\n");
		Sleep( 1 );
	}
};