#include <dinput.h>
#include <iostream.h>
#include <winbase.h>
#include <windows.h>

#include "joystick.h"



/******************************************************************************
 *                          M A C R O S
 *****************************************************************************/
#define KEYDOWN(name, key) (name[key] & 0x80)


/******************************************************************************
 *                       C O N S T A N T S
 *****************************************************************************/
#define MAX_JOYSTICKS_NUM 8
#define MAX_KEYBOARD_NUM 2
#define MAX_MOUSE_NUM 2
#define kBufferSize 20
#define kNumLVScanCodes 105

unsigned char scanCodeMap[kNumLVScanCodes]= {
 DIK_0,
 DIK_1,
 DIK_2,
 DIK_3,
 DIK_4,
 DIK_5,
 DIK_6,
 DIK_7,
 DIK_8,
 DIK_9,
 DIK_A,
 DIK_B,
 DIK_C,
 DIK_D,
 DIK_E,
 DIK_F,
 DIK_G,
 DIK_H,
 DIK_I,
 DIK_J,
 DIK_K,
 DIK_L,
 DIK_M,
 DIK_N,
 DIK_O,
 DIK_P,
 DIK_Q,
 DIK_R,
 DIK_S,
 DIK_T,
 DIK_U,
 DIK_V,
 DIK_W,
 DIK_X,
 DIK_Y,
 DIK_Z,
 DIK_F1,
 DIK_F2,
 DIK_F3,
 DIK_F4,
 DIK_F5,
 DIK_F6,
 DIK_F7,
 DIK_F8,
 DIK_F9,
 DIK_F10,
 DIK_F11,
 DIK_F12,
 DIK_F13,
 DIK_F14,
 DIK_F15,
 DIK_ADD,
 DIK_BACK,
 DIK_BACKSLASH,
 DIK_CAPITAL,
 DIK_COMMA,
 DIK_DECIMAL,
 DIK_DELETE,
 DIK_DIVIDE,
 DIK_DOWN,
 DIK_END,
 DIK_EQUALS,
 DIK_ESCAPE,
 DIK_GRAVE,
 DIK_HOME,
 DIK_INSERT,
 DIK_LBRACKET,
 DIK_LCONTROL,
 DIK_LEFT,
 DIK_LMENU,
 DIK_LSHIFT,
 DIK_LWIN,
 DIK_MINUS,
 DIK_MULTIPLY,
 DIK_NEXT,
 DIK_NUMLOCK,
 DIK_NUMPAD0,
 DIK_NUMPAD1,
 DIK_NUMPAD2,
 DIK_NUMPAD3,
 DIK_NUMPAD4,
 DIK_NUMPAD5,
 DIK_NUMPAD6,
 DIK_NUMPAD7,
 DIK_NUMPAD8,
 DIK_NUMPAD9,
 DIK_PAUSE,
 DIK_PERIOD,
 DIK_PRIOR,
 DIK_RBRACKET,
 DIK_RCONTROL,
 DIK_RETURN,
 DIK_RIGHT,
 DIK_RMENU,
 DIK_RSHIFT,
 DIK_RWIN,
 DIK_SCROLL,
 DIK_SEMICOLON,
 DIK_SLASH,
 DIK_SPACE,
 DIK_SUBTRACT,
 DIK_TAB,
 DIK_UP,
 DIK_NUMPADENTER,
 DIK_APOSTROPHE
};


enum {		                /* cleanup modes (when to call cleanup proc)     */	
	kCleanRemove,           /* Remove the callback.  Must provide identical  */ 
							/* callback and arg as installation call         */	
	kCleanExit,				/* only when LabVIEW exits                       */	
	kCleanOnIdle,			/* whenever active vi goes idle                  */	
	kCleanAfterReset,		/* whenever active vi goes idle after a reset    */	
	kCleanOnIdleIfNotTop,	/* whenever active vi goes idle if active vi is not current vi */	
	kCleanAfterResetIfNotTop/* whenever active vi goes idle after a reset if active vi is not current vi */	
	};

typedef int (*CleanupProcPtr)(void*);


/******************************************************************************
 *                      S T R U C T U R E
 *****************************************************************************/

typedef struct {
	DWORD axisInfo;
	DWORD buttonInfo;
	DWORD povInfo;
	char deviceName [80];
	GUID guidInstance;
	} deviceInfo;

/*******************************************************************************
 *                G L O B A L       V A R I A B L E S
 ******************************************************************************/
// the number of devices in use (only used to know when we can destroy gDInputObject
int gJoyCount;
int gKeyCount;
int gMouseCount;

// the total number of devices found on the system
int gJoyTotal;
int gJoyTotalTmp;
int gKeyTotal;
int gMouseTotal;

/* contains infomation about the devices */
deviceInfo gJoyInfoArray[MAX_JOYSTICKS_NUM];
deviceInfo gJoyInfoArrayTmp[MAX_JOYSTICKS_NUM];
deviceInfo gKeyInfoArray[MAX_KEYBOARD_NUM];
deviceInfo gMouseInfoArray[MAX_MOUSE_NUM];

/*******************************************************************************
 *       D I R E C T  I N P U T         O B J E C T 
 ******************************************************************************/
static LPDIRECTINPUT		gDInputObject = NULL;

static LPDIRECTINPUTDEVICE2	gDI_JoyStickDevice[MAX_JOYSTICKS_NUM];
static LPDIRECTINPUTDEVICE	gDI_JoyStickDeviceObject[MAX_JOYSTICKS_NUM];
static LPDIRECTINPUTDEVICE	gDI_KeyBoardDevice[MAX_KEYBOARD_NUM];
static LPDIRECTINPUTDEVICE  gDI_SysMouseDevice[MAX_MOUSE_NUM];


/*******************************************************************************
 *       F U N C T I O N     P R O T O T Y P E
 ******************************************************************************/


//Three call back function, used to enumerate devices
static BOOL CALLBACK EnumJoyStickDevice(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef);
static BOOL CALLBACK EnumJoyStickDeviceTmp(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef);
static BOOL CALLBACK EnumKeyBoardDevice(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef);
static BOOL CALLBACK EnumSysMouseDevice(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef);
static BOOL CALLBACK EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

