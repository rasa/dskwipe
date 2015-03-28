# Copyright (c) 2002-2015, Ross Smith II. MIT licensed.

## dependencies:
## Cygwin
## NSIS 2.41 (https://sourceforge.net/projects/nsis/files/NSIS%202/2.41/)
## NSIS log build (http://sourceforge.net/projects/nsis/files/NSIS%202/2.41/nsis-2.41-log.zip/download)
## NSIS NsUnzip plugin (http://nsis.sourceforge.net/NsUnzip_plugin)
## NSIS Inetc plugin (http://nsis.sourceforge.net/Inetc_plug-in)
## signtool (provided by Visual Studio or Windows SDK)

## cygwin dependencies:
## curl
## cygpath
## git
## gpg
## md5sum
## sha1sum
## sha256sum
## upx
## zip
## basename, cat, cp, echo, grep, mkdir, rm, sed, sleep, test, touch, etc.

## nsis builds:
ifneq ("$(wildcard version.txt)", "")
VER?=$(strip $(shell cat version.txt))
APP?=$(basename $(wildcard *.nsi))
endif

## c/c++ builds:
ifneq ("$(wildcard version.h)", "")
VER?=$(strip $(shell sed -rne 's/^\s*\#define\s+(VER_STRING2|PACKAGE_VERSION)\s+"([^"]*)".*/\2/p' version.h))
APP?=$(strip $(shell sed -rne 's/^\s*\#define\s+(VER_INTERNAL_NAME|PACKAGE)\s+"([^"]*)".*/\2/p' version.h))
endif

APP_EXE:=$(APP).exe

APP_FILES:=\
$(APP_EXE) \
$(wildcard LICENSE) \
$(wildcard *.md)

#######################################################################

FLAVOR?=win32
REPO?=$(notdir $(PWD))

APP_DESC?=$(APP)
APP_URL?=http://github.com/rasa/$(REPO)/
APP_ZIP?=$(APP)-$(VER)-$(FLAVOR).zip
CODESIGN_PFX?=../codesign.pfx
CURL?=curl
CURL_OPTS?=--netrc --insecure --silent --show-error 
GPG_OPTS?=
HASH_FILES?=md5s.txt sha1s.txt sha256s.txt
RELEASE_APP_EXE?=Release/$(APP_EXE)
RELEASE_URL?=https://api.github.com/repos/rasa/$(REPO)/releases
TAG?=v$(VER)
## fails occasionally: http://timestamp.verisign.com/scripts/timstamp.dll
TIMESTAMP_URL?=http://timestamp.globalsign.com/scripts/timstamp.dll
VIRUSTOTAL_URL?=https://www.virustotal.com/vtapi/v2/file/scan
ZIP_OPTS+=-9

APP_FILES+=$(HASH_FILES)
RELEASED:=.$(TAG).released

#######################################################################

ifneq ("$(wildcard makensis.mk)", "")
NMAKER?=makensis.mk
endif

ifneq ("$(wildcard msbuild.mk)", "")
ifneq ("$(shell which MSBuild.exe 2>/dev/null)", "")
CMAKER?=msbuild.mk
endif
endif

ifneq ("$(wildcard vcbuild.mk)", "")
ifneq ("$(shell which VCBuild.exe 2>/dev/null)", "")
CMAKER?=vcbuild.mk
endif
endif

CMAKER?=nmake.mk

#######################################################################

ifndef SIGNTOOL
	ifneq ($(shell which signtool.exe 2>/dev/null),)
		SIGNTOOL:=$(shell which signtool.exe 2>/dev/null)
	endif
endif

define FIND_SIGNTOOL

ifndef SIGNTOOL
	SDK_DIR:=$$(shell cygpath -m -s $(1) 2>/dev/null)
	ifneq ($$(wildcard $$(SDK_DIR)/signtool.exe),)
		SIGNTOOL:=$$(SDK_DIR)/signtool.exe
	endif
endif

endef

$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v8.2/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v8.1A/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v8.1/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v8.0A/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v8.0/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v7.1A/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v7.1/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v7.0A/Bin))
$(eval $(call FIND_SIGNTOOL,$(PROGRAMFILES)/Microsoft SDKs/Windows/v7.0/Bin))

#######################################################################

UPXED_FILES=

define UPX_FILE

$(2): $(1)
	test -d $(dir $(2)) || mkdir $(dir $(2))
	test ! -f $(2) || rm -f $(2)
	upx -9 -o $(2) $(1)

UPXED_FILES+=$(2)

endef

#######################################################################

SIGNED_FILES=

define SIGN_FILE

