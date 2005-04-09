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

#define MAIN
#define QUIET
#include "build.h"
#include "compat.h"

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"
#include "tags.h"
#include "lists.h"
#include "interp.h"

#include "net.h"
//#include "save.h"
#include "savedef.h"
#include "symutil.h"
#include "jsector.h"
#include "parent.h"
#include "reserve.h"

//#define FILE_TYPE 1
#include "mfile.h"

#include "fx_man.h"
#include "music.h"

#include "weapon.h"
#include "cache.h"
#include "colormap.h"
#include "player.h"

//void TimerFunc(task * Task);

/*
//////////////////////////////////////////////////////////////////////////////
TO DO


//////////////////////////////////////////////////////////////////////////////
*/

extern long lastUpdate;
extern BYTE RedBookSong[40];
extern char UserMapName[80];
extern char palette[256*3];
extern char LevelSong[16];
extern char SaveGameDescr[10][80];
extern long PlayClock;
extern short TotalKillable;
extern short LevelSecrets;
extern short Bunny_Count;
extern BOOL NewGame;
extern char CacheLastLevel[];
extern short PlayingLevel;
extern int GodMode;
//extern short Zombies;

extern BOOL serpwasseen;
extern BOOL sumowasseen;
extern BOOL zillawasseen;
extern short BossSpriteNum[3];

VOID ScreenTileLock(void);
VOID ScreenTileUnLock(void);

int ScreenSaveSetup(PLAYERp pp);
VOID ScreenSave(FILE *fout);

int ScreenLoadSaveSetup(PLAYERp pp);
VOID ScreenLoad(FILE *fin);

#define PANEL_SAVE 1
#define ANIM_SAVE 1
    
extern SW_PACKET loc;
extern char LevelName[20];
extern STATE s_NotRestored[];

OrgTileListP otlist[] = {&orgwalllist, &orgwalloverlist, &orgsectorceilinglist, &orgsectorfloorlist};

int 
PanelSpriteToNdx(PLAYERp pp, PANEL_SPRITEp psprite)
    {
    short ndx = 0;
    PANEL_SPRITEp psp=NULL, next=NULL;

    TRAVERSE(&pp->PanelSpriteList, psp, next)
        {
        if (psp == psprite)
            return(ndx);
            
        ndx++;
        }

    // special case for pointing to the list head
    if ((LIST)psprite == (LIST)&pp->PanelSpriteList)
        return(9999);

    return(-1);    
    }    

    
PANEL_SPRITEp
PanelNdxToSprite(PLAYERp pp, long ndx)
    {
    short count = 0;
    PANEL_SPRITEp psp, next;

    if (ndx == -1)
        return(NULL);

    if (ndx == 9999)
        return((PANEL_SPRITEp)&pp->PanelSpriteList);

    TRAVERSE(&pp->PanelSpriteList, psp, next)
        {
        if (count == ndx)
            return(psp);
            
        count++;
        }

    return(NULL);    
    }    

void SaveSymDataInfo(MFILE fil, void *ptr)    
    {
    unsigned long unrelocated_offset, offset_from_symbol;
    SYM_TABLEp st_ptr;
    char sym_name[80];
    
    if (!ptr)
        {
        offset_from_symbol = -1;
        strcpy(sym_name, "NULL");
        }
    else
        {    
        unrelocated_offset = SymDataPtrToOffset(ptr);
        st_ptr = SearchSymTableByOffset(SymTableData, SymCountData, unrelocated_offset, &offset_from_symbol);
        ASSERT(st_ptr);
        strcpy(sym_name, st_ptr->Name);
        }
    
    MWRITE(sym_name, sizeof(st_ptr->Name), 1, fil);
    MWRITE(&offset_from_symbol, sizeof(offset_from_symbol), 1, fil);
    }

void SaveSymCodeInfo(MFILE fil, void *ptr)    
    {
    unsigned long unrelocated_offset, offset_from_symbol;
    SYM_TABLEp st_ptr;
    char sym_name[80];
    unsigned long test;
    
    if (!ptr)
        {
        offset_from_symbol = -1;
        strcpy(sym_name, "NULL");
        }
    else
        {
        #if 0
        unrelocated_offset = SymCodePtrToOffset((void*)TimerFunc);

        st_ptr = SearchSymTableByOffset(SymTableCode, SymCountCode, unrelocated_offset, &offset_from_symbol);
        ASSERT(st_ptr);
        strcpy(sym_name, st_ptr->Name);
        #endif
        
        unrelocated_offset = SymCodePtrToOffset(ptr);

        st_ptr = SearchSymTableByOffset(SymTableCode, SymCountCode, unrelocated_offset, &offset_from_symbol);
        ASSERT(st_ptr);
        strcpy(sym_name, st_ptr->Name);
        }
        
    MWRITE(sym_name, sizeof(st_ptr->Name), 1, fil);
    MWRITE(&offset_from_symbol, sizeof(offset_from_symbol), 1, fil);
    }

//#define LoadSymDataInfo(fil) _LoadSymDataInfo(__FILE__,__LINE__,fil)    
//#define LoadSymCodeInfo(fil) _LoadSymCodeInfo(__FILE__,__LINE__,fil)    
    
//void *_LoadSymDataInfo(char *file, long line, MFILE fil)    
void *LoadSymDataInfo(MFILE fil)    
    {
    unsigned long offset_from_symbol;
    char sym_name[80];
    SYM_TABLEp st_ptr;
    void *data_ptr;

    MREAD(&sym_name, sizeof(st_ptr->Name), 1, fil);
    MREAD(&offset_from_symbol, sizeof(offset_from_symbol), 1, fil);
    
    if (offset_from_symbol == -1)
        return(NULL);
    
    st_ptr = SearchSymTableByName(SymTableData, SymCountData, sym_name);
    ASSERT(st_ptr);
    data_ptr = SymOffsetToDataPtr(st_ptr->Offset + offset_from_symbol);
    
    return(data_ptr);
    }

//void *_LoadSymCodeInfo(char *file, long line, MFILE fil)    
void *LoadSymCodeInfo(MFILE fil)    
    {
    unsigned long offset_from_symbol;
    char sym_name[80];
    SYM_TABLEp st_ptr;
    void *code_ptr;

    MREAD(&sym_name, sizeof(st_ptr->Name), 1, fil);
    MREAD(&offset_from_symbol, sizeof(offset_from_symbol), 1, fil);

    if (offset_from_symbol == -1)
        return(NULL);
        
    st_ptr = SearchSymTableByName(SymTableCode, SymCountCode, sym_name);
    ASSERT(st_ptr);
    code_ptr = SymOffsetToCodePtr(st_ptr->Offset + offset_from_symbol);
    
    return(code_ptr);
    }
    