extern "C"{

	_declspec (dllexport)  HRESULT inputSystemSetup();

	//Three functions that retrive the device information
	_declspec (dllexport) int getJoyInfo (int,DWORD*, DWORD*, DWORD*, char []); 
	_declspec (dllexport) int getJoyInfoRedo (int,DWORD*, DWORD*, DWORD*, char []); 
	_declspec (dllexport) int getJoyInfoTmp (int,DWORD*, DWORD*, DWORD*, char []); 
	_declspec (dllexport) int getKeyInfo (int,DWORD*, char []);
	_declspec (dllexport) int getMouseInfo (int,DWORD*,DWORD*, char []);

	//Three functions that retrive the capability of the devices.
	HRESULT getJoyCapability (LPDIRECTINPUTDEVICE2, DWORD* ,DWORD* , DWORD* );
	HRESULT getKeyCapability (LPDIRECTINPUTDEVICE, DWORD*);				  
	HRESULT getMouseCapability (LPDIRECTINPUTDEVICE, DWORD* , DWORD* );

	//Three functions that retrive the current state of the device
	_declspec (dllexport) HRESULT getKeyboardState (int, BYTE*);
	_declspec (dllexport) HRESULT getJoystickState(int, long* , BYTE* , long* , long* );

	_declspec (dllexport) HRESULT getMouseState (int, int*, BYTE*);
	_declspec (dllexport) HRESULT getCursorPosition (int* xPoint, int* yPoint);
	_declspec (dllexport) HRESULT getMouseStateBothMode (char, int, int* , BYTE*);

	//close Object closes individual device, 
	//closeAllDevice calls closeObject and close everything
	//closeDirectInput only closes the DirectInput object
	_declspec (dllexport) HRESULT openDevice (int, int);
	_declspec (dllexport) HRESULT closeObject (int, int);
	_declspec (dllexport) HRESULT  closeAllDevice ();
	HRESULT closeDirectInput ();

	// interpret Error
	HRESULT interpretError (HRESULT);

	// call back function
	int CleanupProc (void*);

	int RTSetCleanupProc(CleanupProcPtr, void* , int);
}


	/******************************************************************************
	 *                                                                            *
	 *                           F U N C T I O N S                                *
	 *																			  *
	 ******************************************************************************/


/******************************************************************************
 * inputSystemSetup()
 *
 * usage: Initialize the DirectInput Object, get info about available devices.   
 *
 *****************************************************************************/

_declspec (dllexport)  HRESULT inputSystemSetup() 
	{
	HRESULT err;
	HINSTANCE gameInstance;

	// if we have already created the directinput object, just exit out
	if(gDInputObject)
		return EXIT_SUCCESS;

    gameInstance = GetModuleHandle(NULL);

    // Create the instance of directInput
	if(FAILED(err = DirectInputCreate(gameInstance, DIRECTINPUT_VERSION, &gDInputObject, NULL)))
		return err;

	// Initialize the deviceTotal (total number of devices attached to machine)                
	gJoyTotal = 0;	gKeyTotal = 0;	gMouseTotal = 0;

	//Initialize deviceCount, (which is the number of devices user choose to use)
    gJoyCount = 0;	gKeyCount = 0;	gMouseCount = 0;

	// enumerate all the available joystick-like devices          
	if(FAILED(err = gDInputObject->EnumDevices(DIDEVTYPE_HID, EnumJoyStickDevice, NULL, DIEDFL_ATTACHEDONLY)))
		return err;

	// enumerate all the available keyboard-like devices          
	if(FAILED(err = gDInputObject->EnumDevices(DIDEVTYPE_KEYBOARD, EnumKeyBoardDevice, NULL, DIEDFL_ATTACHEDONLY)))
		return err;

	// enumerate all the available mouse-like devices          
	if(FAILED(err = gDInputObject->EnumDevices(DIDEVTYPE_MOUSE, EnumSysMouseDevice, NULL, DIEDFL_ATTACHEDONLY)))
		return err;

	// set call back function, executed when labVIEW exits abnormally or prematurely.
	if(FAILED(err = RTSetCleanupProc(CleanupProc, NULL, kCleanOnIdle)))
		return err;

	return EXIT_SUCCESS;
	}



/************************************************************************************
 * HRESULT getJoyCapability(LPDIRECTINPUTDEVICE2, DWORD*, DWORD*, DWORD*)
 *
 * usage: takes in an index number for the joystick, and retrieve basic 
 *        information about it.
 *
 * param: int     device_index, the index number for the joystick, 
 *                0 as the first device
 *	      DWORD*  axis_count, the number of axis available to this joystick
 *		  DWORD*  button_count, the number of buttons available to this joystick
 *		  DWORD*  POV_count, the number of Point-Of-View controller available to 
 *		          this joystick
 *
 * return:     HRESULT, err code
 * 
 ************************************************************************************/

HRESULT getJoyCapability (LPDIRECTINPUTDEVICE2 lpDI2Device, DWORD* axis_count, 
						  DWORD* button_count, DWORD* POV_count)
	{
	HRESULT err;
	DIDEVCAPS diDevCaps;

	// return failure if lpDI2Device is NULL
    if(lpDI2Device == NULL){
		return -1;
		}

    //obtaining capability information
	diDevCaps.dwSize = sizeof(DIDEVCAPS);
    if(FAILED(err = lpDI2Device->GetCapabilities(&diDevCaps)))
		return interpretError (err);

	// assining return values
	*axis_count = diDevCaps.dwAxes;
	*button_count = diDevCaps.dwButtons;
	*POV_count = diDevCaps.dwPOVs;

	return EXIT_SUCCESS;
	}


/******************************************************************************
 * EnumJoyStickDevice (LPCDIDEVICEINSTANCE, LPVOID)                           
 *                                                                            
 * usage:   A call back function used to enumerate all the available 
 *          USB device.  
 * return:  DIENUM_STOP when the number of USB device exceeds the maximal
 *          number defined.
 *          DIENUM_CONTINUE otherwise
 ******************************************************************************/


static BOOL CALLBACK EnumJoyStickDevice(LPCDIDEVICEINSTANCE pdInst, LPVOID pvRef)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;
	LPDIRECTINPUTDEVICE2 lpDI2Device = NULL;

	/* store joystick name */
	memcpy(gJoyInfoArray[gJoyTotal].deviceName, pdInst->tszProductName, 80);	// this could be cleaned up a bit

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(pdInst->guidInstance, &lpDIDevice, NULL )))
		return DIENUM_CONTINUE;

	/* Query Direct Input Device  */
	if(FAILED(err = lpDIDevice->QueryInterface(IID_IDirectInputDevice2, (LPVOID*)&lpDI2Device)))
		{
		lpDIDevice->Release();
		return DIENUM_CONTINUE;
		}

	/* get the capability of joystick */
	err = getJoyCapability ( lpDI2Device,
		&(gJoyInfoArray[gJoyTotal].axisInfo),
		&(gJoyInfoArray[gJoyTotal].buttonInfo),
		&(gJoyInfoArray[gJoyTotal].povInfo)
		);

	// save the guid
	gJoyInfoArray[gJoyTotal].guidInstance = pdInst->guidInstance;

	/* increment the number of Device */
	gJoyTotal++;

	// release the temporary device
	lpDI2Device->Release();
	lpDIDevice->Release();

	/* check if we have exceeded the maximum number of device */
	if(gJoyTotal == MAX_JOYSTICKS_NUM)
		return DIENUM_STOP;

	return DIENUM_CONTINUE;
	}


