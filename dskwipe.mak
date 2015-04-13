# Microsoft Developer Studio Generated NMAKE File, Based on dskwipe.dsp
!IF "$(CFG)" == ""
CFG=dskwipe - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dskwipe - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dskwipe - Win32 Release" && "$(CFG)" != "dskwipe - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dskwipe.mak" CFG="dskwipe - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dskwipe - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "dskwipe - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dskwipe - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dskwipe.exe"


CLEAN :
	-@erase "$(INTDIR)\dskwipe.obj"
	-@erase "$(INTDIR)\dskwipe.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dskwipe.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\shared" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\dskwipe.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dskwipe.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dskwipe.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=shared.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\dskwipe.pdb" /machine:I386 /out:"$(OUTDIR)\dskwipe.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dskwipe.obj" \
	"$(INTDIR)\dskwipe.res"

"$(OUTDIR)\dskwipe.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dskwipe - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dskwipe.exe" "$(OUTDIR)\dskwipe.bsc"


CLEAN :
	-@erase "$(INTDIR)\dskwipe.obj"
	-@erase "$(INTDIR)\dskwipe.res"
	-@erase "$(INTDIR)\dskwipe.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dskwipe.bsc"
	-@erase "$(OUTDIR)\dskwipe.exe"
	-@erase "$(OUTDIR)\dskwipe.ilk"
	-@erase "$(OUTDIR)\dskwipe.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\shared" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\dskwipe.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dskwipe.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dskwipe.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dskwipe.sbr"

"$(OUTDIR)\dskwipe.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=sharedd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\dskwipe.pdb" /debug /machine:I386 /out:"$(OUTDIR)\dskwipe.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dskwipe.obj" \
	"$(INTDIR)\dskwipe.res"

"$(OUTDIR)\dskwipe.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dskwipe.dep")
!INCLUDE "dskwipe.dep"
!ELSE 
!MESSAGE Warning: cannot find "dskwipe.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dskwipe - Win32 Release" || "$(CFG)" == "dskwipe - Win32 Debug"
SOURCE=.\dskwipe.cpp

!IF  "$(CFG)" == "dskwipe - Win32 Release"


"$(INTDIR)\dskwipe.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dskwipe - Win32 Debug"


"$(INTDIR)\dskwipe.obj"	"$(INTDIR)\dskwipe.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dskwipe.rc

"$(INTDIR)\dskwipe.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

