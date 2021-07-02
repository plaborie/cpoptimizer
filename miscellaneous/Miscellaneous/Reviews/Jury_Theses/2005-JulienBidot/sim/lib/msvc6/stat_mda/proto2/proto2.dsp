# Microsoft Developer Studio Project File - Name="proto2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=proto2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "proto2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "proto2.mak" CFG="proto2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "proto2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "proto2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "proto2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /MD /W3 /GX /O2 /I "c:\ilog\iimCVS\include" /I "c:\ilog\concertextCVS\include" /I "c:\ilog\concertCVS\include" /I "c:\ilog\solverCVS\include" /I "c:\ilog\schedCVS\include" /I "c:\Julien\sim\include" /D "IL_STD" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /nologo
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 solveriim.lib concert.lib concertext.lib solver.lib schedule.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /subsystem:console /machine:I386 /libpath:"c:\ilog\iimCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\concertextCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\concertCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\solverCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\schedCVS\lib\msvc6\stat_mda"
# SUBTRACT LINK32 /nologo /pdb:none

!ELSEIF  "$(CFG)" == "proto2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /MD /W3 /Gm /GX /ZI /Od /I "c:\ilog\iimCVS\include" /I "c:\ilog\concertextCVS\include" /I "c:\ilog\concertCVS\include" /I "c:\ilog\solverCVS\include" /I "c:\ilog\schedCVS\include" /I "c:\Julien\sim\include" /D "IL_STD" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /nologo
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 solveriim.lib schedule.lib concert.lib concertext.lib solver.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept /libpath:"c:\ilog\iimCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\concertextCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\concertCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\solverCVS\lib\msvc6\stat_mda" /libpath:"c:\ilog\schedCVS\lib\msvc6\stat_mda" /FIXED:NO
# SUBTRACT LINK32 /nologo /pdb:none

!ENDIF 

# Begin Target

# Name "proto2 - Win32 Release"
# Name "proto2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\iluconstraints.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\iluglobalcost.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\iluheuristic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\ilumakespan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\iluneighborhood.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\iluprintsolution.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\ilurandom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\ilusdxloutput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\ilusdxlsolutionoutput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\ilutardiness.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\examples\src\instance.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\iluconstraints.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\iluglobalcost.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\iluheuristic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\ilumakespan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\iluneighborhood.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\iluprintsolution.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\iluprocessplan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\ilurandom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\ilusdxloutput.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\ilusdxlsolutionoutput.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\ilsim\ilutardiness.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
