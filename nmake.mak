all:
	$(MAKE) /f dskwipe.mak CFG="dskwipe - Win32 Release"	all
	$(MAKE) /f dskwipe.mak CFG="dskwipe - Win32 Debug"	all

clean:
	$(MAKE) /f dskwipe.mak CFG="dskwipe - Win32 Release"	clean
	$(MAKE) /f dskwipe.mak CFG="dskwipe - Win32 Debug"	clean

realclean: clean
	-cmd /c del /s *.bak
	-cmd /c del /s *.bsc
	-cmd /c del /s *.idb
	-cmd /c del /s *.pdb
	-cmd /c del /s *.lib
	-cmd /c del /s *.ncb
	-cmd /c del /s *.obj
	-cmd /c del /s *.opt
	-cmd /c del /s *.pch
	-cmd /c del /s *.plg
	-cmd /c del /s *.sbr