/******************************************************************************
 * EnumJoyStickDeviceTmp (LPCDIDEVICEINSTANCE, LPVOID)                           
 *                                                                            
 * usage:   A call back function used to enumerate all the available 
 *          USB devices without affecting the ones already opened.  
 * return:  DIENUM_STOP when the number of USB device exceeds the maximal
 *          number defined.
 *          DIENUM_CONTINUE otherwise
 ******************************************************************************/


static BOOL CALLBACK EnumJoyStickDeviceTmp(LPCDIDEVICEINSTANCE pdInst, LPVOID pvRef)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;
	LPDIRECTINPUTDEVICE2 lpDI2Device = NULL;

	/* store joystick name */
	memcpy(gJoyInfoArrayTmp[gJoyTotalTmp].deviceName, pdInst->tszProductName, 80);	// this could be cleaned up a bit

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(pdInst->guidInstance, &lpDIDevice, NULL )))
		return DIENUM_CONTINUE;

	/* Query Direct Input Device  */
	if(FAILED(err = lpDIDevice->QueryInterface(IID_IDirectInputDevice2, (LPVOID*)&lpDI2Device)))
		{
		lpDIDevice->Release();
		return DIENUM_CONTINUE;
		}

	/* get the capability of joystick */
	err = getJoyCapability ( lpDI2Device,
							 &(gJoyInfoArrayTmp[gJoyTotalTmp].axisInfo),
							 &(gJoyInfoArrayTmp[gJoyTotalTmp].buttonInfo),
							 &(gJoyInfoArrayTmp[gJoyTotalTmp].povInfo)
		);

	// save the guid
	gJoyInfoArrayTmp[gJoyTotalTmp].guidInstance = pdInst->guidInstance;

	/* increment the number of Device */
	gJoyTotalTmp++;

	// release the temporary device
	lpDI2Device->Release();
	lpDIDevice->Release();

	/* check if we have exceeded the maximum number of device */
	if(gJoyTotalTmp == MAX_JOYSTICKS_NUM)
		return DIENUM_STOP;

	return DIENUM_CONTINUE;
	}


/*****************************************************************************
 * EnumJoyAxis (LPCDIDEVICEOBJECTINSTANCE, LPVOID)
 *
 * usage:  set the range for all the axes
 ****************************************************************************/

static BOOL CALLBACK EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
	{
	DIPROPRANGE diprg;
	HRESULT err;
	LPDIRECTINPUTDEVICE2 lpDI2Device = (LPDIRECTINPUTDEVICE2)pvRef;

	diprg.diph.dwSize       = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow        = DIPH_BYID;
	diprg.diph.dwObj        = lpddoi->dwType;
	diprg.lMin              = -32768;
	diprg.lMax              = +32767;

	if (FAILED (err = lpDI2Device->SetProperty(DIPROP_RANGE, &diprg.diph)))
		return interpretError (err);

	return DIENUM_CONTINUE;
	}




/************************************************************************************
 * HRESULT getKeyCapability(LPDIRECTINPUTDEVICE, DWORD*)
 *
 * usage: takes in an index number for the joystick, and retreive basic 
 *        information about it.
 *
 * param: int     device_index, the index number for the joystick, 
 *                0 as the first device
 *		  DWORD*  button_count, the number of buttons available to this joystick
 *
 * return:     HRESULT, err code
 * 
 ************************************************************************************/

HRESULT getKeyCapability (LPDIRECTINPUTDEVICE lpDIDevice, DWORD* button_count)
	{
	HRESULT err;
	DIDEVCAPS diDevCaps;

	// return failure if lpDIDevice is NULL
	if(lpDIDevice == NULL){
		return -1;
		}

    //obtaining capability information
	diDevCaps.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(err = lpDIDevice->GetCapabilities(&diDevCaps)))
        return interpretError (err);

	// assigning return values
	*button_count = diDevCaps.dwButtons;

	return EXIT_SUCCESS;
	}




/******************************************************************************
 * EnumKeyBoardDevice (LPCDIDEVICEINSTANCE, LPVOID)                           
 *                                                                            
 * usage:   A call back function used to enumerate all the available 
 *          keybaord-like device.  
 * return:  DIENUM_STOP when the number of USB device exceeds the maximal
 *          number defined.
 *          DIENUM_CONTINUE otherwise
 ******************************************************************************/

static BOOL CALLBACK EnumKeyBoardDevice(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;

	// store keyboard name
	memcpy(gKeyInfoArray[gKeyTotal].deviceName, pdInst->tszProductName, 80);	// this could be nicer

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(pdInst->guidInstance, &lpDIDevice, NULL )))
		return DIENUM_CONTINUE;

	/* get keyboard capability */
    err = getKeyCapability(lpDIDevice, &(gKeyInfoArray [gKeyTotal].buttonInfo));

	// save the guid
	gKeyInfoArray[gKeyTotal].guidInstance = pdInst->guidInstance;

	/* increment the number of Device */
	gKeyTotal++;

	// release the temporary device
	lpDIDevice->Release();

	/* check if we have exceeded the maximal number of device */
	if ( gKeyTotal == MAX_KEYBOARD_NUM){
		return DIENUM_STOP;
		}

	return DIENUM_CONTINUE;
	}


/************************************************************************************
 * HRESULT getMouseCapability(LPDIRECTINPUTDEVICE, DWORD*, DWORD*)
 *
 * usage: takes in an index number for the mouse, and retreive basic 
 *        information about it.
 *
 * param: int     device_index, the index number for the joystick, 
 *                0 as the first device
 *        DWORD*  axis_count, the number of axis available
 *		  DWORD*  button_count, the number of buttons available to this joystick
 *
 * return:     HRESULT, err code
 * 
 ************************************************************************************/

HRESULT getMouseCapability (LPDIRECTINPUTDEVICE lpDIDevice, DWORD* axis_count, DWORD* buttons_count)
	{
	HRESULT err;
	DIDEVCAPS diDevCaps;

	// return failure if lpDIDevice is NULL
    if (lpDIDevice == NULL){
		return -1;
		}

    //obtaining capability information
	diDevCaps.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(err = lpDIDevice->GetCapabilities(&diDevCaps)))
        return interpretError (err);

	// assigning return values
	*axis_count = diDevCaps.dwAxes;
	*buttons_count = diDevCaps.dwButtons;

	return EXIT_SUCCESS;
	}

