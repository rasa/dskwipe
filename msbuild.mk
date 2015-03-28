all:
	MSBuild.exe /nologo dskwipe.sln /p:Configuration=Debug
	MSBuild.exe /nologo dskwipe.sln /p:Configuration=Release

clean:
	MSBuild.exe /nologo dskwipe.sln /p:Configuration=Debug   /t:clean
	MSBuild.exe /nologo dskwipe.sln /p:Configuration=Release /t:clean

upgrade:
	devenv dskwipe.sln /upgrade
	
.PHONY:	all clean upgrade


