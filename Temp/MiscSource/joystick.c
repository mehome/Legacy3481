/*
 * UAE - The Un*x Amiga Emulator
 *
 * Win32 joystick interfaces
 *
 * Copyright 1997 Mathias Ortmann, Copyright 2000 Brian King
 */

#ifndef _MSC_VER
#define INITGUID
#endif

#define DIRECTINPUT_VERSION         0x0300

#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef _MSC_VER
#include <ddraw.h>
#include <mmsystem.h>
#include <dinput.h>
#else
#include "winstuff.h"
#endif
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>

#include "config.h"
#include "sysconfig.h"
#include "sysdeps.h"
#include "options.h"
#include "osdep/win32gui.h"
#include "include/memory.h"
#include "osdep/resource.h"
#include "osdep/win32.h"

#pragma comment(lib, "dinput")

extern HINSTANCE hInst;


LPDIRECTINPUT lpDInput = NULL;

int found = 0;

int nr_joysticks;

typedef struct JoystickInfoStruct {
    BOOL bPresent;
    BOOL bDInput;
    DWORD dwRetryCountdown;
    JOYCAPS JoyCaps;
    LPDIRECTINPUTDEVICE lpDInputDevice;
	} JoystickInfo;

JoystickInfo JoyInfo[MAXJOYSTICKS];

typedef struct JOYDATA {
    LONG  lX;                   // X-axis goes here.
    LONG  lY;                   // Y-axis goes here.
    BYTE  bButtonA;             // One button goes here.
    BYTE  bButtonB;             // Another button goes here.
    BYTE  bPadding[2];          // Must be DWORD multiple in size.
	} JOYDATA;

// Then, it can use the following data format.

DIOBJECTDATAFORMAT rgodf[]= {
  {&GUID_XAxis,FIELD_OFFSET(JOYDATA,lX),DIDFT_AXIS|DIDFT_ANYINSTANCE,0},
  {&GUID_YAxis,FIELD_OFFSET(JOYDATA,lY),DIDFT_AXIS|DIDFT_ANYINSTANCE,0},
  {&GUID_Button,FIELD_OFFSET(JOYDATA,bButtonA),DIDFT_BUTTON|DIDFT_ANYINSTANCE,0},
  {&GUID_Button,FIELD_OFFSET(JOYDATA,bButtonB),DIDFT_BUTTON|DIDFT_ANYINSTANCE,0},
	};

#define numObjects (sizeof(rgodf) / sizeof(rgodf[0]))

DIDATAFORMAT df = {
    sizeof(DIDATAFORMAT),       // this structure
    sizeof(DIOBJECTDATAFORMAT), // size of object data format
    DIDF_RELAXIS,               // relative axis coordinates
    sizeof(JOYDATA),            // device data size
    numObjects,                 // number of objects
    rgodf,                      // and here they are
	};

static JOYINFOEX ji={sizeof (ji),JOY_RETURNBUTTONS|JOY_RETURNX|JOY_RETURNY,0,0,0,0,0,0,0,0,0,0,0};


void ReadDirectInputJoystick(LPDIRECTINPUTDEVICE pDev,DIJOYSTATE *js) { 
    HRESULT hRes; 
 
    // poll the joystick to read the current state
    //hRes = IDirectInputDevice7_Poll( pDev );
 
    // get data from the joystick 
    hRes=IDirectInputDevice_GetDeviceState(pDev,sizeof(DIJOYSTATE),js);
 
    if(hRes!=DI_OK)
			{ 
        // did the read fail because we lost input for some reason? 
        // if so, then attempt to reacquire. If the second acquire 
        // fails, then the error from GetDeviceData will be 
        // DIERR_NOTACQUIRED, so we won't get stuck an infinite loop. 
        if(hRes == DIERR_INPUTLOST) 
				{
            //ReacquireInput(); 
					}
			 } 
	}


void read_joystick (int nr, short *dir, int *button) {
    int left = 0, right = 0, top = 0, bot = 0;
    int joyerr;
    DIJOYSTATE joystate;

    *dir = *button = 0;
    if(!JoyInfo[nr].bPresent) return;

    if(JoyInfo[nr].dwRetryCountdown) 
		{
		JoyInfo[nr].dwRetryCountdown--;
		return;
	   }

    if(!JoyInfo[nr].bDInput) {
	switch(joyerr = joyGetPosEx(JOYSTICKID1 + nr, &ji )) {
		case JOYERR_NOERROR:
		    JoyInfo[nr].dwRetryCountdown = 0;
		    break;
		case JOYERR_UNPLUGGED:
		  if(!JoyInfo[nr].dwRetryCountdown) {
				write_log( "Joystick %d seems to be unplugged.\n", nr );
				JoyInfo[nr].dwRetryCountdown = 200;
			    }
			 return;
		default:
		  write_log( "Error reading joystick %d: %d.\n", nr, joyerr );
		    JoyInfo[nr].dwRetryCountdown = 100;
		    return;
		}

	if (ji.dwXpos < (JoyInfo[nr].JoyCaps.wXmin + 2 *
		(JoyInfo[nr].JoyCaps.wXmax - JoyInfo[nr].JoyCaps.wXmin) / 5))
	  left = 1;
	else if (ji.dwXpos > (JoyInfo[nr].JoyCaps.wXmin + 3 *
		(JoyInfo[nr].JoyCaps.wXmax - JoyInfo[nr].JoyCaps.wXmin) / 5))
		 right = 1;

	if (ji.dwYpos < (JoyInfo[nr].JoyCaps.wYmin + 2 *
		(JoyInfo[nr].JoyCaps.wYmax - JoyInfo[nr].JoyCaps.wYmin) / 5))
	  top = 1;
	else if (ji.dwYpos > (JoyInfo[nr].JoyCaps.wYmin + 3 *
		(JoyInfo[nr].JoyCaps.wYmax - JoyInfo[nr].JoyCaps.wYmin) / 5))
	   bot = 1;
	   }
    else {
		// DirectInput joysticks
		ReadDirectInputJoystick(JoyInfo[nr].lpDInputDevice,&joystate);

		if(joystate.lX < -800) left=1;
		else if(joystate.lX > 800) right = 1;

		if (joystate.lY < -800) top = 1;
		else if(joystate.lY > 800) bot = 1;

		if(joystate.rgbButtons[0] & 0x80) ji.dwButtons |= 0x1;
		if(joystate.rgbButtons[4] & 0x80) ji.dwButtons |= 0x1;
		if(joystate.rgbButtons[5] & 0x80) ji.dwButtons |= 0x1;
		if(joystate.rgbButtons[6] & 0x80) ji.dwButtons |= 0x1;
		if(joystate.rgbButtons[1] & 0x80) ji.dwButtons |= 0x2;
	   }

    if (left) top=!top;
    if (right) bot=!bot;

    *dir = bot | (right << 1) | (top << 8) | (left << 9);
    *button = ji.dwButtons;
	}


