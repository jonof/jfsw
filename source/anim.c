//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "build.h"
#include "compat.h"
#include "cache1d.h"

#include "keys.h"
#include "mytypes.h"
#include "develop.h"
#include "fx_man.h"
#include "music.h"
#include "scriplib.h"
#include "file_lib.h"
#undef MAXPLAYERS
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"

#include "control.h"
#include "config.h"
#include "sounds.h"
#include "function.h"

#include "game.h"
#include "colormap.h"
#include "net.h"

#include "animlib.h"
#include "anim.h"

#define MAX_ANMS 10
anim_t *anm_ptr[MAX_ANMS];

long ANIMnumframes;
char ANIMpal[3*256];
char ANIMnum = 0;
short SoundState;

char *ANIMname[] = 
{
"sw.anm",
"swend.anm",
"sumocinm.anm",
"zfcin.anm",
};

#define ANIM_TILE(num) (MAXTILES-11 + (num))

VOID AnimShareIntro(int frame, int numframes)
    {
    long zero=0;
        
    if (frame == numframes-1)
        ototalclock += 120;
    else    
    if (frame == 1)
        {
        PlaySound(DIGI_NOMESSWITHWANG,&zero,&zero,&zero,v3df_none);
        ototalclock += 120*3;
        }
    else    
        ototalclock += 8;
    
    if (frame == 5)    
        {
        PlaySound(DIGI_INTRO_SLASH,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 15)    
        {
        PlaySound(DIGI_INTRO_WHIRL,&zero,&zero,&zero,v3df_none);
        }
    }

VOID AnimSerp(int frame, int numframes)
    {
    long zero=0;
    ototalclock += 16;
    
    if (frame == numframes-1)
        ototalclock += 1*120;

    if (frame == 1)
        {
        PlaySound(DIGI_SERPTAUNTWANG,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 16)
        {
        PlaySound(DIGI_SHAREND_TELEPORT,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 35)
        {
        SoundState++;
        PlaySound(DIGI_WANGTAUNTSERP1,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 51)
        {
        SoundState++;
        PlaySound(DIGI_SHAREND_UGLY1,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 64)
        {
        SoundState++;
        PlaySound(DIGI_SHAREND_UGLY2,&zero,&zero,&zero,v3df_none);
        }
    }

VOID AnimSumo(int frame, int numframes)
    {
    long zero=0;
    ototalclock += 10;

    if (frame == numframes-1)
        ototalclock += 1*120;
        
    if (frame == 1)
        ototalclock += 30;
    
    if (frame == 2)
        {
        // hungry
        PlaySound(DIGI_JG41012,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 30)
        {
        PlaySound(DIGI_HOTHEADSWITCH,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 42)
        {
        PlaySound(DIGI_HOTHEADSWITCH,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 59)
        {
        PlaySound(DIGI_JG41028,&zero,&zero,&zero,v3df_none);
        }
    }

VOID AnimZilla(int frame, int numframes)
    {
    long zero=0;
    ototalclock += 16;
    
    if (frame == numframes-1)
        ototalclock += 1*120;

    if (frame == 1)
        {
        PlaySound(DIGI_ZC1,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 5)
        {
        PlaySound(DIGI_JG94024,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 14)
        {
        PlaySound(DIGI_ZC2,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 30)
        {
        PlaySound(DIGI_ZC3,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 32)
        {
        PlaySound(DIGI_ZC4,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 37)
        {
        PlaySound(DIGI_ZC5,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 63)
        {
        PlaySound(DIGI_Z16043,&zero,&zero,&zero,v3df_none);
        PlaySound(DIGI_ZC6,&zero,&zero,&zero,v3df_none);
        PlaySound(DIGI_ZC7,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 72)
        {
        PlaySound(DIGI_ZC7,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 73)
        {
        PlaySound(DIGI_ZC4,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 77)
        {
        PlaySound(DIGI_ZC5,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 87)
        {
        PlaySound(DIGI_ZC8,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 103)
        {
        PlaySound(DIGI_ZC7,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 108)
        {
        PlaySound(DIGI_ZC9,&zero,&zero,&zero,v3df_none);
        }
    else    
    if (frame == 120)
        {
        PlaySound(DIGI_JG94039,&zero,&zero,&zero,v3df_none);
        }
    }
    
char * LoadAnm(short anim_num)
    {
    long handle;
    long length;
    char *animbuf, *palptr;
    long i,j,k;
    
    DSPRINTF(ds,"LoadAnm");
    MONO_PRINT(ds);

    // this seperate allows the anim to be precached easily
    
    ANIMnum = anim_num;
    
    // lock it
    walock[ANIM_TILE(ANIMnum)] = 219;

    if (anm_ptr[anim_num] == 0)
        {
        handle = kopen4load(ANIMname[ANIMnum], 0);
        if (handle == -1)
            return(NULL);
        length = kfilelength(handle);
        
        allocache((long *) &anm_ptr[anim_num], length + sizeof(anim_t), &walock[ANIM_TILE(ANIMnum)]);
        animbuf = (char *) (FP_OFF(anm_ptr[anim_num]) + sizeof(anim_t));
        
        kread(handle, animbuf, length);
        kclose(handle);
        }
    else    
        {
        animbuf = (char *) (FP_OFF(anm_ptr[anim_num]) + sizeof(anim_t));
        }
    
    return(animbuf);
    }    

void 
playanm(short anim_num)
    {
    char *animbuf, *palptr;
    long i, j, k, length = 0, numframes = 0;
    int32 handle = -1;
    char ANIMvesapal[4*256];
    char backup_pal[3*256];
    char tempbuf[256];
    char *palook_bak = palookup[0];

    ANIMnum = anim_num;

    KB_FlushKeyboardQueue();    
    KB_ClearKeysDown();    
    
    DSPRINTF(ds,"PlayAnm");
    MONO_PRINT(ds);

    DSPRINTF(ds,"PlayAnm");
    MONO_PRINT(ds);

    animbuf = LoadAnm(anim_num);
    if (!animbuf)    
        return;
    
    DSPRINTF(ds,"PlayAnm - Palette Stuff");
    MONO_PRINT(ds);

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    palookup[0] = tempbuf;    
    GetPaletteFromVESA(backup_pal);
        
    ANIM_LoadAnim(animbuf);
    ANIMnumframes = ANIM_NumFrames();
    numframes = ANIMnumframes;
    
    palptr = ANIM_GetPalette();
    for (i = 0; i < 256; i++)
        {
	/*
        j = (i << 2);
        k = j - i;
        ANIMvesapal[j + 0] = (palptr[k + 2] >> 2);
        ANIMvesapal[j + 1] = (palptr[k + 1] >> 2);
        ANIMvesapal[j + 2] = (palptr[k + 0] >> 2);
        ANIMvesapal[j + 3] = 0;
	*/
	j = i*3;
        ANIMvesapal[j + 0] = (palptr[j + 0] >> 2);
        ANIMvesapal[j + 1] = (palptr[j + 1] >> 2);
        ANIMvesapal[j + 2] = (palptr[j + 2] >> 2);
        }

    tilesizx[ANIM_TILE(ANIMnum)] = 200;
    tilesizy[ANIM_TILE(ANIMnum)] = 320;

    if (ANIMnum == 1)
        {
        // draw the first frame
        waloff[ANIM_TILE(ANIMnum)] = FP_OFF(ANIM_DrawFrame(1));
	invalidatetile(ANIM_TILE(ANIMnum), 0, 1<<4);
        rotatesprite(0 << 16, 0 << 16, 65536L, 512, ANIM_TILE(ANIMnum), 0, 0, 2 + 4 + 8 + 16 + 64, 0, 0, xdim - 1, ydim - 1);
             }
    //VBE_setPalette(0L, 256L, ANIMvesapal);
    setbrightness(0,ANIMvesapal,4|2);

    SoundState = 0;
    //ototalclock = totalclock + 120*2;
    ototalclock = totalclock;

    for (i = 1; i < numframes; i++)
        {
        while (totalclock < ototalclock)
            {
            switch (ANIMnum)
                {
                case 0:
                    if (KB_KeyWaiting())
                        goto ENDOFANIMLOOP;
                    break;
                case 1:
                    if (KEY_PRESSED(KEYSC_ESC))
                        goto ENDOFANIMLOOP;
                    break;
                }    
                
            getpackets();
            }

        switch (ANIMnum)
            {
            case ANIM_INTRO:
                AnimShareIntro(i,numframes);    
                break;
            case ANIM_SERP:
                AnimSerp(i,numframes);    
                break;
            case ANIM_SUMO:
                AnimSumo(i,numframes);    
                break;
            case ANIM_ZILLA:
                AnimZilla(i,numframes);    
                break;
            }    
            
        waloff[ANIM_TILE(ANIMnum)] = FP_OFF(ANIM_DrawFrame(i));
	invalidatetile(ANIM_TILE(ANIMnum), 0, 1<<4);

        rotatesprite(0 << 16, 0 << 16, 65536L, 512, ANIM_TILE(ANIMnum), 0, 0, 2 + 4 + 8 + 16 + 64, 0, 0, xdim - 1, ydim - 1);
        nextpage();
        }

    // pause on final frame    
    while (totalclock < ototalclock)
        getpackets();
        
ENDOFANIMLOOP:

    //GetPaletteFromVESA(palette_data);
    //FadeOut(0,5);
    //memcpy(palette_data, backup_pal, 3*256);
    clearview(0);
    nextpage();
    palookup[0] = palook_bak;
    SetPaletteToVESA(backup_pal);
    //GetPaletteFromVESA(palette_data);
    //FadeOut(0,0);
    clearview(0);
    nextpage();
    
    KB_FlushKeyboardQueue();    
    KB_ClearKeysDown();    
    ANIM_FreeAnim();
    walock[ANIM_TILE(ANIMnum)] = 1;
    }