/******************************************************************************
 * EnumSysMouseDevice (LPCDIDEVICEINSTANCE, LPVOID)                           
 *                                                                            
 * usage:   A call back function used to enumerate all the available 
 *          USB mouse-like device.  
 * return:  DIENUM_STOP when the number of USB device exceeds the maximal
 *          number defined.
 *          DIENUM_CONTINUE otherwise
 ******************************************************************************/

static BOOL CALLBACK EnumSysMouseDevice(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;

	/* store mouse name */
	memcpy(gMouseInfoArray[gMouseTotal].deviceName, pdInst->tszProductName, 80);	// this could be cleaned up a bit

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(pdInst->guidInstance, &lpDIDevice, NULL )))
		return DIENUM_CONTINUE;

	/* get mouse capability */
    err = getMouseCapability(lpDIDevice, &(gMouseInfoArray[gMouseTotal].axisInfo),
		&(gMouseInfoArray [gMouseTotal].buttonInfo));

	// save the guid
	gMouseInfoArray[gMouseTotal].guidInstance = pdInst->guidInstance;

	/* increment the number of Device */
	gMouseTotal++;

	// release the temporary device
	lpDIDevice->Release();

	/* check if we have exceeded the maximal number of device */
	if ( gMouseTotal == MAX_MOUSE_NUM){
		return DIENUM_STOP;
		}

	return DIENUM_CONTINUE;
	}



/*****************************************************************************
 * int getJoyInfo (int, DWORD*, DWORD*, DWORD*, char*)
 *
 * usage:  This returns the name and capability of joystick-like devices
 *
 * param:  int index, the index of the device
 *         DWORD* rtnAxis, the number of axis this device supports
 *         DWORD* rtnButtons, the number of buttons this device supports
 *         DWORD* rtnPov, the number of point-of-view hat this device supports
 *         char rtnName [], the name of the device
 *****************************************************************************/

_declspec (dllexport) int getJoyInfo(int index, DWORD* rtnAxis, DWORD* rtnButtons, DWORD* rtnPov, char rtnName[])
{
	HRESULT err;

	if(!gDInputObject && (FAILED( err = inputSystemSetup())))
			return interpretError(err);

	// enumerate all the available joystick-like devices          
	if(!gJoyTotal && FAILED(err = gDInputObject->EnumDevices(DIDEVTYPE_HID | LOBYTE(DIDEVTYPE_JOYSTICK) | HIBYTE(DIDEVTYPEJOYSTICK_UNKNOWN),
																EnumJoyStickDevice, NULL, DIEDFL_ATTACHEDONLY)))
		return err;

		if(index >= gJoyTotal)
			return -1;
		else {	// copy return values
			*rtnAxis = gJoyInfoArray[index].axisInfo;
			*rtnButtons = gJoyInfoArray[index].buttonInfo;
			*rtnPov = gJoyInfoArray[index].povInfo;
			memcpy(rtnName, gJoyInfoArray[index].deviceName, 80);	// this could be nicer
		}

	return EXIT_SUCCESS;
}


/*****************************************************************************
 * int getJoyInfoRedo (int, DWORD*, DWORD*, DWORD*, char*)
 *
 * usage:  This returns the name and capability of joystick-like devices
 *
 * param:  int index, the index of the device
 *         DWORD* rtnAxis, the number of axis this device supports
 *         DWORD* rtnButtons, the number of buttons this device supports
 *         DWORD* rtnPov, the number of point-of-view hat this device supports
 *         char rtnName [], the name of the device
 *****************************************************************************/

_declspec (dllexport) int getJoyInfoRedo(int index, DWORD* rtnAxis, DWORD* rtnButtons, DWORD* rtnPov, char rtnName[])
{
	// enumerate all the available joystick-like devices          
	if(index == 0)
		gJoyTotal= 0;
	return getJoyInfo(index, rtnAxis, rtnButtons, rtnPov, rtnName);

}


/*****************************************************************************
 * int getJoyInfoTmp (int, DWORD*, DWORD*, DWORD*, char*)
 *
 * usage:  This returns the name and capability of joystick-like devices
 *
 * param:  int index, the index of the device
 *         DWORD* rtnAxis, the number of axis this device supports
 *         DWORD* rtnButtons, the number of buttons this device supports
 *         DWORD* rtnPov, the number of point-of-view hat this device supports
 *         char rtnName [], the name of the device
 *****************************************************************************/

_declspec (dllexport) int getJoyInfoTmp(int index, DWORD* rtnAxis, DWORD* rtnButtons, DWORD* rtnPov, char rtnName[])
{
	HRESULT err;

	if(!gDInputObject && (FAILED( err = inputSystemSetup())))
		return interpretError(err);

	if(index == 0) {
		gJoyTotalTmp= 0;
		// enumerate all the available joystick-like devices          
		if(FAILED(err = gDInputObject->EnumDevices(DIDEVTYPE_HID, EnumJoyStickDeviceTmp, NULL, DIEDFL_ATTACHEDONLY)))
			return err;
	}

	if(index >= gJoyTotalTmp)
			return -1;
		else {	// copy return values
		*rtnAxis = gJoyInfoArrayTmp[index].axisInfo;
		*rtnButtons = gJoyInfoArrayTmp[index].buttonInfo;
		*rtnPov = gJoyInfoArrayTmp[index].povInfo;
		memcpy(rtnName, gJoyInfoArrayTmp[index].deviceName, 80);	// this could be nicer
	}

	return EXIT_SUCCESS;
}



/*****************************************************************************
 * int getKeyInfo (int, DWORD*,char*)
 *
 * usage:  This returns the name and capability of joystick-like devices
 *
 * param:  int index, the index of the device
 *         DWORD* rtnButtons, the number of keys this device supports
 *         char rtnName [], the name of the device
 *****************************************************************************/

_declspec (dllexport) int getKeyInfo(int index, DWORD* rtnButtons, char rtnName[])
	{
	HRESULT err;

	if (!gDInputObject && (FAILED( err = inputSystemSetup())))
		return interpretError(err);

	if (index >= gKeyTotal)
		return -1;
	else {	// copy return values
		*rtnButtons = gKeyInfoArray[index].buttonInfo;
		memcpy(rtnName, gKeyInfoArray[index].deviceName, 80);	// this could be nicer
		}

	return EXIT_SUCCESS;
	}


/*****************************************************************************
 * int getMouseInfo (int, DWORD*,DWORD*, char*)
 *
 * usage:  This returns the name and capability of mouse-like devices
 *
 * param:  int index, the index of the device
 *         DWORD* rtnAxis, the number of axis this device supports
 *         DWORD* rtnButtons, the number of buttons this device supports
 *         char rtnName [], the name of the device
 *****************************************************************************/

