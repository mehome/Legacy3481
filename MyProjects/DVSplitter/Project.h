/**
 * $Id: Project.h,v 1.6 2000/03/24 07:42:05 markusm Exp $
 *
 * Copyright 1999 by MainConcept GmbH
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#ifdef __OS2__
#define FOURCC_BGRA FOURCC_BGR4
#endif
#ifdef WIN32
#include <windows.h>
#define FOURCC_LUT8 0x3854554c
#define FOURCC_R555 0x35353552
#define FOURCC_R565 0x35363552
#define FOURCC_BGR3 0x33524742
#define FOURCC_RGB3 0x33424752
#define FOURCC_BGRA 0x41524742
#define FOURCC_YUV9 0x39565559
#include "typedefs.h"
#define MCI_WAVE_FORMAT_PCM 100
struct maBITMAPINFO
{
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD          bmiColors[256];
};

struct maLOGPALETTE
{
  WORD palVersion;
  WORD palNumEntries;
  PALETTEENTRY palEntries[256];
};
#endif

#define __MAVERSION__ "3.5"

#define DUMMY                  100
#define DEFAULT_PICTURE_TCODE  1000

#ifdef unix
#define __cdecl
#include "typedefs.h"
#define FOURCC_LUT8 0x3854554c
#define FOURCC_R555 0x35353552
#define FOURCC_R565 0x35363552
#define FOURCC_BGR3 0x33524742
#define FOURCC_RGB3 0x33424752
#define FOURCC_BGRA 0x41524742
#define FOURCC_YUV9 0x39565559
#define MCI_WAVE_FORMAT_PCM 100
#endif

/* ----------------------------------- Color Enumeration */

#ifdef __OS2__
#define COLOR_RED                2
#define COLOR_GRN                1
#define COLOR_BLU                0
#define COLOR_ALP                3

struct maCOLORTABLE {
   BYTE bBlue;
   BYTE bGreen;
   BYTE bRed;
   BYTE bAlpha;
};
#endif

#ifdef WIN32
#define COLOR_RED                0
#define COLOR_GRN                1
#define COLOR_BLU                2
#define COLOR_ALP                3

struct maCOLORTABLE {
   BYTE bRed;
   BYTE bGreen;
   BYTE bBlue;
   BYTE bAlpha;
};
#endif

#ifdef unix
#define COLOR_RED                0
#define COLOR_GRN                1
#define COLOR_BLU                2
#define COLOR_ALP                3

struct maCOLORTABLE {
   BYTE bRed;
   BYTE bGreen;
   BYTE bBlue;
   BYTE bAlpha;
};
#endif


/* ----------------------------------- errorInfo */

#define ERR_LEVEL_NONE           0
#define ERR_LEVEL_WARNING        1
#define ERR_LEVEL_FATAL          2

/* ----------------------------------- exNextFrame Return Codes */

#define PR_EXNFRAME_OK             0
#define PR_EXNFRAME_ENDOFPROJECT   1
#define PR_EXNFRAME_FRAMECORRUPT   2
#define PR_EXNFRAME_GENERALERROR   3

/* ------------------------------------ ProjectData.prType */

#define PR_TYPE_ANIMATION          1
#define PR_TYPE_PICTURE            2
#define PR_TYPE_SOUND              3
#define PR_TYPE_VIDEOSTREAM        4

#define PR_TYPE_TRANSITION       100
#define PR_TYPE_VIDEOEFFECT      101
#define PR_TYPE_VIDEOPATH        102
#define PR_TYPE_AUDIOEFFECT      103

/* ------------------------------------ ProjectData.prFlags passed To module */

#define PR_FLAGS_QUICKLOADING      0x00000001

/* ------------------------------------ ProjectData.prFlags set by module */

#define PR_FLAGS_SINGLEBUFFERED    0x00000001
#define PR_FLAGS_DOUBLEBUFFERED    0x00000002

#define PR_FLAGS_LINEARENCODED     0x00000004

#define PR_FLAGS_GLOBALTIMECODE    0x00000010
#define PR_FLAGS_LOCALTIMECODES    0x00000020

#define PR_FLAGS_CACHING           0x00000100

#define PR_FLAGS_NEEDSDESTRUCTION  0x00001000

/* ------------------------------------ FrameData.frFlags */

#define FR_FLAGS_KEYFRAME          0x00000001
#define FR_FLAGS_AUDIO_SILENCE     0x00000002

/* ------------------------------------ CODEC flags */

#define MACODEC_QUALITY            0x00000001
#define MACODEC_KEYFRAMERATE       0x00000002

#define MACODEC_GETCODECNAME       0x00000004
#define MACODEC_USEBITMAPHEADER    0x00000008