int SaveGame(short save_num)
{
    MFILE fil;
    long i,j;
    short ndx;
    SPRITE tsp;
    SPRITEp sp;
    PLAYER tp;
    PLAYERp pp;
    SECT_USERp sectu;
    USER tu;
    USERp u;
    ANIM tanim;
    ANIMp a;
    CHAR code;
    BYTE data_code;
    SHORT data_ndx;
    PANEL_SPRITE tpanel_sprite;
    PANEL_SPRITEp psp,cur,next;
    SECTOR_OBJECTp sop;
    char game_name[80];
    long cnt = 0;
    OrgTileP otp, next_otp;

    LoadSymTable("swdata.sym", &SymTableData, &SymCountData);
    LoadSymTable("swcode.sym", &SymTableCode, &SymCountCode);
    
    if (SymCountData <= 0 || SymCountCode <= 0)
        return(-1);

    sprintf(game_name,"game%d.sav",save_num);
    if ((fil = MOPEN_WRITE(game_name)) == MF_ERR)
        return(-1);

    MWRITE(SaveGameDescr[save_num],sizeof(SaveGameDescr[save_num]),1,fil);
    
    MWRITE(&Level,sizeof(Level),1,fil);
    MWRITE(&Skill,sizeof(Skill),1,fil);
    
    ScreenSaveSetup(&Player[myconnectindex]);

    ScreenSave(fil);

    ScreenTileUnLock();
    
    MWRITE(&numplayers,sizeof(numplayers),1,fil);
    MWRITE(&myconnectindex,sizeof(myconnectindex),1,fil);
    MWRITE(&connecthead,sizeof(connecthead),1,fil);
    MWRITE(connectpoint2,sizeof(connectpoint2),1,fil);
    
    //save players info
    pp = &tp;
    for (i = 0; i < numplayers; i++)
        {
        memcpy(&tp, &Player[i], sizeof(PLAYER));
        
        // this does not point to global data - this is allocated link list based
        // save this inside the structure
        #if PANEL_SAVE
        pp->CurWpn = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->CurWpn);
        for (ndx = 0; ndx < MAX_WEAPONS; ndx++)
            pp->Wpn[ndx] = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->Wpn[ndx]);
        pp->Chops = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->Chops);
        for (ndx = 0; ndx < MAX_INVENTORY; ndx++)
            pp->InventorySprite[ndx] = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->InventorySprite[ndx]);
        pp->InventorySelectionBox = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->InventorySelectionBox);
        pp->MiniBarHealthBox = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->MiniBarHealthBox);
        pp->MiniBarAmmo = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->MiniBarAmmo);
        for (ndx = 0; ndx < SIZ(pp->MiniBarHealthBoxDigit); ndx++)
            pp->MiniBarHealthBoxDigit[ndx] = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->MiniBarHealthBoxDigit[ndx]);
        for (ndx = 0; ndx < SIZ(pp->MiniBarAmmoDigit); ndx++)
            pp->MiniBarAmmoDigit[ndx] = (PANEL_SPRITEp)PanelSpriteToNdx(&Player[i], pp->MiniBarAmmoDigit[ndx]);
        #endif    
        
        MWRITE(&tp, sizeof(PLAYER),1,fil);
        
        //////
     
        SaveSymDataInfo(fil, pp->remote_sprite);
        SaveSymDataInfo(fil, pp->remote.sop_control);
        SaveSymDataInfo(fil, pp->sop_remote);
        SaveSymDataInfo(fil, pp->sop);
        SaveSymDataInfo(fil, pp->hi_sectp);
        SaveSymDataInfo(fil, pp->lo_sectp);
        SaveSymDataInfo(fil, pp->hi_sp);
        SaveSymDataInfo(fil, pp->lo_sp);
        
        SaveSymDataInfo(fil, pp->last_camera_sp);
        SaveSymDataInfo(fil, pp->SpriteP);
        SaveSymDataInfo(fil, pp->UnderSpriteP);
        
        SaveSymCodeInfo(fil, pp->DoPlayerAction);
        
        SaveSymDataInfo(fil, pp->sop_control);
        SaveSymDataInfo(fil, pp->sop_riding);
        }

    #if PANEL_SAVE
    // local copy
    psp = &tpanel_sprite;
    for (i = 0; i < numplayers; i++)
        {
        int j;
        pp = &Player[i];
        ndx = 0;
        
        TRAVERSE(&pp->PanelSpriteList, cur, next)
            {
            // this is a HEADER
            MWRITE(&ndx, sizeof(ndx),1,fil);
            
            memcpy(psp, cur, sizeof(PANEL_SPRITE));
            
            // Panel Sprite - save in structure
            psp->sibling = (PANEL_SPRITEp)PanelSpriteToNdx(pp, cur->sibling);
            MWRITE(psp, sizeof(PANEL_SPRITE),1,fil);
          
            SaveSymDataInfo(fil, psp->PlayerP);
            SaveSymDataInfo(fil, psp->State);
            SaveSymDataInfo(fil, psp->RetractState);
            SaveSymDataInfo(fil, psp->PresentState);
            SaveSymDataInfo(fil, psp->ActionState);
            SaveSymDataInfo(fil, psp->RestState);
            SaveSymCodeInfo(fil, psp->PanelSpriteFunc);
            
            for (j = 0; j < SIZ(psp->over); j++)
                {
                SaveSymDataInfo(fil, psp->over[j].State);
                }
                      
            ndx++;
            }
            
        // store -1 when done for player
        ndx = -1;    
        MWRITE(&ndx, sizeof(ndx),1,fil);
        }
    #endif    
        
    MWRITE(&numsectors,sizeof(numsectors),1,fil);
    MWRITE(sector,sizeof(SECTOR), numsectors, fil);

    //Sector User information
    for (i = 0; i < numsectors; i++)
        {
        sectu = SectUser[i];
        ndx = i;
        if (sectu)         
            {
            // write header
            MWRITE(&ndx,sizeof(ndx),1,fil);
            
            MWRITE(sectu,sizeof(SECT_USER),1,fil);
            }
        else 
            {
            // write trailer
            ndx = -1;
            MWRITE(&ndx,sizeof(ndx),1,fil);
            }
        }

    MWRITE(&numwalls,sizeof(numwalls),1,fil);
    MWRITE(wall,sizeof(WALL),numwalls,fil);

    for (i = 0; i < MAXSPRITES; i++)
        {
        if (sprite[i].statnum != MAXSTATUS)         
            {
            MWRITE(&i,sizeof(i),1,fil);
            
            MWRITE(&sprite[i],sizeof(SPRITE),1,fil);
            }    
        }
    i = -1;    
    MWRITE(&i,sizeof(i),1,fil);
    
    MWRITE(headspritesect,sizeof(headspritesect),1,fil);
    MWRITE(prevspritesect,sizeof(prevspritesect),1,fil);
    MWRITE(nextspritesect,sizeof(nextspritesect),1,fil);
    MWRITE(headspritestat,sizeof(headspritestat),1,fil);
    MWRITE(prevspritestat,sizeof(prevspritestat),1,fil);
    MWRITE(nextspritestat,sizeof(nextspritestat),1,fil);
    
    //User information
    for (i = 0; i < MAXSPRITES; i++)
        {
        ndx = i;
        if (User[i])         
            {
            // write header
            MWRITE(&ndx,sizeof(ndx),1,fil);
            
            sp = &sprite[i];
            memcpy(&tu, User[i], sizeof(USER));
            u = &tu;
            
            MWRITE(u,sizeof(USER),1,fil);
            
            if (u->WallShade)
                {
                MWRITE(u->WallShade,sizeof(*u->WallShade)*u->WallCount,1,fil);
                }    
            
            if (u->rotator)
                {
                MWRITE(u->rotator,sizeof(*u->rotator),1,fil);
                if (u->rotator->origx)
                    MWRITE(u->rotator->origx,sizeof(*u->rotator->origx)*u->rotator->num_walls,1,fil);
                if (u->rotator->origy)
                    MWRITE(u->rotator->origy,sizeof(*u->rotator->origy)*u->rotator->num_walls,1,fil);
                }    
            
            SaveSymDataInfo(fil, u->WallP);
            SaveSymDataInfo(fil, u->State);
            SaveSymDataInfo(fil, u->Rot);
            SaveSymDataInfo(fil, u->StateStart);
            SaveSymDataInfo(fil, u->StateEnd);
            SaveSymDataInfo(fil, u->StateFallOverride);
            SaveSymCodeInfo(fil, u->ActorActionFunc);
            SaveSymDataInfo(fil, u->ActorActionSet);
            SaveSymDataInfo(fil, u->Personality);
            SaveSymDataInfo(fil, u->Attrib);
            SaveSymDataInfo(fil, u->sop_parent);
            SaveSymDataInfo(fil, u->hi_sectp);
            SaveSymDataInfo(fil, u->lo_sectp);
            SaveSymDataInfo(fil, u->hi_sp);
            SaveSymDataInfo(fil, u->lo_sp);
            SaveSymDataInfo(fil, u->SpriteP);
            SaveSymDataInfo(fil, u->PlayerP);
            SaveSymDataInfo(fil, u->tgt_sp);
            }
        }
    ndx = -1;
    MWRITE(&ndx,sizeof(ndx),1,fil);
    
    //
    // Sector object
    //    

    MWRITE(SectorObject, sizeof(SectorObject),1,fil);
    
    for (ndx = 0; ndx < SIZ(SectorObject); ndx++)
        {
        sop = &SectorObject[ndx];
        
        SaveSymCodeInfo(fil, sop->PreMoveAnimator);
        SaveSymCodeInfo(fil, sop->PostMoveAnimator);
        SaveSymCodeInfo(fil, sop->Animator);
        SaveSymDataInfo(fil, sop->controller);
        SaveSymDataInfo(fil, sop->sp_child);
        }
    
    
    MWRITE(SineWaveFloor, sizeof(SineWaveFloor),1,fil);
    MWRITE(SineWall, sizeof(SineWall),1,fil);
    MWRITE(SpringBoard, sizeof(SpringBoard),1,fil);
    //MWRITE(Rotate, sizeof(Rotate),1,fil);
    //MWRITE(DoorAutoClose, sizeof(DoorAutoClose),1,fil);
    MWRITE(&x_min_bound, sizeof(x_min_bound),1,fil);
    MWRITE(&y_min_bound, sizeof(y_min_bound),1,fil);
    MWRITE(&x_max_bound, sizeof(x_max_bound),1,fil);
    MWRITE(&y_max_bound, sizeof(y_max_bound),1,fil);


    MWRITE(Track, sizeof(Track),1,fil);
    for (i = 0; i < MAX_TRACKS; i++)
        {
        ASSERT(Track[i].TrackPoint);
        if (Track[i].NumPoints == 0)
            MWRITE(Track[i].TrackPoint, sizeof(TRACK_POINT),1,fil);
        else
            MWRITE(Track[i].TrackPoint, Track[i].NumPoints * sizeof(TRACK_POINT),1,fil);
        }
    
    MWRITE(&vel,sizeof(vel),1,fil);
    MWRITE(&svel,sizeof(svel),1,fil);
    MWRITE(&angvel,sizeof(angvel),1,fil);

    MWRITE(&loc,sizeof(loc),1,fil);
    //MWRITE(&oloc,sizeof(oloc),1,fil);
    //MWRITE(&fsync,sizeof(fsync),1,fil);

    MWRITE(LevelName,sizeof(LevelName),1,fil);
    MWRITE(&screenpeek,sizeof(screenpeek),1,fil);
    MWRITE(&totalsynctics,sizeof(totalsynctics),1,fil);

    // do all sector manipulation structures

    #if ANIM_SAVE
    #if 1
    MWRITE(&AnimCnt,sizeof(AnimCnt),1,fil);
    
    for(i = 0, a = &tanim; i < AnimCnt; i++)
        {
        long offset;
        memcpy(a,&Anim[i],sizeof(ANIM));
        
        // maintain compatibility with sinking boat which points to user data
        for (j=0; j<MAXSPRITES; j++)
            {
            if (User[j])
                {
                                // CTW MODIFICATION
                                BYTEp bp = (BYTEp)User[j];
                                //USERp bp = User[j];
                                // CTW MODIFICATION END
                
                if (a->ptr >= bp && a->ptr < bp + sizeof(USER))
                    {
                    // CTW MODIFICATION
                     offset = (long)((BYTEp)a->ptr - bp); // offset from user data
                    //offset = (long*)a->ptr - bp; // offset from user data
                                        // CTW MODIFICATION END
                    a->ptr = -2;
                    break;
                    }
                }    
            }
        
        if (a->ptr != -2)
            {
            for (j=0; j<numsectors; j++)
                {
                if (SectUser[j])
                    {
                                        // CTW MODIFICATION
                                        BYTEp bp = (BYTEp)SectUser[j];
                                        //SECT_USERp bp = SectUser[j];
                                        // CTW MODIFICATION END
                    
                    if (a->ptr >= bp && a->ptr < bp + sizeof(SECT_USER))
                        {
                                                // CTW MODIFICATION
                        offset = (long)((BYTEp)a->ptr - bp); // offset from user data
                        //offset = (long*)a->ptr - bp; // offset from user data
                                                // CTW MODIFICATION END
                        a->ptr = -3;
                        break;
                        }
                    }    
                }
            }    
        MWRITE(a,sizeof(ANIM),1,fil);
            
        if (a->ptr == -2 || a->ptr == -3)
            {
            MWRITE(&j, sizeof(j),1,fil);
            MWRITE(&offset, sizeof(offset),1,fil);
            }
        else    
            {
            SaveSymDataInfo(fil, a->ptr);
            }
            
        SaveSymCodeInfo(fil, a->callback);
        SaveSymDataInfo(fil, a->callbackdata);
        }
    
    #else
    ndx = 0;
    for(i = AnimCnt - 1, a = &tanim; i >= 0; i--)
        {
        // write header
        MWRITE(&ndx,sizeof(ndx),1,fil);
        
        memcpy(a,&Anim[i],sizeof(ANIM));
        MWRITE(a,sizeof(ANIM),1,fil);
        
        SaveSymDataInfo(fil, a->ptr);
        SaveSymCodeInfo(fil, a->callback);
        SaveSymDataInfo(fil, a->callbackdata);
        
        ndx++;
        }
        
    // write trailer
    ndx = -1;    
    MWRITE(&ndx,sizeof(ndx),1,fil);
    #endif
    #endif

    MWRITE(&totalclock,sizeof(totalclock),1,fil);
    MWRITE(&numframes,sizeof(numframes),1,fil);
    MWRITE(&randomseed,sizeof(randomseed),1,fil);
    MWRITE(&numpalookups,sizeof(numpalookups),1,fil);

    MWRITE(&NormalVisibility,sizeof(NormalVisibility),1,fil);
    MWRITE(&visibility,sizeof(visibility),1,fil);
    MWRITE(&parallaxtype,sizeof(parallaxtype),1,fil);
    MWRITE(&parallaxyoffs,sizeof(parallaxyoffs),1,fil);
    MWRITE(pskyoff,sizeof(pskyoff),1,fil);
    MWRITE(&pskybits,sizeof(pskybits),1,fil);
    
    MWRITE(&BorderInfo,sizeof(BorderInfo),1,fil);
    MWRITE(&MoveSkip2,sizeof(MoveSkip2),1,fil);
    MWRITE(&MoveSkip4,sizeof(MoveSkip4),1,fil);
    MWRITE(&MoveSkip8,sizeof(MoveSkip8),1,fil);
    
    // long interpolations
    MWRITE(&numinterpolations,sizeof(numinterpolations),1,fil);
    MWRITE(&startofdynamicinterpolations,sizeof(startofdynamicinterpolations),1,fil);
    MWRITE(oldipos,sizeof(oldipos),1,fil);
    MWRITE(bakipos,sizeof(bakipos),1,fil);
    for (i = numinterpolations - 1; i >= 0; i--)
        SaveSymDataInfo(fil, curipos[i]);
    
    // short interpolations
    MWRITE(&short_numinterpolations,sizeof(short_numinterpolations),1,fil);
    MWRITE(&short_startofdynamicinterpolations,sizeof(short_startofdynamicinterpolations),1,fil);
    MWRITE(short_oldipos,sizeof(short_oldipos),1,fil);
    MWRITE(short_bakipos,sizeof(short_bakipos),1,fil);
    for (i = short_numinterpolations - 1; i >= 0; i--)
        SaveSymDataInfo(fil, short_curipos[i]);

    
    // parental lock
    for (i = 0; i < SIZ(otlist); i++)
        {
        ndx = 0;
        TRAVERSE(otlist[i], otp, next_otp)
            {
            MWRITE(&ndx,sizeof(ndx),1,fil);
            MWRITE(&otp,sizeof(*otp),1,fil);
            ndx++;
            }
        ndx = -1;    
        MWRITE(&ndx, sizeof(ndx),1,fil);
        }
    
    // mirror
    MWRITE(mirror,sizeof(mirror),1,fil);
    MWRITE(&mirrorcnt,sizeof(mirrorcnt),1,fil);
    MWRITE(&mirrorinview,sizeof(mirrorinview),1,fil);

    // queue
    MWRITE(&StarQueueHead,sizeof(StarQueueHead),1,fil);
    MWRITE(StarQueue,sizeof(StarQueue),1,fil);
    MWRITE(&HoleQueueHead,sizeof(HoleQueueHead),1,fil);
    MWRITE(HoleQueue,sizeof(HoleQueue),1,fil);
    MWRITE(&WallBloodQueueHead,sizeof(WallBloodQueueHead),1,fil);
    MWRITE(WallBloodQueue,sizeof(WallBloodQueue),1,fil);
    MWRITE(&FloorBloodQueueHead,sizeof(FloorBloodQueueHead),1,fil);
    MWRITE(FloorBloodQueue,sizeof(FloorBloodQueue),1,fil);
    MWRITE(&GenericQueueHead,sizeof(GenericQueueHead),1,fil);
    MWRITE(GenericQueue,sizeof(GenericQueue),1,fil);
    MWRITE(&LoWangsQueueHead,sizeof(LoWangsQueueHead),1,fil);
    MWRITE(LoWangsQueue,sizeof(LoWangsQueue),1,fil);

    MWRITE(&PlayClock,sizeof(PlayClock),1,fil);
    MWRITE(&TotalKillable,sizeof(TotalKillable),1,fil);
    
    // game settings
    MWRITE(&gNet,sizeof(gNet),1,fil);
    
    MWRITE(LevelSong,sizeof(LevelSong),1,fil);
    
    MWRITE(palette,sizeof(palette),1,fil);
    MWRITE(palette_data,sizeof(palette_data),1,fil);
    MWRITE(&gs,sizeof(gs),1,fil);
    MWRITE(picanm,sizeof(picanm),1,fil);
    
    MWRITE(&LevelSecrets,sizeof(LevelSecrets),1,fil);
    
    MWRITE(show2dwall,sizeof(show2dwall),1,fil);
    MWRITE(show2dsprite,sizeof(show2dsprite),1,fil);
    MWRITE(show2dsector,sizeof(show2dsector),1,fil);
    
    MWRITE(&Bunny_Count,sizeof(Bunny_Count),1,fil);
    
    MWRITE(UserMapName,sizeof(UserMapName),1,fil);
    MWRITE(&GodMode,sizeof(GodMode),1,fil);

    MWRITE(&serpwasseen, sizeof(serpwasseen), 1, fil);
    MWRITE(&sumowasseen, sizeof(sumowasseen), 1, fil);
    MWRITE(&zillawasseen, sizeof(zillawasseen), 1, fil);
    MWRITE(BossSpriteNum, sizeof(BossSpriteNum), 1, fil);
    //MWRITE(&Zombies, sizeof(Zombies), 1, fil);
    
    MCLOSE(fil);
    
    FreeMem(SymTableData);
    SymTableData = NULL;
    FreeMem(SymTableCode);
    SymTableCode = NULL;
    
    ////DSPRINTF(ds, "done saving");
    //MONO_PRINT(ds);
    
    return(0);
}