$(2): $(1) $(CODESIGN_PFX)
	test -d $(dir $(2)) || mkdir $(dir $(2))
	for try in 1 2 3 4 5 6 7 8 9 10; do \
		"$(SIGNTOOL)" verify /pa /q $(1) &>/dev/null && break ;\
		"$(SIGNTOOL)" sign \
			/d "$(3)" \
			/du "$(4)" \
			/f "$(CODESIGN_PFX)" \
			/q \
			/t "$(TIMESTAMP_URL)" \
			$(1) ;\
		sleep $$$$(( 2 ** $$$${try} )) ;\
	done
	cp $(1) $(2)

SIGNED_FILES+=$(2)

endef

#######################################################################

UPLOADED_FILES=

define UPLOAD_FILE

.$(1).uploaded:	$(1) $(RELEASED)
	$(CURL) $(CURL_OPTS) \
		--write-out "%{http_code}" \
		--output /tmp/$(1).tmp \
		--include \
		"$(RELEASE_URL)/tags/$(TAG)" |\
			grep -q ^2 || (cat /tmp/$(1).tmp ; echo ; exit 1)
	sed -nr -e 's/.*upload_url": "(.*)\{.*/url=\1?name=$(1)/p' /tmp/$(1).tmp >/tmp/$(1).curlrc
	$(CURL) $(CURL_OPTS) \
		--request POST \
		--data-binary @$(1) \
		-H "Content-Type:$(2)" \
		--write-out %{http_code} \
		--output /tmp/$(1)-2.tmp \
		--include \
		--config /tmp/$(1).curlrc | \
			grep -q ^2 || (cat /tmp/$(1)*.tmp ; echo ; exit 1)
	-rm -f /tmp/$(1).tmp /tmp/$(1)-2.tmp
	@-echo $(1) uploaded to Github
	touch .$(1).uploaded

UPLOADED_FILES+=.$(1).uploaded

endef

#######################################################################

SCANNED_FILES=

define SCAN_FILE

.$(1).scanned: $(1)
	test -n "$(VIRUSTOTAL_API_KEY)" || (echo VIRUSTOTAL_API_KEY not set >&2 ; exit 1)
	$(CURL) $(CURL_OPTS) \
		-X POST \
		--form apikey=$(VIRUSTOTAL_API_KEY) \
		--form file=@$(1) \
		--write-out "%{http_code}" \
		--output /tmp/$(1).tmp \
		--include \
		$(VIRUSTOTAL_URL) | \
			grep -q ^2 || (cat /tmp/$(1).tmp ; echo ; exit 1)
	-rm -f /tmp/$(1).tmp
	touch .$(1).scanned

SCANNED_FILES+=.$(1).scanned

endef

#######################################################################

ifneq ("$(wildcard local.mk)", "")
include local.mk
endif

#######################################################################

.PHONY:	help
help:
	@echo "make all       # build Release/.exe"
	@echo "make upx       # compress Release/.exe (implies all)"
	@echo "make sign      # sign .exe (implies upx)"
	@echo "make hashes    # create hash files (implies all)"
	@echo "make zip       # build .zip (implies sign)"
	@echo "make tag       # create tag $(TAG) on Github (implies zip)"
	@echo "make release   # create release $(TAG) on Github (implies tag)"
	@echo "make sig       # create .asc (implies zip)"
	@echo "make sum       # create .sha256 (implies zip)"
	@echo "make upload    # upload .zip/.asc/.sha256 to Github (implies release/sig/sum)"
	@echo "make scan      # upload .exe & .zip to Virustotal (implies zip)"
	@echo "make publish   # same as 'make upload scan'"
	@echo
	@echo "make clean     # remove Release/.exe and related build files"
	@echo "make distclean # remove .zip/.asc/.sha256 (implies clean)"
	@echo "make tidy      # remove .exe & hash .txt's (implies clean)"
	@echo "make help      # display this help text"

#######################################################################

ifdef NMAKER
include $(NMAKER)
else
include $(CMAKER)
$(RELEASE_APP_EXE):	all

endif

#######################################################################

RELEASE_UPX_EXE:=$(dir $(RELEASE_APP_EXE))upxed/$(notdir $(RELEASE_APP_EXE))

$(eval $(call UPX_FILE,$(RELEASE_APP_EXE),$(RELEASE_UPX_EXE)))

.PHONY:	upx
upx:	$(RELEASE_UPX_EXE)

#######################################################################

$(CODESIGN_PFX):
	@echo The file $(CODESIGN_PFX) is required to sign code with. >&2
	exit 1

$(eval $(call SIGN_FILE,$(RELEASE_UPX_EXE),$(APP_EXE),$(APP_DESC),$(APP_URL)))

.PHONY:	sign
sign:	$(SIGNED_FILES)

#######################################################################

md5s.txt: $(APP_EXE)
	md5sum $^ >$@

sha1s.txt: $(APP_EXE)
	sha1sum $^ >$@

sha256s.txt: $(APP_EXE)
	sha256sum $^ >$@

.PHONY:	hashes
hashes:	$(HASH_FILES)

#######################################################################