/* ----------------------------------- Video Stream: decodeFrame return values */

#define FR_VIDEOSTREAM_OK             0
#define FR_VIDEOSTREAM_ENDOFSTREAM    1

/* ------------------------------------ Support Structures */

struct  codecInfo {
  char  *CodecName;
  ULONG  CodecID;
};

struct DisplayData {
  PBYTE  dData;
  ULONG  dWidth, dHeight, dModulo;

  // New Stuff used for SmartRendering

  ULONG  srType;
  ULONG  srData, srDataSize;
  ULONG  srWidth, srHeight;
};

struct ErrorInfo {
  char   errorText[200];
  int    errorData, errorLevel;
};

struct CodecOptions {
  ULONG  flags;
  LONG   quality, keyframerate;
};

struct ModuleInfo {
  struct ModuleInfo *nextInfo;
  char   moduleName[20], version[10], writtenby[40], copyrightby[40], comments[40];
  char   modulePattern[10];

  int    modulePrType;         /* PR_TYPE */
  int    moduleSupportsSound;

  ULONG  moduleHandle, moduleSubType, moduleFuture1;

  char   codec1[50], codec2[50], codec3[50], codec4[50], codec5[50];
  char   codec6[50], codec7[50], codec8[50], codec9[50], codec10[50];
  char   codec11[50], codec12[50], codec13[50], codec14[50], codec15[50];
  char   codec16[50], codec17[50], codec18[50], codec19[50], codec20[50];
};

/* ----------------------------------- New for v2.0: AddProjectData   */
/* This structure is passed to initProject() and initEncodeFrame() in */
/* the ProjectData.prPrivate[1] variable. May be ignored and is fully */
/* optional. We introduced it mainly to support real fps values which */
/* were previously hard to identify through the frLocalTimecode fields*/

struct AddProjectData
{
  double fps;                   /* if != 0: fps value of project */
  /* For Server / Client 32bit support (Alpha Channel) */
  int    serverSupports32Bit, clientSupports32Bit;
  ULONG  future[32];
};

/* ----------------------------------- Project Structures */

struct FrameData;

struct ProjectData
{
  struct FrameData *prFirstFrame;
#ifdef __cplusplus
  union {
    BYTE                    prColourTable[256 * 4];
    struct maCOLORTABLE     prColorTable[256];
  };
#else
  BYTE              prColourTable[256 * 4];
#endif
  char              prName[80];
  ULONG             prType, prFlags, prSize;
  FOURCC            prColourFormat;
  USHORT            prWidth, prHeight, prDepth;
  ULONG             prFrames, prLoopFrames;
  ULONG             prGlobalTimecode;
  ULONG             prCaching;
  ULONG             prCurrentFilePosition, prMaxDataSize;
  struct codecInfo *prCodecs;
  PBYTE             prMaxData;

  ULONG             prSamplesPerSecond, prBitsPerSample, prSampleChannels, prSampleFormat;
  ULONG             prAudioStreamSize, prSoundIsValid;

  PBYTE             (*prAllocMem) ( struct ProjectData *, ULONG );
  void              (*prFreeMem) ( struct ProjectData *, PBYTE );

  int               (*findCODEC) (ULONG, struct ProjectData *, struct CodecOptions *);
  int               (*openCODEC) ( struct ProjectData *, struct DisplayData *);
  void              (*closeCODEC) ( void );
  int               (*decodeCODEC) ( struct FrameData *, BYTE *);
  int               (*encodeCODEC) (BYTE *, ULONG, ULONG *);

  PVOID             prMemory[20];
  ULONG             prPrivate[10];
};

struct FrameData
{
  struct FrameData *frNextFrame;
  ULONG             frDataSize, frDataOffset, frSampleSize;
  ULONG             frColDataSize, frColDataOffset, frSampleOffset;
  PBYTE             frData, frColData, frSampleData;

  ULONG             frTimecode, frTimecodeOffset;

  USHORT            frWidth, frHeight, frDepth, frChangeColours;
  ULONG             frCompressMethod, frFlags;
  char*             frPictureName;
};

/* ------------------------------------------------------- General Module Stuff */

/* ----------------------------------- Language Info */

#define MALANG_ENGLISH           0
#define MALANG_GERMAN            1

/* ----------------------------------- Commands for localeHandler Callback */

#define MALOCALE_OPEN             -1
#define MALOCALE_CLOSE            -2

/* --------------------------------------------- Transition Modules */

#define TRANS_MAXSTEPS       10000
#define TRANS_BMPRES_START       1     /* Absolete */

/* ----------------------------------- Types */