BOOL CALLBACK DIEnumDevicesCallback( LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
    BOOL bError = FALSE;
    int i;

	switch(lpddi->dwDevType)  {
		case DIDEVTYPEJOYSTICK_RUDDER:
		case DIDEVTYPEJOYSTICK_WHEEL:
		case DIDEVTYPEJOYSTICK_HEADTRACKER:
			break;
		default:
			write_log("DirectInput: Found %s %s\n",lpddi->tszProductName,lpddi->tszInstanceName);
			for(i = 0; i < MAXJOYSTICKS; i++) {
				if(!JoyInfo[i].bPresent) {
					if(SUCCEEDED(IDirectInput_CreateDevice(lpDInput,&lpddi->guidInstance,&JoyInfo[i].lpDInputDevice,NULL))) {
					if(SUCCEEDED(IDirectInputDevice_SetDataFormat(JoyInfo[i].lpDInputDevice,&df))) {
							DIPROPRANGE diprg;

							diprg.diph.dwSize       = sizeof(diprg);
							diprg.diph.dwHeaderSize = sizeof(diprg.diph);
							diprg.diph.dwObj        = DIJOFS_X;
							diprg.diph.dwHow        = DIPH_BYOFFSET;
							diprg.lMin              = -1000;
							diprg.lMax              = +1000;

							// Set the X-axis range
							if(FAILED(IDirectInputDevice_SetProperty(JoyInfo[i].lpDInputDevice,DIPROP_RANGE,&diprg.diph))) bError = TRUE;
						//	}
					//else {
				    // Set the Y-axis range
					  diprg.diph.dwObj=DIJOFS_Y;
					  if(FAILED(IDirectInputDevice_SetProperty(JoyInfo[i].lpDInputDevice,DIPROP_RANGE,&diprg.diph))) bError=TRUE;
					   }
			    else
					if (SUCCEEDED(IDirectInputDevice_Acquire(JoyInfo[i].lpDInputDevice))) {
						JoyInfo[i].bPresent = TRUE;
						JoyInfo[i].bDInput = TRUE;
						found++;
						break; // exit the for-loop
						}
				 else {
					bError = TRUE;
					}
				}
			}
	    else {
			bError = TRUE;
			 }
	    if(bError) {
			IDirectInputDevice_Release( JoyInfo[found].lpDInputDevice );
			JoyInfo[found].lpDInputDevice = NULL;
			break; // exit the for-loop
			}
		}
   // }
	//} // for
	}
	return DIENUM_CONTINUE;
	}
// define this to go straight to DInput enumeration
//#define DEBUG_DIRECTINPUT


// Modified to handle DirectInput joysticks (such as USB devices)
void init_joystick(void) {
	int nr;

#ifndef DEBUG_DIRECTINPUT
	for (nr = 0; nr < MAXJOYSTICKS; nr++) {
		if (joyGetDevCaps(JOYSTICKID1+nr,&JoyInfo[nr].JoyCaps, sizeof(JOYCAPS)) == JOYERR_NOERROR) {
			write_log( "Joystick %d: %s with %d buttons\n", nr,JoyInfo[nr].JoyCaps.szPname, JoyInfo[nr].JoyCaps.wNumButtons );
		   JoyInfo[nr].bPresent = TRUE;
			JoyInfo[nr].bDInput = FALSE;
			found++;
			}
	   }
#endif

    // We use the joyport joysticks first, but if not all were found, we try DirectInput devices
	if (found!=MAXJOYSTICKS) {
		if(SUCCEEDED(DirectInputCreate(hInst,0x300,&lpDInput,NULL))) {
			IDirectInput_EnumDevices( lpDInput, DIDEVTYPE_JOYSTICK,
			DIEnumDevicesCallback, 0, DIEDFL_ATTACHEDONLY );
			}
		}

	nr_joysticks = found;
	write_log( "%d joystick(s) found.\n", found );
	}


void close_joystick (void) {
    int i;

    if(lpDInput) {
		for(i = 0;i < MAXJOYSTICKS;i++) {
		   if(JoyInfo[i].lpDInputDevice) {
				IDirectInputDevice_Unacquire( JoyInfo[i].lpDInputDevice );
				IDirectInputDevice_Release( JoyInfo[i].lpDInputDevice );
				JoyInfo[i].lpDInputDevice = NULL;
				}
			}
	//lpDInput->Release();
	   }
	}

