# Shadow Warrior Makefile for GNU Make

# Create Makefile.user yourself to provide your own overrides
# for configurable values
-include Makefile.user

##
##
## CONFIGURABLE OPTIONS
##
##

# Engine options
#  USE_POLYMOST   - enables Polymost renderer
#  USE_OPENGL     - enables OpenGL support in Polymost
#     Define as 0 to disable OpenGL
#     Define as USE_GL2 (or 1 or 2) for GL 2.0/2.1 profile
#     Define as USE_GL3 (or 3) for GL 3.2 Core profile
#     Define as USE_GLES2 (or 12) for GLES 2.0 profile
#  USE_ASM        - enables the use of assembly code if supported
#
USE_POLYMOST ?= 1
USE_OPENGL ?= 1
USE_ASM ?= 1

# Debugging options
#  RELEASE - 1 = optimised release build, no debugging aids
RELEASE ?= 1

# Path where game data is installed
DATADIR ?= /usr/local/share/games/jfsw

##
##
## HERE BE DRAGONS
##
##

ENGINEROOT=jfbuild
ENGINEINC=$(ENGINEROOT)/include
MACTROOT=jfmact
AUDIOLIBROOT=jfaudiolib
SRC=src
RSRC=rsrc

CC?=gcc
CXX?=g++
RC?=windres

OURCFLAGS=-g -W -Wall -fno-strict-aliasing -std=c99
OURCXXFLAGS=-g -W -Wall -fno-exceptions -fno-rtti -std=c++98
OURCPPFLAGS=-I$(SRC) -I$(ENGINEINC) -I$(MACTROOT) -I$(AUDIOLIBROOT)/include
OURLDFLAGS=

ifneq ($(RELEASE),0)
	# Default optimisation settings
	CFLAGS?=-fomit-frame-pointer -O2
	CXXFLAGS?=-fomit-frame-pointer -O2
else
	CFLAGS?=-Og
	CXXFLAGS?=-Og
endif

# Filename extensions, used in Makefile.deps etc
o=o
res=o

include $(ENGINEROOT)/Makefile.shared

# Apply shared flags
OURCFLAGS+= $(BUILDCFLAGS)
OURCXXFLAGS+= $(BUILDCXXFLAGS)
OURCPPFLAGS+= $(BUILDCPPFLAGS)
OURLDFLAGS+= $(BUILDLDFLAGS)

include $(AUDIOLIBROOT)/Makefile.shared

OURLDFLAGS+= $(JFAUDIOLIB_LDFLAGS)

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
	$(SRC)/osdcmds.$o \
	$(SRC)/osdfuncs.$o \
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
	$(SRC)/saveable.$o

GAMEOBJS+= \
	$(MACTROOT)/util_lib.$o \
	$(MACTROOT)/file_lib.$o \
	$(MACTROOT)/control.$o \
	$(MACTROOT)/keyboard.$o \
	$(MACTROOT)/mouse.$o \
	$(MACTROOT)/mathutil.$o \
	$(MACTROOT)/scriplib.$o \
	$(MACTROOT)/animlib.$o

EDITOROBJS=$(SRC)/jnstub.$o \
	$(SRC)/brooms.$o \
	$(SRC)/bldscript.$o \
	$(SRC)/jbhlp.$o \
	$(SRC)/colormap.$o

# Specialise for the platform
ifeq ($(PLATFORM),LINUX)
	OURLDFLAGS+= $(JFAUDIOLIB_LDFLAGS)
	OURCPPFLAGS+= -DDATADIR=\"$(DATADIR)\"
endif
ifeq ($(PLATFORM),BSD)
	OURLDFLAGS+= $(JFAUDIOLIB_LDFLAGS) -pthread
	OURCPPFLAGS+= -DDATADIR=\"$(DATADIR)\"
endif
ifeq ($(PLATFORM),WINDOWS)
	GAMEOBJS+= $(SRC)/gameres.$(res) \
		$(SRC)/startwin_game.$o
	EDITOROBJS+= $(SRC)/buildres.$(res)
endif
ifeq ($(PLATFORM),DARWIN)
	OURLDFLAGS+= -framework Foundation
endif

