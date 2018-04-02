# Shadow Warrior Makefile for GNU Make

# Create Makefile.user yourself to provide your own overrides
# for configurable values
-include Makefile.user

##
##
## CONFIGURABLE OPTIONS
##
##

# Debugging options
RELEASE ?= 1

# Base path of app installation
PREFIX ?= /usr/local/share/games/jfsw

# DirectX SDK location
DXROOT ?= $(USERPROFILE)/sdks/directx/dx81

# Engine source code path
EROOT ?= jfbuild

# JMACT library source path
MACTROOT ?= jfmact

# JFAudioLib source path
AUDIOLIBROOT ?= jfaudiolib

# Engine options
SUPERBUILD ?= 1
POLYMOST ?= 1
USE_OPENGL ?= 1
DYNAMIC_OPENGL ?= 1
NOASM ?= 0


##
##
## HERE BE DRAGONS
##
##

# build locations
SRC=src
RSRC=rsrc
EINC=$(EROOT)/include
ELIB=$(EROOT)
INC=$(SRC)
o=o

ifneq (0,$(RELEASE))
  # debugging disabled
  debug=-fomit-frame-pointer -O2
else
  # debugging enabled
  debug=-ggdb -Og
endif

include $(AUDIOLIBROOT)/Makefile.shared

CC?=gcc
CXX?=g++
NASM?=nasm
RC?=windres
OURCFLAGS=$(debug) -W -Wall -Wimplicit -Wno-unused \
	-fno-strict-aliasing -DNO_GCC_BUILTINS \
	-I$(INC) -I$(EINC) -I$(MACTROOT) -I$(AUDIOLIBROOT)/include
OURCXXFLAGS=-fno-exceptions -fno-rtti
LIBS=-lm
GAMELIBS=
NASMFLAGS=-s #-g
EXESUFFIX=

JMACTOBJ=$(MACTROOT)/util_lib.$o \
	$(MACTROOT)/file_lib.$o \
	$(MACTROOT)/control.$o \
	$(MACTROOT)/keyboard.$o \
	$(MACTROOT)/mouse.$o \
	$(MACTROOT)/mathutil.$o \
	$(MACTROOT)/scriplib.$o \
	$(MACTROOT)/animlib.$o

GAMEOBJS= \
	$(SRC)/actor.$o \
	$(SRC)/ai.$o \
	$(SRC)/anim.$o \
	$(SRC)/border.$o \
	$(SRC)/break.$o \
	$(SRC)/bunny.$o \
	$(SRC)/cache.$o \
	$(SRC)/cheats.$o \
	$(SRC)/colormap.$o \
	$(SRC)/config.$o \
	$(SRC)/console.$o \
	$(SRC)/coolg.$o \
	$(SRC)/coolie.$o \
	$(SRC)/copysect.$o \
	$(SRC)/demo.$o \
	$(SRC)/draw.$o \
	$(SRC)/eel.$o \
	$(SRC)/game.$o \
	$(SRC)/girlninj.$o \
	$(SRC)/goro.$o \
	$(SRC)/grpscan.$o \
	$(SRC)/hornet.$o \
	$(SRC)/interp.$o \
	$(SRC)/interpsh.$o \
	$(SRC)/inv.$o \
	$(SRC)/jplayer.$o \
	$(SRC)/jsector.$o \
	$(SRC)/jweapon.$o \
	$(SRC)/lava.$o \
	$(SRC)/light.$o \
	$(SRC)/mclip.$o \
	$(SRC)/menus.$o \
	$(SRC)/miscactr.$o \
	$(SRC)/morph.$o \
	$(SRC)/net.$o \
	$(SRC)/ninja.$o \
	$(SRC)/panel.$o \
	$(SRC)/player.$o \
	$(SRC)/predict.$o \
	$(SRC)/quake.$o \
	$(SRC)/ripper.$o \
	$(SRC)/ripper2.$o \
	$(SRC)/rooms.$o \
	$(SRC)/rotator.$o \
	$(SRC)/rts.$o \
	$(SRC)/save.$o \
	$(SRC)/scrip2.$o \
	$(SRC)/sector.$o \
	$(SRC)/serp.$o \
	$(SRC)/setup.$o \
	$(SRC)/skel.$o \
	$(SRC)/skull.$o \
	$(SRC)/slidor.$o \
	$(SRC)/sounds.$o \
	$(SRC)/spike.$o \
	$(SRC)/sprite.$o \
	$(SRC)/sumo.$o \
	$(SRC)/swconfig.$o \
	$(SRC)/sync.$o \
	$(SRC)/text.$o \
	$(SRC)/track.$o \
	$(SRC)/vator.$o \
	$(SRC)/vis.$o \
	$(SRC)/wallmove.$o \
	$(SRC)/warp.$o \
	$(SRC)/weapon.$o \
	$(SRC)/zilla.$o \
	$(SRC)/zombie.$o \
	$(SRC)/saveable.$o \
	$(JMACTOBJ)

EDITOROBJS=$(SRC)/jnstub.$o \
	$(SRC)/brooms.$o \
	$(SRC)/bldscript.$o \
	$(SRC)/jbhlp.$o \
	$(SRC)/colormap.$o

include $(EROOT)/Makefile.shared

ifeq ($(PLATFORM),LINUX)
	NASMFLAGS+= -f elf
	GAMELIBS+= $(JFAUDIOLIB_LDFLAGS)
