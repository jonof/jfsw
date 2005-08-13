# Shadow Warrior Makefile for GNU Make

SRC=source/
RSRC=rsrc/
OBJ=obj/
EROOT=../build/
EINC=$(EROOT)include/
EOBJ=eobj/
INC=$(SRC)
o=o

# debugging enabled
debug=-ggdb -O0
# debugging disabled
#debug=-fomit-frame-pointer -O1


DXROOT=c:/sdks/msc/dx61
#FMODROOT=c:/mingw32/fmodapi360win32/api

# SETSPRITEZ is mandatory!
ENGINEOPTS=-DSETSPRITEZ -DSUPERBUILD -DPOLYMOST -DUSE_OPENGL -DDYNAMIC_OPENGL

CC=gcc
CFLAGS=$(debug) -W -Wall -Wimplicit \
	-Wno-char-subscripts -Wno-unused \
	-funsigned-char -fno-strict-aliasing -march=pentium -DNO_GCC_BUILTINS \
	-I$(INC:/=) -I$(EINC:/=) -I$(SRC)jmact -I$(SRC)jaudiolib -I../jfaud/inc \
	$(ENGINEOPTS) \
	-DUSE_GCC_PRAGMAS
LIBS=-lm #../jfaud/jfaud.a # -L../jfaud -ljfaud
NASMFLAGS=-s #-g
EXESUFFIX=

JMACTOBJ=$(OBJ)util_lib.$o \
	$(OBJ)file_lib.$o \
	$(OBJ)control.$o \
	$(OBJ)keyboard.$o \
	$(OBJ)mouse.$o \
	$(OBJ)mathutil.$o \
	$(OBJ)scriplib.$o \
	$(OBJ)animlib.$o

AUDIOLIB_FX_STUB=$(OBJ)audiolib_fxstub.$o
AUDIOLIB_MUSIC_STUB=$(OBJ)audiolib_musicstub.$o
#AUDIOLIB_FX=$(OBJ)audiolib_fx_fmod.$o
AUDIOLIB_FX=$(OBJ)mv_mix.$o \
	  $(OBJ)mv_mix16.$o \
	  $(OBJ)mvreverb.$o \
	  $(OBJ)pitch.$o \
	  $(OBJ)multivoc.$o \
	  $(OBJ)ll_man.$o \
	  $(OBJ)fx_man.$o \
	  $(OBJ)dsoundout.$o
AUDIOLIB_MUSIC=$(OBJ)midi.$o \
	  $(OBJ)mpu401.$o \
	  $(OBJ)music.$o

GAMEOBJS= \
	$(OBJ)actor.$o \
	$(OBJ)ai.$o \
	$(OBJ)anim.$o \
	$(OBJ)border.$o \
	$(OBJ)break.$o \
	$(OBJ)bunny.$o \
	$(OBJ)cache.$o \
	$(OBJ)cd.$o \
	$(OBJ)cheats.$o \
	$(OBJ)colormap.$o \
	$(OBJ)config.$o \
	$(OBJ)console.$o \
	$(OBJ)coolg.$o \
	$(OBJ)coolie.$o \
	$(OBJ)copysect.$o \
	$(OBJ)demo.$o \
	$(OBJ)draw.$o \
	$(OBJ)eel.$o \
	$(OBJ)game.$o \
	$(OBJ)girlninj.$o \
	$(OBJ)goro.$o \
	$(OBJ)hornet.$o \
	$(OBJ)interp.$o \
	$(OBJ)interpsh.$o \
	$(OBJ)inv.$o \
	$(OBJ)jplayer.$o \
	$(OBJ)jsector.$o \
	$(OBJ)jweapon.$o \
	$(OBJ)lava.$o \
	$(OBJ)light.$o \
	$(OBJ)mclip.$o \
	$(OBJ)mdastr.$o \
	$(OBJ)menus.$o \
	$(OBJ)miscactr.$o \
	$(OBJ)morph.$o \
	$(OBJ)net.$o \
	$(OBJ)ninja.$o \
	$(OBJ)panel.$o \
	$(OBJ)player.$o \
	$(OBJ)predict.$o \
	$(OBJ)quake.$o \
	$(OBJ)ripper.$o \
	$(OBJ)ripper2.$o \
	$(OBJ)rooms.$o \
	$(OBJ)rotator.$o \
	$(OBJ)rts.$o \
	$(OBJ)save.$o \
	$(OBJ)scrip2.$o \
	$(OBJ)sector.$o \
	$(OBJ)serp.$o \
	$(OBJ)setup.$o \
	$(OBJ)skel.$o \
	$(OBJ)skull.$o \
	$(OBJ)slidor.$o \
	$(OBJ)sounds.$o \
	$(OBJ)spike.$o \
	$(OBJ)sprite.$o \
	$(OBJ)sumo.$o \
	$(OBJ)swconfig.$o \
	$(OBJ)sync.$o \
	$(OBJ)text.$o \
	$(OBJ)track.$o \
	$(OBJ)usrhooks.$o \
	$(OBJ)vator.$o \
	$(OBJ)vis.$o \
	$(OBJ)wallmove.$o \
	$(OBJ)warp.$o \
	$(OBJ)weapon.$o \
	$(OBJ)zilla.$o \
	$(OBJ)zombie.$o \
	$(OBJ)saveable.$o \
	$(JMACTOBJ)

