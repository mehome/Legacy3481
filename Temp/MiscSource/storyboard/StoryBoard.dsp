# Microsoft Developer Studio Project File - Name="StoryBoard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

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
!MESSAGE "StoryBoard - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "StoryBoard - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Controls/StoryBoard", PVBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StoryBoard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Release/"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STORYBOARD_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GR /GX /O2 /I "../../Controls/Include/" /I "../Additional/Toaster2/Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STORYBOARD_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 DDraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Release/StoryBoard.Toast" /libpath:"..\..\Release" /libpath:"../../Additional/AVICodec"

!ELSEIF  "$(CFG)" == "StoryBoard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Debug/"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STORYBOARD_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "../../Controls/Include/" /I "../Additional/Toaster2/Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STORYBOARD_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 DDraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../Debug/StoryBoard.Toast" /pdbtype:sept /libpath:"../../Debug" /libpath:"../../Additional/Toaster/Lib/i386" /libpath:"../../Additional/RTVLib" /libpath:"../../Additional/AVICodec"

!ENDIF 

# Begin Target

# Name "StoryBoard - Win32 Release"
# Name "StoryBoard - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\InOuts_Control.cpp
# End Source File
# Begin Source File

SOURCE=.\Script1.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoard.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoard_Crouton.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoard_CurCopyPaste.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoard_Item.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoardPath.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryBoardView.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoPreviewWindow.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\InOuts_Control.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoard.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoard_Codec.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoard_Crouton.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoard_Item.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoardPath.h
# End Source File
# Begin Source File

SOURCE=.\StoryBoardView.h
# End Source File
# Begin Source File

SOURCE=.\VideoPreviewWindow.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\cursor2.cur
# End Source File
# Begin Source File

SOURCE=.\cursor3.cur
# End Source File
# End Group
# Begin Group "Codec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Additional\AVICodec\C_List.h
# End Source File
# Begin Source File

SOURCE=..\..\Additional\AVICodec\C_TransitionDVE.h
# End Source File
# Begin Source File

SOURCE=..\..\Additional\AVICodec\C_Video.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Debug\Skins\StoryBoard\SB.txt
# End Source File
# End Target
# End Project