$(APP_ZIP):	$(APP_FILES)
	-rm -f $@
	zip $(ZIP_OPTS) $@ $^
	
.PHONY:	zip
zip:	$(APP_ZIP)

.PHONY:	dist
dist:	zip

#######################################################################

TAGGED:=.$(TAG).tagged

$(TAGGED):	$(APP_ZIP)
	if ! git tag | grep -q -P "\b$(TAG)\b"; then \
		git tag -a $(TAG) -m "Version $(VER)" ;\
	fi
	git push origin --tags
	touch $@
	@-echo Tag $(TAG) created on Github

.PHONY:	tag
tag:	$(TAGGED)

#######################################################################

$(RELEASED):	$(TAGGED)
	$(CURL) $(CURL_OPTS) \
		--request POST \
		--data "{\"tag_name\": \"$(TAG)\",\"name\": \"$(TAG)\", \"body\": \"Release $(TAG)\"}" \
		--write-out "%{http_code}" \
		--output /tmp/$(RELEASED).tmp \
		--include \
		"$(RELEASE_URL)" | grep -q ^2 || (cat /tmp/$(RELEASED).tmp ; echo ; exit 1)
	touch $@
	@-echo Release $(TAG) created on Github

.PHONY:	release
release: $(RELEASED)

#######################################################################

APP_ZIP_ASC:=$(APP_ZIP).asc

$(APP_ZIP_ASC):	$(APP_ZIP)
	gpg $(GPG_OPTS) --yes --armor --detach-sign --output $@ $^

.PHONY:	sig
sig:	$(APP_ZIP_ASC)

#######################################################################

APP_ZIP_SHA:=$(APP_ZIP).sha256

$(APP_ZIP_SHA): $(APP_ZIP)
	sha256sum $^ >$@

.PHONY:	sum
sum:	$(APP_ZIP_SHA)

#######################################################################

$(eval $(call UPLOAD_FILE,$(APP_ZIP),application/zip))

$(eval $(call UPLOAD_FILE,$(APP_ZIP_ASC),text/plain))

$(eval $(call UPLOAD_FILE,$(APP_ZIP_SHA),text/plain))

.PHONY:	upload
upload:	$(UPLOADED_FILES)

#######################################################################

$(eval $(call SCAN_FILE,$(APP_EXE)))

$(eval $(call SCAN_FILE,$(APP_ZIP)))

.PHONY:	scan
scan:	$(SCANNED_FILES)

#######################################################################

.PHONY:	publish
publish:	$(UPLOADED_FILES) $(SCANNED_FILES)

#######################################################################

.PHONY:	tidy
tidy:	clean
	-rm -f  $(CLEANED_FILES) \
		$(HASH_FILES) \
		$(SIGNED_FILES) \
		$(UPXED_FILES)

#######################################################################

.PHONY:	distclean
distclean:	tidy
	-rm -f  $(APP_ZIP) \
		$(APP_ZIP_ASC) \
		$(APP_ZIP_SHA)

#######################################################################

.PHONY: debug
debug:
	@echo APP=$(APP)
	@echo APP_DESC=$(APP_DESC)
	@echo APP_EXE=$(APP_EXE)
	@echo APP_FILES=$(APP_FILES)
	@echo APP_URL=$(APP_URL)
	@echo APP_ZIP=$(APP_ZIP)
	@echo APP_ZIP_ASC=$(APP_ZIP_ASC)
	@echo APP_ZIP_SHA=$(APP_ZIP_SHA)
	@echo CMAKER=$(CMAKER)
	@echo CODESIGN_PFX=$(CODESIGN_PFX)
	@echo CURL=$(CURL)
	@echo CURL_OPTS=$(CURL_OPTS)
	@echo FLAVOR=$(FLAVOR)
	@echo GPG_OPTS=$(GPG_OPTS)
	@echo HASH_FILES=$(HASH_FILES)
	@echo NMAKER=$(NMAKER)
	@echo RELEASED=$(RELEASED)
	@echo RELEASE_APP_EXE=$(RELEASE_APP_EXE)
	@echo RELEASE_UPX_EXE=$(RELEASE_UPX_EXE)
	@echo RELEASE_URL=$(RELEASE_URL)
	@echo REPO=$(REPO)
	@echo SCANNED_FILES=$(SCANNED_FILES)
	@echo SIGNED_FILES=$(SIGNED_FILES)
	@echo TAG=$(TAG)
	@echo TAGGED=$(TAGGED)
	@echo TIMESTAMP_URL=$(TIMESTAMP_URL)
	@echo UPLOADED_FILES=$(UPLOADED_FILES)
	@echo UPXED_FILES=$(UPXED_FILES)
	@echo VER=$(VER)
	@echo VIRUSTOTAL_URL=$(VIRUSTOTAL_URL)
	@echo ZIP_OPTS=$(ZIP_OPTS)