_declspec (dllexport) int getMouseInfo(int index, DWORD* rtnAxis, DWORD* rtnButtons, char rtnName[])
	{
	HRESULT err;

	if (!gDInputObject && (FAILED( err = inputSystemSetup())))
		return interpretError(err);

	if (index >= gMouseTotal)
		return -1;
	else {	// copy return values
		*rtnAxis = gMouseInfoArray[index].axisInfo;
		*rtnButtons = gMouseInfoArray[index].buttonInfo;
		memcpy(rtnName, gMouseInfoArray[index].deviceName, 80);	// this could be nicer
		}

	return EXIT_SUCCESS;
	}

/*******************************************************************************
 * HRESULT GetJoyStickState (int, int, long*, char*, DWORD*)
 * 
 * usage:  Polls the device, and obtain current device status 
 * 
 * parameter:
 *            int,   the index of the intended device
 *            long*  the array that contains the axis information.
 *            char*  the array that contains the button information.
 *            DWORD* the array that contains the direction information.
 *            long*  the array that contains some extra information about the joystick   
 *
 * return:     HRESULT, err code
 *
 * attention:  This is a high level function, which is going to be called
 *             from labview.   This means err needs to be analyzed by 
 *             interpretError(err), so a systematic error code can be
 *             sent to calling labview program.
 *             error code is defined in joystick.h
 ********************************************************************************/

_declspec (dllexport) HRESULT getJoystickState(int deviceIndex, long* axisInfo, BYTE* buttonInfo , long* directionInfo, long* extraInfo)
	{
	int i;
	HRESULT err = 0;
	DIJOYSTATE2 joyState;
	LPDIRECTINPUTDEVICE2 lpDI2Device;

	// error checking, deviceIndex has to be less than the total number of devices
	if (deviceIndex >= gJoyTotal)
		return ERR_INVALIDDEVICEINDEX;

	// aquiring device, using deviceIndex to determine the target device
	lpDI2Device = gDI_JoyStickDevice[deviceIndex];

	// return failure if lpDI2Device is NULL
	if (lpDI2Device == NULL)
		return ERR_INVALIDDEVICEINDEX;

	// polling... this tells directx to update it's internal data
	// since you want to retrieve immediate rather than buffered data from the device
	if (FAILED (err = lpDI2Device->Poll()))
		return interpretError (err);

	// retrieves the internal data we just polled for
	// GetDeviceState, information is stored in joyState
	if(FAILED(err = lpDI2Device->GetDeviceState(sizeof(joyState), &joyState ))) {
		if( err == DIERR_INPUTLOST ) {	// if we lost our input device, try to reaquire and get the state
			lpDI2Device->Acquire();
			if(FAILED(err = lpDI2Device->GetDeviceState(sizeof(joyState), &joyState )))
				return interpretError (err);
			}
		}

	/* convert the DIJOYSTATE2 structure to several individual */ 
	/* arrays, so it can be returned */

	//axisInfo [8]
	axisInfo[0] = joyState.lX;					// X-Axis 
	axisInfo[1] = joyState.lY;					// Y-Axis
	axisInfo[2] = joyState.lZ;					// Z-Axis  (throttle)
	axisInfo[3] = joyState.lRx;					// X-Axis rotation
	axisInfo[4] = joyState.lRy;					// Y-Axis rotation
	axisInfo[5] = joyState.lRz;					// Z-Axis rotation
	axisInfo[6] = joyState.rglSlider[0];		// two extra axis position
	axisInfo[7] = joyState.rglSlider[1];		// same as above

	//buttonInfo [32]
	for (i=0; i<32; ++i)
		buttonInfo[i] = (joyState.rgbButtons[i] >> 7) & 0x01;

	//directionInfo[4]
	for (i=0; i<4; ++i) {
		if(joyState.rgdwPOV[i] == -1)
			directionInfo[i] = -1;
		else
			directionInfo[i] = joyState.rgdwPOV[i] / 100;
		}

	//extraInfo[24]
	extraInfo[0] = joyState.lVX;				// X-Axis velocity
	extraInfo[1] = joyState.lVY;				// Y-Axis velocity
	extraInfo[2] = joyState.lVZ;				// Z-Axis velocity
	extraInfo[3] = joyState.lVRx;				// X-Axis angular velocity
	extraInfo[4] = joyState.lVRy;				// Y-Axis angular velocity
	extraInfo[5] = joyState.lVRz;				// Z-Axis angular velocity
	extraInfo[6] = joyState.rglVSlider[0];		// extra axis velocities
	extraInfo[7] = joyState.rglVSlider[1];		// same as above
	extraInfo[8] = joyState.lAX;				// X-Axis acceleration
	extraInfo[9] = joyState.lAY;				// Y-Axis acceleration
	extraInfo[10] = joyState.lAZ;				// Z-Axis acceleration
	extraInfo[11] = joyState.lARx;				// X-Axis angular acceleration
	extraInfo[12] = joyState.lARy;				// Y-Axis augular acceleration
	extraInfo[13] = joyState.lARz;				// Z-Axis angular acceleration
	extraInfo[14] = joyState.rglASlider[0];		// extra axis acceleration
	extraInfo[15] = joyState.rglASlider[1];		// same as above
	extraInfo[16] = joyState.lFX;				// X-Axis force
	extraInfo[17] = joyState.lFY;				// Y-Axis force
	extraInfo[18] = joyState.lFZ;				// Z-Axis force
	extraInfo[19] = joyState.lFRx;				// X-Axis torque
	extraInfo[20] = joyState.lFRy;				// Y-Axis torque
	extraInfo[21] = joyState.lFRz;				// Z-Axis torque
	extraInfo[22] = joyState.rglFSlider[0];		// extra axis forces
	extraInfo[23] = joyState.rglFSlider[1];		// same as above

	return EXIT_SUCCESS;
	}

/***********************************************************************************
 * HRESULT getBufferedKeyboardState (BYTE*)
 * 
 * usage:  returns a list of BYTE, if a certain key is pressed, '1' is stored 
 *         in the corresponding index.
 *
 * attention:
 *          'getDeviceState' is called to return a list of keys (124 of them).
 *           However, arranging those keys in a systematic order is necessary.
 *           The char[] returned by 'getDeviceState' is by default a 256 BYTE 
 *			 array, in which 124 BYTE are used to store the key state.  It is
 *			 very hard to develope an ENUM system in LABVIEW to index those 
 *			 keys without organizing them into a more compact array first. 
 *           That's why in this program, I mapped the size 256 array into a 
 *           more compact size 105 array.
 * 
 * param:   BYTE* array that stores the keyState (size= 105) 
 * 
 * return:  HRESULT;
 *
 **********************************************************************************/

