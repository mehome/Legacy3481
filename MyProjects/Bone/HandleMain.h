#ifndef HANDLEMAIN_H
#define HANDLEMAIN_H
#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

/* All objects headers */
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h> //added to compile for NT
// DirectX includes
#include <dsound.h>
#include <ddraw.h>
#include <dvp.h>
// other multimedia
#include <vfw.h>

#include <stdio.h>
#include <math.h>
#include <commctrl.h>

#include "CBasic.h"
#include "GBasic.h"
#include "console.h"
#include "test.h"
#include "resource.h"
/* end include object headers*/

/*Global Vars*/
/* End Globals */

/* Here are the function prototypes */
LRESULT CALLBACK handlemain(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK handleabout(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK gObjCallback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
void resizewindows();
void initchildren();
void print(char *string);
void printc(char *string);
/* End Global function prototypes */


#endif /* HANDLEMAIN_H */
