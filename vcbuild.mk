all:
	VCBuild.exe /nologo dskwipe.vcproj /rebuild

clean:
	VCBuild.exe /nologo dskwipe.vcproj /clean
	
upgrade:
	devenv dskwipe.sln /upgrade

.PHONY:	all clean upgrade

