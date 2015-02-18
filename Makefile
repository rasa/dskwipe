# Makefile to build getjpegs
# Copyright (c) 2004-2006 Ross Smith II (http://smithii.com). All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2 of the GNU General Public License 
# as published by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# $Id$

VER?=$(shell perl -n -e '/define\s+VER_STRING2\s+"(.*)"/ && print $$1' version.h)
APP?=$(shell perl -n -e '/define\s+VER_INTERNAL_NAME\s+"(.*)"/ && print $$1' version.h)
APP_FILES=Release/$(APP).exe changelog.txt COPYING readme.txt
SRC_FILES=$(APP_FILES) $(shell ls Makefile *.cpp *.dep *.dsp *.dsw *.h *.ico *.mak *.rc 2>/dev/null)

APP_ZIP?=$(APP)-$(VER)-win32.zip
SRC_ZIP?=$(APP)-$(VER)-win32-src.zip
ZIP?=zip
ZIP_OPTS?=-9jquX

.PHONY:	dist
dist:	all $(APP_ZIP) $(SRC_ZIP)

$(APP_ZIP):	$(APP_FILES)
	-rm -f $(APP_ZIP)
	chmod a+x $^
	${ZIP} ${ZIP_OPTS} $@ $^

$(SRC_ZIP):	$(SRC_FILES)
	-rm -f $(SRC_ZIP)
	chmod a+x $^
	${ZIP} ${ZIP_OPTS} $@ $^
	
.PHONY:	distclean
distclean:	clean
	rm -f $(APP_ZIP) $(SRC_ZIP)

.PHONY:	all clean realclean

include nmake.mak
