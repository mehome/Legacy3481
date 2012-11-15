# Microsoft Developer Studio Project File - Name="StoryBoard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=StoryBoard - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StoryBoard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StoryBoard.mak" CFG="StoryBoard - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StoryBoard - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "StoryBoard - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StoryBoard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "StoryBoard___Win32_Release"
# PROP BASE Intermediate_Dir "StoryBoard___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "StoryBoard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "StoryBoard___Win32_Debug"
# PROP BASE Intermediate_Dir "StoryBoard___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "StoryBoard - Win32 Release"
# Name "StoryBoard - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\audio.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioGUI.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\DV_kit\AVIparser.cpp
# End Source File
# Begin Source File

SOURCE=.\Capture.cpp
# End Source File
# Begin Source File

SOURCE=..\General\CBasic.cpp
# End Source File
# Begin Source File

SOURCE=.\CGpoints.cpp
# End Source File
# Begin Source File

SOURCE=.\console.cpp
# End Source File
# Begin Source File

SOURCE=.\controls.cpp
# End Source File
# Begin Source File

SOURCE=.\DragObject.cpp
# End Source File
# Begin Source File

SOURCE=.\DVCam.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterCG.cpp
# End Source File
# Begin Source File

SOURCE=.\Filters.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterTitlerCG.cpp
# End Source File
# Begin Source File

SOURCE=.\FXobject.cpp
# End Source File
# Begin Source File

SOURCE=..\General\GBasic.cpp
# End Source File
# Begin Source File

SOURCE=.\HandleMain.cpp
# End Source File
# Begin Source File

SOURCE=.\HandleMain.rc
# End Source File
# Begin Source File

SOURCE=.\loaderAVI.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderIFF.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderJPG.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderPCX.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderRTV.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderTGA.cpp
# End Source File
# Begin Source File

SOURCE=.\loaderTIF.cpp
# End Source File
# Begin Source File

SOURCE=.\miniscrub.cpp
# End Source File
# Begin Source File

SOURCE=.\Preview.cpp
# End Source File
# Begin Source File

SOURCE=.\Project.cpp
# End Source File
# Begin Source File

SOURCE=.\RGBvideorender.cpp
# End Source File
# Begin Source File

SOURCE=.\Security.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectImage.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoardObject.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoardTools.cpp
# End Source File
# Begin Source File

SOURCE=.\StorySourceBase.cpp
# End Source File
# Begin Source File

SOURCE=.\StorySourceClass.cpp
# End Source File
# Begin Source File

SOURCE=.\textedit.cpp
# End Source File
# Begin Source File

SOURCE=.\Toaster.cpp
# End Source File
# Begin Source File

SOURCE=.\UnLZWstrip.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\audio.h
# End Source File
# Begin Source File

SOURCE=.\Capture.h
# End Source File
# Begin Source File

SOURCE=.\controls.h
# End Source File
# Begin Source File

SOURCE=.\DragObject.h
# End Source File
# Begin Source File

SOURCE=.\filters.h
# End Source File
# Begin Source File

SOURCE=.\FXobject.h
# End Source File
# Begin Source File

SOURCE=.\HandleMain.h
# End Source File
# Begin Source File

SOURCE=.\loaders.h
# End Source File
# Begin Source File

SOURCE=.\Preview.h
# End Source File
# Begin Source File

SOURCE=.\Project.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\RGBvideorender.h
# End Source File
# Begin Source File

SOURCE=.\Security.h
# End Source File
# Begin Source File

SOURCE=.\SelectImage.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoardObject.h
# End Source File
# Begin Source File

SOURCE=.\StorySourceBase.h
# End Source File
# Begin Source File

SOURCE=.\StorySourceClass.h
# End Source File
# Begin Source File

SOURCE=.\structstay.h
# End Source File
# Begin Source File

SOURCE=.\Toaster.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\amiga.ico
# End Source File
# Begin Source File

SOURCE=.\exodus.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\Exodusmm.bmp
# End Source File
# Begin Source File

SOURCE=.\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\RTVAudioPaths.ini
# End Source File
# End Group
# End Target
# End Project
