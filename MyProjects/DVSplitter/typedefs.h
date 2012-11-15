/*
 * $Id: typedefs.h,v 1.5 2000/01/21 06:41:31 kalle Exp $
 */

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#ifdef unix

// typedef short INT16;
// typedef unsigned short UINT16;
// typedef float REAL;
// typedef unsigned char UINT8;
// typedef unsigned int UINT32;
#define __cdecl
#define __export
#define __stdcall
#define DLLEXPORT
#define DLLIMPORT
#define DLLExportC extern "C"
#define SOLIBPREFIX lib
#define SOLIBSUFFIX so
#define MC_DLLX_C
typedef int xaLONG;
typedef unsigned int xaULONG;
typedef short xaSHORT;
typedef unsigned short xaUSHORT;
typedef signed char xaBYTE;
typedef unsigned char xaUBYTE;
#define xaFALSE  0
#define xaTRUE   1
typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned char* PBYTE;
typedef unsigned long ULONG;
typedef unsigned long* PULONG;
typedef unsigned char BYTE;
typedef long LONG;
typedef int INT;
typedef unsigned long FOURCC;
typedef unsigned short USHORT;
typedef short SHORT;
typedef void* PVOID;
typedef int HMODULE;
typedef char* PCHAR;
typedef unsigned short* PUSHORT;
#ifndef WORD
typedef short WORD;
#endif
#ifndef DWORD
typedef long DWORD;
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#if( !defined __cplusplus )
#ifndef __MAFULLSCREEN_EXCLUDE_BOOL__
typedef unsigned char BOOL;
#endif
typedef unsigned char Boolean;
#else
#ifndef __MAFULLSCREEN_EXCLUDE_BOOL__
typedef bool BOOL;
#endif
typedef bool Boolean;
#endif
typedef int IModuleHandle;

#undef APIENTRY
#define APIENTRY

#else

#define xaFALSE  0
#define xaTRUE   1
typedef int xaLONG;
typedef unsigned int xaULONG;
typedef short xaSHORT;
typedef unsigned short xaUSHORT;
typedef signed char xaBYTE;
typedef unsigned char xaUBYTE;

typedef unsigned long FOURCC;
typedef unsigned char UCHAR;
typedef unsigned char* PBYTE;
typedef unsigned long ULONG;
typedef unsigned long* PULONG;
typedef unsigned char BYTE;
typedef long LONG;
typedef unsigned short USHORT;
typedef short SHORT;
typedef void* PVOID;
typedef char* PCHAR;
typedef unsigned short* PUSHORT;
#define SOLIBPREFIX
#define SOLIBSUFFIX dll
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#define DLLExportC extern "C" DLLEXPORT
typedef int IModuleHandle;

#endif


#ifdef unix
typedef int MAFILEHANDLETYPE;
#else
typedef void* MAFILEHANDLETYPE;
#endif

#define PRODUCTINFO_VERSION "3.5"

#define PRODUCTINFO_COUNTER "1"

/*

#else

#include <wtypes.h>

#define MAINACTORDIR "g:\kunden\MainActor\qt"

#endif
*/
#endif

/**
 * $Log: typedefs.h,v $
 * Revision 1.5  2000/01/21 06:41:31  kalle
 * morphing asynchronously, works on Linux again
 *
 * Revision 1.4  1999/11/23 10:52:57  kalle
 * more integration between sequencer and video editor
 *
 * Revision 1.3  1999/10/29 04:52:06  markusm
 * *** empty log message ***
 *
 * Revision 1.2  1999/10/29 04:46:30  markusm
 * * Changed the DLLExportC macro to the typedef file
 *
 * Revision 1.1  1999/09/13 08:26:29  markusm
 * * New structure of directories
 * * Added first set of source files
 *
 * Revision 1.8  1999/07/01 19:37:58  kalle
 * Compiles on Linux
 *
 * Revision 1.7  1999/06/30 06:25:21  kalle
 * - preview works on Linux
 * - fixed image loading on Linux
 * - implemented SQPosEntry and used it in SQExport
 * - moved SQOverlaySettings to SQVidCnv.{cpp,hpp}
 *
 * Revision 1.6  1999/06/25 10:45:20  kalle
 * Added tmake files, compiles on Linux
 *
 * Revision 1.5  1999/06/20 11:36:01  kalle
 * - More work on Export dialog (all widgets connected, needs testing when
 *   export modules are available)
 * - added missing button
 *
 * Revision 1.4  1999/06/15 11:36:28  kalle
 * - started work on new AVI loader for Linux
 * - implemented MCDisplayBitmap() for Qt
 * - AVI loading seems to work, preview doesn't
 *
 * Revision 1.3  1999/06/14 17:48:15  kalle
 * - Portability fixes
 * - impmactr module for Linux
 * - SQBrowser is a dialog now
 *
 * Revision 1.2  1999/06/12 08:58:43  kalle
 * The big code cleanup:
 *
 * - removed approx. 500 warnings
 * - made double->int conversion correct and explicit
 * - put in QPaintDevice* where appropriate
 * - added CVS headers and footers and copyright headers
 *
 */
