xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\*.dll"  /Exclude:ExcludeDebugDLL.txt
xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\osgPlugins-2.9.5\*.dll"  /Exclude:ExcludeDebugDLL.txt
xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\osgviewer.exe"
xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\osgversion.exe"

xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\*.idb"  /Exclude:ExcludeDebugPDB.txt
xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\osgPlugins-2.9.5\*.idb"  /Exclude:ExcludeDebugPDB.txt
xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\osgviewer.idb"
xcopy /y /d "..\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin\osgversion.idb"

xcopy /y /d "..\Source\Utilities\OSG_SVN\3rdParty_Win32binaries_vs80sp1_2008-04-28\bin\*.dll"  /Exclude:ExcludeDebugDLL.txt

xcopy /y /d "..\Source\Utilities\FMOD\Win32\api\*.dll"
xcopy /y /d "..\Source\Utilities\lua\lua5_1_2_Win32_dll7_lib\lua5.1.dll"
xcopy /y /d "..\Source\Utilities\RakNet\Lib\RakNet.dll"
xcopy /y /d "..\Source\Utilities\xmlParser\Release\xmlParser.dll"