_declspec (dllexport) HRESULT getBufferedKeyboardState (int index, BYTE * keysPressed, BYTE * keyState)
	{

	DIDEVICEOBJECTDATA buffer[kBufferSize];
	HRESULT err;
	ULONG items, i,j;

	// error checking, deviceIndex has to be less than the total number of devices
	if (index >= gKeyTotal)
		return ERR_INVALIDDEVICEINDEX;

	if (gDI_KeyBoardDevice[index]==NULL)
		return -1;

	// GetDeviceData, retrieve sequence of key transitions
	items= kBufferSize;
	if(FAILED(err = gDI_KeyBoardDevice[index]->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),buffer, &items, 0))) {
		if(err == DI_BUFFEROVERFLOW)
			return interpretError (err);
		else if( err == DIERR_INPUTLOST ) {	// if we lost our input device, try to reaquire and get the state
			gDI_KeyBoardDevice[index]->Acquire();
			if(FAILED(err = gDI_KeyBoardDevice[index]->GetDeviceState(sizeof (buffer),(LPVOID)&buffer)))
				return interpretError (err);
			}
		}
	// loop through setting up key results
	for(i= 0; i < items; i++) {
		char sc= buffer[i].dwOfs;
		for(j= 0; j < kNumLVScanCodes; j++)
			if(scanCodeMap[j]  == sc) {
				if(buffer[i].dwData & 0x80) {
					keysPressed[j]= 1;
					keyState[j]= 1;
				}
				else
					keyState[j]= 0;
				break;
			}
	}
	return err;
}

/***********************************************************************************
 * HRESULT getKeyboardState (BYTE*)
 * 
 * usage:  returns a list of BYTE, if a certain key is pressed, '1' is stored 
 *         in the corresponding index.
 *
 * attention:
 *          'getDeviceState' is called to return a list of keys (124 of them).
 *           However, arranging those keys in a systematic order is necessary.
 *           The char[] returned by 'getDeviceState' is by default a 256 BYTE 
 *			 array, in which 124 BYTE are used to store the key state.  It is
 *			 very hard to develope an ENUM system in LABVIEW to index those 
 *			 keys without organizing them into a more compact array first. 
 *           That's why in this program, I mapped the size 256 array into a 
 *           more compact size 105 array.
 * 
 * param:   BYTE* array that stores the keyState (size= 105) 
 * 
 * return:  HRESULT;
 *
 **********************************************************************************/

_declspec (dllexport) HRESULT getKeyboardState (int index, BYTE * keyInfo)
	{
	char buffer[256];
	ULONG i;
	HRESULT err;

	// error checking, deviceIndex has to be less than the total number of devices
	if (index >= gKeyTotal)
		return ERR_INVALIDDEVICEINDEX;

	if (gDI_KeyBoardDevice[index]==NULL)
		return -1;

	// GetDeviceState, information is stored in buffer
	if(FAILED(err = gDI_KeyBoardDevice[index]->GetDeviceState(sizeof (buffer),(LPVOID)&buffer))) {
		if( err == DIERR_INPUTLOST ) {	// if we lost our input device, try to reaquire and get the state
			gDI_KeyBoardDevice[index]->Acquire();
			if(FAILED(err = gDI_KeyBoardDevice[index]->GetDeviceState(sizeof (buffer),(LPVOID)&buffer)))
				return interpretError (err);
			}
		}

	for(i= 0; i < kNumLVScanCodes; i++)
		if(KEYDOWN(buffer, scanCodeMap[i]))
			keyInfo[i]= 1;

	return EXIT_SUCCESS;

	}
 
 
/**************************************************************************
 * GetMouseStateBothMode ()
 * a wrapper function that calls both GetMouseState and GetCursorPosition
 **************************************************************************/
 
_declspec (dllexport) HRESULT getMouseStateBothMode (char mode, int index, int* axisInfo, BYTE* buttonInfo)
	{
	DIMOUSESTATE mouseState;
	HRESULT err;
	int i;

	// error checking, deviceIndex has to be less than the total number of devices
	if (index >= gMouseTotal)
		return ERR_INVALIDDEVICEINDEX;

	if (gDI_SysMouseDevice[index]==NULL)
		return -1;

	// GetDeviceState, information is stored in mouseState
	if(FAILED(err = gDI_SysMouseDevice[index]->GetDeviceState(sizeof(DIMOUSESTATE),(LPVOID)&mouseState))) {
		if( err == DIERR_INPUTLOST ) {	// if we lost our input device, try to reaquire and get the state
			gDI_SysMouseDevice[index]->Acquire();
			if(FAILED(err = gDI_SysMouseDevice[index]->GetDeviceState(sizeof(DIMOUSESTATE),(LPVOID)&mouseState)))
				return interpretError (err);
			}
		}

	if(mode == 1) {
		if (FAILED (err = getCursorPosition(&axisInfo[0], &axisInfo[1])))
			return err;
		}
	else {
		axisInfo[0] = mouseState.lX;
		axisInfo[1] = mouseState.lY;
		}
	axisInfo[2] = mouseState.lZ;
	for(i=0;i<4;++i)
		buttonInfo[i] = (mouseState.rgbButtons[i] >> 7) & 0x01;

	return EXIT_SUCCESS;
	}


/**************************************************************************
 * getMouseState (int, BYTE*)	!!I don't think this is ever used any more!!
 **************************************************************************/