int LoadGameFullHeader(short save_num, char *descr, short *level, short *skill)
{
    MFILE fil;
    char game_name[80];
    short tile;

    sprintf(game_name,"game%d.sav",save_num);
    if ((fil = MOPEN_READ(game_name)) == MF_ERR)
        return(-1);
    
    MREAD(descr, sizeof(SaveGameDescr[0]), 1,fil);
    
    MREAD(level,sizeof(*level),1,fil);
    MREAD(skill,sizeof(*skill),1,fil);
    
    tile = ScreenLoadSaveSetup(Player + myconnectindex);
    ScreenLoad(fil);
    
    MCLOSE(fil);
    
    return(tile);
}    

void LoadGameDescr(short save_num, char *descr)
{
    MFILE fil;
    char game_name[80];
    short tile;
    
    sprintf(game_name,"game%d.sav",save_num);
    if ((fil = MOPEN_READ(game_name)) == MF_ERR)
        return;
    
    MREAD(descr, sizeof(SaveGameDescr[0]),1,fil);
    
    MCLOSE(fil);
}    


int LoadGame(short save_num)
{
    MFILE fil;
    long i,j;
    short ndx,SpriteNum,sectnum;
    PLAYERp pp;
    SPRITEp sp;
    USERp u;
    SECTOR_OBJECTp sop;
    SECT_USERp sectu;
    CHAR code;
    ANIMp a;
    BYTE data_code;
    SHORT data_ndx;
    PANEL_SPRITEp psp,next,cur;
    PANEL_SPRITE tpanel_sprite;
    char game_name[80];
    OrgTileP otp, next_otp;

    long RotNdx;
    long StateStartNdx;
    long StateNdx;
    long StateEndNdx;
    extern BOOL InMenuLevel;

    LoadSymTable("swdata.sym", &SymTableData, &SymCountData);
    LoadSymTable("swcode.sym", &SymTableCode, &SymCountCode);
    
    if (SymCountData <= 0 || SymCountCode <= 0)
        return(-1);
        
    sprintf(game_name,"game%d.sav",save_num);
    if ((fil = MOPEN_READ(game_name)) == MF_ERR)
        return(-1);

    
    // Don't terminate until you've made sure conditions are valid for loading.
    if (InMenuLevel)
        StopSong();
    else    
        TerminateLevel();
    Terminate3DSounds();
    
    Terminate3DSounds();
    
    MREAD(SaveGameDescr[save_num], sizeof(SaveGameDescr[save_num]),1,fil);
    
    MREAD(&Level,sizeof(Level),1,fil);
    MREAD(&Skill,sizeof(Skill),1,fil);
    
    ScreenLoadSaveSetup(Player + myconnectindex);
    ScreenLoad(fil);
    ScreenTileUnLock();

    MREAD(&numplayers, sizeof(numplayers),1,fil);
    MREAD(&myconnectindex,sizeof(myconnectindex),1,fil);
    MREAD(&connecthead,sizeof(connecthead),1,fil);
    MREAD(connectpoint2,sizeof(connectpoint2),1,fil);
    
    //save players
    //MREAD(Player,sizeof(PLAYER), numplayers,fil);
    
    //save players info
    for (i = 0; i < numplayers; i++)
        {
        pp = &Player[i];
        
        MREAD(pp, sizeof(*pp), 1, fil);

        pp->remote_sprite = LoadSymDataInfo(fil);
        pp->remote.sop_control = LoadSymDataInfo(fil);
        pp->sop_remote = LoadSymDataInfo(fil);
        pp->sop = LoadSymDataInfo(fil);
        
        pp->hi_sectp = LoadSymDataInfo(fil);
        pp->lo_sectp = LoadSymDataInfo(fil);
        
        pp->hi_sp = LoadSymDataInfo(fil);
        pp->lo_sp = LoadSymDataInfo(fil);
        
        pp->last_camera_sp = LoadSymDataInfo(fil);
        pp->SpriteP = LoadSymDataInfo(fil);
        pp->UnderSpriteP = LoadSymDataInfo(fil);
        pp->DoPlayerAction = LoadSymCodeInfo(fil);
        pp->sop_control = LoadSymDataInfo(fil);
        pp->sop_riding = LoadSymDataInfo(fil);
        }
    
    
    #if PANEL_SAVE
    for (i = 0; i < numplayers; i++)
        {
        int j;
        pp = &Player[i];
        
        INITLIST(&pp->PanelSpriteList);

        while (TRUE)
            {
            MREAD(&ndx, sizeof(ndx),1,fil);
        
            if (ndx == -1)
                break;
            
            psp = CallocMem(sizeof(PANEL_SPRITE), 1);
            ASSERT(psp);

            MREAD(psp, sizeof(PANEL_SPRITE),1,fil);
            INSERT_TAIL(&pp->PanelSpriteList,psp);
            
            psp->PlayerP = LoadSymDataInfo(fil);
            psp->State = LoadSymDataInfo(fil);
            psp->RetractState = LoadSymDataInfo(fil);
            psp->PresentState = LoadSymDataInfo(fil);
            psp->ActionState = LoadSymDataInfo(fil);
            psp->RestState = LoadSymDataInfo(fil);
            psp->PanelSpriteFunc = LoadSymCodeInfo(fil);

            for (j = 0; j < SIZ(psp->over); j++)
                {
                psp->over[j].State = LoadSymDataInfo(fil);
                }

            }
        }
    #endif    

    MREAD(&numsectors,sizeof(numsectors),1,fil);
    MREAD(sector,sizeof(SECTOR),numsectors,fil);

    //Sector User information
    for (i = 0; i < numsectors; i++)
        {
        MREAD(&sectnum,sizeof(sectnum),1,fil);
        if (sectnum != -1)         
            {
            SectUser[sectnum] = sectu = (SECT_USERp)CallocMem(sizeof(SECT_USER), 1);
            MREAD(sectu,sizeof(SECT_USER),1,fil);
            }
        }
        
    MREAD(&numwalls,sizeof(numwalls),1,fil);
    MREAD(wall,sizeof(WALL),numwalls,fil);

    //Store all sprites to preserve indeces
    MREAD(&i, sizeof(i),1,fil);
    while(i != -1)
        {
        MREAD(&sprite[i], sizeof(SPRITE),1,fil);
        MREAD(&i, sizeof(i),1,fil);
        }
        
    MREAD(headspritesect,sizeof(headspritesect),1,fil);
    MREAD(prevspritesect,sizeof(prevspritesect),1,fil);
    MREAD(nextspritesect,sizeof(nextspritesect),1,fil);
    MREAD(headspritestat,sizeof(headspritestat),1,fil);
    MREAD(prevspritestat,sizeof(prevspritestat),1,fil);
    MREAD(nextspritestat,sizeof(nextspritestat),1,fil);

    //User information
    memset(User, NULL, sizeof(User));
    
    MREAD(&SpriteNum, sizeof(SpriteNum),1,fil);
    while(SpriteNum != -1)
        {
        sp = &sprite[SpriteNum];
        User[SpriteNum] = u = (USERp)CallocMem(sizeof(USER), 1);
        MREAD(u,sizeof(USER),1,fil);

        if (u->WallShade)
            {
            u->WallShade = CallocMem(u->WallCount * sizeof(*u->WallShade), 1);
            MREAD(u->WallShade,sizeof(*u->WallShade)*u->WallCount,1,fil);
            }    
        
        if (u->rotator)
            {
            u->rotator = CallocMem(sizeof(*u->rotator), 1);
            MREAD(u->rotator,sizeof(*u->rotator),1,fil);
            
            if (u->rotator->origx)
                {
                u->rotator->origx = CallocMem(u->rotator->num_walls * sizeof(*u->rotator->origx), 1);
                MREAD(u->rotator->origx,sizeof(*u->rotator->origx)*u->rotator->num_walls,1,fil);
                }
            if (u->rotator->origy)
                {
                u->rotator->origy = CallocMem(u->rotator->num_walls * sizeof(*u->rotator->origy), 1);
                MREAD(u->rotator->origy,sizeof(*u->rotator->origy)*u->rotator->num_walls,1,fil);
                }
            }    
        
        u->WallP = LoadSymDataInfo(fil);
        u->State = LoadSymDataInfo(fil);
        u->Rot = LoadSymDataInfo(fil);
        u->StateStart = LoadSymDataInfo(fil);
        u->StateEnd = LoadSymDataInfo(fil);
        u->StateFallOverride = LoadSymDataInfo(fil);
        u->ActorActionFunc = LoadSymCodeInfo(fil);
        u->ActorActionSet = LoadSymDataInfo(fil);
        u->Personality = LoadSymDataInfo(fil);
        u->Attrib = LoadSymDataInfo(fil);
        u->sop_parent = LoadSymDataInfo(fil);
        u->hi_sectp = LoadSymDataInfo(fil);
        u->lo_sectp = LoadSymDataInfo(fil);
        u->hi_sp = LoadSymDataInfo(fil);
        u->lo_sp = LoadSymDataInfo(fil);
        u->SpriteP = LoadSymDataInfo(fil);
        u->PlayerP = LoadSymDataInfo(fil);
        u->tgt_sp = LoadSymDataInfo(fil);
        
        MREAD(&SpriteNum,sizeof(SpriteNum),1,fil);
        }
        
    MREAD(SectorObject, sizeof(SectorObject),1,fil);
    
    for (ndx = 0; ndx < SIZ(SectorObject); ndx++)
        {
        sop = &SectorObject[ndx];
        
        sop->PreMoveAnimator = LoadSymCodeInfo(fil);
        sop->PostMoveAnimator = LoadSymCodeInfo(fil);
        sop->Animator = LoadSymCodeInfo(fil);
        sop->controller = LoadSymDataInfo(fil);
        sop->sp_child = LoadSymDataInfo(fil);
        }
    
    MREAD(SineWaveFloor, sizeof(SineWaveFloor),1,fil);
    MREAD(SineWall, sizeof(SineWall),1,fil);
    MREAD(SpringBoard, sizeof(SpringBoard),1,fil);
    //MREAD(Rotate, sizeof(Rotate),1,fil);
    //MREAD(DoorAutoClose, sizeof(DoorAutoClose),1,fil);
    MREAD(&x_min_bound, sizeof(x_min_bound),1,fil);
    MREAD(&y_min_bound, sizeof(y_min_bound),1,fil);
    MREAD(&x_max_bound, sizeof(x_max_bound),1,fil);
    MREAD(&y_max_bound, sizeof(y_max_bound),1,fil);
    
    MREAD(Track, sizeof(Track),1,fil);
    for (i = 0; i < MAX_TRACKS; i++)
        {
        if (Track[i].NumPoints == 0)
            {
            Track[i].TrackPoint = (TRACK_POINTp)CallocMem(sizeof(TRACK_POINT), 1);
            MREAD(Track[i].TrackPoint, sizeof(TRACK_POINT),1,fil);
            }
        else    
            {
            Track[i].TrackPoint = (TRACK_POINTp)CallocMem(Track[i].NumPoints * sizeof(TRACK_POINT), 1);
            MREAD(Track[i].TrackPoint, Track[i].NumPoints * sizeof(TRACK_POINT),1,fil);
            }
        }
        
    MREAD(&vel,sizeof(vel),1,fil);
    MREAD(&svel,sizeof(svel),1,fil);
    MREAD(&angvel,sizeof(angvel),1,fil);

    MREAD(&loc,sizeof(loc),1,fil);

    MREAD(LevelName,sizeof(LevelName),1,fil);
    MREAD(&screenpeek,sizeof(screenpeek),1,fil);
    MREAD(&totalsynctics,sizeof(totalsynctics),1,fil);  // same as kens lockclock

    // do all sector manipulation structures

    #if ANIM_SAVE
    #if 1
    MREAD(&AnimCnt,sizeof(AnimCnt),1,fil);
    
    for(i = 0; i < AnimCnt; i++)
        {
        a = &Anim[i];
        MREAD(a,sizeof(ANIM),1,fil);
        
        if (a->ptr == -2)
            {
            // maintain compatibility with sinking boat which points to user data
            long offset;
            MREAD(&j, sizeof(j),1,fil);
            MREAD(&offset, sizeof(offset),1,fil);
                        // CTW MODIFICATION
                        a->ptr = (long*)(((char *)User[j]) + offset);
                        //a->ptr = ((long *)User[j]) + offset;
                        // CTW MODIFICATION END
            }
        else
        if (a->ptr == -3)
            {
            // maintain compatibility with sinking boat which points to user data
            long offset;
            MREAD(&j, sizeof(j),1,fil);
            MREAD(&offset, sizeof(offset),1,fil);
                        // CTW MODIFICATION
                        a->ptr = (long*)(((char *)SectUser[j]) + offset);
                        //a->ptr = ((long *)SectUser[j]) + offset;
                        // CTW MODIFICATION END
            }
        else   
            {
            a->ptr = LoadSymDataInfo(fil);
            }
                
        a->callback = LoadSymCodeInfo(fil);
        a->callbackdata = LoadSymDataInfo(fil);
        }
    #else
    AnimCnt = 0;
    for(i = MAXANIM - 1; i >= 0; i--)
        {
        a = &Anim[i];
        
        MREAD(&ndx,sizeof(ndx),1,fil);
        
        if (ndx == -1)
            break;
            
        AnimCnt++;
        
        MREAD(a,sizeof(ANIM),1,fil);
        
        a->ptr = LoadSymDataInfo(fil);
        a->callback = LoadSymCodeInfo(fil);
        a->callbackdata = LoadSymDataInfo(fil);
        }
    #endif    
    #endif    

    MREAD(&totalclock,sizeof(totalclock),1,fil);
    MREAD(&numframes,sizeof(numframes),1,fil);
    MREAD(&randomseed,sizeof(randomseed),1,fil);
    MREAD(&numpalookups,sizeof(numpalookups),1,fil);

    MREAD(&NormalVisibility,sizeof(NormalVisibility),1,fil);
    MREAD(&visibility,sizeof(visibility),1,fil);
    MREAD(&parallaxtype,sizeof(parallaxtype),1,fil);
    MREAD(&parallaxyoffs,sizeof(parallaxyoffs),1,fil);
    MREAD(pskyoff,sizeof(pskyoff),1,fil);
    MREAD(&pskybits,sizeof(pskybits),1,fil);

    MREAD(&BorderInfo,sizeof(BorderInfo),1,fil);
    MREAD(&MoveSkip2,sizeof(MoveSkip2),1,fil);
    MREAD(&MoveSkip4,sizeof(MoveSkip4),1,fil);
    MREAD(&MoveSkip8,sizeof(MoveSkip8),1,fil);
    
    // long interpolations
    MREAD(&numinterpolations,sizeof(numinterpolations),1,fil);
    MREAD(&startofdynamicinterpolations,sizeof(startofdynamicinterpolations),1,fil);
    MREAD(oldipos,sizeof(oldipos),1,fil);
    MREAD(bakipos,sizeof(bakipos),1,fil);
    for (i = numinterpolations - 1; i >= 0; i--)
        curipos[i] = LoadSymDataInfo(fil);

    // short interpolations
    MREAD(&short_numinterpolations,sizeof(short_numinterpolations),1,fil);
    MREAD(&short_startofdynamicinterpolations,sizeof(short_startofdynamicinterpolations),1,fil);
    MREAD(short_oldipos,sizeof(short_oldipos),1,fil);
    MREAD(short_bakipos,sizeof(short_bakipos),1,fil);
    for (i = short_numinterpolations - 1; i >= 0; i--)
        short_curipos[i] = LoadSymDataInfo(fil);
    
    // parental lock
    for (i = 0; i < SIZ(otlist); i++)
        {
        INITLIST(otlist[i]);
            
        while (TRUE)
            {
            MREAD(&ndx, sizeof(ndx),1,fil);
        
            if (ndx == -1)
                break;
                
            otp = CallocMem(sizeof(*otp), 1);
            ASSERT(otp);

            MREAD(otp, sizeof(*otp),1,fil);
            INSERT_TAIL(otlist[i],otp);
            }
        }

    // mirror    
    MREAD(mirror,sizeof(mirror),1,fil);
    MREAD(&mirrorcnt,sizeof(mirrorcnt),1,fil);
    MREAD(&mirrorinview,sizeof(mirrorinview),1,fil);

    // queue
    MREAD(&StarQueueHead,sizeof(StarQueueHead),1,fil);
    MREAD(StarQueue,sizeof(StarQueue),1,fil);
    MREAD(&HoleQueueHead,sizeof(HoleQueueHead),1,fil);
    MREAD(HoleQueue,sizeof(HoleQueue),1,fil);
    MREAD(&WallBloodQueueHead,sizeof(WallBloodQueueHead),1,fil);
    MREAD(WallBloodQueue,sizeof(WallBloodQueue),1,fil);
    MREAD(&FloorBloodQueueHead,sizeof(FloorBloodQueueHead),1,fil);
    MREAD(FloorBloodQueue,sizeof(FloorBloodQueue),1,fil);
    MREAD(&GenericQueueHead,sizeof(GenericQueueHead),1,fil);
    MREAD(GenericQueue,sizeof(GenericQueue),1,fil);
    MREAD(&LoWangsQueueHead,sizeof(LoWangsQueueHead),1,fil);
    MREAD(LoWangsQueue,sizeof(LoWangsQueue),1,fil);

    // init timing vars before PlayClock is read
    MREAD(&PlayClock,sizeof(PlayClock),1,fil);
    MREAD(&TotalKillable,sizeof(TotalKillable),1,fil);
    
    // game settings
    MREAD(&gNet,sizeof(gNet),1,fil);
    
    MREAD(LevelSong,sizeof(LevelSong),1,fil);
    
    MREAD(palette,sizeof(palette),1,fil);
    MREAD(palette_data,sizeof(palette_data),1,fil);
    
    {
    BOOL AmbBak = gs.Ambient;
    BOOL MusicBak = gs.MusicOn;
    BOOL FxBak = gs.FxOn;
    short SndVolBak = gs.SoundVolume;
    short MusVolBak = gs.MusicVolume;
    MREAD(&gs,sizeof(gs),1,fil);
    gs.MusicOn = MusicBak;
    gs.FxOn = FxBak;
    gs.Ambient = AmbBak;
    gs.SoundVolume = SndVolBak;
    gs.MusicVolume = MusVolBak;
    }

    
    //COVERsetbrightness(gs.Brightness,(char *)palette_data);
    
    MREAD(picanm,sizeof(picanm),1,fil);
    
    MREAD(&LevelSecrets,sizeof(LevelSecrets),1,fil);
    
    MREAD(show2dwall,sizeof(show2dwall),1,fil);
    MREAD(show2dsprite,sizeof(show2dsprite),1,fil);
    MREAD(show2dsector,sizeof(show2dsector),1,fil);
    
    MREAD(&Bunny_Count,sizeof(Bunny_Count),1,fil);
    
    MREAD(UserMapName,sizeof(UserMapName),1,fil);
    MREAD(&GodMode,sizeof(GodMode),1,fil);

    MREAD(&serpwasseen, sizeof(serpwasseen), 1, fil);
    MREAD(&sumowasseen, sizeof(sumowasseen), 1, fil);
    MREAD(&zillawasseen, sizeof(zillawasseen), 1, fil);
    MREAD(BossSpriteNum, sizeof(BossSpriteNum), 1, fil);
    //MREAD(&Zombies, sizeof(Zombies), 1, fil);
    
    MCLOSE(fil);
    
    FreeMem(SymTableData);
    SymTableData = NULL;
    FreeMem(SymTableCode);
    SymTableCode = NULL;
    
    
    //!!IMPORTANT - this POST stuff will not work here now becaus it does actual reads
    
    
    //
    // POST processing of info MREAD in
    //
    
    #if PANEL_SAVE
    for (i = 0; i < numplayers; i++)
        {
        pp = &Player[i];
        TRAVERSE(&pp->PanelSpriteList, psp, next)
            {
            // dont need to set Next and Prev this was done
            // when sprites were inserted
            
            // sibling is the only PanelSprite (malloced ptr) in the PanelSprite struct
            psp->sibling = PanelNdxToSprite(pp, (long)psp->sibling);
            }
        }
    #endif

    if (Bstrcasecmp(CacheLastLevel, LevelName) != 0)
        {
        SetupPreCache();
        DoTheCache();
        }
    
    // what is this for? don't remember
    totalclock = totalsynctics;
    ototalclock = totalsynctics;
    
    // this is ok - just duplicating sector list with pointers
    for (sop = SectorObject; sop < &SectorObject[SIZ(SectorObject)]; sop++)
        {
        for (i = 0; i < sop->num_sectors; i++)
            sop->sectp[i] = &sector[sop->sector[i]];
        }
    
    //!!Again this will not work here
    //restore players info
    for (i = 0; i < numplayers; i++)
        {
        #if PANEL_SAVE
        pp->CurWpn = PanelNdxToSprite(pp, (long)pp->CurWpn);

        for (ndx = 0; ndx < MAX_WEAPONS; ndx++)
            pp->Wpn[ndx] = PanelNdxToSprite(pp, (long)pp->Wpn[ndx]);

        for (ndx = 0; ndx < MAX_INVENTORY; ndx++)
            pp->InventorySprite[ndx] = PanelNdxToSprite(pp, (long)pp->InventorySprite[ndx]);

        pp->Chops = PanelNdxToSprite(pp, (long)pp->Chops);
        pp->InventorySelectionBox = PanelNdxToSprite(pp, (long)pp->InventorySelectionBox);
        pp->MiniBarHealthBox = PanelNdxToSprite(pp, (long)pp->MiniBarHealthBox);
        pp->MiniBarAmmo = PanelNdxToSprite(pp, (long)pp->MiniBarAmmo);

        for (ndx = 0; ndx < SIZ(pp->MiniBarHealthBoxDigit); ndx++)
            pp->MiniBarHealthBoxDigit[ndx] = PanelNdxToSprite(pp, (long)pp->MiniBarHealthBoxDigit[ndx]);

        for (ndx = 0; ndx < SIZ(pp->MiniBarAmmoDigit); ndx++)
            pp->MiniBarAmmoDigit[ndx] = PanelNdxToSprite(pp, (long)pp->MiniBarAmmoDigit[ndx]);

        #endif
        }

    {    
    long SavePlayClock = PlayClock;
    InitTimingVars();
    PlayClock = SavePlayClock;
    }
    InitNetVars();
    
    SetupAspectRatio();
    SetRedrawScreen(Player + myconnectindex);
    
    COVERsetbrightness(gs.Brightness,(char *)palette_data);

    screenpeek = myconnectindex;
    PlayingLevel = Level;

    if (SW_SHAREWARE) {
        if (gs.MusicOn)
            PlaySong(LevelSong);
    } else {
        if (gs.MusicOn)
            CDAudio_Play(RedBookSong[Level], TRUE);  // track, loop - Level songs are looped
    }
    if (gs.Ambient)    
        StartAmbientSound();
    FX_SetVolume(gs.SoundVolume);
    if (!SW_SHAREWARE) {
        CDAudio_SetVolume(gs.MusicVolume);
    } else {
        MUSIC_SetVolume(gs.MusicVolume);
    }

    TRAVERSE_CONNECT(i)
        {
        Player[i].PlayerTalking = FALSE; 
        Player[i].TalkVocnum = -1; 
        Player[i].TalkVocHandle = -1;
        Player[i].StartColor = 0; 
        }
    
    // this is not a new game
    NewGame = FALSE;    
    
    
    DoPlayerDivePalette(Player+myconnectindex);
    DoPlayerNightVisionPalette(Player+myconnectindex);
    
    return(0);
}

VOID
ScreenSave(MFILE fout)
    {
    long num;
    num = MWRITE((void*)waloff[SAVE_SCREEN_TILE], SAVE_SCREEN_XSIZE * SAVE_SCREEN_YSIZE, 1, fout);
    ASSERT(num == 1);
    }
    
VOID
ScreenLoad(MFILE fin)
    {
    long num;
    
    setviewtotile(SAVE_SCREEN_TILE, SAVE_SCREEN_YSIZE, SAVE_SCREEN_XSIZE);
    
    num = MREAD((void*)waloff[SAVE_SCREEN_TILE], SAVE_SCREEN_XSIZE * SAVE_SCREEN_YSIZE, 1, fin);
    
    setviewback();
    }

