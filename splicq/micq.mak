# Microsoft Developer Studio Generated NMAKE File, Based on micq.dsp
!IF "$(CFG)" == ""
CFG=micq - Win32 Debug
!MESSAGE No configuration specified. Defaulting to micq - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "micq - Win32 Release" && "$(CFG)" != "micq - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "micq.mak" CFG="micq - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "micq - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "micq - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "micq - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\micq.exe"

!ELSE 

ALL : "$(OUTDIR)\micq.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\icq_response.obj"
	-@erase "$(INTDIR)\micq.obj"
	-@erase "$(INTDIR)\sendmsg.obj"
	-@erase "$(INTDIR)\ui.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\micq.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\micq.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\micq.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\micq.pdb" /machine:I386 /out:"$(OUTDIR)\micq.exe" 
LINK32_OBJS= \
	"$(INTDIR)\icq_response.obj" \
	"$(INTDIR)\micq.obj" \
	"$(INTDIR)\sendmsg.obj" \
	"$(INTDIR)\ui.obj" \
	"$(INTDIR)\util.obj"

"$(OUTDIR)\micq.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "micq - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\micq.exe"

!ELSE 

ALL : "$(OUTDIR)\micq.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\icq_response.obj"
	-@erase "$(INTDIR)\micq.obj"
	-@erase "$(INTDIR)\sendmsg.obj"
	-@erase "$(INTDIR)\ui.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\micq.exe"
	-@erase "$(OUTDIR)\micq.ilk"
	-@erase "$(OUTDIR)\micq.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Zp1 /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_CONSOLE" /D "_MBCS" /D "INTEL_END" /D "FUNNY_MSGS" /Fp"$(INTDIR)\micq.pch"\
 /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\micq.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\micq.pdb" /debug /machine:I386 /out:"$(OUTDIR)\micq.exe"\
 /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\icq_response.obj" \
	"$(INTDIR)\micq.obj" \
	"$(INTDIR)\sendmsg.obj" \
	"$(INTDIR)\ui.obj" \
	"$(INTDIR)\util.obj"

"$(OUTDIR)\micq.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "micq - Win32 Release" || "$(CFG)" == "micq - Win32 Debug"
SOURCE=.\icq_response.c

!IF  "$(CFG)" == "micq - Win32 Release"

DEP_CPP_ICQ_R=\
	".\datatype.h"\
	".\micq.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\icq_response.obj" : $(SOURCE) $(DEP_CPP_ICQ_R) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "micq - Win32 Debug"

DEP_CPP_ICQ_R=\
	".\datatype.h"\
	".\micq.h"\
	

"$(INTDIR)\icq_response.obj" : $(SOURCE) $(DEP_CPP_ICQ_R) "$(INTDIR)"


!ENDIF 

SOURCE=.\micq.c

!IF  "$(CFG)" == "micq - Win32 Release"

DEP_CPP_MICQ_=\
	".\datatype.h"\
	".\micq.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\micq.obj" : $(SOURCE) $(DEP_CPP_MICQ_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "micq - Win32 Debug"

DEP_CPP_MICQ_=\
	".\datatype.h"\
	".\micq.h"\
	

"$(INTDIR)\micq.obj" : $(SOURCE) $(DEP_CPP_MICQ_) "$(INTDIR)"


!ENDIF 

SOURCE=.\sendmsg.c

!IF  "$(CFG)" == "micq - Win32 Release"

DEP_CPP_SENDM=\
	".\datatype.h"\
	".\micq.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\sendmsg.obj" : $(SOURCE) $(DEP_CPP_SENDM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "micq - Win32 Debug"

DEP_CPP_SENDM=\
	".\datatype.h"\
	".\micq.h"\
	

"$(INTDIR)\sendmsg.obj" : $(SOURCE) $(DEP_CPP_SENDM) "$(INTDIR)"


!ENDIF 

SOURCE=.\ui.c

!IF  "$(CFG)" == "micq - Win32 Release"

DEP_CPP_UI_C6=\
	".\datatype.h"\
	".\micq.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ui.obj" : $(SOURCE) $(DEP_CPP_UI_C6) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "micq - Win32 Debug"

DEP_CPP_UI_C6=\
	".\datatype.h"\
	".\micq.h"\
	

"$(INTDIR)\ui.obj" : $(SOURCE) $(DEP_CPP_UI_C6) "$(INTDIR)"


!ENDIF 

SOURCE=.\util.c
DEP_CPP_UTIL_=\
	".\datatype.h"\
	".\micq.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"



!ENDIF 