_declspec (dllexport) HRESULT getMouseState (int index, int* axisInfo, BYTE* buttonInfo)
	{
	HRESULT err;
	DIDEVICEOBJECTDATA od;
	DWORD dwElements = 1;   // number of items to be retrieved
	int bOneDown = 0;
	int bTwoDown = 0;
	int bThreeDown = 0;
	int bFourDown = 0;

	// this function doesn't work on Windows.  I don't know why we need to keep it. - JF
	return ERR_UNKNOWN;

	/* acquiring information about the system-mouse     */
	if (FAILED (err = gDI_SysMouseDevice[index]->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0)))
		return interpretError(err);
	
	/* if mouseButton is still down, automatically assign that to 1 */
	if (bOneDown)
		buttonInfo[0] = 1;
	if (bTwoDown)
		buttonInfo[1] = 1;
	if (bThreeDown)
		buttonInfo[2] = 1;
	if (bFourDown)
		buttonInfo[3] = 1;
	
	// Look at the element to see what occurred         
	switch (od.dwOfs) 
		{
		// Mouse horizontal motion
		case DIMOFS_X: 
			axisInfo [0] = od.dwData; 
			return EXIT_SUCCESS;
			
			// Mouse vertical motion
		case DIMOFS_Y: 
			axisInfo [1] = od.dwData; 
			return EXIT_SUCCESS; 
			
		case DIMOFS_Z:
			axisInfo [2] = od.dwData;
			return EXIT_SUCCESS;
			
			// DIMOFS_BUTTON0: Right button pressed or released 
		case DIMOFS_BUTTON0:
			if(od.dwData & 0x80 ) 
				{ // left button pressed
				bOneDown = 1;
				buttonInfo[0] = 1;
				} 
			else
				{  
				bOneDown = 0;
				buttonInfo[0] = 0;
				}
			return EXIT_SUCCESS;
			
			// DIMOFS_BUTTON1: Left button pressed or released 
		case DIMOFS_BUTTON1:
			if(od.dwData & 0x80 ) 
				{ // left button pressed
				bTwoDown = 1;
				buttonInfo[1] = 1;
				} 
			else
				{  
				bTwoDown = 0;
				buttonInfo[1] = 0;
				}
			return EXIT_SUCCESS;     
			
			// DIMOFS_BUTTON2: Left button pressed or released 
		case DIMOFS_BUTTON2:
			if(od.dwData & 0x80 ) 
				{ // left button pressed
				bThreeDown = 1;
				buttonInfo[2] = 1;
				} 
			else
				{  
				bThreeDown = 0;
				buttonInfo[2] = 0;
				}
			return EXIT_SUCCESS;     
			
			
			// DIMOFS_BUTTON3: Left button pressed or released 
		case DIMOFS_BUTTON3:
			if(od.dwData & 0x80 ) 
				{ // left button pressed
				bFourDown = 1;
				buttonInfo[3] = 1;
				} 
			else
				{  
				bFourDown = 0;
				buttonInfo[3] = 0;
				}
			return EXIT_SUCCESS;     
			
			
		}
	
	
	axisInfo[0] = 0;
	axisInfo[1] = 0;
	axisInfo[2] = 0;
	return EXIT_SUCCESS;
}



/*****************************************************
 * getCursorPosition
 *
 * int, xPoint,
 * int, yPoint,
 ****************************************************/

_declspec (dllexport) HRESULT getCursorPosition (int* xPoint, int* yPoint)
	{
	POINT aPoint;

	//get the cursor positin
	if (GetCursorPos(&aPoint)==0)
		return ERR_CURSORFAILURE;

	*xPoint = aPoint.x;
	*yPoint = aPoint.y;
	return EXIT_SUCCESS;
	}


/***********************************************************************************
 *HRESULT CreateJoystick (int deviceIndex)
 *
 *usage:  create joystick
 * 
 *param:  int, deviceIndex
 *
 *HRESULT: error
 **********************************************************************************/

HRESULT CreateJoystick(int deviceIndex)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;
	LPDIRECTINPUTDEVICE2 lpDI2Device = NULL;

	// if it is already created, just bail out
	if(gDI_JoyStickDevice[deviceIndex])
		return EXIT_SUCCESS;

	// error checking, deviceIndex has to be less than the total number of devices
	if(deviceIndex >= gJoyTotal)
		return ERR_INVALIDDEVICEINDEX;

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(gJoyInfoArray[deviceIndex].guidInstance, &lpDIDevice, NULL )))
		return err;

	/* Query Direct Input Device */
	if(FAILED(err = lpDIDevice->QueryInterface(IID_IDirectInputDevice2, (LPVOID*)&lpDI2Device)))
		{
		lpDIDevice->Release();
		return err;
		}

	/* set data format */
	if (FAILED (err = lpDI2Device->SetDataFormat(&c_dfDIJoystick2)))
		{
		lpDI2Device->Release();
		lpDI2Device = NULL;
		return err;
		}

	/* set the range for the axes */
	if (FAILED (err = lpDI2Device->EnumObjects(EnumJoyAxis, lpDI2Device, DIDFT_AXIS)))
		{
		lpDI2Device->Release();
		lpDIDevice->Release();
		return err;
		}

	/* Aquire the DirectInput Device */
	if (FAILED (err = lpDI2Device->Acquire())) {
		lpDI2Device->Release();
		lpDIDevice->Release();
		return err;
		}

	gDI_JoyStickDevice[deviceIndex] = lpDI2Device;
	gDI_JoyStickDeviceObject[deviceIndex] = lpDIDevice;
	++gJoyCount;

	return EXIT_SUCCESS;
	}


/***********************************************************************************
 *HRESULT CreateKeyboard (int deviceIndex)
 *
 *usage:  create keyboard
 * 
 *param:  int, deviceIndex
 *
 *HRESULT: error
 **********************************************************************************/

HRESULT CreateKeyboard(int deviceIndex)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;
	DIPROPDWORD  dipdw;

	// if it is already created, just bail out
	if(gDI_KeyBoardDevice[deviceIndex])
		return EXIT_SUCCESS;

	// error checking, deviceIndex has to be less than the total number of devices
	if(deviceIndex >= gKeyTotal)
		return ERR_INVALIDDEVICEINDEX;

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(gKeyInfoArray[deviceIndex].guidInstance, &lpDIDevice, NULL )))
		return err;

	/* set dataformat for the keyboard device */
	if (FAILED (err = lpDIDevice->SetDataFormat(&c_dfDIKeyboard))) {
		lpDIDevice->Release();
		lpDIDevice = NULL;
		return err;
		}

		dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		dipdw.diph.dwObj = 0; 
		dipdw.diph.dwHow = DIPH_DEVICE; 
		dipdw.dwData = kBufferSize;
		err= lpDIDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		if(FAILED(err))
			return err;


	/* Aquire the DirectInput Device */
	if (FAILED (err = lpDIDevice->Acquire())) {
		lpDIDevice->Release();
		lpDIDevice = NULL;
		return err;
		}

	gDI_KeyBoardDevice[deviceIndex] = lpDIDevice;
	++gKeyCount;

	return EXIT_SUCCESS;
	}


/***********************************************************************************
 *HRESULT CreateMouse (int deviceIndex)
 *
 *usage:  create keyboard
 * 
 *param:  int, deviceIndex
 *
 *HRESULT: error
 **********************************************************************************/

HRESULT CreateMouse(int deviceIndex)
	{
	HRESULT err;
	LPDIRECTINPUTDEVICE lpDIDevice = NULL;

	// if it is already created, just bail out
	if(gDI_SysMouseDevice[deviceIndex])
		return EXIT_SUCCESS;

	// error checking, deviceIndex has to be less than the total number of devices
	if(deviceIndex >= gMouseTotal)
		return ERR_INVALIDDEVICEINDEX;

	/* Create Direct Input Device */
	if(FAILED(err = gDInputObject->CreateDevice(gMouseInfoArray[deviceIndex].guidInstance, &lpDIDevice, NULL )))
		return err;

	/* set dataformat for the mouse device */
	if (FAILED (err = lpDIDevice->SetDataFormat(&c_dfDIMouse))) {
		lpDIDevice->Release();
		lpDIDevice = NULL;
		return err;
		}

	/* Aquire the DirectInput Device */
	if (FAILED (err = lpDIDevice->Acquire())) {
		lpDIDevice->Release();
		lpDIDevice = NULL;
		return err;
		}

	gDI_SysMouseDevice[deviceIndex] = lpDIDevice;
	++gMouseCount;

	return EXIT_SUCCESS;
	}