ifeq ($(RENDERTYPE),SDL)
	OURCFLAGS+= $(SDLCONFIG_CFLAGS)
	OURLDFLAGS+= $(SDLCONFIG_LIBS)

	ifeq (1,$(HAVE_GTK))
		OURCFLAGS+= $(GTKCONFIG_CFLAGS)
		OURLDFLAGS+= $(GTKCONFIG_LIBS)

		GAMEOBJS+= $(SRC)/startgtk_game.$o \
			$(RSRC)/startgtk_game_gresource.$o
		EDITOROBJS+= $(RSRC)/startgtk_build_gresource.$o
	endif

	GAMEOBJS+= $(RSRC)/game_bmp.$o
	EDITOROBJS+= $(RSRC)/build_bmp.$o
endif

# Source-control version stamping
ifneq (,$(findstring git version,$(shell git --version)))
GAMEOBJS+= $(SRC)/version-auto.$o
EDITOROBJS+= $(SRC)/version-auto.$o
else
GAMEOBJS+= $(SRC)/version.$o
EDITOROBJS+= $(SRC)/version.$o
endif

# TARGETS
.PHONY: clean veryclean all
all: sw$(EXESUFFIX) build$(EXESUFFIX)

include Makefile.deps

$(ENGINEROOT)/%:
	$(MAKE) -C $(@D) -f Makefile \
		USE_POLYMOST=$(USE_POLYMOST) \
		USE_OPENGL=$(USE_OPENGL) \
		USE_ASM=$(USE_ASM) \
		RELEASE=$(RELEASE) $(@F)

$(AUDIOLIBROOT)/%:
	$(MAKE) -C $(@D) \
		RELEASE=$(RELEASE) $(@F)

sw$(EXESUFFIX): $(GAMEOBJS) $(ENGINEROOT)/$(ENGINELIB) $(AUDIOLIBROOT)/$(JFAUDIOLIB)
	$(CXX) $(CPPFLAGS) $(OURCPPFLAGS) $(CXXFLAGS) $(OURCXXFLAGS) -o $@ $^ $(LDFLAGS) $(OURLDFLAGS)

build$(EXESUFFIX): $(EDITOROBJS) $(ENGINEROOT)/$(EDITORLIB) $(ENGINEROOT)/$(ENGINELIB)
	$(CXX) $(CPPFLAGS) $(OURCPPFLAGS) $(CXXFLAGS) $(OURCXXFLAGS) -o $@ $^ $(LDFLAGS) $(OURLDFLAGS)

# RULES
$(SRC)/%.$o: $(SRC)/%.c
	$(CC) $(CPPFLAGS) $(OURCPPFLAGS) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(SRC)/%.$o: $(SRC)/%.m
	$(CC) $(CPPFLAGS) $(OURCPPFLAGS) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(MACTROOT)/%.$o: $(MACTROOT)/%.c
	$(CC) $(CPPFLAGS) $(OURCPPFLAGS) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(RSRC)/%.$o: $(RSRC)/%.c
	$(CC) $(CPPFLAGS) $(OURCPPFLAGS) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(SRC)/%.$(res): $(SRC)/%.rc
	$(RC) -i $< -o $@ --include-dir=$(SRC) --include-dir=$(ENGINEINC)

$(RSRC)/%_gresource.c: $(RSRC)/%.gresource.xml
	glib-compile-resources --generate-source --manual-register --c-name=startgtk --target=$@ --sourcedir=$(RSRC) $<
$(RSRC)/%_gresource.h: $(RSRC)/%.gresource.xml
	glib-compile-resources --generate-header --manual-register --c-name=startgtk --target=$@ --sourcedir=$(RSRC) $<

$(RSRC)/%_bmp.c: $(RSRC)/%.bmp | $(ENGINEROOT)/bin2c$(EXESUFFIX)
	$(ENGINEROOT)/bin2c$(EXESUFFIX) $< appicon_bmp > $@
$(RSRC)/%.bmp: $(RSRC)/%.png
	$(shell which magick convert true | head -1) $< $@

# PHONIES
clean::
	-rm -f $(GAMEOBJS) $(EDITOROBJS)
veryclean:: clean
	-rm -f sw$(EXESUFFIX) build$(EXESUFFIX) core*

clean::
	$(MAKE) -C $(ENGINEROOT) $@
	$(MAKE) -C $(AUDIOLIBROOT) $@
veryclean::
	$(MAKE) -C $(ENGINEROOT) $@

.PHONY: $(SRC)/version-auto.c
$(SRC)/version-auto.c:
	printf "const char *game_version = \"%s\";\n" "$(shell git describe --always || echo git error)" > $@
	echo "const char *game_date = __DATE__;" >> $@
	echo "const char *game_time = __TIME__;" >> $@