#define TRN_TYPE_3D              1
#define TRN_TYPE_DISSOLVE        2
#define TRN_TYPE_IRIS            3
#define TRN_TYPE_PUSH            4
#define TRN_TYPE_SLIDE           5
#define TRN_TYPE_SPECIAL         6
#define TRN_TYPE_STRETCH         7
#define TRN_TYPE_WIPE            8
#define TRN_TYPE_ZOOM            9
#define TRN_TYPE_ADVANCED       10

/* ----------------------------------- Color Types */

#define COLOR_BACKGROUND         1
#define COLOR_SURFACE            2
#define COLOR_PEN                3

/* ----------------------------------- Transition GUI Info */

/* Use pack(2) under Windows to make the structure compatible with v3.0 */

#ifdef WIN32
#pragma pack(2)
#endif

struct TRANSGUI
{
  ULONG             trnType;          /* Type of transition: TRN_TYPE_* */

  int               supportsDivision; /* Module supports Y/X Division */
  int               xDiv, yDiv;       /* Y/X Division values: 1..8 */

  int               supportsFilter;   /* Module supports Gauss Filter */
  int               filter;           /* 0=NONE,1=SMALL,2=MEDIUM,3=LARGE*/

  int               supportsAntiAlias;/* Module supports AntiAliasing */
  int               antiAliasIsOn;    /* 0=OFF,1=ON */

  int               supportsColor;    /* Module supports additional Color */
  int               colorType;        /* COLOR_BACKGROUND,COLOR_SURFACE,COLOR_PEN */
  int               colorRed;         /* Color Red Component */
  int               colorGreen;       /* Color Green Component */
  int               colorBlue;        /* Color Blue Component */
  int               colorAlpha;       /* Color Alpha Component */
  int               penWidth;         /* If colorType=COLOR_PEN, contains penWidth: 0..10 */

  int               activeButton;     /* Active Button: 1-8 */
                                      /* Button Images: Maximal 40*40 */
                                      /* Transparent Color is RGB: 255, 0, 255 */
  int               button1;          /* ResourceId of Button1, 0 for unused */
  int               button2;          /* ResourceId of Button2, 0 for unused */
  int               button3;          /* ResourceId of Button3, 0 for unused */
  int               button4;          /* ResourceId of Button4, 0 for unused */
  int               button5;          /* ResourceId of Button5, 0 for unused */
  int               button6;          /* ResourceId of Button6, 0 for unused */
  int               button7;          /* ResourceId of Button7, 0 for unused */
  int               button8;          /* ResourceId of Button8, 0 for unused */

  char              futureExp1[40];   /* Reserved */

  /* ----------------------------------- Internal MainActor variables */

  int               vBStarts;         /* Shoud be set to 0 during resetTransitionGUI(),
                                         indicates that MainActor should swap the displays */

  void             *customData;       /* Only used together with customTransitionConfig(), i.e. */
  ULONG             customDataSize;   /* when the module is entirely not based on the standard (Morph) */

  char              futureExp2[4];    /* Reserved */
};

/* ----------------------------------- Transition Callback Structure, used by customTransitionConfig() */

struct TRANSCB
{
  struct DisplayData * (__cdecl *getVideoAt) ( LONG track, ULONG ms );

  int    tWidth, tHeight;
  double tBase;

  int    maLanguage;
  void   (__cdecl *localeHandler) ( int, char * );

  ULONG  objStartMs, objEndMs;

  int    objInterlace;

  char   maHelpFile[20];
};

/* ------------------------------------------------------- Video Effect Modules */

#define VIDEOFX_MAXSTEPS    100000

/* ----------------------------------- Types */

#define VFX_TYPE_2DWARP          1
#define VFX_TYPE_3DWARP          2
#define VFX_TYPE_COLOR           3
#define VFX_TYPE_COLORADJUST     4
#define VFX_TYPE_FADE            5
#define VFX_TYPE_PAINT           6
#define VFX_TYPE_PERSPECTIVE     7
#define VFX_TYPE_SPECIAL         8
#define VFX_TYPE_STANDARD        9
#define VFX_TYPE_WEATHER        10

/* ------------------------------------------------------- Audio Effect Modules */

/* ----------------------------------- Types */

#define AFX_TYPE_ALL             1
#define AFX_TYPE_EFFECT          1
#define AFX_TYPE_FILTER          2

/* ----------------------------------- AudioInstance Structure */

struct AudioInstance
{
  int   samplesPerSecond;         /* - Audio Frequency */
  int   bitsPerSample;            /* - The Quality of the resulting Audio Stream */
  int   numberOfChannels;         /* - Number of Channels */

  ULONG totalMs;                  /* - Total Milliseconds of Audio Stream */
  ULONG currentMs;                /* - Current Ms Position in Audio Stream */

