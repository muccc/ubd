# Microsoft Developer Studio Project File - Name="bootloader" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=bootloader - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bootloader.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bootloader.mak" CFG="bootloader - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bootloader - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "bootloader - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "bootloader - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f bootloader.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "bootloader.exe"
# PROP BASE Bsc_Name "bootloader.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "make"
# PROP Rebuild_Opt "clean"
# PROP Target_File "bootloader.hex"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "bootloader - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f bootloader.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "bootloader.exe"
# PROP BASE Bsc_Name "bootloader.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "make_msdev"
# PROP Rebuild_Opt "clean"
# PROP Target_File "Bootloader_EM_UEM_atmega8.hex"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "bootloader - Win32 Release"
# Name "bootloader - Win32 Debug"

!IF  "$(CFG)" == "bootloader - Win32 Release"

!ELSEIF  "$(CFG)" == "bootloader - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Src_Bootloader\boot.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "AVR"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\avr\boot.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\util\crc16.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\util\delay.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\avr\eeprom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\avr\interrupt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\avr\pgmspace.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\avr\sleep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\..\..\Programme\Developer\WinAVR\avr\include\stdint.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Src_Bootloader\boot.h
# End Source File
# Begin Source File

SOURCE=..\Src_Bootloader\hal.h
# End Source File
# Begin Source File

SOURCE=..\Src_Bootloader\Platform_AVR\hal_platform.h
# End Source File
# Begin Source File

SOURCE=..\Src_Bootloader\Platform_AVR\uart_platform.h
# End Source File
# End Group
# Begin Group "Make Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\makefile
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