endif
ifeq ($(PLATFORM),WINDOWS)
	OURCFLAGS+= -I$(DXROOT)/include
	NASMFLAGS+= -f win32 --prefix _
	GAMEOBJS+= $(SRC)/gameres.$o $(SRC)/startwin_game.$o
	EDITOROBJS+= $(SRC)/buildres.$o
	GAMELIBS+= -ldsound \
	       $(AUDIOLIBROOT)/third-party/mingw32/lib/libvorbisfile.a \
	       $(AUDIOLIBROOT)/third-party/mingw32/lib/libvorbis.a \
	       $(AUDIOLIBROOT)/third-party/mingw32/lib/libogg.a
endif

ifeq ($(RENDERTYPE),SDL)
	OURCFLAGS+= $(SDLCONFIG_CFLAGS)
	LIBS+= $(SDLCONFIG_LIBS)

	ifeq (1,$(HAVE_GTK))
		OURCFLAGS+= $(GTKCONFIG_CFLAGS)
		LIBS+= $(GTKCONFIG_LIBS)
		GAMEOBJS+= $(SRC)/startgtk_game.$o $(RSRC)/startgtk_game_gresource.$o
		EDITOROBJS+= $(RSRC)/startgtk_build_gresource.$o
	endif

	GAMEOBJS+= $(RSRC)/sdlappicon_game.$o
	EDITOROBJS+= $(RSRC)/sdlappicon_build.$o
endif

OURCFLAGS+= $(BUILDCFLAGS)

ifneq ($(PLATFORM),WINDOWS)
	OURCFLAGS+= -DPREFIX=\"$(PREFIX)\"
endif

.PHONY: clean all engine $(ELIB)/$(ENGINELIB) $(ELIB)/$(EDITORLIB) $(AUDIOLIBROOT)/$(JFAUDIOLIB)

# TARGETS

# Invoking Make from the terminal in OSX just chains the build on to xcode
ifeq ($(PLATFORM),DARWIN)
ifeq ($(RELEASE),0)
style=Debug
else
style=Release
endif
.PHONY: alldarwin
alldarwin:
	cd osx && xcodebuild -target All -buildstyle $(style)
endif

all: sw$(EXESUFFIX) build$(EXESUFFIX)

sw$(EXESUFFIX): $(GAMEOBJS) $(ELIB)/$(ENGINELIB) $(AUDIOLIBROOT)/$(JFAUDIOLIB)
	$(CXX) $(CXXFLAGS) $(OURCXXFLAGS) $(OURCFLAGS) -o $@ $^ $(LIBS) $(GAMELIBS) -Wl,-Map=$@.map

build$(EXESUFFIX): $(EDITOROBJS) $(ELIB)/$(EDITORLIB) $(ELIB)/$(ENGINELIB)
	$(CXX) $(CXXFLAGS) $(OURCXXFLAGS) $(OURCFLAGS) -o $@ $^ $(LIBS) -Wl,-Map=$@.map

include Makefile.deps

.PHONY: enginelib editorlib
enginelib editorlib:
	$(MAKE) -C $(EROOT) \
		SUPERBUILD=$(SUPERBUILD) POLYMOST=$(POLYMOST) \
		USE_OPENGL=$(USE_OPENGL) DYNAMIC_OPENGL=$(DYNAMIC_OPENGL) \
		NOASM=$(NOASM) RELEASE=$(RELEASE) $@
$(EROOT)/generatesdlappicon$(EXESUFFIX):
	$(MAKE) -C $(EROOT) generatesdlappicon$(EXESUFFIX)

$(ELIB)/$(ENGINELIB): enginelib
$(ELIB)/$(EDITORLIB): editorlib
$(AUDIOLIBROOT)/$(JFAUDIOLIB):
	$(MAKE) -C $(AUDIOLIBROOT) RELEASE=$(RELEASE)

# RULES
$(SRC)/%.$o: $(SRC)/%.nasm
	nasm $(NASMFLAGS) $< -o $@

$(SRC)/%.$o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@
$(SRC)/%.$o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) $(OURCXXFLAGS) $(OURCFLAGS) -c $< -o $@
$(MACTROOT)/%.$o: $(MACTROOT)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(SRC)/%.$o: $(SRC)/%.rc
	$(RC) -i $< -o $@ --include-dir=$(EINC) --include-dir=$(SRC)

$(SRC)/%.$o: $(SRC)/util/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(RSRC)/%.$o: $(RSRC)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(RSRC)/%_gresource.c: $(RSRC)/%.gresource.xml
	glib-compile-resources --generate --manual-register --c-name=startgtk --target=$@ --sourcedir=$(RSRC) $<
$(RSRC)/%_gresource.h: $(RSRC)/%.gresource.xml
	glib-compile-resources --generate --manual-register --c-name=startgtk --target=$@ --sourcedir=$(RSRC) $<
$(RSRC)/sdlappicon_%.c: $(RSRC)/%.png $(EROOT)/generatesdlappicon$(EXESUFFIX)
	$(EROOT)/generatesdlappicon$(EXESUFFIX) $< > $@

# PHONIES
clean:
ifeq ($(PLATFORM),DARWIN)
	cd osx && xcodebuild -target All clean
else
	-rm -f $(GAMEOBJS) $(EDITOROBJS)
	$(MAKE) -C $(EROOT) clean
	$(MAKE) -C $(AUDIOLIBROOT) clean
endif

veryclean: clean
ifeq ($(PLATFORM),DARWIN)
else
	-rm -f sw$(EXESUFFIX) build$(EXESUFFIX) core*
	$(MAKE) -C $(EROOT) veryclean
endif

ifeq ($(PLATFORM),WINDOWS)
.PHONY: datainst
datainst:
	cd datainst && $(MAKE) GAME=SW
endif