  void *instanceData;             /* - Audio Effect Module can allocate Storage here */
  int   instanceDataSize;         /* - Size of Storage */
};

/* ------------------------------------------------------- Video Path Modules */

#define VIDPATH_MAXSTEPS    100000

/* ----------------------------------- Types */

#define VPT_TYPE_2D              1
#define VPT_TYPE_3D              2
#define VPT_TYPE_SPECIAL         3

/* ----------------------------------- Video Path Mode */

#define VPT_MODE_LINE            1
#define VPT_MODE_SPLINE          2

/* ----------------------------------- Video Image Align */

#define VPT_ALIGN_TOP_LEFT           1
#define VPT_ALIGN_TOP_CENTER         2
#define VPT_ALIGN_TOP_RIGHT          3
#define VPT_ALIGN_MIDDLE_LEFT        4
#define VPT_ALIGN_MIDDLE_CENTER      5
#define VPT_ALIGN_MIDDLE_RIGHT       6
#define VPT_ALIGN_BOTTOM_LEFT        7
#define VPT_ALIGN_BOTTOM_CENTER      8
#define VPT_ALIGN_BOTTOM_RIGHT       9

/* ----------------------------------- Video Path Info */

struct PathPoint
{
  int               pointIsInUse;     /* Point is active */
  int               pointX, pointY;   /* X,Y Positon, 0,0 is the upper left corner of the visible display */
  int               imageWidth;       /* Width of the image at this point in the path */
  int               imageHeight;      /* Height of the image at this point in the path */

  int               imageAlign;       /* VPT_ALIGN_* */
  int               imageIsFullscreen;/* Image at current point is fullscreen, ignore X, Y and size */

  ULONG             specialData[10];  /* Used for specialData */
};

struct PATHINFO
{
  int               supportedModes;   /* Combination of VPT_MODE_* */
  int               currentMode;      /* VPT_MODE_* */

  struct PathPoint  startPoint;       /* startPoint - Always used */
  struct PathPoint  endPoint;         /* endPoint - Always used */

  struct PathPoint  pathPoints[8];    /* Optional Points in the Path */

  double            xRatio, yRatio;   /* Current X, Y Ratios */

  char              futureExp1[80];   /* Reserved */
};

#ifdef WIN32
#pragma pack(8)
#endif

#endif

/**
 * $Log: Project.h,v $
 * Revision 1.6  2000/03/24 07:42:05  markusm
 * * Smart Rendering works now for DV and MJPG for the MainActor modules, not much tested yet!
 *
 * Revision 1.5  2000/03/21 09:27:00  markusm
 * * A fix in the video effect rendering engine
 *
 * Revision 1.4  2000/01/30 19:17:32  kalle
 * GbR -> GmbH
 *
 * Revision 1.3  1999/12/16 08:28:27  markusm
 * * Now Windows v3.5 should be able to read v3.0 config files, added loads of pack(2) instructions around
 *   structures to be v3.0 compatible (Windows only)
 * * VideoEditor can now be compiled under Windows (but there is a lot of work to do!!)
 *
 * Revision 1.2  1999/10/12 08:00:01  markusm
 * * Implemented SQObjectInfo
 *
 * Revision 1.1  1999/09/13 08:26:29  markusm
 * * New structure of directories
 * * Added first set of source files
 *
 * Revision 1.8  1999/08/07 12:10:59  markusm
 * Added fonts directory and some fonts
 * Changed all file operations to ANSI C fopen/fread etc. . Renamed all HMODULEs to FILE *
 * filter_function.c now uses the ANSI C stuff
 * Made the AVI/BMP/TGA modules to work with the ANSI C operations (MPEG doesnt work yet)
 *
 * Revision 1.7  1999/07/24 11:27:35  martin
 * - Fixed sytax error: anonymous unions only supported by C++,
 *   not by plain ANSI C
 *
 * Revision 1.6  1999/07/23 20:00:54  martin
 *
 * Modified ldrbmp to work with Linux.
 * Modified Project.h, too. (added union for colortable.)
 * martin 23-Jul-1999
 *
 * Revision 1.5  1999/06/19 13:25:20  kalle
 * - removed dummy field from ProjectData
 * - Started implementing export dialog (so far widgets only)
 *
 * Revision 1.4  1999/06/18 19:56:48  kalle
 * Finally: icons shown in browser. Much more work needed.
 *
 * Revision 1.3  1999/06/15 11:36:28  kalle
 * - started work on new AVI loader for Linux
 * - implemented MCDisplayBitmap() for Qt
 * - AVI loading seems to work, preview doesn't
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