/***********************************************************************************
 *HRESULT openDevice (int deviceType, int deviceIndex)
 *
 *usage:  initialize device, aquire for directInput use
 * 
 *param:  int, deviceIndex
 *
 *HRESULT: error
 **********************************************************************************/

_declspec (dllexport) HRESULT openDevice (int deviceType, int deviceIndex)
	{
	HRESULT err;

	// if gDinputObject hasn't been initialized yet, do a system setup
	if(!gDInputObject && (FAILED(err = inputSystemSetup())))
		return interpretError (err);

	// create the correct device
	if(deviceType == JOYSTICK) {
		if(FAILED(err=CreateJoystick(deviceIndex)))
			return interpretError(err);
		}
	else if(deviceType == KEYBOARD) {
		if(FAILED(err=CreateKeyboard(deviceIndex)))
			return interpretError(err);
		}
	else if(deviceType == MOUSE) {
		if(FAILED(err=CreateMouse(deviceIndex)))
			return interpretError(err);
		}

	return EXIT_SUCCESS;
	}



/*****************************************************************************
 * HRESULT closeAllDevice ()
 * usage:  This function closes all the devices and frees memory created for 
 *		   directInput.
 * return: HRESULT
 ****************************************************************************/

_declspec (dllexport) HRESULT closeAllDevice ()
	{
	int i;

	for (i = 0; i < gJoyTotal; i++)
		closeObject(JOYSTICK, i);

	for (i = 0; i < gKeyTotal; i++)
		closeObject(KEYBOARD, i);

	for (i = 0; i < gMouseTotal; i++)
		closeObject(MOUSE, i);

	if((gJoyCount != 0) || (gKeyCount != 0) || (gMouseCount != 0))
		return ERR_UNKNOWN;

	return EXIT_SUCCESS;
	}


/******************************************************************************
 * closeObject (int, int)
 *
 * usage:  UNAQURE and close one particular device, 
 *         If every device is unacquired, it closes all the device and free memory
 *
 * param:  int,   deviceType  (JOYSTICK | MOUSE | KEYBOARD)
 *		   int,   deviceIndex
 *
 * return: HRESULT
 *		
 *****************************************************************************/

_declspec (dllexport) HRESULT closeObject(int deviceType, int deviceIndex)
	{
	HRESULT err;				 
	LPDIRECTINPUTDEVICE deviceToClose=NULL;

	// find the right device to close, make appropriate update
	if(deviceType == JOYSTICK) {
		if(deviceIndex < gJoyTotal) {
			if(gDI_JoyStickDevice[deviceIndex]) {
				gDI_JoyStickDevice[deviceIndex]->Unacquire();
				gDI_JoyStickDevice[deviceIndex]->Release();
				gDI_JoyStickDevice[deviceIndex] = NULL;
				}
			if(gDI_JoyStickDeviceObject[deviceIndex]) {
				gDI_JoyStickDeviceObject[deviceIndex]->Release();
				gDI_JoyStickDeviceObject[deviceIndex] = NULL;
				}
			gJoyCount--;
			}
		}
	else if((deviceIndex < gKeyTotal) && deviceType == KEYBOARD) {
		if(gDI_KeyBoardDevice[deviceIndex]) {
			gDI_KeyBoardDevice[deviceIndex]->Unacquire();
			gDI_KeyBoardDevice[deviceIndex]->Release();
			gDI_KeyBoardDevice[deviceIndex] = NULL;
			gKeyCount--;
			}
		}
	else if((deviceIndex < gMouseTotal) && deviceType == MOUSE) {
		if(gDI_SysMouseDevice[deviceIndex]) {
			gDI_SysMouseDevice[deviceIndex]->Unacquire();
			gDI_SysMouseDevice[deviceIndex]->Release();
			gDI_SysMouseDevice[deviceIndex] = NULL;
			gMouseCount--;
			}
		}

	//if no more devices are in use, destroy the directInputObject itself
	if((gJoyCount == 0) && (gKeyCount == 0) && (gMouseCount == 0)) {
		if (FAILED (err = closeDirectInput()))
			return err;
		}
	return EXIT_SUCCESS;
	}


/***********************************************************************************
 *HRESULT closeDirectInput ()
 *
 *usage:  close the directInput object.
 * 
 *param:  int, deviceIndex
 *
 *HRESULT: error
 **********************************************************************************/

HRESULT closeDirectInput()
	{
	HRESULT err;

	// release directInputObject
	if (gDInputObject){
		gDInputObject->Release();
		gDInputObject = NULL;
		}

	// remove the call back function since the program did not exit abnormally.
	if (FAILED (err = RTSetCleanupProc(CleanupProc, NULL, kCleanRemove)))
		return err;
	
	return EXIT_SUCCESS;
	}


/***********************************************************************************
 * a call back function used with labview.
 * It is called when Labview exited prematurely
 **********************************************************************************/
int CleanupProc (void* arg)
	{
	HRESULT err;
	
	if (FAILED (err = closeAllDevice()))
		return err;
	return EXIT_SUCCESS;
	}


/************************************************************************************
 * HRESULT interpretError (HRESULT)
 * This function converts error code from directInput to error codes defined in this 
 * program
 *
 * param:    HRESULT, directInput err code
 *
 * return:   HRESULT, modified err code
 ************************************************************************************/


HRESULT interpretError (HRESULT err)
	{
	switch (err){

	case DIERR_ACQUIRED:
		return ERR_ACQUIRED;

	case DIERR_BETADIRECTINPUTVERSION: 
		return ERR_BETADIRECTINPUTVERSION;

	case DIERR_INVALIDPARAM:
		return ERR_INVALIDPARAM;

	case DIERR_OLDDIRECTINPUTVERSION :
		return ERR_OLDDIRECTINPUTVERSION;

	case DIERR_OUTOFMEMORY :
		return ERR_OUTOFMEMORY;

	case DIERR_NOTINITIALIZED  :
        return ERR_NOTINITIALIZED;

	case DIERR_INPUTLOST:
		return ERR_INPUTLOST;

	case DIERR_NOTACQUIRED :
		return ERR_NOTACQUIRED;

	case DIERR_NOTBUFFERED:
		return ERR_NOTBUFFERED;

	case DIERR_OBJECTNOTFOUND:
		return ERR_OBJECTNOTFOUND;

	case E_PENDING:
		return ERR_PENDING;

	default:
		return err;
	}
}