EDITOROBJS=$(OBJ)jnstub.$o \
	$(OBJ)brooms.$o \
	$(OBJ)bldscript.$o \
	$(OBJ)jbhlp.$o \
	$(OBJ)colormap.$o

include $(EROOT)Makefile.shared

ifeq ($(PLATFORM),LINUX)
	NASMFLAGS+= -f elf
endif
ifeq ($(PLATFORM),WINDOWS)
	override CFLAGS+= -DUNDERSCORES -I$(DXROOT)/include
	NASMFLAGS+= -DUNDERSCORES -f win32
	GAMEOBJS+= $(OBJ)cda_win32.$o $(OBJ)gameres.$o #$(OBJ)winbits.$o
	EDITOROBJS+= $(OBJ)buildres.$o
endif

ifeq ($(RENDERTYPE),SDL)
	override CFLAGS+= $(subst -Dmain=SDL_main,,$(shell sdl-config --cflags))
	AUDIOLIBOBJ=$(AUDIOLIB_MUSIC_STUB) $(AUDIOLIB_FX_STUB)

	ifeq (1,$(HAVE_GTK2))
		override CFLAGS+= -DHAVE_GTK2 $(shell pkg-config --cflags gtk+-2.0)
		GAMEOBJS+= $(OBJ)game_banner.$o
		EDITOROBJS+= $(OBJ)editor_banner.$o
	endif

	GAMEOBJS+= $(OBJ)game_icon.$o
	EDITOROBJS+= $(OBJ)build_icon.$o
endif
ifeq ($(RENDERTYPE),WIN)
	AUDIOLIBOBJ=$(AUDIOLIB_MUSIC) $(AUDIOLIB_FX)
endif

GAMEOBJS+= $(AUDIOLIBOBJ)

.PHONY: clean all engine $(EOBJ)$(ENGINELIB) $(EOBJ)$(EDITORLIB)

# TARGETS
all: sw$(EXESUFFIX) build$(EXESUFFIX)

sw$(EXESUFFIX): $(GAMEOBJS) $(EOBJ)$(ENGINELIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -Wl,-Map=$@.map
#	-rm sw.sym$(EXESUFFIX)
#	cp sw$(EXESUFFIX) sw.sym$(EXESUFFIX)
#	strip sw$(EXESUFFIX)
	
build$(EXESUFFIX): $(EDITOROBJS) $(EOBJ)$(EDITORLIB) $(EOBJ)$(ENGINELIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -Wl,-Map=$@.map
#	-rm build.sym$(EXESUFFIX)
#	cp build$(EXESUFFIX) build.sym$(EXESUFFIX)
#	strip build$(EXESUFFIX)

include Makefile.deps

$(EOBJ)$(ENGINELIB):
	-mkdir $(EOBJ)
	$(MAKE) -C $(EROOT) "OBJ=$(CURDIR)/$(EOBJ)" "CFLAGS=$(ENGINEOPTS)" enginelib

$(EOBJ)$(EDITORLIB):
	-mkdir $(EOBJ)
	$(MAKE) -C $(EROOT) "OBJ=$(CURDIR)/$(EOBJ)" "CFLAGS=$(ENGINEOPTS)" editorlib

# RULES
$(OBJ)%.$o: $(SRC)%.nasm
	nasm $(NASMFLAGS) $< -o $@
$(OBJ)%.$o: $(SRC)jaudiolib/%.nasm
	nasm $(NASMFLAGS) $< -o $@

$(OBJ)%.$o: $(SRC)%.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>&1
$(OBJ)%.$o: $(SRC)jmact/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>&1
$(OBJ)%.$o: $(SRC)jaudiolib/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>&1

$(OBJ)%.$o: $(SRC)misc/%.rc
	windres -i $^ -o $@

$(OBJ)%.$o: $(SRC)util/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>&1

$(OBJ)%.$o: $(RSRC)%.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>&1

$(OBJ)game_banner.$o: $(RSRC)game_banner.c
$(OBJ)editor_banner.$o: $(RSRC)editor_banner.c
$(RSRC)game_banner.c: $(RSRC)game.bmp
	echo "#include <gdk-pixbuf/gdk-pixdata.h>" > $@
	gdk-pixbuf-csource --extern --struct --raw --name=startbanner_pixdata $^ | sed 's/load_inc//' >> $@
$(RSRC)editor_banner.c: $(RSRC)build.bmp
	echo "#include <gdk-pixbuf/gdk-pixdata.h>" > $@
	gdk-pixbuf-csource --extern --struct --raw --name=startbanner_pixdata $^ | sed 's/load_inc//' >> $@

# PHONIES	
clean:
	-rm -f $(OBJ)* sw$(EXESUFFIX) sw.sym$(EXESUFFIX) build$(EXESUFFIX) build.sym$(EXESUFFIX) core*
	
veryclean: clean
	-rm -f $(EOBJ)*


