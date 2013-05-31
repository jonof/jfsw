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

// CTW NOTE
/*
Known remaining issues:
- Audio stuttering.
- CD Audio not looping properly (currently hard coded to restart about every 200 seconds.
- Hitting F5 to change resolution causes a crash (currently disabled).
- Multiplayer untested.

Things required to make savegames work:
- Load makesym.wpj and build it.
- In a DOS prompt, run "makesym sw.map swdata.map swcode.map"
- Copy swcode.map to swcode.sym and swdata.map to swdata.sym
*/
// CTW NOTE END

#define MAIN
#define QUIET
#include "build.h"
#include "baselayer.h"
#include "cache1d.h"
#include "osd.h"
#ifdef RENDERTYPEWIN
# include "winlayer.h"
#endif

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"
#include "tags.h"
#include "sector.h"
#include "sprite.h"
#include "weapon.h"
#include "player.h"
#include "lists.h"
#include "net.h"
#include "pal.h"
#include "fx_man.h"

#include "mytypes.h"
//#include "config.h"

#include "menus.h"

#include "control.h"
#include "function.h"

#include "demo.h"
#include "cache.h"
//#include "exports.h"

#include "anim.h"

#include "colormap.h"
#include "break.h"
#include "ninja.h"
#include "light.h"
#include "track.h"
#include "jsector.h"
#include "keyboard.h"
#include "text.h"
#include "music.h"

#include "crc32.h"

#if DEBUG
#define BETA 0
#endif

#define STAT_SCREEN_PIC 5114
#define TITLE_PIC 2324
#define THREED_REALMS_PIC 2325
#define TITLE_ROT_FLAGS (ROTATE_SPRITE_CORNER|ROTATE_SPRITE_SCREEN_CLIP|ROTATE_SPRITE_NON_MASK)
#define PAL_SIZE (256*3)

char DemoName[15][16];

// Stupid WallMart version!
//#define PLOCK_VERSION TRUE

#if PLOCK_VERSION
BOOL Global_PLock = TRUE;
#else
BOOL Global_PLock = FALSE; 
#endif

int GameVersion = 13;   // 12 was original source release. For future releases increment by two.
char DemoText[3][64];
int DemoTextYstart = 0;

BOOL DoubleInitAWE32 = FALSE;
int Follow_posx=0,Follow_posy=0;

BOOL NoMeters = FALSE;
short IntroAnimCount = 0;
short PlayingLevel = -1;
BOOL GraphicsMode = FALSE;
char CacheLastLevel[32] = "";
char PlayerNameArg[32] = "";
BOOL CleanExit = FALSE;
extern char cachedebug;
BOOL DemoModeMenuInit = FALSE;
BOOL FinishAnim = 0;
BOOL CachePrintMode = FALSE;
BOOL ShortGameMode = FALSE;
BOOL ReloadPrompt = FALSE;
BOOL ReloadPromptMode = FALSE;
BOOL NewGame = TRUE;
BOOL InMenuLevel = FALSE;
BOOL LoadGameOutsideMoveLoop = FALSE;
BOOL LoadGameFromDemo = FALSE;
BOOL ArgCheat = FALSE;
extern BOOL NetBroadcastMode, NetModeOverride;
BOOL MultiPlayQuitFlag = FALSE;
//Miscellaneous variables
char MessageInputString[256];
char MessageOutputString[256];
BOOL MessageInputMode = FALSE;
BOOL ConInputMode = FALSE;
BOOL ConPanel = FALSE;
BOOL FinishedLevel = FALSE;
BOOL HelpInputMode = FALSE;
BOOL PanelUpdateMode = TRUE;
short HelpPage = 0;
short HelpPagePic[] = { 5115, 5116, 5117 };
BOOL InputMode = FALSE;
BOOL MessageInput = FALSE;
extern BOOL GamePaused;
short screenpeek = 0;
BOOL NoDemoStartup = FALSE;
BOOL FirstTimeIntoGame;
extern BYTE RedBookSong[40];

BOOL BorderAdjust = TRUE;
BOOL LocationInfo = 0;
VOID drawoverheadmap(int cposx, int cposy, int czoom, short cang);
int DispFrameRate = FALSE;
int DispMono = TRUE;
int Fog = FALSE;
int FogColor;
int PreCaching = TRUE;
int GodMode = FALSE;
int BotMode = FALSE;
short Skill = 2;
short BetaVersion = 900;
short TotalKillable;

AUTO_NET Auto;    
BOOL AutoNet = FALSE;
BOOL HasAutoColor = FALSE;
BYTE AutoColor;

const GAME_SET gs_defaults = {
32768, // mouse speed
128, // music vol
192, // fx vol
2, // border
0, // brightness
0, // border tile
FALSE, // mouse aiming
FALSE, // mouse look
FALSE, // mouse invert
TRUE, // bobbing
FALSE, // tilting
TRUE, // shadows
FALSE, // auto run
TRUE, // crosshair
TRUE, // auto aim
TRUE, // messages
TRUE, // fx on
TRUE, // Music on
TRUE, // talking
TRUE, // ambient
FALSE, // Flip Stereo

// Network game settings
0, // GameType
0, // Level
0, // Monsters
FALSE, // HurtTeammate
TRUE, // SpawnMarkers Markers
FALSE, // TeamPlay
0, // Kill Limit
0, // Time Limit
0, // Color
0, // Parental Lock
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", // Password
TRUE, // nuke
TRUE, // voxels
FALSE, // stats
FALSE, // mouse aiming on
FALSE, // play cd
"track??.ogg", // ogg track name
};
GAME_SET gs;

char PlaxBits = 0;
BOOL PlayerTrackingMode = FALSE;
BOOL PauseMode = FALSE;
BOOL PauseKeySet = FALSE;
BOOL SlowMode = FALSE;
BOOL FrameAdvanceTics = 3;
BOOL ScrollMode2D = FALSE;

BOOL DebugSO = FALSE;
BOOL DebugPanel = FALSE;
BOOL DebugSector = FALSE;
BOOL DebugActor = FALSE;
BOOL DebugAnim = FALSE;
BOOL DebugOperate = FALSE;
BOOL DebugActorFreeze = FALSE;
VOID LoadingLevelScreen(char *level_name);

BYTE FakeMultiNumPlayers;

int totalsynctics;
int turn_scale = 256;
int move_scale = 256;

short Level = 0;
BOOL ExitLevel = FALSE;
SHORT OrigCommPlayers=0;
extern BYTE CommPlayers;
extern BOOL CommEnabled;
extern int bufferjitter;

BOOL CameraTestMode = FALSE;

char ds[256];                           // debug string

extern short NormalVisibility;

extern int quotebot, quotebotgoal;     // Multiplayer typing buffer
char recbuf[80];                        // Used as a temp buffer to hold typing text

extern unsigned char palette_data[256][3];             // Global palette array

#define ACT_STATUE 0

int score;
BOOL QuitFlag = FALSE; 
BOOL InGame = FALSE;

BOOL CommandSetup = FALSE;

char UserMapName[80]="", buffer[80], ch;
char LevelName[20];

BYTE DebugPrintColor = 255;

int krandcount;

/// L O C A L   P R O T O T Y P E S /////////////////////////////////////////////////////////
void BOT_DeleteAllBots( void );
VOID BotPlayerInsert(PLAYERp pp);
VOID SybexScreen(VOID);
VOID DosScreen(VOID);
VOID MenuLevel(VOID);
VOID StatScreen(PLAYERp mpp);
VOID InitRunLevel(VOID);
VOID RunLevel(VOID);
/////////////////////////////////////////////////////////////////////////////////////////////

static FILE *debug_fout = NULL;
    
VOID DebugWriteString(char *string)
    {
    
    #if BETA || !DEBUG
    return;
    #endif
    
    if (!debug_fout)
        {
        if ((debug_fout = fopen("dbg.foo", "ab+")) == NULL)
            return;
        }    
        
    fprintf(debug_fout, "%s\n", string);    
    
    //fclose(debug_fout);
    //debug_fout = NULL;
    
    fflush(debug_fout);
    }

VOID DebugWriteLoc(char *fname, int line)
    {
    
    #if BETA || !DEBUG
    return;
    #endif
    
    if (!debug_fout)
        {
        if ((debug_fout = fopen("dbg.foo", "ab+")) == NULL)
            return;
        }    
        
    fprintf(debug_fout, "%s, %d\n", fname, line);    
    
    //fclose(debug_fout);
    //debug_fout = NULL;
    
    fflush(debug_fout);
    }

void Mono_Print(char *str)
{
MONO_PRINT(str);
}
    

extern BOOL DrawScreen;
#if RANDOM_DEBUG
FILE *fout_err;
BOOL RandomPrint;
int krand1(char *file, unsigned line)
    {
    ASSERT(!DrawScreen);
    if (RandomPrint && !Prediction)
        { extern ULONG MoveThingsCount;
        sprintf(ds,"mtc %d, %s, line %d, %d",MoveThingsCount,file,line,randomseed);
        DebugWriteString(ds);
        }
    randomseed = ((randomseed * 21 + 1) & 65535);
    return (randomseed);
    }

int krand2()
    {
    ASSERT(!DrawScreen);
    randomseed = ((randomseed * 21 + 1) & 65535);
    return (randomseed);
    }

#else
int krand1(void)
    {
    ASSERT(!DrawScreen);
    krandcount++;
    randomseed = ((randomseed * 21 + 1) & 65535);
    return (randomseed);
    }

#endif

VOID *
CacheAlloc(void **ptr, int size, unsigned char *lock_byte)
    {
    if (*ptr == NULL)
        {
        *lock_byte = CACHE_LOCK_START;
        allocache(ptr, size, lock_byte);
        }
    else
        {
        if (*lock_byte < CACHE_LOCK_START)
            *lock_byte = CACHE_LOCK_START;
        else
            (*lock_byte)++;
       }
   
    return(*ptr);
    }

VOID 
CacheFree(void **ptr, unsigned char *lock_byte)
    {
    if (*ptr == NULL)
        {
        ASSERT(*lock_byte == NULL);
        }
    else
        {
        if (*lock_byte < CACHE_LOCK_START)
            *lock_byte = CACHE_UNLOCK_START;
       }
    }

/*
void HeapCheck(char *file, int line)
{
    switch( _heapchk() ) 
        {
        case _HEAPOK:
            //printf( "OK - heap is good\n" );
            break;
        case _HEAPEMPTY:
            //printf( "OK - heap is empty\n" );
            break;
        case _HEAPBADBEGIN:
            sprintf(ds, "ERROR - heap is damaged: %s, %d", file, line);
            MONO_PRINT(ds);
            DebugWriteString(ds);
            setvmode(0x3);     
            printf( "%s\n", ds);
            exit(0);
            break;
        case _HEAPBADNODE:
            sprintf(ds, "ERROR - bad node in heap: %s, %d", file, line);
            MONO_PRINT(ds);
            DebugWriteString(ds);
            setvmode(0x3);     
            printf( "%s\n", ds);
            exit(0);
            break;
        }
}
    */
    
#if DEBUG
BOOL
ValidPtr(VOID * ptr)
    {
    MEM_HDRp mhp;
    BYTEp check;

    ASSERT(ptr != NULL);

    mhp = (MEM_HDRp) (((BYTEp) ptr) - sizeof(MEM_HDR));

    if (mhp->size == 0 || mhp->checksum == 0)
        {
    printf("ValidPtr(): Size or Checksum == 0!\n");
        return (FALSE);
        }

    check = (BYTEp) & mhp->size;

    if (mhp->checksum == check[0] + check[1] + check[2] + check[3])
        return (TRUE);

    printf("ValidPtr(): Checksum bad!\n");
    return (FALSE);
    }

VOID
PtrCheckSum(VOID * ptr, unsigned int *stored, unsigned int *actual)
    {
    MEM_HDRp mhp;
    BYTEp check;

    ASSERT(ptr != NULL);

    mhp = (MEM_HDRp) (((BYTEp) ptr) - sizeof(MEM_HDR));

    check = (BYTEp) & mhp->size;

    *stored = mhp->checksum;
    *actual = check[0] + check[1] + check[2] + check[3];
    }

VOID *
AllocMem(int size)
    {
    BYTEp bp;
    MEM_HDRp mhp;
    BYTEp check;

    ASSERT(size != 0);

    bp = (BYTEp) malloc(size + sizeof(MEM_HDR));

    // Used for debugging, we can remove this at ship time
    if(bp == NULL)
        {
        TerminateGame();
        printf("Memory could NOT be allocated in AllocMem: size = %d\n",size);
        exit(0);
        }

    ASSERT(bp != NULL);

    mhp = (MEM_HDRp) bp;

    mhp->size = size;
    check = (BYTEp) & mhp->size;
    mhp->checksum = check[0] + check[1] + check[2] + check[3];

    bp += sizeof(MEM_HDR);

    return (bp);
    }

VOID *
ReAllocMem(VOID * ptr, int size)
    {
    BYTEp bp;
    MEM_HDRp mhp;
    BYTEp check;

    ASSERT(size != 0);

    ASSERT(ValidPtr(ptr));

    mhp = (MEM_HDRp) (((BYTEp) ptr) - sizeof(MEM_HDR));

    bp = (BYTEp) realloc(mhp, size + sizeof(MEM_HDR));

    ASSERT(bp != NULL);

    mhp = (MEM_HDRp) bp;

    mhp->size = size;
    check = (BYTEp) & mhp->size;
    mhp->checksum = check[0] + check[1] + check[2] + check[3];

    bp += sizeof(MEM_HDR);

    ASSERT(ValidPtr(bp));

    return (bp);
    }


VOID *
CallocMem(int size, int num)
    {
    BYTEp bp;
    MEM_HDRp mhp;
    BYTEp check;
    int num_bytes;

    ASSERT(size != 0 && num != 0);

    num_bytes = (size * num) + sizeof(MEM_HDR);
    bp = (BYTEp) calloc(num_bytes, 1);

    // Used for debugging, we can remove this at ship time
    if(bp == NULL)
        {
        TerminateGame();
        printf("Memory could NOT be allocated in CallocMem: size = %d, num = %d\n",size,num);
        exit(0);
        }

    ASSERT(bp != NULL);

    mhp = (MEM_HDRp) bp;

    mhp->size = size;
    check = (BYTEp) & mhp->size;
    mhp->checksum = check[0] + check[1] + check[2] + check[3];

    bp += sizeof(MEM_HDR);

    return (bp);
    }
    
VOID
FreeMem(VOID * ptr)
    {
    MEM_HDRp mhp;
    BYTEp check;

    ASSERT(ptr != NULL);

    ASSERT(ValidPtr(ptr));

    mhp = (MEM_HDRp) (((BYTEp) ptr) - sizeof(MEM_HDR));
    check = (BYTEp)&mhp->size;

    memset(mhp, 0xCC, mhp->size + sizeof(MEM_HDR));

    free(mhp);
    }

#else
BOOL
ValidPtr(VOID * ptr)
    {
    return (TRUE);
    }

VOID *
AllocMem(int size)
    {
    return (malloc(size));
    }

VOID *
CallocMem(int size, int num)
    {
    return (calloc(size, num));
    }

VOID *
ReAllocMem(VOID * ptr, int size)
    {
    return (realloc(ptr, size));
    }

VOID
FreeMem(VOID * ptr)
    {
    free(ptr);
    }

#endif

int PointOnLine(int x, int y, int x1, int y1, int x2, int y2)
    {
    // the closer to 0 the closer to the line the point is
    return( ((x2 - x1) * (y - y1)) - ((y2 - y1) * (x - x1)) );
    }
    
int
Distance(int x1, int y1, int x2, int y2)
    {
    int min;

    if ((x2 = x2 - x1) < 0)
        x2 = -x2;

    if ((y2 = y2 - y1) < 0)
        y2 = -y2;

    if (x2 > y2)
        min = y2;
    else
        min = x2;

    return (x2 + y2 - DIV2(min));
    }

VOID
MapSetAll2D(BYTE fill)
    {
    int i;

    for (i = 0; i < (MAXWALLS >> 3); i++)
        show2dwall[i] = fill;
    for (i = 0; i < (MAXSPRITES >> 3); i++)
        show2dsprite[i] = fill;
        
    //for (i = 0; i < (MAXSECTORS >> 3); i++)
    for (i = 0; i < MAXSECTORS; i++)
        {
        if (sector[i].ceilingpicnum != 342 && sector[i].floorpicnum != 342)
            show2dsector[i>>3] |= (1<<(i&7));
        //show2dsector[i] = fill;
        }
    }

VOID
MapSetup(VOID)
    {
    #define NO_AUTO_MAPPING FALSE

    #if NO_AUTO_MAPPING
    MapSetAll2D(0xFF);
    #else
    automapping = TRUE;
    #endif
    }

VOID
setup2dscreen(VOID)
    {
      // qsetmode640350();
    }


    
VOID
TerminateGame(VOID)
    {
    int i,j;
    int oldtotalclock;

    DemoTerm();
    
    ErrorCorrectionQuit();

                uninitmultiplayers();
    
    if (CleanExit)
        {
        SybexScreen();
        //TenScreen();
        }

    ////--->>>> sound stuff was there
    //uninitkeys();
    KB_Shutdown();

    uninitengine();
    TermSetup();

    //Terminate3DSounds();                // Kill the sounds linked list
    UnInitSound();

    uninittimer();

    if (CleanExit)
        DosScreen();

    uninitgroupfile();
    }

VOID
LoadLevel(char *filename)
    {
    int pos;
    
    if (loadboard(filename, SW_SHAREWARE ? 1 : 0, &Player[0].posx, &Player[0].posy, &Player[0].posz, &Player[0].pang, &Player[0].cursectnum) == -1)
        {
        TerminateGame();
#ifdef RENDERTYPEWIN
    {
        char msg[256];
        Bsnprintf(msg, 256, "Level not found: %s", filename);
        wm_msgbox(apptitle, msg);
    }
#else
        printf("Level Not Found: %s\n", filename);
#endif
        exit(0);
        }
    }

VOID
LoadImages(char *filename)
    {
    short ndx;
    FILE *fin;
    
    if (loadpics(filename, 32*1048576) == -1)
        {
        TerminateGame();
#ifdef RENDERTYPEWIN
    {
        wm_msgbox(apptitle, "Art not found. Please check your GRP file.");
    }
#else
        printf("Art not found. Please check your GRP file.\n");
#endif
        exit(-1);
        }
    }

void LoadDemoRun(void)
    {
    short i;
    FILE *fin;

    fin = fopen("demos.run","r");
    if (fin)
        {
        memset(DemoName,'\0',sizeof(DemoName));
        for (i = 0; TRUE; i++)
            {
            if (fscanf(fin, "%s", DemoName[i]) == EOF)
                break;
            }

        fclose(fin);
        }
    
    memset(DemoText,'\0',sizeof(DemoText));
    fin = fopen("demotxt.run","r");
    if (fin)
        {
        fgets(ds, 6, fin);
        sscanf(ds,"%d",&DemoTextYstart);
        for (i = 0; TRUE; i++)
            {
            if (fgets(DemoText[i], SIZ(DemoText[0])-1, fin) == NULL)
                break;
            }

        fclose(fin);
        }
    }

void DisplayDemoText(void)
    {
    short w,h;
    short i;
    
    for (i = 0; i < 3; i++)
        {
        MNU_MeasureString(DemoText[i], &w, &h);
        PutStringTimer(Player, TEXT_TEST_COL(w), DemoTextYstart+(i*12), DemoText[i], 999);
        }
    }    
    

void Set_GameMode(void)
    {
    extern int ScreenMode, ScreenWidth, ScreenHeight, ScreenBPP;
    int result;
    char ch;
    
    //DSPRINTF(ds,"ScreenMode %d, ScreenWidth %d, ScreenHeight %d",ScreenMode, ScreenWidth, ScreenHeight);
    //MONO_PRINT(ds);
    result = COVERsetgamemode(ScreenMode, ScreenWidth, ScreenHeight, ScreenBPP);
    
    if (result < 0)
        {
        buildprintf("Failure setting video mode %dx%dx%d %s! Attempting safer mode...",
                ScreenWidth,ScreenHeight,ScreenBPP,ScreenMode?"fullscreen":"windowed");
        ScreenMode = 0;
        ScreenWidth = 640;
        ScreenHeight = 480;
        ScreenBPP = 8;

        result = COVERsetgamemode(ScreenMode, ScreenWidth, ScreenHeight, ScreenBPP);
        if (result < 0)
            {
            uninitmultiplayers();
                //uninitkeys();
            KB_Shutdown();
            uninitengine();
            TermSetup();
            UnInitSound();
            uninittimer();
            DosScreen();
            uninitgroupfile();
            exit(0);
            }
        }
    }

void MultiSharewareCheck(void)
    {
    if (!SW_SHAREWARE) return;
    if (numplayers > 4)
        {
#ifdef RENDERTYPEWIN
    wm_msgbox(apptitle,"To play a Network game with more than 4 players you must purchase "
        "the full version.  Read the Ordering Info screens for details.");
#else
        printf(
"\n\nTo play a Network game with more than 4 players you must purchase the\n"
"full version.  Read the Ordering Info screens for details.\n\n");
#endif
                        uninitmultiplayers();
        //uninitkeys();
    KB_Shutdown();
        uninitengine();
        TermSetup();
        UnInitSound();
        uninittimer();
        uninitgroupfile();
        exit(0);
        }
     }
 

// Some mem crap for Jim 
// I reserve 1 meg of heap space for our use out side the cache   
int TotalMemory = 0;
int ActualHeap = 0;

VOID InitAutoNet(VOID)
    {
    if (!AutoNet)
        return;
    
    gs.NetGameType      = Auto.Rules;    
    gs.NetLevel         = Auto.Level;    
    gs.NetMonsters      = Auto.Enemy;    
    gs.NetSpawnMarkers  = Auto.Markers;    
    gs.NetTeamPlay      = Auto.Team;    
    gs.NetHurtTeammate  = Auto.HurtTeam;    
    gs.NetKillLimit     = Auto.Kill;    
    gs.NetTimeLimit     = Auto.Time;    
    gs.NetColor         = Auto.Color;    
    gs.NetNuke          = Auto.Nuke;    
    }


void AnimateCacheCursor(void)
    {    
#if 0
    struct rccoord old_pos;
    static short cursor_num = 0;
    static char cache_cursor[] =  {'|','/','-','\\'};
    
    if (GraphicsMode)
        return;
        
    cursor_num++;
    if (cursor_num > 3)
        cursor_num = 0;

    //old_pos = _gettextposition();
    //_settextposition( old_pos.row, old_pos.col );
    //_settextposition( 24,  25);
    _settextposition( 25,  0);
    sprintf(ds,"Loading sound and graphics %c", cache_cursor[cursor_num]);
    _outtext(ds);
    //_settextposition( old_pos.row, old_pos.col );
#endif
    }
    
void COVERsetbrightness(int bright, unsigned char *pal)
{
setbrightness(bright, pal, 0);
}    
    

static int firstnet = 0;    // JBF
int nextvoxid = 0;  // JBF
static const char *deffile = "sw.def";

extern int startwin_run(void);

VOID
InitGame(VOID)
    {
    extern int MovesPerPacket;
    //void *ReserveMem=NULL;
    int i;

        DSPRINTF(ds,"InitGame...");
    MONO_PRINT(ds);


    if (initengine()) {
       wm_msgbox("Build Engine Initialisation Error",
               "There was a problem initialising the Build engine: %s", engineerrstr);
       exit(1);
    }

    //initgroupfile("sw.grp");  // JBF: moving this close to start of program to detect shareware
    InitSetup();
    
    InitAutoNet();

    inittimer(120);

    CON_InitConsole();  // Init console command list

    ////DSPRINTF(ds,"%s, %d",__FILE__,__LINE__);   MONO_PRINT(ds);

    //InitFX();
    
    memcpy(palette_data,palette,768);
    InitPalette();
    // sets numplayers, connecthead, connectpoint2, myconnectindex

    if (!firstnet)
        initmultiplayers(0, NULL, 0, 0, 0);
    else if (initmultiplayersparms(_buildargc - firstnet, &_buildargv[firstnet])) {
        buildputs("Waiting for players...\n");
        while (initmultiplayerscycle()) {
            handleevents();
            if (quitevent) {
                QuitFlag = TRUE;
                return;
            }
        }
    }
    initsynccrc();
    
    // code to duplicate packets
    if (numplayers > 4 && MovesPerPacket == 1)
        {
        MovesPerPacket = 2;
        }
    
    MultiSharewareCheck();
    
    if (numplayers > 1)
        {
        CommPlayers = numplayers;
        OrigCommPlayers = CommPlayers;
        CommEnabled = TRUE;
        if(!BotMode)
            gNet.MultiGameType = MULTI_GAME_COMMBAT;
        else
            gNet.MultiGameType = MULTI_GAME_AI_BOTS;

        #if 0 //def NET_MODE_MASTER_SLAVE    
        if (!NetModeOverride)
            {
            if (numplayers <= 4)
                NetBroadcastMode = TRUE;
            else
                NetBroadcastMode = FALSE;
            }
        #endif        
        }

    LoadDemoRun();
    // Save off total heap for later calculations
    //TotalMemory = Z_AvailHeap();
    //DSPRINTF(ds,"Available Heap before LoadImages =  %d", TotalMemory);
    //MONO_PRINT(ds);
    // Reserve 1.5 megs for normal program use
    // Generally, SW is consuming about a total of 11 megs including 
    // all the cached in graphics, etc. per level, so even on a 16 meg
    // system, reserving 1.5 megs is fine.
    // Note that on a 16 meg machine, Ken was leaving us about
    // 24k for use outside the cache!  This was causing out of mem problems
    // when songs, etc., greater than the remaining heap were being loaded.
    // Even if you pre-cache songs, etc. to help, reserving some heap is
    // a very smart idea since the game uses malloc throughout execution.
    //ReserveMem = AllocMem(1L<<20);
    //if(ReserveMem == 0) MONO_PRINT("Could not allocate 1.5 meg reserve!");

    // LoadImages will now proceed to steal all the remaining heap space    
    //_outtext("\n\n\n\n\n\n\n\n");
    //AnimateCacheCursor();
    buildputs("Loading sound and graphics...\n");
    LoadImages("tiles000.art");
    
    // Now free it up for later use
    /*
    if(ReserveMem)
        {
        // Recalc TotalMemory for later reference
        ActualHeap = Z_AvailHeap() + 1536000L;
        FreeMem(ReserveMem);
        }
    */

    Connect();
    SortBreakInfo();
    //parallaxyoffs = 40;
    parallaxyoffs = 0;
    parallaxtype = 1;
    pskyoff[0] = 0;
    pskybits = PlaxBits;
    // Default scale value for parallax skies
    parallaxyscale = 8192;

    memset(Track, 0, sizeof(Track));

    memset(Player, 0, sizeof(Player));
    for (i = 0; i < MAX_SW_PLAYERS; i++)
        INITLIST(&Player[i].PanelSpriteList);

    LoadKVXFromScript( "swvoxfil.txt" );  // Load voxels from script file
    LoadPLockFromScript( "swplock.txt" ); // Get Parental Lock setup info
    if (!SW_SHAREWARE)
    LoadCustomInfoFromScript( "swcustom.txt" ); // Load user customisation information
    
    if (!loaddefinitionsfile(deffile)) buildputs("Definitions file loaded.\n");

    DemoModeMenuInit = TRUE;
    // precache as much stuff as you can
    if (UserMapName[0] == '\0')
        {
        AnimateCacheCursor();
        LoadLevel("$dozer.map");
        AnimateCacheCursor();
        SetupPreCache();
        DoTheCache();    
        }
    else
        {
        AnimateCacheCursor();
        LoadLevel(UserMapName);
        AnimateCacheCursor();
        SetupPreCache();
        DoTheCache();    
        }    

    Set_GameMode();    
    GraphicsMode = TRUE;
    SetupAspectRatio();
    
    COVERsetbrightness(gs.Brightness,&palette_data[0][0]);

    InitFX();   // JBF: do it down here so we get a hold of the window handle
    InitMusic();
        
    }


/*
Directory of C:\DEV\SW\MIDI
EXECUT11 MID
HROSHMA6 MID
HOSHIA02 MID
INTRO131 MID
KOTEC2   MID
KOTOKI12 MID
NIPPON34 MID
NOKI41   MID
SANAI    MID
SIANRA23 MID
TKYO2007 MID
TYTAIK16 MID
YOKOHA03 MID
*/

char LevelSong[16];
short SongLevelNum;
//#ifndef SW_SHAREWARE
LEVEL_INFO LevelInfo[MAX_LEVELS_REG+2] =
    {
    {"title.map",      "theme.mid", " ", " ", " "  },
    {"$bullet.map",    "e1l01.mid", "Seppuku Station", "0 : 55", "5 : 00"  },
    {"$dozer.map",     "e1l03.mid", "Zilla Construction", "4 : 59", "8 : 00"  },
    {"$shrine.map",    "e1l02.mid", "Master Leep's Temple", "3 : 16", "10 : 00"  },
    {"$woods.map",     "e1l04.mid", "Dark Woods of the Serpent", "7 : 06", "16 : 00"  },
    {"$whirl.map",     "yokoha03.mid", "Rising Son", "5 : 30", "10 : 00"   },
    {"$tank.map",      "nippon34.mid", "Killing Fields", "1 : 46", "4 : 00"   },
    {"$boat.map",      "execut11.mid", "Hara-Kiri Harbor", "1 : 56", "4 : 00"   },
    {"$garden.map",    "execut11.mid", "Zilla's Villa", "1 : 06", "2 : 00"   },
    {"$outpost.map",   "sanai.mid",    "Monastery", "1 : 23", "3 : 00"      },
    {"$hidtemp.map",   "kotec2.mid",   "Raider of the Lost Wang", "2 : 05", "4 : 10"     },
    {"$plax1.map",     "kotec2.mid",   "Sumo Sky Palace", "6 : 32", "12 : 00"     },
    {"$bath.map",      "yokoha03.mid", "Bath House", "10 : 00", "10 : 00"   },
    {"$airport.map",   "nippon34.mid", "Unfriendly Skies", "2 : 59", "6 : 00"   },
    {"$refiner.map",   "kotoki12.mid", "Crude Oil", "2 : 40", "5 : 00"   },
    {"$newmine.map",   "hoshia02.mid", "Coolie Mines", "2 : 48", "6 : 00"   },
    {"$subbase.map",   "hoshia02.mid", "Subpen 7", "2 : 02", "4 : 00"   },
    {"$rock.map",      "kotoki12.mid", "The Great Escape", "3 : 18", "6 : 00"   },
    {"$yamato.map",    "sanai.mid",    "Floating Fortress", "11 : 38", "20 : 00"      },
    {"$seabase.map",   "kotec2.mid",   "Water Torture", "5 : 07", "10 : 00"     },
    {"$volcano.map",   "kotec2.mid",   "Stone Rain", "9 : 15", "20 : 00"     },
    {"$shore.map",     "kotec2.mid",   "Shanghai Shipwreck", "3 : 58", "8 : 00"     },
    {"$auto.map",      "kotec2.mid",   "Auto Maul", "4 : 07", "8 : 00"     },
    {"tank.map",       "kotec2.mid",   "Heavy Metal (DM only)", "10 : 00", "10 : 00"     },
    {"$dmwoods.map",   "kotec2.mid",   "Ripper Valley (DM only)", "10 : 00", "10 : 00"     },
    {"$dmshrin.map",   "kotec2.mid",   "House of Wang (DM only)", "10 : 00", "10 : 00"     },
    {"$rush.map",      "kotec2.mid",   "Lo Wang Rally (DM only)", "10 : 00", "10 : 00"     },
    {"shotgun.map",    "kotec2.mid",   "Ruins of the Ronin (CTF)", "10 : 00", "10 : 00"     },
    {"$dmdrop.map",    "kotec2.mid",   "Killing Fields (CTF)", "10 : 00", "10 : 00"     },
    {NULL, NULL, NULL, NULL, NULL}
    };
/*#else
LEVEL_INFO LevelInfo[MAX_LEVELS+2] =  // Shareware
    {
    {"title.map",      "theme.mid", " ", " ", " "  },
    {"$bullet.map",    "e1l01.mid", "Seppuku Station", "0 : 55", "5 : 00"  },
    {"$dozer.map",     "e1l03.mid", "Zilla Construction", "4 : 59", "8 : 00"  },
    {"$shrine.map",    "e1l02.mid", "Master Leep's Temple", "3 : 16", "10 : 00"  },
    {"$woods.map",     "e1l04.mid", "Dark Woods of the Serpent", "7 : 06", "16 : 00"  },
    {NULL, NULL, NULL, NULL, NULL}
    };
#endif*/

char EpisodeNames[2][MAX_EPISODE_NAME_LEN+2] = {
    "^Enter the Wang",
    "^Code of Honor"
};
char EpisodeSubtitles[2][MAX_EPISODE_SUBTITLE_LEN+1] = {
    "Four levels (Shareware Version)",
    "Eighteen levels (Full Version Only)"
};
char SkillNames[4][MAX_SKILL_NAME_LEN+2] = {
    "^Tiny grasshopper",
    "^I Have No Fear",
    "^Who Wants Wang",
    "^No Pain, No Gain"
};

VOID InitNewGame(VOID)
    {
    int i, ready_bak;
    int ver_bak;
    
        //waitforeverybody();           // since ready flag resets after this point, need to carefully sync
    
    for (i = 0; i < MAX_SW_PLAYERS; i++)
        {
        // don't jack with the playerreadyflag
        ready_bak = Player[i].playerreadyflag;
        ver_bak = Player[i].PlayerVersion;
        memset(&Player[i], 0, sizeof(Player[i]));
        Player[i].playerreadyflag = ready_bak;
        Player[i].PlayerVersion = ver_bak;
        INITLIST(&Player[i].PanelSpriteList);
        }

    memset(puser, 0, sizeof(puser));
    }

void FindLevelInfo(char *map_name, short *level)
    {
    char *ptr;
    char buff[16];
    short i,j;

    for (j = 1; j <= MAX_LEVELS; j++)
        {
        if (LevelInfo[j].LevelName)
            {
            if (Bstrcasecmp(map_name, LevelInfo[j].LevelName) == 0)
                {
                *level = j;
                return;
                }
            }
        }

    *level = 0;
    return;
    }

int ChopTics;    
VOID InitLevelGlobals(VOID)
    {
    extern char PlayerGravity;
    extern short wait_active_check_offset;
    //extern short Zombies;
    extern int PlaxCeilGlobZadjust, PlaxFloorGlobZadjust;
    extern BOOL left_foot;  
    extern BOOL serpwasseen;
    extern BOOL sumowasseen;
    extern BOOL zillawasseen;
    extern short BossSpriteNum[3];
    
    // A few IMPORTANT GLOBAL RESETS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    MapSetup();
    //Zombies = 0;
    ChopTics = 0;
    dimensionmode = 3;
    zoom = 768;
    PlayerGravity = 24;
    wait_active_check_offset = 0;
    PlaxCeilGlobZadjust = PlaxFloorGlobZadjust = Z(500);
    pskyoff[0] = 0;
    pskybits = PlaxBits;
    FinishedLevel = FALSE;
    AnimCnt = 0;
    left_foot = FALSE;  
    screenpeek = myconnectindex;
    
    gNet.TimeLimitClock = gNet.TimeLimit;
    
    serpwasseen = FALSE;
    sumowasseen = FALSE;
    zillawasseen = FALSE;
    memset(BossSpriteNum,-1,sizeof(BossSpriteNum));
    }

VOID InitLevelGlobals2(VOID)
    {
    extern short Bunny_Count;
    // GLOBAL RESETS NOT DONE for LOAD GAME
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    InitTimingVars();
    TotalKillable = 0;
    Bunny_Count = 0;
    }
    
VOID
InitLevel(VOID)
    {
    static int DemoNumber = 0;
    
    MONO_PRINT("InitLevel");
    Terminate3DSounds();

    // A few IMPORTANT GLOBAL RESETS
    InitLevelGlobals();
    MONO_PRINT("InitLevelGlobals");
    if (!DemoMode)
        StopSong();    
    
    if (LoadGameOutsideMoveLoop)
        {
          MONO_PRINT("Returning from InitLevel");
          return;
        }

    InitLevelGlobals2();
    MONO_PRINT("InitLevelGlobals2");
    if (DemoMode)
        {
        Level = 0;
        NewGame = TRUE;
        DemoInitOnce = FALSE;
        strcpy(DemoFileName, DemoName[DemoNumber]);
        DemoNumber++;
        if (!DemoName[DemoNumber][0])
            DemoNumber = 0;

        // read header and such
        DemoPlaySetup();

        strcpy(LevelName, DemoLevelName);

        FindLevelInfo(LevelName, &Level);
        if (Level > 0)
            {
            strcpy(LevelSong, LevelInfo[Level].SongName);
            strcpy(LevelName, LevelInfo[Level].LevelName);
            UserMapName[0] = '\0';
            }
        else
            {
            strcpy(UserMapName, DemoLevelName);
            Level = 0;
            }    

        }
    else
        {
        if (Level < 0)
            Level = 0;

        if (Level > MAX_LEVELS)
            Level = 1;
    
        // extra code in case something is resetting these values
        if (NewGame)
            {
            //Level = 1; 
            //DemoPlaying = FALSE;
            DemoMode = FALSE;
            //DemoRecording = FALSE;
            //DemoEdit = FALSE;
            }

        if (UserMapName[0])
            {
            strcpy(LevelName, UserMapName);

            Level = 0;
            FindLevelInfo(UserMapName, &Level);

            if (Level > 0)
                {
                // user map is part of game - treat it as such
                strcpy(LevelSong, LevelInfo[Level].SongName);
                strcpy(LevelName, LevelInfo[Level].LevelName);
                UserMapName[0] = '\0';
                }
            }
        else
            {
            strcpy(LevelName, LevelInfo[Level].LevelName);
            strcpy(LevelSong, LevelInfo[Level].SongName);
            }
        }

    PlayingLevel = Level;
    
    if (NewGame)
        InitNewGame();

    LoadingLevelScreen(LevelName);
    MONO_PRINT("LoadintLevelScreen");
    if (!DemoMode && !DemoInitOnce)
        DemoPlaySetup();
    
    LoadLevel(LevelName);

    if (Bstrcasecmp(CacheLastLevel, LevelName) != 0)
        // clears gotpic and does some bit setting
        SetupPreCache();
    else
        memset(gotpic,0,sizeof(gotpic));    

    if (sector[0].extra != -1)
        {
        NormalVisibility = visibility = sector[0].extra;
        sector[0].extra = 0;
        }
    else
        NormalVisibility = visibility;

    //
    // Do Player stuff first
    //

    InitAllPlayers();

    #if DEBUG
    // fake Multi-player game setup
    if (FakeMultiNumPlayers && !BotMode)
        {
        BYTE i;

        // insert all needed players except the first one - its already tere
        for (i = 0; i < FakeMultiNumPlayers - 1; i++)
            {
            ManualPlayerInsert(Player);
            // reset control back to 1st player
            myconnectindex = 0;
            screenpeek = 0;
            }
        }
    #endif

    // Put in the BOTS if called for
    if (FakeMultiNumPlayers && BotMode)
        {
        BYTE i;

        // insert all needed players except the first one - its already tere
        for (i = 0; i < FakeMultiNumPlayers; i++)
            {
            BotPlayerInsert(Player);
            // reset control back to 1st player
            myconnectindex = 0;
            screenpeek = 0;
            }
        }

    QueueReset();
    PreMapCombineFloors();
    InitMultiPlayerInfo();
    InitAllPlayerSprites();

    //
    // Do setup for sprite, track, panel, sector, etc
    //

    // Set levels up
    InitTimingVars();

    SpriteSetup();
    SpriteSetupPost(); // post processing - already gone once through the loop
    InitLighting();

    TrackSetup();

    PlayerPanelSetup();
    MapSetup();
    SectorSetup();
    JS_InitMirrors();
    JS_InitLockouts();   // Setup the lockout linked lists
    JS_ToggleLockouts(); // Init lockouts on/off

    PlaceSectorObjectsOnTracks();
    PlaceActorsOnTracks();
    PostSetupSectorObject();
    SetupMirrorTiles();
    initlava();

    SongLevelNum = Level;
    
    if (DemoMode)
        {
        DisplayDemoText();
        }
        

    if (ArgCheat)
        {
        BOOL bak = gs.Messages;
        gs.Messages = FALSE;
        EveryCheatToggle(&Player[0],NULL);
        gs.Messages = bak;
        GodMode = TRUE;
        }
        
    // reset NewGame
    NewGame = FALSE;

    DSPRINTF(ds,"End of InitLevel...");
    MONO_PRINT(ds);

    #if 0
    #if DEBUG
    if (!cansee(43594, -92986, 0x3fffffff, 290,
        43180, -91707, 0x3fffffff, 290))
        {        
        DSPRINTF(ds,"cansee failed");
        MONO_PRINT(ds);
        }
    #endif    
    #endif
        
    }


VOID
TerminateLevel(VOID)
    {
    VOID pClearSpriteList(PLAYERp pp);
    int i, nexti, stat, pnum, ndx;
    SECT_USERp *sectu;

//HEAP_CHECK();
    
    DemoTerm();

    // Free any track points
    for (ndx = 0; ndx < MAX_TRACKS; ndx++)
        {
        if (Track[ndx].TrackPoint)
            {
            FreeMem(Track[ndx].TrackPoint);
            // !JIM! I added null assigner
            Track[ndx].TrackPoint = NULL;
            }
        }

    // Clear the tracks
    memset(Track, 0, sizeof(Track));

    StopSound();
    Terminate3DSounds();        // Kill the 3d sounds linked list
    //ClearSoundLocks();

    // Clear all anims and any memory associated with them
    // Clear before killing sprites - save a little time
    //AnimClear();

    for (stat = STAT_PLAYER0; stat < STAT_PLAYER0 + numplayers; stat++)
        {

        pnum = stat - STAT_PLAYER0;

        TRAVERSE_SPRITE_STAT(headspritestat[stat], i, nexti)
            {
            if (User[i])
                memcpy(&puser[pnum], User[i], sizeof(USER));
            }
        }

    // Kill User memory and delete sprites
    // for (stat = 0; stat < STAT_ALL; stat++)
    for (stat = 0; stat < MAXSTATUS; stat++)
        {
        TRAVERSE_SPRITE_STAT(headspritestat[stat], i, nexti)
            {
            KillSprite(i);
            }
        }

    // Free SectUser memory
    for (sectu = &SectUser[0];
        sectu < &SectUser[MAXSECTORS];
        sectu++)
        {
        if (*sectu)
            {
            ////DSPRINTF(ds,"Sect User Free %d",sectu-SectUser);
            //MONO_PRINT(ds);
            FreeMem(*sectu);
            *sectu = NULL;
            }
        }

    //memset(&User[0], 0, sizeof(User));
    memset(&SectUser[0], 0, sizeof(SectUser));

    TRAVERSE_CONNECT(pnum)
        {
        PLAYERp pp = Player + pnum;

        // Free panel sprites for players
        pClearSpriteList(pp);

        pp->DoPlayerAction = NULL;

        pp->SpriteP = NULL;
        pp->PlayerSprite = -1;

        pp->UnderSpriteP = NULL;
        pp->PlayerUnderSprite = -1;

        memset(pp->HasKey, 0, sizeof(pp->HasKey));

        //pp->WpnFlags = 0;
        pp->CurWpn = NULL;

        memset(pp->Wpn, 0, sizeof(pp->Wpn));
        memset(pp->InventorySprite, 0, sizeof(pp->InventorySprite));
        memset(pp->InventoryTics, 0, sizeof(pp->InventoryTics));

        pp->Killer = -1;

        INITLIST(&pp->PanelSpriteList);
        }

    JS_UnInitLockouts();

//HEAP_CHECK();    
    }

VOID
NewLevel(VOID)
    {

    DSPRINTF(ds,"NewLevel");
    MONO_PRINT(ds);

    if (DemoPlaying)
        {
        FX_SetVolume(0); // Shut the hell up while game is loading!
        InitLevel();        
        InitRunLevel();
        
        DemoInitOnce = FALSE;
        if (DemoMode)
            {
            if (DemoModeMenuInit)
                {
                DemoModeMenuInit = FALSE;
                KEY_PRESSED(KEYSC_ESC) = TRUE;
                }
            }
            
        DemoPlayBack();
        
        if (DemoRecording && DemoEdit)
            {
            RunLevel();
            }
        }
    else
        {
        DSPRINTF(ds,"Calling FX_SetVolume");
        MONO_PRINT(ds);
        FX_SetVolume(0); // Shut the hell up while game is loading!

        DSPRINTF(ds,"Calling InitLevel");
        MONO_PRINT(ds);
        InitLevel();

        DSPRINTF(ds,"Calling RunLevel");
        MONO_PRINT(ds);
        RunLevel();

        if (!QuitFlag)
            {
            // for good measure do this
            ready2send = 0;
            waitforeverybody();
            }

        StatScreen(&Player[myconnectindex]);
        }
    
    if (LoadGameFromDemo)
        LoadGameFromDemo = FALSE;
    else    
        TerminateLevel();
    
    InGame = FALSE;
    
    if (SW_SHAREWARE)
        {
        if (FinishAnim)
            MenuLevel();
        }
    else
        {
        if (FinishAnim == ANIM_ZILLA || FinishAnim == ANIM_SERP)
            MenuLevel();
        }
    FinishAnim = 0;
    }

VOID
ResetKeys(VOID)
    {
    int i;

    for (i = 0; i < MAXKEYBOARDSCAN; i++)
        {
        KEY_PRESSED(i) = 0;
        }
    }

BOOL
KeyPressed(VOID)
    {
    int i;

    for (i = 0; i < MAXKEYBOARDSCAN; i++)
        {
        if (KEY_PRESSED(i))
            return (TRUE);
        }

    return (FALSE);
    }

BYTEp
KeyPressedRange(BYTEp kb, BYTEp ke)
    {
    BYTEp k;

    for (k = kb; k <= ke; k++)
        {
        if (*k)
            return (k);
        }

    return (NULL);
    }

VOID
ResetKeyRange(BYTEp kb, BYTEp ke)
    {
    BYTEp k;

    for (k = kb; k <= ke; k++)
        {
        *k = 0;
        }
    }


VOID
LogoLevel(VOID)
    {
    char called;
    int fin;
    unsigned char backup_pal[256*3];
    unsigned char pal[PAL_SIZE];
    unsigned char tempbuf[256];
    unsigned char *palook_bak = palookup[0];
    int i;
    

    DSPRINTF(ds,"LogoLevel...");
    MONO_PRINT(ds);

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    palookup[0] = tempbuf;    

    DSPRINTF(ds,"Created palookup...");
    MONO_PRINT(ds);

    // start music at logo
    strcpy(LevelSong,"theme.mid");
    PlaySong(LevelSong, RedBookSong[0], TRUE, TRUE);

    DSPRINTF(ds,"After music stuff...");
    MONO_PRINT(ds);    

    //GetPaletteFromVESA(pal);
    //memcpy(backup_pal, pal, PAL_SIZE);

    DSPRINTF(ds,"Got Palette from VESA...");
    MONO_PRINT(ds);

    // PreCache Anim
    LoadAnm(0);

    if ((fin = kopen4load("3drealms.pal", 0)) != -1)
        {
        kread(fin, pal, PAL_SIZE);
        kclose(fin);
        setbrightness(gs.Brightness, pal, 2);
        }
    DSPRINTF(ds,"Just read in 3drealms.pal...");
    MONO_PRINT(ds);

    //FadeOut(0, 0);
    ready2send = 0;
    totalclock = 0;
    ototalclock = 0;

    DSPRINTF(ds,"About to display 3drealms pic...");
    MONO_PRINT(ds);

    clearview(0);
    rotatesprite(0, 0, RS_SCALE, 0, THREED_REALMS_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    nextpage();
    //FadeIn(0, 3);

    ResetKeys();
    while (TRUE)
        {
        handleevents();

        if (quitevent) { QuitFlag = TRUE; break; }
    
        // taken from top of faketimerhandler
        // limits checks to max of 40 times a second
        if (totalclock >= ototalclock + synctics)
            {
            ototalclock += synctics;
            }

        if (totalclock > 5*120 || KeyPressed())
            {
            break;
            }
        }

    palookup[0] = palook_bak;
        
    clearview(0);
    nextpage();
    //SetPaletteToVESA(backup_pal);
    setbrightness(gs.Brightness, &palette_data[0][0], 2);

    // put up a blank screen while loading

    DSPRINTF(ds,"End of LogoLevel...");
    MONO_PRINT(ds);

    }

VOID
CreditsLevel(VOID)
    {
    char called;
    int fin;
    int i;
    int curpic;
    int handle;
    ULONG timer = 0;
    int zero=0;
    short save;
    #define CREDITS1_PIC 5111
    #define CREDITS2_PIC 5118

    // put up a blank screen while loading
    
    // get rid of all PERM sprites!
    flushperms();
    save = gs.BorderNum;
    SetBorder(Player + myconnectindex,0);
    ClearStartMost();
    gs.BorderNum = save;
    clearview(0);
    nextpage();
    
    // Lo Wang feel like singing!
    handle = PlaySound(DIGI_JG95012,&zero,&zero,&zero,v3df_none);
    
    if (handle > 0)
        while(FX_SoundActive(handle));
    
    // try 14 then 2 then quit
    if (!PlaySong(NULL, 14, FALSE, TRUE)) {
        if (!PlaySong(NULL, 2, FALSE, TRUE)) {
            handle = PlaySound(DIGI_NOLIKEMUSIC,&zero,&zero,&zero,v3df_none);
            if (handle > 0)
                while(FX_SoundActive(handle)) handleevents();
            return;
        }
    }

    ready2send = 0;
    totalclock = 0;
    ototalclock = 0;

    ResetKeys();
    curpic = CREDITS1_PIC;
    
    while (TRUE)
        {
        // taken from top of faketimerhandler
        // limits checks to max of 40 times a second
        if (totalclock >= ototalclock + synctics)
            {
            ototalclock += synctics;
            timer += synctics;
            }

        rotatesprite(0, 0, RS_SCALE, 0, curpic, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);

        nextpage();
        
        if (timer > 8*120)
            {
            curpic = CREDITS2_PIC;
            }
            
        if (timer > 16*120)
            {
            timer = 0;
            curpic = CREDITS1_PIC;
            }
        

        if (!SongIsPlaying())
            break;
            
        if (KEY_PRESSED(KEYSC_ESC))
            break;
        }

    // put up a blank screen while loading
    clearview(0);
    nextpage();
    ResetKeys();
    StopSong();
    }


VOID
SybexScreen(VOID)
    {
    if (!SW_SHAREWARE) return;

    if (CommEnabled)
        return;

    rotatesprite(0, 0, RS_SCALE, 0, 5261, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    nextpage();
    
    ResetKeys();
    while (!KeyPressed() && !quitevent) handleevents();
    }

// CTW REMOVED
/*
VOID
TenScreen(VOID)
    {
    char called;
    int fin;
    char backup_pal[256*3];
    char pal[PAL_SIZE];
    char tempbuf[256];
    char *palook_bak = palookup[0];
    int i;
    ULONG bak;
    int bakready2send;
    
    if (CommEnabled)
        return;

    bak = totalclock;
    
    flushperms();
    clearview(0);
    nextpage();

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    palookup[0] = tempbuf;    

    GetPaletteFromVESA(pal);
    memcpy(backup_pal, pal, PAL_SIZE);

    if ((fin = kopen4load("ten.pal", 0)) != -1)
        {
        kread(fin, pal, PAL_SIZE);
        kclose(fin);
        }

    // palette to black    
    FadeOut(0, 0);
    bakready2send = ready2send;
    //totalclock = 0;
    //ototalclock = 0;

    flushperms();
    // draw it
    rotatesprite(0, 0, RS_SCALE, 0, TEN_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    // bring to the front - still back palette
    nextpage();
    // set pal
    SetPaletteToVESA(pal);
    //FadeIn(0, 3);
    ResetKeys();
    
    while (!KeyPressed());

    palookup[0] = palook_bak;
        
    clearview(0);
    nextpage();
    SetPaletteToVESA(backup_pal);

    // put up a blank screen while loading
    clearview(0);
    nextpage();
    
    ready2send = bakready2send;
    totalclock = bak;
    }
*/
// CTW REMOVED END
    
VOID
TitleLevel(VOID)
    {
    char called;
    int fin;
    unsigned char backup_pal[256*3];
    unsigned char pal[PAL_SIZE];
    unsigned char tempbuf[256];
    unsigned char *palook_bak = palookup[0];
    int i;
    
    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    palookup[0] = tempbuf;    

    //GetPaletteFromVESA(pal);
    //memcpy(backup_pal, pal, PAL_SIZE);

    clearview(0);
    nextpage();

//    if ((fin = kopen4load("title.pal", 0)) != -1)
//        {
//        kread(fin, pal, PAL_SIZE);
//        kclose(fin);
//        SetPaletteToVESA(pal);
//        }

//    clearview(0);
//    nextpage();

    //FadeOut(0, 0);
    ready2send = 0;
    totalclock = 0;
    ototalclock = 0;

    rotatesprite(0, 0, RS_SCALE, 0, TITLE_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    nextpage();
    //FadeIn(0, 3);

    ResetKeys();
    while (TRUE)
        {
        handleevents();
        OSD_DispatchQueued();

        // taken from top of faketimerhandler
        // limits checks to max of 40 times a second
        if (totalclock >= ototalclock + synctics)
            {
            //void MNU_CheckForMenusAnyKey( void );

            ototalclock += synctics;
            //MNU_CheckForMenusAnyKey();
            }

        //if (UsingMenus)
        //    MNU_DrawMenu();

        //drawscreen as fast as you can
        rotatesprite(0, 0, RS_SCALE, 0, TITLE_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);

        nextpage();

        if (totalclock > 5*120 || KeyPressed())
            {
            DemoMode = TRUE;
            DemoPlaying = TRUE;
            break;
            }
        }

    palookup[0] = palook_bak;
    
//    clearview(0);
//    nextpage();
    //SetPaletteToVESA(backup_pal);

    // put up a blank screen while loading
//    clearview(0);
//    nextpage();
    }


VOID DrawMenuLevelScreen(VOID)
    {
    flushperms();
    clearview(0);
    rotatesprite(0, 0, RS_SCALE, 0, TITLE_PIC, 20, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    }

VOID DrawStatScreen(VOID)
    {
    flushperms();
    clearview(0);
    rotatesprite(0, 0, RS_SCALE, 0, STAT_SCREEN_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    }
    
VOID DrawLoadLevelScreen(VOID)
    {
    flushperms();
    clearview(0);
    rotatesprite(0, 0, RS_SCALE, 0, TITLE_PIC, 20, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    }
    
short PlayerQuitMenuLevel = -1;

VOID
IntroAnimLevel(VOID)
    {
      DSPRINTF(ds,"IntroAnimLevel");
      MONO_PRINT(ds);
      playanm(0);
    }
    
VOID
MenuLevel(VOID)
    {
    BOOL MNU_StartNetGame(void);
    char called;
    int fin;
    extern int totalclocklock;
    short w,h;

    DSPRINTF(ds,"MenuLevel...");
    MONO_PRINT(ds);

    if (gs.MusicOn)
        {
        PlaySong(NULL, RedBookSong[0], TRUE, FALSE);
        }    
    
    if (AutoNet)
        {
        DrawMenuLevelScreen();
        
        if (CommEnabled)
            {
            sprintf(ds,"Lo Wang is waiting for other players...");
            MNU_MeasureString(ds, &w, &h);
            MNU_DrawString(TEXT_TEST_COL(w), 170, ds, 1, 16);
            
            sprintf(ds,"They are afraid!");
            MNU_MeasureString(ds, &w, &h);
            MNU_DrawString(TEXT_TEST_COL(w), 180, ds, 1, 16);
            }
            
        nextpage();
    
        waitforeverybody();
                FirstTimeIntoGame = TRUE;
        MNU_StartNetGame();
                FirstTimeIntoGame = FALSE;
        waitforeverybody();
        ExitLevel = FALSE;
        FinishedLevel = FALSE;
        BorderAdjust = TRUE;
        UsingMenus = FALSE;
        InMenuLevel = FALSE;
        return;
        }

    // do demos only if not playing multi play    
    if (!CommEnabled && numplayers <= 1 && !FinishAnim && !NoDemoStartup)    
        {
        // demos exist - do demo instead
        if (DemoName[0][0] != '\0')
            {
            DemoMode = TRUE;
            DemoPlaying = TRUE;
            return;
            }
        }    

    DemoMode = FALSE;
    DemoPlaying = FALSE;
        
    clearview(0);
    nextpage();

    //FadeOut(0, 0);
    ready2send = 0;
    totalclock = 0;
    ototalclock = 0;
    ExitLevel = FALSE;
    InMenuLevel = TRUE;

    DrawMenuLevelScreen();
    
    if (CommEnabled)
        {
        sprintf(ds,"Lo Wang is waiting for other players...");
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(TEXT_TEST_COL(w), 170, ds, 1, 16);
        
        sprintf(ds,"They are afraid!");
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(TEXT_TEST_COL(w), 180, ds, 1, 16);
        }
    
    nextpage();
    //FadeIn(0, 3);
    
    waitforeverybody();

    // don't allow BorderAdjusting in these menus
    BorderAdjust = FALSE;
    
    ResetKeys();
    
    if (SW_SHAREWARE) {
    // go to ordering menu only if shareware
    if (FinishAnim)
        {
        KEY_PRESSED(KEYSC_ESC) = 1;
        ControlPanelType = ct_ordermenu;
        FinishAnim = 0;
        }
    } else {
    FinishAnim = 0;
    }
    
    while (TRUE)
        {
        handleevents();
        OSD_DispatchQueued();
    
        if (quitevent) QuitFlag = TRUE;

        // taken from top of faketimerhandler
        // limits checks to max of 40 times a second
        if (totalclock >= ototalclock + synctics)
            {
            ototalclock += synctics;
            MNU_CheckForMenusAnyKey();
            if (CommEnabled)
                getpackets();
            }
    
        if (CommEnabled)
            {
            if (MultiPlayQuitFlag)
                {
                short pnum;
                BYTE pbuf[1];
                QuitFlag = TRUE;
                                pbuf[0] = PACKET_TYPE_MENU_LEVEL_QUIT;
                                netbroadcastpacket(pbuf, 1);                      // TENSW
                break;
                }
            
            if (PlayerQuitMenuLevel >= 0)    
                {
                MenuCommPlayerQuit(PlayerQuitMenuLevel);
                PlayerQuitMenuLevel = -1;
                }
            }    
            
        if (ExitLevel)
            {
            // Quiting Level
            ExitLevel = FALSE;
            break;
            }

        if (QuitFlag)
            {
            // Quiting Game
            break;
            }
            
        // force the use of menus at all time
        if (!UsingMenus && !ConPanel)
            {
            KEY_PRESSED(KEYSC_ESC) = TRUE;    
            MNU_CheckForMenusAnyKey();
            }
        
        // must lock the clock for drawing so animations will happen    
        totalclocklock = totalclock;
            
        //drawscreen as fast as you can
        DrawMenuLevelScreen();

        if (UsingMenus)
            MNU_DrawMenu();

        nextpage();
        }

    BorderAdjust = TRUE;
    //LoadGameOutsideMoveLoop = FALSE;
    KEY_PRESSED(KEYSC_ESC) = FALSE;    
    KB_ClearKeysDown();
    //ExitMenus();
    UsingMenus = FALSE;
    InMenuLevel = FALSE;
    clearview(0);
    nextpage();
    }

VOID
SceneLevel(VOID)
    {
    BOOL dp_bak;
    BOOL dm_bak;
    FILE *fin;
    #define CINEMATIC_DEMO_FILE "$scene.dmo"

    // make sure it exists
    if ((fin = fopen(CINEMATIC_DEMO_FILE,"rb")) == NULL)
        return;
    else
        fclose(fin);

    strcpy(DemoFileName,CINEMATIC_DEMO_FILE);

    dp_bak = DemoPlaying;
    dm_bak = DemoMode;

    DemoMode = TRUE;
    DemoPlaying = TRUE;
    DemoOverride = TRUE;
    InitLevel();
    DemoOverride = FALSE;

    ScenePlayBack();
    TerminateLevel();
    DemoMode = dm_bak;
    DemoPlaying = dp_bak;
    }

VOID
LoadingLevelScreen(char *level_name)
    {
    short w,h;
    extern BOOL DemoMode;
    extern char *MNU_LevelName[28];
    DrawLoadLevelScreen();
    
    if (DemoMode)
        sprintf(ds,"DEMO");
    else    
        sprintf(ds,"ENTERING");
        
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 170, ds,1,16);
    
    if (UserMapName[0])
        sprintf(ds,"%s",UserMapName);
    else    
        sprintf(ds,"%s",LevelInfo[Level].Description);
        
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 180, ds,1,16);
    
    nextpage();
    }

VOID
gNextState(STATEp *State)
    {
    // Transition to the next state
    *State = (*State)->NextState;

    if (TEST((*State)->Tics, SF_QUICK_CALL))
        {
        (*(*State)->Animator)(0);
        *State = (*State)->NextState;
        }
    }
    
// Generic state control 
VOID   
gStateControl(STATEp *State, int *tics)
    {    
    *tics += synctics;

    // Skip states if too much time has passed
    while (*tics >= (*State)->Tics)
        {
        // Set Tics
        *tics -= (*State)->Tics;
        gNextState(State);
        }

    // Call the correct animator
    if ((*State)->Animator)
        (*(*State)->Animator)(0);
    }    

int BonusPunchSound(short SpriteNum)
    {
    PLAYERp pp = Player + myconnectindex;
    PlaySound(DIGI_PLAYERYELL3, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    return(0);
    }    

int BonusKickSound(short SpriteNum)
    {
    PLAYERp pp = Player + myconnectindex;
    PlaySound(DIGI_PLAYERYELL2, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    return(0);
    }    

int BonusGrabSound(short SpriteNum)
    {
    PLAYERp pp = Player + myconnectindex;
    PlaySound(DIGI_BONUS_GRAB, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    return(0);
    }    
    
VOID
BonusScreen(PLAYERp pp)
    {
    int minutes,seconds,second_tics;
    extern BOOL FinishedLevel;
    extern int PlayClock;
    extern short LevelSecrets;
    extern short TotalKillable;
    short w,h;
    short pic,limit;
    int zero=0;
    int handle = 0;
    short LI_Num;
    
    
    #define BONUS_SCREEN_PIC 5120
    #define BONUS_ANIM 5121
    #define BONUS_ANIM_FRAMES (5159-5121)

    #define BREAK_LIGHT_RATE 18
    
    #define BONUS_PUNCH 5121
    #define BONUS_KICK 5136
    #define BONUS_GRAB 5151
    #define BONUS_REST 5121
    
    #define BONUS_TICS 8
    #define BONUS_GRAB_TICS 20
    #define BONUS_REST_TICS 50
    
    static STATE s_BonusPunch[] =
        {
        {BONUS_PUNCH + 0, BONUS_TICS, NULL, &s_BonusPunch[1]},
        {BONUS_PUNCH + 1, BONUS_TICS, NULL, &s_BonusPunch[2]},
        {BONUS_PUNCH + 2, BONUS_TICS, NULL, &s_BonusPunch[3]},
        {BONUS_PUNCH + 2, 0|SF_QUICK_CALL, BonusPunchSound, &s_BonusPunch[4]},
        {BONUS_PUNCH + 3, BONUS_TICS, NULL, &s_BonusPunch[5]},
        {BONUS_PUNCH + 4, BONUS_TICS, NULL, &s_BonusPunch[6]},
        {BONUS_PUNCH + 5, BONUS_TICS, NULL, &s_BonusPunch[7]},
        {BONUS_PUNCH + 6, BONUS_TICS, NULL, &s_BonusPunch[8]},
        {BONUS_PUNCH + 7, BONUS_TICS, NULL, &s_BonusPunch[9]},
        {BONUS_PUNCH + 8, BONUS_TICS, NULL, &s_BonusPunch[10]},
        {BONUS_PUNCH + 9, BONUS_TICS, NULL, &s_BonusPunch[11]},
        {BONUS_PUNCH + 10, BONUS_TICS, NULL, &s_BonusPunch[12]},
        {BONUS_PUNCH + 11, BONUS_TICS, NULL, &s_BonusPunch[13]},
        {BONUS_PUNCH + 12, BONUS_TICS, NULL, &s_BonusPunch[14]},
        {BONUS_PUNCH + 14, 90,        NULL, &s_BonusPunch[15]},
        {BONUS_PUNCH + 14, BONUS_TICS, NULL, &s_BonusPunch[15]},
        };

    static STATE s_BonusKick[] =
        {
        {BONUS_KICK + 0, BONUS_TICS, NULL, &s_BonusKick[1]},
        {BONUS_KICK + 1, BONUS_TICS, NULL, &s_BonusKick[2]},
        {BONUS_KICK + 2, BONUS_TICS, NULL, &s_BonusKick[3]},
        {BONUS_KICK + 2, 0|SF_QUICK_CALL, BonusKickSound, &s_BonusKick[4]},
        {BONUS_KICK + 3, BONUS_TICS, NULL, &s_BonusKick[5]},
        {BONUS_KICK + 4, BONUS_TICS, NULL, &s_BonusKick[6]},
        {BONUS_KICK + 5, BONUS_TICS, NULL, &s_BonusKick[7]},
        {BONUS_KICK + 6, BONUS_TICS, NULL, &s_BonusKick[8]},
        {BONUS_KICK + 7, BONUS_TICS, NULL, &s_BonusKick[9]},
        {BONUS_KICK + 8, BONUS_TICS, NULL, &s_BonusKick[10]},
        {BONUS_KICK + 9, BONUS_TICS, NULL, &s_BonusKick[11]},
        {BONUS_KICK + 10, BONUS_TICS, NULL, &s_BonusKick[12]},
        {BONUS_KICK + 11, BONUS_TICS, NULL, &s_BonusKick[13]},
        {BONUS_KICK + 12, BONUS_TICS, NULL, &s_BonusKick[14]},
        {BONUS_KICK + 14, 90,        NULL, &s_BonusKick[15]},
        {BONUS_KICK + 14, BONUS_TICS, NULL, &s_BonusKick[15]},
        };

    static STATE s_BonusGrab[] =
        {
        {BONUS_GRAB + 0, BONUS_GRAB_TICS, NULL, &s_BonusGrab[1]},
        {BONUS_GRAB + 1, BONUS_GRAB_TICS, NULL, &s_BonusGrab[2]},
        {BONUS_GRAB + 2, BONUS_GRAB_TICS, NULL, &s_BonusGrab[3]},
        {BONUS_GRAB + 2, 0|SF_QUICK_CALL, BonusGrabSound, &s_BonusGrab[4]},
        {BONUS_GRAB + 3, BONUS_GRAB_TICS, NULL, &s_BonusGrab[5]},
        {BONUS_GRAB + 4, BONUS_GRAB_TICS, NULL, &s_BonusGrab[6]},
        {BONUS_GRAB + 5, BONUS_GRAB_TICS, NULL, &s_BonusGrab[7]},
        {BONUS_GRAB + 6, BONUS_GRAB_TICS, NULL, &s_BonusGrab[8]},
        {BONUS_GRAB + 7, BONUS_GRAB_TICS, NULL, &s_BonusGrab[9]},
        {BONUS_GRAB + 8, BONUS_GRAB_TICS, NULL, &s_BonusGrab[10]},
        {BONUS_GRAB + 9, 90,             NULL, &s_BonusGrab[11]},
        {BONUS_GRAB + 9, BONUS_GRAB_TICS, NULL, &s_BonusGrab[11]},
        };

    #if 1 // Turned off the standing animate because he looks like a FAG!
    static STATE s_BonusRest[] =
        {
        {BONUS_REST + 0, BONUS_REST_TICS, NULL, &s_BonusRest[1]},
        {BONUS_REST + 1, BONUS_REST_TICS, NULL, &s_BonusRest[2]},
        {BONUS_REST + 2, BONUS_REST_TICS, NULL, &s_BonusRest[3]},
        {BONUS_REST + 1, BONUS_REST_TICS, NULL, &s_BonusRest[0]},
        };
    #else        
    static STATE s_BonusRest[] =
        {
        {BONUS_REST + 0, BONUS_REST_TICS, NULL, &s_BonusRest[1]},
        {BONUS_REST + 0, BONUS_REST_TICS, NULL, &s_BonusRest[0]},
        };
    #endif        
    
    static STATEp s_BonusAnim[] = 
        {
        s_BonusPunch,
        s_BonusKick,
        s_BonusGrab
        };
        
    STATEp State = s_BonusRest;        
    
    int Tics = 0;
    int line = 0;
    BOOL BonusDone;

    if(Level < 0) Level = 0;
    
    clearview(0);
    nextpage();
    
    KB_ClearKeysDown();
    
    totalclock = ototalclock = 0;
    limit = synctics;

    if (gs.MusicOn)
        {
        PlaySong(voc[DIGI_ENDLEV].name, 3, TRUE, TRUE);
        }    
        
    // special case code because I don't care any more!
    if (FinishAnim)
        {
        flushperms();
        rotatesprite(0, 0, RS_SCALE, 0, 5120, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        rotatesprite(158<<16, 86<<16, RS_SCALE, 0, State->Pic, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        nextpage();
        FadeIn(0,0);
        }
    
    BonusDone = FALSE;
    while (!BonusDone)
        {
        handleevents();

        // taken from top of faketimerhandler
        if (totalclock < ototalclock + limit)
            {
            continue;
            }
        ototalclock += limit;
        
        if (KEY_PRESSED(KEYSC_SPACE) || KEY_PRESSED(KEYSC_ENTER))
            {
            if (State >= s_BonusRest && State < &s_BonusRest[SIZ(s_BonusRest)])
                {
                State = s_BonusAnim[STD_RANDOM_RANGE(SIZ(s_BonusAnim))];
                Tics = 0;
                }
            }
        
        gStateControl(&State, &Tics);
        rotatesprite(0, 0, RS_SCALE, 0, 5120, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        
        if (UserMapName[0])
            {
            sprintf(ds,"%s",UserMapName);
            MNU_MeasureString(ds, &w, &h);
            MNU_DrawString(TEXT_TEST_COL(w), 20, ds,1,19);
            }
        else
            {
            if (PlayingLevel <= 1)
                PlayingLevel = 1;
            sprintf(ds,"%s",LevelInfo[PlayingLevel].Description);
            MNU_MeasureString(ds, &w, &h);
            MNU_DrawString(TEXT_TEST_COL(w), 20, ds,1,19);
            }

        sprintf(ds,"Completed");
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(TEXT_TEST_COL(w), 30, ds,1,19);

        rotatesprite(158<<16, 86<<16, RS_SCALE, 0, State->Pic, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        
        #define BONUS_LINE(i) (50 + ((i)*20))
        
        line = 0;
        second_tics = (PlayClock/120);
        minutes = (second_tics/60);
        seconds = (second_tics%60);
        sprintf(ds,"Your Time:  %2d : %02d", minutes, seconds);
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(60, BONUS_LINE(line), ds,1,16);

        if (!UserMapName[0])
            {
            line++;
            sprintf(ds,"3D Realms Best Time:  %s", LevelInfo[PlayingLevel].BestTime);
            MNU_MeasureString(ds, &w, &h);
            MNU_DrawString(40, BONUS_LINE(line), ds,1,16);

            line++;
            sprintf(ds,"Par Time:  %s", LevelInfo[PlayingLevel].ParTime);
            MNU_MeasureString(ds, &w, &h);
            MNU_DrawString(40, BONUS_LINE(line), ds,1,16);
            }

        
        // always read secrets and kills from the first player    
        line++;
        sprintf(ds,"Secrets:  %d / %d", Player->SecretsFound, LevelSecrets);
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(60, BONUS_LINE(line), ds,1,16);

        line++;
        sprintf(ds,"Kills:  %d / %d", Player->Kills, TotalKillable);
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(60, BONUS_LINE(line), ds,1,16);
        

        sprintf(ds,"Press SPACE to continue");
        MNU_MeasureString(ds, &w, &h);
        MNU_DrawString(TEXT_TEST_COL(w), 185, ds,1,19);

        nextpage();
        ScreenCaptureKeys();
        
        if (State == State->NextState)
            BonusDone = TRUE;
        }

    StopSound();
    Terminate3DSounds();
    }

VOID EndGameSequence(VOID)
    {
    BOOL anim_ok = TRUE;
    FadeOut(0, 5);

    if ((gs.ParentalLock || Global_PLock) && FinishAnim == ANIM_SUMO)
        anim_ok = FALSE;

    if (anim_ok)    
        playanm(FinishAnim);
        
    BonusScreen(Player + myconnectindex);

    ExitLevel = FALSE;
    QuitFlag = FALSE;
    AutoNet = FALSE;
    
    if (FinishAnim == ANIM_ZILLA)
        CreditsLevel();

    ExitLevel = FALSE;
    QuitFlag = FALSE;
    AutoNet = FALSE;
    
    if (SW_SHAREWARE) {
        Level = 0;
    } else {
        if (Level == 4 || Level == 20)
            {
            //CDAudio_Stop();
            //CDAudio_Play(2,TRUE); // Play theme after game ends
            Level=0;
            }
        else
            Level++;        
    }
    }

VOID
StatScreen(PLAYERp mpp)
    {
    int minutes,seconds,second_tics;
    extern BOOL FinishedLevel;
    extern int PlayClock;
    extern short LevelSecrets;
    extern short TotalKillable;
    short w,h;
    int zero=0;
    int handle=0;
    
    short rows,cols,i,j;
    PLAYERp pp = NULL;
    int x,y;
    short death_total[MAX_SW_PLAYERS_REG];
    short kills[MAX_SW_PLAYERS_REG];
    short pal;
    
    #define STAT_START_X 20
    #define STAT_START_Y 85
    #define STAT_OFF_Y 9
    #define STAT_HEADER_Y 14
    
    #define SM_SIZ(num) ((num)*4)
    
    #define STAT_TABLE_X (STAT_START_X + SM_SIZ(15))
    #define STAT_TABLE_XOFF SM_SIZ(6)

    // No stats in bot games    
    //if (BotMode) return;

    ResetPalette(mpp);
    COVER_SetReverb(0); // Reset reverb    
    StopSound();

    if (FinishAnim)
        {
        EndGameSequence();
        return;
        }
    
    if (gNet.MultiGameType != MULTI_GAME_COMMBAT)
        {
        if (!FinishedLevel)
            return;
        BonusScreen(mpp);
        return;
        }
        
    flushperms();
    DrawStatScreen();
        
    memset(death_total,0,sizeof(death_total));
    memset(kills,0,sizeof(kills));

    sprintf(ds,"MULTIPLAYER TOTALS");
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 68, ds, 0, 0);
    
    sprintf(ds,"PRESS SPACE BAR TO CONTINUE");
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 189, ds, 0, 0);
    
    x = STAT_START_X;
    y = STAT_START_Y;
    
    sprintf(ds,"  NAME         1     2     3     4     5     6     7    8     KILLS");
    DisplayMiniBarSmString(mpp, x, y, 0, ds);
    rows = OrigCommPlayers;
    cols = OrigCommPlayers;
    mpp = Player + myconnectindex;
    
    y += STAT_HEADER_Y;

    for (i = 0; i < rows; i++)
        {
        x = STAT_START_X;
        pp = Player + i;
        
        sprintf(ds,"%d", i+1);
        DisplayMiniBarSmString(mpp, x, y, 0, ds);

        sprintf(ds,"  %-13s", pp->PlayerName);
        DisplayMiniBarSmString(mpp, x, y, User[pp->PlayerSprite]->spal, ds);
        
        x = STAT_TABLE_X;
        for (j = 0; j < cols; j++)
            {
            pal = 0;
            death_total[j] += pp->KilledPlayer[j];
            
            if (i == j)
                {
                // don't add kill for self or team player
                pal = PALETTE_PLAYER0 + 4;
                kills[i] -= pp->KilledPlayer[j];  // subtract self kills
                }
            else
            if (gNet.TeamPlay)
                {
                if (User[pp->PlayerSprite]->spal == User[Player[j].PlayerSprite]->spal)
                    {
                    // don't add kill for self or team player
                    pal = PALETTE_PLAYER0 + 4;
                    kills[i] -= pp->KilledPlayer[j];  // subtract self kills
                    } else
                    kills[i] += pp->KilledPlayer[j];  // kills added here
                } 
            else    
                {
                kills[i] += pp->KilledPlayer[j];  // kills added here
                }
                   
            sprintf(ds,"%d", pp->KilledPlayer[j]);
            DisplayMiniBarSmString(mpp, x, y, pal, ds);
            x += STAT_TABLE_XOFF;
            }
        
        y += STAT_OFF_Y;
        }

        
    // Deaths
    
    x = STAT_START_X;
    y += STAT_OFF_Y;
    
    sprintf(ds,"   DEATHS");
    DisplayMiniBarSmString(mpp, x, y, 0, ds);
    x = STAT_TABLE_X;
    
    for (j = 0; j < cols; j++)
        {
        sprintf(ds,"%d",death_total[j]);
        DisplayMiniBarSmString(mpp, x, y, 0, ds);
        x += STAT_TABLE_XOFF;
        }
    
    x = STAT_START_X;
    y += STAT_OFF_Y;
    
    // Kills
    x = STAT_TABLE_X + SM_SIZ(50);
    y = STAT_START_Y + STAT_HEADER_Y;
    
    for (i = 0; i < rows; i++)
        {
        pp = Player + i;
        
        sprintf(ds,"%d", kills[i]);//pp->Kills);
        DisplayMiniBarSmString(mpp, x, y, 0, ds);
        
        y += STAT_OFF_Y;
        }
    
    nextpage();
    
    if (KeyPressed())
        {
        while(KeyPressed());
        }
    
    KEY_PRESSED(KEYSC_SPACE) = 0;
    KEY_PRESSED(KEYSC_ENTER) = 0;
    
    if (gs.MusicOn)
        {
        PlaySong(voc[DIGI_ENDLEV].name, 3, TRUE, TRUE);
        }
    
    while (!KEY_PRESSED(KEYSC_SPACE) && !KEY_PRESSED(KEYSC_ENTER))
        {
        handleevents();
    
        ScreenCaptureKeys();
        }    

    StopSound();
    Terminate3DSounds();
    }
    
VOID
GameIntro(VOID)
    {

    DSPRINTF(ds,"GameIntro...");
    MONO_PRINT(ds);

    if (DemoPlaying)
        return;
        
    // this could probably be taken out and you could select skill level
    // from menu to start the game    
    if (!CommEnabled && UserMapName[0])
        return;
        
    Level = 1;




    if (!AutoNet)
        {
        LogoLevel();
        //CreditsLevel();
        //SceneLevel();
        //TitleLevel();
        IntroAnimLevel();
        IntroAnimCount = 0;
        }
    
    MenuLevel();
    }

VOID
Control(VOID)
    {

    InitGame();

        MONO_PRINT("InitGame done");
    MNU_InitMenus();
    InGame = TRUE;
    GameIntro();
    //NewGame = TRUE;

    while (!QuitFlag)
        {
        handleevents();
        OSD_DispatchQueued();

        if (quitevent) QuitFlag = TRUE;

        NewLevel();
        }

    CleanExit = TRUE;    
    TerminateGame();
    }


void
_Assert(char *expr, char *strFile, unsigned uLine)
    {
    sprintf(ds, "Assertion failed: %s %s, line %u", expr, strFile, uLine);
    MONO_PRINT(ds);
    TerminateGame();
#if 1 //def RENDERTYPEWIN
    wm_msgbox(apptitle, ds);
#else
    printf("Assertion failed: %s\n %s, line %u\n", expr, strFile, uLine);
#endif
    exit(0);
    }


void
_ErrMsg(char *strFile, unsigned uLine, char *format, ...)
    {
    va_list arglist;
    
    //DSPRINTF(ds, "Error: %s, line %u", strFile, uLine);
    //MONO_PRINT(ds);
    TerminateGame();
    
#if 1 //def RENDERTYPEWIN
    {
        char msg[256], *p;
        Bsnprintf(msg, sizeof(msg), "Error: %s, line %u\n", strFile, uLine);
        p = &msg[strlen(msg)];
        va_start( arglist, format );
        Bvsnprintf(msg, sizeof(msg) - (p-msg), format, arglist);
        va_end(arglist);
        wm_msgbox(apptitle, msg);
    }
#else
    printf("Error: %s, line %u\n", strFile, uLine);
    
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );
#endif
    
    exit(0);
    }

void
dsprintf(char *str, char *format, ...)
    {
    va_list arglist;
    
    va_start( arglist, format );
    vsprintf( str, format, arglist );
    va_end( arglist );
    }

void
dsprintf_null(char *str, char *format, ...)
    {
    va_list arglist;
    }
    
void MoveLoop(void)    
    {
    int pnum;
    
    getpackets();
    
    if (PredictionOn && CommEnabled)
        {
        while (predictmovefifoplc < Player[myconnectindex].movefifoend)
            {
            DoPrediction(ppp);
            }
        }
    
       //While you have new input packets to process...
    if (!CommEnabled) 
        bufferjitter = 0;

    while (Player[myconnectindex].movefifoend - movefifoplc > bufferjitter)
        {
           //Make sure you have at least 1 packet from everyone else
         for(pnum=connecthead;pnum>=0;pnum=connectpoint2[pnum])
            {
            if (movefifoplc == Player[pnum].movefifoend)
                                {
                break;
                                }
            }    
            
           //Pnum is >= 0 only if last loop was broken, meaning a player wasn't caught up
        if (pnum >= 0)    
            break;
        
        domovethings();
        
        #if DEBUG
        //if (DemoSyncRecord)
        //    demosync_record();
        #endif    
        }        
        
    if (!InputMode && !PauseKeySet)    
        MNU_CheckForMenus();
    }    


void InitPlayerGameSettings(void)
    {
    int pnum;
    
    // don't jack with auto aim settings if DemoMode is going
    // what the hell did I do this for?????????
    //if (DemoMode)
    //    return;
    
    if (CommEnabled)
        {
        // everyone gets the same Auto Aim
        TRAVERSE_CONNECT(pnum)
            {
            if (gNet.AutoAim)
                SET(Player[pnum].Flags, PF_AUTO_AIM);
            else    
                RESET(Player[pnum].Flags, PF_AUTO_AIM);
            }    
        }
    else
        {
        if (gs.AutoAim)
            SET(Player[myconnectindex].Flags, PF_AUTO_AIM);
        else    
            RESET(Player[myconnectindex].Flags, PF_AUTO_AIM);
        }
    
    // everyone had their own Auto Run
    if (gs.AutoRun)
        SET(Player[myconnectindex].Flags, PF_LOCK_RUN);
    else    
        RESET(Player[myconnectindex].Flags, PF_LOCK_RUN);
    
    if (gs.MouseAimingOn)
        SET(Player[myconnectindex].Flags, PF_MOUSE_AIMING_ON);
    else    
        RESET(Player[myconnectindex].Flags, PF_MOUSE_AIMING_ON);
    }    


VOID InitRunLevel(VOID)
    {
    int i;
    if (DemoEdit)
        return;

    if (LoadGameOutsideMoveLoop)
        {
        int SavePlayClock;
        extern int PlayClock;
        LoadGameOutsideMoveLoop = FALSE;
        // contains what is needed from calls below
        if (gs.Ambient)    
            StartAmbientSound();
        SetCrosshair();
        PlaySong(LevelSong, -1, TRUE, TRUE);
        SetRedrawScreen(Player + myconnectindex);
        // crappy little hack to prevent play clock from being overwritten
        // for load games
        SavePlayClock = PlayClock;
        InitTimingVars();
        PlayClock = SavePlayClock;
        MONO_PRINT("Done with InitRunLevel");
        return;
        }

    #if 0    
        // ensure we are through the initialization code before sending the game
        // version. Otherwise, it is possible to send this too early and have it
        // blown away on the other side.
        waitforeverybody();
    #endif
    
    SendVersion(GameVersion);

    waitforeverybody();

    StopSong();    

    if (Bstrcasecmp(CacheLastLevel, LevelName) != 0)
        DoTheCache();
    
    if (CachePrintMode)
        cachedebug = TRUE;

    // auto aim / auto run / etc    
    InitPlayerGameSettings();
    
    // send packets with player info    
    InitNetPlayerOptions();

    // Initialize Game part of network code (When ready2send != 0)
    InitNetVars();
    
    {
        int track;
        if (Level == 0) {
            track = RedBookSong[4+RANDOM_RANGE(10)];
        } else {
            track = RedBookSong[Level];
        }
        PlaySong(LevelSong, track, TRUE, TRUE);
    }

    InitPrediction(&Player[myconnectindex]);

    if (!DemoInitOnce)
        DemoRecordSetup();

    // everything has been inited at least once for RECORD
    DemoInitOnce = TRUE;

//DebugWriteLoc(__FILE__, __LINE__);
    waitforeverybody();
        
    CheckVersion(GameVersion);
    
    // IMPORTANT - MUST be right before game loop AFTER waitforeverybody
    InitTimingVars();
    
    SetRedrawScreen(Player + myconnectindex);
    
    FX_SetVolume(gs.SoundVolume); // Turn volume back up
    if (gs.Ambient)    
        StartAmbientSound();
    }    

VOID
RunLevel(VOID)
    {
    int i;
    InitRunLevel();
    
    FX_SetVolume(gs.SoundVolume);
    SetSongVolume(gs.MusicVolume);

#if 0    
    waitforeverybody(); 
#endif
    ready2send = 1;
    
    while (TRUE)
        {
        handleevents();
        OSD_DispatchQueued();

        if (quitevent) QuitFlag = TRUE;

          //MONO_PRINT("Before MoveLoop");
        MoveLoop();
        //MONO_PRINT("After MoveLoop");
        //MONO_PRINT("Before DrawScreen");
        drawscreen(Player + screenpeek);
        //MONO_PRINT("After DrawScreen");

        if (QuitFlag)
            break;

        if (ExitLevel)
            {
            ExitLevel = FALSE;
            break;
            }
        }

    ready2send = 0;
    }

void swexit(int exitval)        
    {
    exit(exitval);
    }

VOID DosScreen(VOID)
    {
#if 0
    #ifdef SW_SHAREWARE
    #define DOS_SCREEN_NAME "SHADSW.BIN"
    #else
    #define DOS_SCREEN_NAME "SWREG.BIN"
    #endif

    #define DOS_SCREEN_SIZE (4000-(80*2))
    #define DOS_SCREEN_PTR ((void *)(0xB8000))
    int fin;
    int i;
    char buffer[DOS_SCREEN_SIZE];

    fin = kopen4load(DOS_SCREEN_NAME,0);
    if (fin == -1)
        return;

    kread(fin, buffer, sizeof(buffer));
    memcpy(DOS_SCREEN_PTR, buffer, DOS_SCREEN_SIZE);
    kclose(fin);
    move_cursor(23,0);
    _displaycursor( _GCURSORON );
#endif
    }

#if 0 //PLOCK_VERSION
VOID AlphaMessage(VOID)
    {
    Global_PLock = TRUE; // Set the hardwired parental lock mode!
    buildputs(""
    "                          SHADOW WARRIOR(tm) Version 1.2                      \n"
    "Copyright (c) 1997 3D Realms Entertainment\n"
    "\n\n"
    "     NOTE: This version of Shadow Warrior has been modified from it's\n"
    "     original form.  All of the violent and mature content has been\n"
    "     removed.  To download a patch to restore this version to its\n"
    "     original form visit www.3drealms.com, www.gtinteractive.com, or look\n"
    "     inside your retail packaging for information about this version.\n\n\n"
    );
    }
#endif    

#if 0 //UK_VERSION
VOID AlphaMessage(VOID)
    {
    buildputs(""
    "                    SHADOW WARRIOR(tm) Version 1.2 (UK Version)               \n"
    "Copyright (c) 1997 3D Realms Entertainment\n"
    "\n\n"
    "     NOTE: This is a modified version of Shadow Warrior created for the UK.\n"
    "     It has been altered from its original version to replace \"shurikens\" \n"
    "     with darts.  We apologize for the inconvenience and hope you enjoy the\n"
    "     game.  Visit us on the web at www.3drealms.com.\n\n\n"
    );
    }
#endif    

#if 1 //!UK_VERSION && !PLOCK_VERSION
VOID AlphaMessage(VOID)
    {
    if (SW_SHAREWARE) {
        buildputs("SHADOW WARRIOR(tm) Version 1.2 (Shareware Version)\n");
    } else {
        buildputs("SHADOW WARRIOR(tm) Version 1.2\n");
    }
    buildputs("Copyright (c) 1997 3D Realms Entertainment\n\n\n");
    }
#endif    

typedef struct
{
char    notshareware;
char    *arg_switch;
short   arg_match_len;
char    *arg_fmt;
char    *arg_descr;
}CLI_ARG;    

#if DEBUG
CLI_ARG cli_dbg_arg[] =
{
{0, "/demosyncrecord",     13,     "-demosyncrecord",      "Demo sync record"                      },
{0, "/demosynctest",       13,     "-demosynctest",        "Demo sync test"                        },
{0, "/cam",                4,      "-cam",                 "Camera test mode"                      },
{0, "/debugactor",         11,     "-debugactor",          "No Actors"                             },
{0, "/debuganim",          10,     "-debuganim",           "No Anims"                              },
{0, "/debugso",            8,      "-debugso",             "No Sector Objects"                     },
{0, "/debugsector",        12,     "-debugsector",         "No Sector Movement"                    },
{0, "/debugpanel",         11,     "-debugpanel",          "No Panel"                              },
{0, "/mono",               5,      "-mono",                "Mono"                                  },
};
#endif


CLI_ARG cli_arg[] =
{
{0, "/?",                  2,      "-?",                   "This help message"                     },
//#ifndef SW_SHAREWARE
//{"/l",                  2,      "-l#",                  "Level (1-11)"                          },
//{"/v",                  2,      "-v#",                  "Volume (1-3)"                          },
{1, "/map",                4,      "-map [mapname]",       "Load a map"                            },
{1, "/nocdaudio",          5,      "-nocd<audio>",         "No CD Red Book Audio"                  },
//#endif

{0, "/name",               5,      "-name [playername]",   "Player Name"                           },
{0, "/s",                  2,      "-s#",                  "Skill (1-4)"                           },
{0, "/f#",                 3,      "-f#",                  "Packet Duplication - 2, 4, 8"          },
{0, "/nopredict",          7,      "-nopred<ict>",         "Disable Net Prediction Method"         },
{0, "/level#",             5,      "-level#",              "Start at level# (Shareware: 1-4, full version 1-28)"      },
{0, "/dr",                 3,      "-dr[filename.dmo]",    "Demo record. NOTE: Must use -level# with this option."           },
{0, "/dp",                 3,      "-dp[filename.dmo]",    "Demo playback. NOTE: Must use -level# with this option."         },
{0, "/m",                  6,      "-monst<ers>",          "No Monsters"                           },
{0, "/nodemo",             6,      "-nodemo",              "No demos on game startup"              },
{0, "/nometers",           9,      "-nometers",            "Don't show air or boss meter bars in game"},
{0, "/movescale #",        9,      "-movescale",           "Adjust movement scale: 256 = 1 unit"},
{0, "/turnscale #",        9,      "-turnscale",           "Adjust turning scale: 256 = 1 unit"},
{0, "/extcompat",          9,      "-extcompat",           "Controller compatibility mode (with Duke 3D)"},
{1, "/g#",                 2,      "-g[filename.grp]",     "Load an extra GRP or ZIP file"},
{1, "/h#",                 2,      "-h[filename.def]",     "Use filename.def instead of SW.DEF"},
{0, "/setup",              5,      "-setup",               "Displays the configuration dialogue box"},
#if DEBUG
{0, "/coop",               5,      "-coop#",               "Single Player Cooperative Mode"        },
{0, "/commbat",            8,      "-commbat#",            "Single Player Commbat Mode"            },
{0, "/debug",              6,      "-debug",               "Debug Help Options"                    },
#endif

#if 0 //def NET_MODE_MASTER_SLAVE    
{0, "/broadcast",          6,      "-broad<cast>",         "Broadcast network method (default)"    },
{0, "/masterslave",        7,      "-master<slave>",       "Master/Slave network method"           },
#endif
};

#if 0
Map       ->    User Map Name
Auto      ->    Auto Start Game
Rules     ->    0=WangBang 1=WangBang (No Respawn) 2=CoOperative
Level     ->    0 to 24(?)
Enemy     ->    0=None 1=Easy 2=Norm 3=Hard 4=Insane
Markers   ->    0=Off 1=On
Team      ->    0=Off 1=On
HurtTeam  ->    0=Off 1=On
KillLimit ->    0=Infinite 1=10 2=20 3=30 4=40 5=50 6=60 7=70 8=80 9=90 10=100
TimeLimit ->    0=Infinite 1=3 2=5 3=10 4=20 5=30 6=45 7=60
Color     ->    0=Brown 1=Purple 2=Red 3=Yellow 4=Olive 5=Green
Nuke      ->    0=Off 1=On

Example Command Line:
sw -map testmap.map -autonet 0,0,1,1,1,0,3,2,1,1 -f4 -name 1234567890 -net 12345678
commit -map grenade -autonet 0,0,1,1,1,0,3,2,1,1 -name frank
#endif

char isShareware = FALSE, useDarts = FALSE;

int DetectShareware(void)
    {
    #define DOS_SCREEN_NAME_SW  "SHADSW.BIN"
    #define DOS_SCREEN_NAME_REG "SWREG.BIN"

    int h;

    h = kopen4load(DOS_SCREEN_NAME_SW,1);
    if (h >= 0) {
    isShareware = TRUE;
    kclose(h);
    return 0;
    }

    h = kopen4load(DOS_SCREEN_NAME_REG,1);
    if (h >= 0) {
    isShareware = FALSE;
    kclose(h);
    return 0;
    }

    return 1;   // heavens knows what this is...
}


void CommandLineHelp(void)
{
    int i;
#ifdef RENDERTYPEWIN
    char *str;
    int strl;

    strl = 30 + 70;
    for (i=0;i < (int)SIZ(cli_arg);i++)
        if (cli_arg[i].arg_fmt && (!SW_SHAREWARE || (!cli_arg[i].notshareware && SW_SHAREWARE)))
            strl += strlen(cli_arg[i].arg_fmt) + 1 + strlen(cli_arg[i].arg_descr) + 1;

    str = (char*)malloc(strl);
    if (str) {
        strcpy(str,"Usage: sw [options]\n");
        strcat(str,"options:  ('/' may be used instead of '-', <> text is optional)\n\n");
        for (i=0; i < (int)SIZ(cli_arg); i++) {
            if (cli_arg[i].arg_fmt && (!SW_SHAREWARE || (!cli_arg[i].notshareware && SW_SHAREWARE)))
            {
                strcat(str, cli_arg[i].arg_fmt);
                strcat(str, "\t");
                strcat(str, cli_arg[i].arg_descr);
                strcat(str, "\n");
            }
        }
        wm_msgbox("Shadow Warrior Help",str);
        free(str);
    }
#else
    if (SW_SHAREWARE)
        printf("Usage: %s [options]\n", _buildargv[0]);
    else
            printf("Usage: %s [options] [map]\n", _buildargv[0]);
    printf("options:  ('/' may be used instead of '-', <> text is optional)\n\n");

    for (i = 0; i < (int)SIZ(cli_arg); i++)
    {
        if (cli_arg[i].arg_fmt && (!SW_SHAREWARE || (!cli_arg[i].notshareware && SW_SHAREWARE)))
                {
            printf(" %-20s   %-30s\n",cli_arg[i].arg_fmt, cli_arg[i].arg_descr);
        }
    }
#endif
}

char *grpfile = "sw.grp";
int app_main(int argc, char const * const argv[])
    {
    int i;
    int stat, nexti;
    char type;
    extern int MovesPerPacket;
    VOID DoSector(VOID);
    VOID gameinput(VOID);
    int cnt = 0;
    ULONG TotalMemory;
    extern int32 ForceSetup;    // config.c
    int32 CONFIG_ReadSetup(void);

    for (i=1;i<argc;i++) {
    if (argv[i][0] != '-' && argv[i][0] != '/') continue;
    if (!Bstrcasecmp(argv[i]+1, "setup")) CommandSetup = TRUE;
    else if (!Bstrcasecmp(argv[i]+1, "?")) {
        CommandLineHelp();
        return(0);
    }
    }

#ifdef RENDERTYPEWIN
    if (win_checkinstance()) {
        if (!wm_ynbox("Shadow Warrior","Another Build game is currently running. "
                    "Do you wish to continue starting this copy?"))
            return 0;
    }
#endif

#if defined(__linux) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    addsearchpath("/usr/share/games/jfsw");
    addsearchpath("/usr/local/share/games/jfsw");
#elif defined(__APPLE__)
    addsearchpath("/Library/Application Support/JFShadowWarrior");
#endif
    
    // default behaviour is to write to the user profile directory, but
    // creating a 'user_profiles_disabled' file makes the installation "portable"
    if (access("user_profiles_disabled", F_OK) != 0) {
        char cwd[BMAX_PATH];
        char *homedir;
        int asperr;

        if (getcwd(cwd,BMAX_PATH)) addsearchpath(cwd);
        if ((homedir = Bgethomedir())) {
            Bsnprintf(cwd,sizeof(cwd),"%s/"
#if defined(_WIN32)
                "JFShadowWarrior"
#elif defined(__APPLE__)
                "Library/Application Support/JFShadowWarrior"
#else
                ".jfsw"
#endif
            ,homedir);
            asperr = addsearchpath(cwd);
            if (asperr == -2) {
                if (Bmkdir(cwd,S_IRWXU) == 0) asperr = addsearchpath(cwd);
                else asperr = -1;
            }
            if (asperr == 0)
                chdir(cwd);
            free(homedir);
        }
    }
    
    buildsetlogfile("sw.log");
    {
        char *newgrp;
        newgrp = getenv("SWGRP");
        if (newgrp) {
            grpfile = newgrp;
            buildprintf("Using alternative GRP file: %s\n", newgrp);
        }
    }

    wm_setapptitle("Shadow Warrior");
    if (preinitengine()) {
       wm_msgbox("Build Engine Initialisation Error",
               "There was a problem initialising the Build engine: %s", engineerrstr);
       exit(1);
    }

    i = CONFIG_ReadSetup();

#if defined RENDERTYPEWIN || (defined RENDERTYPESDL && (defined __APPLE__ || defined HAVE_GTK2))
    if (i < 0 || ForceSetup || CommandSetup) {
        if (quitevent || !startwin_run()) {
            uninitengine();
            exit(0);
        }
    }
#endif

    initgroupfile(grpfile);
    if (!DetectShareware()) {
        if (SW_SHAREWARE) buildputs("Detected shareware GRP\n");
        else buildputs("Detected registered GRP\n");
    }
    
    if (SW_SHAREWARE) {
        wm_setapptitle("Shadow Warrior Shareware");

        // Zero out the maps that aren't in shareware version
        memset(&LevelInfo[MAX_LEVELS_SW+1], 0, sizeof(LEVEL_INFO)*(MAX_LEVELS_REG-MAX_LEVELS_SW));
        GameVersion++;
    } else {
        wm_setapptitle("Shadow Warrior");
    }
    
    for (i = 0; i < MAX_SW_PLAYERS; i++)
        INITLIST(&Player[i].PanelSpriteList);

    DebugOperate = TRUE;
    
    AlphaMessage();
    
    buildputs("\nType 'SW -?' for command line options.\n\n");

    UserMapName[0] = '\0';
    
    //LocationInfo = TRUE;
    
    #if 0
    //#if DEBUG && SYNC_TEST
    // automatically record a demo
    DemoRecording = TRUE;
    DemoPlaying = FALSE;
    PreCaching = TRUE;
    DemoRecCnt = 0;
    strcpy(DemoFileName, "DMOTEST.DMO");
    //DemoSyncRecord = TRUE;
    #endif    
    
    #if DEBUG
    {
    FILE *fout;
    if ((fout = fopen("dbg.foo", "wb")) != NULL)
        {
        fprintf(fout, "Whoo-oo-ooooo wants some wang?\n");    
        fclose(fout);
        }
    }    
    #endif
    
    for (cnt = 1; cnt < argc; cnt++)
        {
        char const *arg = argv[cnt];
        
        if (*arg != '/' && *arg != '-') continue;
        
        if (firstnet > 0) {
            arg++;
            switch (arg[0]) {
                case 'n':
                case 'N':
                    if (arg[1] == '0') {
                        NetBroadcastMode = FALSE;
                        buildputs("Network mode: master/slave\n");
                        wm_msgbox("Multiplayer Option Error",
                            "This release unfortunately does not support a master-slave networking "
                            "mode because of certain bugs we have not been able to locate and fix "
                            "at this time. However, peer-to-peer networking has been found to be "
                            "playable, so we suggest attempting to use that for now. Details can be "
                            "found in the release notes. Sorry for the inconvenience.");
                        return 0;
                    } else if (arg[1] == '1') {
                        NetBroadcastMode = TRUE;
                        buildputs("Network mode: peer-to-peer\n");
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
    
        // Store arg in command line array!
        CON_StoreArg(arg);
        arg++;

        if (Bstrncasecmp(arg, "autonet",7) == 0)
            {   
            AutoNet = TRUE;
            cnt++;
            sscanf(argv[cnt],"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&Auto.Rules,&Auto.Level,&Auto.Enemy,&Auto.Markers,
                &Auto.Team,&Auto.HurtTeam,&Auto.Kill,&Auto.Time,&Auto.Color,&Auto.Nuke);
            }
        else
        if (Bstrncasecmp(arg, "turnscale",9) == 0)
            {
            if (cnt <= argc-2)
                {
                cnt++;
                sscanf(argv[cnt], "%d",&turn_scale);
                }
            }
        else    
        if (Bstrncasecmp(arg, "movescale",9) == 0)
            {
            if (cnt <= argc-2)
                {
                cnt++;
                sscanf(argv[cnt], "%d",&move_scale);
                }
            }
        else    
        if (Bstrncasecmp(arg, "extcompat",9) == 0)
            {
            move_scale *= 5; 
            turn_scale *= 5;
            }
        else
        if (Bstrncasecmp(arg, "cacheprint",10) == 0)
            {
            CachePrintMode = TRUE;
            }
        else    
        if (Bstrncasecmp(arg, "setupfile",8) == 0)
            {
            // Passed by setup.exe
            // skip setupfile name
            cnt++;
            }
        else
        if (Bstrncasecmp(arg, "net",3) == 0)
            {
        if (cnt+1 < argc)
            {
        firstnet = cnt+1;
            }
            }
        #if DEBUG    
        else    
        if (Bstrncasecmp(arg, "debug",5) == 0)
            {
#ifdef RENDERTYPEWIN
        char *str;
        int strl;

        strl = 24 + 70;
        for (i=0;i < (int)SIZ(cli_dbg_arg);i++)
        strl += strlen(cli_dbg_arg[i].arg_fmt) + 1 + strlen(cli_dbg_arg[i].arg_descr) + 1;

        str = (char*)malloc(strl);
        if (str) {
        strcpy(str,
            "Usage: sw [options]\n"
            "options:  ('/' may be used instead of '-', <> text is optional)\n\n"
              );
        for (i=0; i < (int)SIZ(cli_dbg_arg); i++) {
            strcat(str, cli_dbg_arg[i].arg_fmt);
            strcat(str, "\t");
            strcat(str, cli_dbg_arg[i].arg_descr);
            strcat(str, "\n");
        }
        wm_msgbox("Shadow Warrior Debug Help",str);
        free(str);
        }
#else
            printf("Usage: %s [options]\n", argv[0]);
            printf("options:  ('/' may be used instead of '-', <> text is optional)\n\n");
            for (i = 0; i < (int)SIZ(cli_dbg_arg); i++)
                {
                if (cli_dbg_arg[i].arg_fmt)
                    {
                    printf(" %-20s   %-30s\n",cli_dbg_arg[i].arg_fmt, cli_dbg_arg[i].arg_descr);
                    }
                }
#endif
            swexit(0);    
            }
        #endif    
        else
        if (Bstrncasecmp(arg, "short",5) == 0)
            {
            ShortGameMode = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "nodemo",6) == 0)
            {
            NoDemoStartup = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "allsync",3) == 0)
            {
            NumSyncBytes = 8;
            }
        else
        if (Bstrncasecmp(arg, "name",4) == 0)
            {
            if (cnt <= argc-2)
                {
                strncpy(PlayerNameArg, argv[++cnt], SIZ(PlayerNameArg)-1);
                PlayerNameArg[SIZ(PlayerNameArg)-1] = '\0';
                }
            }
        else
        if (Bstrncasecmp(arg, "f8",2) == 0)
            {
            MovesPerPacket = 8;
            }
        else
        if (Bstrncasecmp(arg, "f4",2) == 0)
            {
            MovesPerPacket = 4;
            }
        else
        if (Bstrncasecmp(arg, "f2",2) == 0)
            {
            MovesPerPacket = 2;
            }
        else
        if (Bstrncasecmp(arg, "monst", 5) == 0)
            {
            DebugActor = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "nopredict",6) == 0)
            {
            extern BOOL PredictionOn;
            PredictionOn = FALSE;
            }
                else
        if (Bstrncasecmp(arg, "col", 3) == 0)
                // provides a way to force the player color for joiners
                // since -autonet does not seem to work for them
            {
                        int temp;
            cnt++;
            sscanf(argv[cnt],"%d",&temp);
                        AutoColor = temp;
                        HasAutoColor = TRUE;
            }
        else 
        if (Bstrncasecmp(arg, "level", 5) == 0)
            {
            if (strlen(arg) > 5)
                {
                strcpy(UserMapName,LevelInfo[atoi(&arg[5])].LevelName);
                }
            }
        else
        if (Bstrncasecmp(arg, "s", 1) == 0)
            {
            if (strlen(arg) > 1)
                Skill = atoi(&arg[1])-1;
            
            Skill = max(Skill,0);
            Skill = min(Skill,3);
            }
        else
        if (Bstrncasecmp(arg, "commbat", 7) == 0)
            {
            if (strlen(arg) > 7)
                {
                FakeMultiNumPlayers = atoi(&arg[7]);
                gNet.MultiGameType = MULTI_GAME_COMMBAT;
                }
            }
        else
        /* bots suck, bye bye!
        #ifndef SW_SHAREWARE
                if (memcmp(argv[cnt], "-bots", 5) == 0)
            {
            if (strlen(argv[cnt]) > 5)
                {
                FakeMultiNumPlayers = atoi(&argv[cnt][5]);
                printf("Adding %d BOT(s) to the game!\n",FakeMultiNumPlayers);
                gNet.MultiGameType = MULTI_GAME_AI_BOTS;
                BotMode = TRUE;
                }
            }
        else
        #endif
        */
        if (Bstrncasecmp(arg, "nometers", 8) == 0)
            {
            NoMeters = TRUE;
            }
        else 
        if (Bstrncasecmp(arg, "coop", 4) == 0)
            {
            if (strlen(arg) > 4)
                {
                FakeMultiNumPlayers = atoi(&arg[4]);
                gNet.MultiGameType = MULTI_GAME_COOPERATIVE;
                }
            }
        else
        if (FALSE && Bstrncasecmp(arg, "ddr", 3) == 0)
            {
            //NumSyncBytes = 8;
            DemoRecording = TRUE;
            DemoPlaying = FALSE;
            DemoRecCnt = 0;
            DemoDebugMode = TRUE;

            if (strlen(arg) > 3)
                {
                strcpy(DemoFileName, &arg[2]);
                if (strchr(DemoFileName, '.') == 0)
                    strcat(DemoFileName, ".dmo");
                }    
            }
        else
        if (FALSE && Bstrncasecmp(arg, "dr", 2) == 0)
            {
            //NumSyncBytes = 8;
            DemoRecording = TRUE;
            DemoPlaying = FALSE;
            DemoRecCnt = 0;

            if (strlen(arg) > 2)
                {
                strcpy(DemoFileName, &arg[2]);
                if (strchr(DemoFileName, '.') == 0)
                    strcat(DemoFileName, ".dmo");
                }    
            }
        else
        if (FALSE && Bstrncasecmp(arg, "dp", 2) == 0)
            {
            DemoPlaying = TRUE;
            DemoRecording = FALSE;
            PreCaching = TRUE;

            if (strlen(arg) > 2)
                {
                strcpy(DemoFileName, &arg[2]);
                if (strchr(DemoFileName, '.') == 0)
                    strcat(DemoFileName, ".dmo");
                }    
            }
            
        #if 0 //def NET_MODE_MASTER_SLAVE    
        else    
        if (Bstrncasecmp(arg, "masterslave",6) == 0)
            {
            NetModeOverride = TRUE;
            NetBroadcastMode = FALSE;
            }
        else
        if (Bstrncasecmp(arg, "broadcast",5) == 0)
            {
            NetModeOverride = TRUE;
            NetBroadcastMode = TRUE;
            }
        #endif    

        else
        if (Bstrncasecmp(arg, "cheat",5) == 0)
            {
            ArgCheat = TRUE;
            }
        else    
        if (Bstrncasecmp(arg, "demosynctest",12) == 0)
            {
            NumSyncBytes = 8;
            DemoSyncTest = TRUE;
            DemoSyncRecord = FALSE;
            }
        else
        if (Bstrncasecmp(arg, "demosyncrecord",12) == 0)
            {
            NumSyncBytes = 8;
            DemoSyncTest = FALSE;
            DemoSyncRecord = TRUE;
            }
        else  
        if (Bstrncasecmp(arg, "cam",3) == 0)
            {
            CameraTestMode = TRUE;
            }
            
        #if DEBUG
        else
        if (FALSE && Bstrncasecmp(arg, "de", 2) == 0)
            {
            #if DEMO_FILE_TYPE == DEMO_FILE_GROUP
            DemoPlaying = TRUE;
            DemoRecording = FALSE;

            if (strlen(arg) > 2)
                {
                strcpy(DemoFileName, &arg[2]);
                if (strchr(DemoFileName, '.') == 0)
                    strcat(DemoFileName, ".dmo");
                }    
            #else
            DemoEdit = TRUE;
            DemoPlaying = TRUE;
            DemoRecording = FALSE;

            if (strlen(arg) > 2)
                {
                strcpy(DemoFileName, &arg[2]);
                if (strchr(DemoFileName, '.') == 0)
                    strcat(DemoFileName, ".dmo");
                }    
            #endif    
            }
        else    
        if (Bstrncasecmp(arg, "randprint",5) == 0)
            {
            RandomPrint = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "level", 5) == 0)
            {
            if (strlen(arg) > 5)
                {
                strcpy(UserMapName,LevelInfo[atoi(&arg[5])].LevelName);
                }
            }
        else
        if (Bstrncasecmp(arg, "debugsecret", 10) == 0)
            {
            extern BOOL DebugSecret;
            DebugSecret = TRUE;
            }
        else    
        if (Bstrncasecmp(arg, "debugactor", 10) == 0)
            {
            DebugActor = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "mono", 4) == 0)
            {
            DispMono = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "debugso", 7) == 0)
            {
            DebugSO = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "nosyncprint",10) == 0)
            {
            extern BOOL SyncPrintMode;
            SyncPrintMode = FALSE;
            }
        else
        if (Bstrncasecmp(arg, "debuganim", 9) == 0)
            {
            DebugAnim = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "debugsector", 11) == 0)
            {
            DebugSector = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "debugpanel", 10) == 0)
            {
            DebugPanel = TRUE;
            }
        else
        if (FALSE && Bstrncasecmp(arg, "dt", 2) == 0)
            {
            if (strlen(arg) > 2)
                {
                strcpy(DemoTmpName, &arg[2]);
                if (strchr(DemoFileName, '.') == 0)
                    strcat(DemoFileName, ".dmo");
                }    
            }
        else 
        if (Bstrncasecmp(arg, "nodemo", 6) == 0)
            {
            DemoRecording = FALSE;
            DemoPlaying = FALSE;
            PreCaching = TRUE;
            DemoRecCnt = 0;
            
            DemoSyncTest = FALSE;
            DemoSyncRecord = FALSE;
            }

        #endif
            
        else
        if (Bstrncasecmp(arg, "map", 3) == 0 && !SW_SHAREWARE)
            {
            int fil;

            strcpy(UserMapName, argv[++cnt]);
            if (strchr(UserMapName, '.') == 0)
                strcat(UserMapName, ".map");

            if ((fil = kopen4load(UserMapName,0)) == -1)
                {
#ifdef RENDERTYPEWIN
                char msg[256];
                Bsnprintf(msg, 256, "ERROR: Could not find user map %s!",UserMapName);
                wm_msgbox(apptitle, msg);
#else
                printf("ERROR: Could not find user map %s!\n\n",UserMapName);
#endif
                kclose(fil);    
                swexit(0);
                }
            else    
                kclose(fil);    
            }

        else
        if (Bstrncasecmp(arg, "g", 1) == 0 && !SW_SHAREWARE)
            {
            if (strlen(arg) > 1)
                {
                if (initgroupfile(arg+1) >= 0)
                    buildprintf("Added %s\n", arg+1);
                }
            }
        else
        if (Bstrncasecmp(arg, "h", 1) == 0 && !SW_SHAREWARE)
            {
            if (strlen(arg) > 1)
                {
                deffile = (arg+1);
                buildprintf("Using DEF file %s.\n", arg+1);
                }
            }
        }

    Control();

    return (0);
    }

VOID
ManualPlayerInsert(PLAYERp pp)
    {
    PLAYERp npp = Player + numplayers;
    int i;

    if (numplayers < MAX_SW_PLAYERS)
        {
        connectpoint2[numplayers - 1] = numplayers;
        connectpoint2[numplayers] = -1;

        npp->posx = pp->posx;
        npp->posy = pp->posy;
        npp->posz = pp->posz;
        npp->pang = pp->pang;
        npp->cursectnum = pp->cursectnum;

        myconnectindex = numplayers;
        screenpeek = numplayers;

        sprintf(Player[myconnectindex].PlayerName,"PLAYER %d",myconnectindex+1);

        Player[numplayers].movefifoend = Player[0].movefifoend;

        // If IsAI = TRUE, new player will be a bot
        Player[myconnectindex].IsAI = FALSE;
        
        numplayers++;
        }

    }

VOID
BotPlayerInsert(PLAYERp pp)
    {
    PLAYERp npp = Player + numplayers;
    int i;

    if (numplayers < MAX_SW_PLAYERS)
        {
        connectpoint2[numplayers - 1] = numplayers;
        connectpoint2[numplayers] = -1;

        npp->posx = pp->posx;
        npp->posy = pp->posy;
        npp->posz = pp->posz-Z(100);
        npp->pang = pp->pang;
        npp->cursectnum = pp->cursectnum;

        //myconnectindex = numplayers;
        //screenpeek = numplayers;

        sprintf(Player[numplayers].PlayerName,"BOT %d",numplayers+1);

        Player[numplayers].movefifoend = Player[0].movefifoend;

        // If IsAI = TRUE, new player will be a bot
        Player[numplayers].IsAI = TRUE;
        
        numplayers++;
        }

//    SetFragBar(pp);
    }
    
VOID
ManualPlayerDelete(PLAYERp cur_pp)
    {
    short i, nexti;
    USERp u;
    short save_myconnectindex;
    PLAYERp pp;

    if (numplayers > 1)
        {
        numplayers--;
        connectpoint2[numplayers - 1] = -1;

        pp = Player + numplayers;

        KillSprite(pp->PlayerSprite);
        pp->PlayerSprite = -1;

        // Make sure enemys "forget" about deleted player
        TRAVERSE_SPRITE_STAT(headspritestat[STAT_ENEMY], i, nexti)
            {
            u = User[i];
            if (u->tgt_sp == pp->SpriteP)
                u->tgt_sp = Player[0].SpriteP;
            }

        if (myconnectindex >= numplayers)
            myconnectindex = 0;

        if (screenpeek >= numplayers)
            screenpeek = 0;
        }
    }

#if DEBUG
VOID
SinglePlayInput(PLAYERp pp)
    {
    int pnum = myconnectindex;
    BYTEp kp;

    if (BUTTON(gamefunc_See_Co_Op_View) && !UsingMenus && !ConPanel && dimensionmode == 3)
        {
        short oldscreenpeek = screenpeek;

        CONTROL_ClearButton(gamefunc_See_Co_Op_View);

        screenpeek = connectpoint2[screenpeek];

        if (screenpeek < 0)
            screenpeek = connecthead;

        if (dimensionmode == 2 || dimensionmode == 5 || dimensionmode == 6)
            setup2dscreen();

        if (dimensionmode != 2)
            {
            PLAYERp tp;

            tp = Player + screenpeek;
            PlayerUpdatePanelInfo(tp);
        if (getrendermode() < 3)
            COVERsetbrightness(gs.Brightness,(char *)palette_data);
        else
        setpalettefade(0,0,0,0);
            memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
            DoPlayerDivePalette(tp);
            DoPlayerNightVisionPalette(tp);
//          printf("SingPlayInput set_pal: tp->PlayerSprite = %d\n",tp->PlayerSprite);
            }
        }

    if (!(KEY_PRESSED(KEYSC_ALT) | KEY_PRESSED(KEYSC_RALT)))
        return;

        
    if (!SW_SHAREWARE && KEY_PRESSED(KEYSC_M))
        {
        extern BOOL DebugActorFreeze;

        KEY_PRESSED(KEYSC_M) = 0;
        DebugActorFreeze++;
        if (DebugActorFreeze > 2)
            DebugActorFreeze = 0;

        if (DebugActorFreeze == 2)
            {
            short i, nexti;

            TRAVERSE_SPRITE_STAT(headspritestat[STAT_ENEMY], i, nexti)
                {
                SET(sprite[i].cstat, CSTAT_SPRITE_INVISIBLE);
                if (TEST(sprite[i].cstat, CSTAT_SPRITE_BLOCK))
                    {
                    SET(sprite[i].extra, SPRX_BLOCK);
                    RESET(sprite[i].cstat, CSTAT_SPRITE_BLOCK);
                    }
                }
            }

        if (DebugActorFreeze == 0)
            {
            short i, nexti;

            TRAVERSE_SPRITE_STAT(headspritestat[STAT_ENEMY], i, nexti)
                {
                RESET(sprite[i].cstat, CSTAT_SPRITE_INVISIBLE);
                if (TEST(sprite[i].extra, SPRX_BLOCK))
                    SET(sprite[i].cstat, CSTAT_SPRITE_BLOCK);
                }
            }
        }


    // Insert a player
    if (KEY_PRESSED(KEYSC_INS))
        // player
        {
        KEY_PRESSED(KEYSC_INS) = 0;
        ManualPlayerInsert(pp);
        // comes back looking through screenpeek
        InitPlayerSprite(Player + screenpeek);
        PlayerDeathReset(Player + screenpeek);
        SetFragBar(pp);
        }


    // Delete a player
    if (KEY_PRESSED(KEYSC_DEL))
        {
        KEY_PRESSED(KEYSC_DEL) = 0;
        ManualPlayerDelete(pp);
        }

    // Move control to numbered player

    if ((kp = KeyPressedRange(&KEY_PRESSED(KEYSC_1), &KEY_PRESSED(KEYSC_9))) && numplayers > 1)
        {
        short save_myconnectindex;

        save_myconnectindex = myconnectindex;

        myconnectindex = (intptr_t)kp - (intptr_t)(&KEY_PRESSED(KEYSC_1));

        if (myconnectindex >= numplayers)
            myconnectindex = save_myconnectindex;

        screenpeek = myconnectindex;

        DoPlayerDivePalette(pp);

        // Now check for item or pain palette stuff
        // This sets the palette to whatever it is of the player you
        // just chose to view the game through.
//      printf("SingPlayInput ALT+1-9 set_pal: pp->PlayerSprite = %d\n",pp->PlayerSprite);
        COVERsetbrightness(gs.Brightness,(char *)palette_data); // JBF: figure out what's going on here

        DoPlayerNightVisionPalette(pp);

        ResetKeyRange(&KEY_PRESSED(KEYSC_1), &KEY_PRESSED(KEYSC_9));
        }

    #if 0    
    if (KEY_PRESSED(KEYSC_T))
        {
        KEY_PRESSED(KEYSC_T) = 0;
        PlayerTrackingMode ^= 1;
        }
    #endif    

    if (KEY_PRESSED(KEYSC_H))
        {
        short pnum;

        KEY_PRESSED(KEYSC_H) = 0;

        TRAVERSE_CONNECT(pnum)
            {
            User[Player[pnum].PlayerSprite]->Health = 100;
            }
        }
    }

VOID
DebugKeys(PLAYERp pp)
    {
    short w, h;

    if (!(KEY_PRESSED(KEYSC_ALT) || KEY_PRESSED(KEYSC_RALT)))
        return;

    if (InputMode)
        return;

    if (CommEnabled)
        return;

    //
    // visiblity adjust
    //

    if (KEY_PRESSED(KEYSC_L) > 0)
        {
        if (KEY_PRESSED(KEYSC_LSHIFT) | KEY_PRESSED(KEYSC_RSHIFT))      // SHIFT
            {
            visibility = visibility - (visibility >> 3);

            if (visibility < 128)
                visibility = 16348;

            //if (visibility > 16384)
            //    visibility = 128;
            }
        else
            {
            KEY_PRESSED(KEYSC_L) = 0;

            visibility = visibility - (visibility >> 3);

            if (visibility > 16384)
                visibility = 128;
            }
        }

    //
    // parallax changes
    //

    if (KEY_PRESSED(KEYSC_X))
        {
        if (KEY_PRESSED(KEYSC_LSHIFT))
            {
            KEY_PRESSED(KEYSC_LSHIFT) = FALSE;
            KEY_PRESSED(KEYSC_X) = 0;

            parallaxyoffs += 10;

            if (parallaxyoffs > 100)
                parallaxyoffs = 0;
            }
        else
            {
            KEY_PRESSED(KEYSC_X) = 0;
            parallaxtype++;
            if (parallaxtype > 2)
                parallaxtype = 0;
            }
        }
    }

#endif

VOID 
ConKey( void )
    {
    #if DEBUG
    // Console Input Panel
    if (!ConPanel && dimensionmode == 3)
        {
        //if (KEY_PRESSED(KEYSC_TILDE) && KEY_PRESSED(KEYSC_LSHIFT))
        if (KEY_PRESSED(KEYSC_TILDE))
            {
            KEY_PRESSED(KEYSC_TILDE) = FALSE;
            //KEY_PRESSED(KEYSC_LSHIFT) = FALSE;
            KB_FlushKeyboardQueue();
            ConPanel = TRUE;
            InputMode = TRUE;
            ConInputMode = TRUE;
            if (!CommEnabled)
                GamePaused = TRUE;
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            }
        }
    else
    if (ConPanel)
        {
        //if (KEY_PRESSED(KEYSC_TILDE) && KEY_PRESSED(KEYSC_LSHIFT))
        if (KEY_PRESSED(KEYSC_TILDE))
            {
            KEY_PRESSED(KEYSC_TILDE) = FALSE;
            //KEY_PRESSED(KEYSC_LSHIFT) = FALSE;
            KB_FlushKeyboardQueue();
            ConPanel = FALSE;
            ConInputMode = FALSE;
            InputMode = FALSE;
            if (!CommEnabled)
                GamePaused = FALSE;
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            SetFragBar(Player + myconnectindex);    
            }
        }
    #endif    
    }

char WangBangMacro[10][64];

VOID
FunctionKeys(PLAYERp pp)
    {
    extern BOOL GamePaused;
    extern short QuickLoadNum;
    static int rts_delay = 0;
    int fn_key = 0;
    
    rts_delay++;

    if (KEY_PRESSED(sc_F1))   { fn_key = 1; }
    if (KEY_PRESSED(sc_F2))   { fn_key = 2; }
    if (KEY_PRESSED(sc_F3))   { fn_key = 3; }
    if (KEY_PRESSED(sc_F4))   { fn_key = 4; }
    if (KEY_PRESSED(sc_F5))   { fn_key = 5; }
    if (KEY_PRESSED(sc_F6))   { fn_key = 6; }
    if (KEY_PRESSED(sc_F7))   { fn_key = 7; }
    if (KEY_PRESSED(sc_F8))   { fn_key = 8; }
    if (KEY_PRESSED(sc_F9))   { fn_key = 9; }
    if (KEY_PRESSED(sc_F10))  { fn_key = 10; }
    
    if (KEY_PRESSED(KEYSC_ALT) || KEY_PRESSED(KEYSC_RALT))
    {
        if (rts_delay > 16 && fn_key && CommEnabled && !gs.ParentalLock && !Global_PLock)
            {
            KEY_PRESSED(sc_F1 + fn_key - 1) = 0;
            
            rts_delay = 0;
            
            PlaySoundRTS(fn_key);
            
            if (CommEnabled)
                {
                short pnum;
                PACKET_RTS p;
                
                p.PacketType = PACKET_TYPE_RTS;
                p.RTSnum = fn_key;
                
                netbroadcastpacket((BYTEp)(&p), sizeof(p));            // TENSW
                }
            }
            
        return;    
        }    

    if (KEY_PRESSED(KEYSC_LSHIFT) || KEY_PRESSED(KEYSC_RSHIFT))
        {
        if (fn_key && CommEnabled)
            {
            KEY_PRESSED(sc_F1 + fn_key - 1) = 0;
            
            if (CommEnabled)
                {
                short pnum;
                
                sprintf(ds,"SENT: %s",WangBangMacro[fn_key-1]);
                adduserquote(ds);
                
                TRAVERSE_CONNECT(pnum)        
                    {
                    if (pnum != myconnectindex)
                        {
                        sprintf(ds,"%s: %s",pp->PlayerName, WangBangMacro[fn_key-1]);
                        SendMessage(pnum, ds);
                        }
                    }
                }
            }
            
        return;    
        }    

    
    if (numplayers <= 1)    
        {
        // F2 save menu
        if (KEY_PRESSED(KEYSC_F2))
            {
            KEY_PRESSED(KEYSC_F2) = 0;
            if (!TEST(pp->Flags, PF_DEAD))
                {
                KEY_PRESSED(KEYSC_ESC) = 1;
                ControlPanelType = ct_savemenu;
                }
            }
            
        // F3 load menu
        if (KEY_PRESSED(KEYSC_F3))
            {
            KEY_PRESSED(KEYSC_F3) = 0;
            if (!TEST(pp->Flags, PF_DEAD))
                {
                KEY_PRESSED(KEYSC_ESC) = 1;
                ControlPanelType = ct_loadmenu;
                }
            }
            
        // F6 option menu
        if (KEY_PRESSED(KEYSC_F6))
            {
            extern BOOL QuickSaveMode;
            KEY_PRESSED(KEYSC_F6) = 0;
            if (!TEST(pp->Flags, PF_DEAD))
                {
                KEY_PRESSED(KEYSC_ESC) = 1;
                ControlPanelType = ct_savemenu;
                QuickSaveMode = TRUE;
                }
            }

        // F9 quick load    
        if (KEY_PRESSED(KEYSC_F9))
            {
            KEY_PRESSED(KEYSC_F9) = 0;
            
            if (!TEST(pp->Flags, PF_DEAD))
                {
                if (QuickLoadNum < 0)
                    {
                    PutStringInfoLine(pp, "Last saved game not found.");
                    }
                else
                    {
                    KB_ClearKeysDown();
                    KEY_PRESSED(KEYSC_ESC) = 1;
                    ControlPanelType = ct_quickloadmenu;
                    }    
                }    
            }

        }    
        

    // F4 sound menu
    if (KEY_PRESSED(KEYSC_F4))
        {
        KEY_PRESSED(KEYSC_F4) = 0;
        KEY_PRESSED(KEYSC_ESC) = 1;
        ControlPanelType = ct_soundmenu;
        }

        
    // F7 VIEW control
    if (KEY_PRESSED(KEYSC_F7))
        {
        KEY_PRESSED(KEYSC_F7) = 0;

        if (KEY_PRESSED(KEYSC_LSHIFT) || KEY_PRESSED(KEYSC_RSHIFT))
            {
            if (TEST(pp->Flags, PF_VIEW_FROM_OUTSIDE))
                pp->view_outside_dang = NORM_ANGLE(pp->view_outside_dang + 256);
            }
        else
            {
            if (TEST(pp->Flags, PF_VIEW_FROM_OUTSIDE))
                {
                RESET(pp->Flags, PF_VIEW_FROM_OUTSIDE);
                }
            else
                {
                SET(pp->Flags, PF_VIEW_FROM_OUTSIDE);
                pp->camera_dist = 0;
                }
            }
        }
        
    // F8 toggle messages    
    if (KEY_PRESSED(KEYSC_F8))
        {
        KEY_PRESSED(KEYSC_F8) = 0;
        
        gs.Messages ^= 1;
        
        if (gs.Messages)
            PutStringInfoLine(pp, "Messages ON");
        else    
            PutStringInfoLine(pp, "Messages OFF");
        }

    // F10 quit menu
    if (KEY_PRESSED(KEYSC_F10))
        {
        KEY_PRESSED(KEYSC_F10) = 0;
        KEY_PRESSED(KEYSC_ESC) = 1;
        ControlPanelType = ct_quitmenu;
        }
    
    // F11 gamma correction
    if (KEY_PRESSED(KEYSC_F11) > 0)
        {
        KEY_PRESSED(KEYSC_F11) = 0;

        gs.Brightness++;
        if (gs.Brightness >= SLDR_BRIGHTNESSMAX)
            gs.Brightness = 0;
           
        sprintf(ds,"Brightness level (%d)",gs.Brightness+1);   
        PutStringInfoLine(pp, ds);            
        
        if(!pp->NightVision && pp->FadeAmt <= 0)
            {
            COVERsetbrightness(gs.Brightness,&palette_data[0][0]);
            }

        //DoPlayerDivePalette(pp);
        //DoPlayerNightVisionPalette(pp);
        }
        
    }

VOID PauseKey(PLAYERp pp)
    {
    extern BOOL GamePaused,CheatInputMode;
    extern short QuickLoadNum;
    extern BOOL enabled;
    
    if (KEY_PRESSED(sc_Pause) && !CommEnabled && !InputMode && !UsingMenus && !CheatInputMode && !ConPanel)
        {
        KEY_PRESSED(sc_Pause) = 0;
        
        PauseKeySet ^= 1;
        
        if (PauseKeySet)
            GamePaused = TRUE;
        else    
            GamePaused = FALSE;
            
        if (GamePaused)
            {
            short w,h;
            #define MSG_GAME_PAUSED "Game Paused"
            MNU_MeasureString(MSG_GAME_PAUSED, &w, &h);
            PutStringTimer(pp, TEXT_TEST_COL(w), 100, MSG_GAME_PAUSED, 999);
            PauseSong(TRUE);
            }
        else
            {
            pClearTextLine(pp, 100);
            PauseSong(FALSE);
            }    
        }
    
    if (!CommEnabled && TEST(pp->Flags, PF_DEAD))
        {
        if (ReloadPrompt)
            {
            if (QuickLoadNum < 0)
                {
                ReloadPrompt = FALSE;
                }
            else    
                {
                KEY_PRESSED(KEYSC_ESC) = 1;
                ControlPanelType = ct_quickloadmenu;
                }
            }
        }    
    }    



VOID GetMessageInput(PLAYERp pp)
    {    
    int pnum = myconnectindex;
    short w,h;
    signed char MNU_InputSmallString(char *, short);
    signed char MNU_InputString(char *, short);
    static BOOL cur_show;
    static BOOL TeamSendAll, TeamSendTeam;
    #define TEAM_MENU "A - Send to ALL,  T - Send to TEAM"
    static char HoldMessageInputString[256];
    int i;
    BOOL IsCommand(char *str);
    
    if (!MessageInputMode && !ConInputMode)
        {
        if (BUTTON(gamefunc_SendMessage))
            {
            CONTROL_ClearButton(gamefunc_SendMessage);
            KB_FlushKeyboardQueue();
            MessageInputMode = TRUE;
            InputMode = TRUE;
            TeamSendTeam = FALSE;
            TeamSendAll = FALSE;
            
            if (MessageInputMode)
                {
                memset(MessageInputString, '\0', sizeof(MessageInputString));
                }
            }
        }
    else
    if (MessageInputMode && !ConInputMode)
        {
        if (gs.BorderNum > BORDER_BAR+1)
            SetRedrawScreen(pp);
        
        // get input
        switch(MNU_InputSmallString(MessageInputString, 320-20))
            {
            case -1: // Cancel Input (pressed ESC) or Err
                MessageInputMode = FALSE;
                InputMode = FALSE;
                KB_ClearKeysDown();
                KB_FlushKeyboardQueue();
                break;
            case FALSE: // Input finished (RETURN)
                if (MessageInputString[0] == '\0')
                    {
                    // no input
                    MessageInputMode = FALSE;
                    InputMode = FALSE;
                    KB_ClearKeysDown();
                    KB_FlushKeyboardQueue();
                    CONTROL_ClearButton(gamefunc_Inventory);
                    }
                else
                    {
                    if (gNet.TeamPlay)
                        {
                        if (memcmp(MessageInputString, TEAM_MENU, sizeof(TEAM_MENU)) != 0)    
                            {
                            // see if its a command
                            if (IsCommand(MessageInputString))
                                {
                                TeamSendAll = TRUE;
                                }
                            else    
                                {
                                strcpy(HoldMessageInputString, MessageInputString);
                                strcpy(MessageInputString, TEAM_MENU);
                                break;
                                }
                            }
                        else
                        if (memcmp(MessageInputString, TEAM_MENU, sizeof(TEAM_MENU)) == 0)    
                            {
                            strcpy(MessageInputString, HoldMessageInputString);
                            TeamSendAll = TRUE;
                            }    
                        }
                    
                    SEND_MESSAGE:
                    
                    // broadcast message
                    MessageInputMode = FALSE;
                    InputMode = FALSE;
                    KB_ClearKeysDown();
                    KB_FlushKeyboardQueue();
                    CONTROL_ClearButton(gamefunc_Inventory);
                    CON_ProcessUserCommand();     // Check to see if it's a cheat or command
                    
                    for (i = 0; i < NUMGAMEFUNCTIONS; i++)
                        CONTROL_ClearButton(i);

                    // Put who sent this
                    sprintf(ds,"%s: %s",pp->PlayerName,MessageInputString);
                    
                    if (gNet.TeamPlay)
                        {
                        TRAVERSE_CONNECT(pnum)
                            {
                            if (pnum != myconnectindex)
                                {
                                if (TeamSendAll)
                                    SendMessage(pnum, ds);
                                else    
                                if (User[pp->PlayerSprite]->spal == User[Player[pnum].PlayerSprite]->spal)
                                    SendMessage(pnum, ds);
                                }
                            }
                        }
                    else    
                    TRAVERSE_CONNECT(pnum)
                        {
                        if (pnum != myconnectindex)
                            {
                            SendMessage(pnum, ds);
                            }
                         }
                     adduserquote(MessageInputString);
                     quotebot += 8;
                     quotebotgoal = quotebot;
                    }
                break;
                
            case TRUE: // Got input
            
                if (gNet.TeamPlay)
                    {
                    if (memcmp(MessageInputString, TEAM_MENU"a", sizeof(TEAM_MENU)+1) == 0)    
                        {
                        strcpy(MessageInputString, HoldMessageInputString);
                        TeamSendAll = TRUE;
                        goto SEND_MESSAGE;
                        }    
                    else    
                    if (memcmp(MessageInputString, TEAM_MENU"t", sizeof(TEAM_MENU)+1) == 0)    
                        {
                        strcpy(MessageInputString, HoldMessageInputString);
                        TeamSendTeam = TRUE;
                        goto SEND_MESSAGE;
                        }
                    else
                        {
                        // reset the string if anything else is typed
                        if (strlen(MessageInputString)+1 > sizeof(TEAM_MENU))
                            {
                            strcpy(MessageInputString, TEAM_MENU);
                            }
                        }        
                    }    
            
                break;
            }
        }
    }
    
VOID GetConInput(PLAYERp pp)
    {    
    int pnum = myconnectindex;
    short w,h;
    signed char MNU_InputSmallString(char *, short);
    signed char MNU_InputString(char *, short);
    static BOOL cur_show;

    if (MessageInputMode || HelpInputMode)
        return;
    
    ConKey();
    
    // Console input commands
    if (ConInputMode && !MessageInputMode)
        {
        // get input
        switch(MNU_InputSmallString(MessageInputString, 250))
            {
            case -1: // Cancel Input (pressed ESC) or Err
                InputMode = FALSE;
                KB_ClearKeysDown();
                KB_FlushKeyboardQueue();
                memset(MessageInputString, '\0', sizeof(MessageInputString));
                break;
            case FALSE: // Input finished (RETURN)
                if (MessageInputString[0] == '\0')
                    {
                    InputMode = FALSE;
                    KB_ClearKeysDown();
                    KB_FlushKeyboardQueue();
                    CONTROL_ClearButton(gamefunc_Inventory);
                    memset(MessageInputString, '\0', sizeof(MessageInputString));
                    }
                else
                    {
                    InputMode = FALSE;
                    KB_ClearKeysDown();
                    KB_FlushKeyboardQueue();
                    CONTROL_ClearButton(gamefunc_Inventory);
                    CON_ConMessage(MessageInputString);
                    CON_ProcessUserCommand();     // Check to see if it's a cheat or command
                    
                    conbot += 6;
                    conbotgoal = conbot;
                    //addconquote(MessageInputString);
                    // Clear it out after every entry
                    memset(MessageInputString, '\0', sizeof(MessageInputString));
                    }
                break;
            case TRUE: // Got input
                break;
            }
        }
    }    


VOID GetHelpInput(PLAYERp pp)
    {
    extern BOOL GamePaused;
    
    if (KEY_PRESSED(KEYSC_ALT) || KEY_PRESSED(KEYSC_RALT))
        return;
    
    if (KEY_PRESSED(KEYSC_LSHIFT) || KEY_PRESSED(KEYSC_RSHIFT))
        return;
    
    if (MessageInputMode || ConInputMode)
        return;
    
    // F1 help menu
    if (!HelpInputMode)
        {
        if (KEY_PRESSED(KEYSC_F1))
            {
            KEY_PRESSED(KEYSC_F1) = FALSE;
            HelpPage = 0;
            HelpInputMode = TRUE;
            PanelUpdateMode = FALSE;
            InputMode = TRUE;
            if (!CommEnabled)
                GamePaused = TRUE;
            }
        }
    else
    if (HelpInputMode)
        {
        if (KEY_PRESSED(KEYSC_ESC))
            {
            KEY_PRESSED(KEYSC_ESC) = 0;
            KB_ClearKeysDown();
            PanelUpdateMode = TRUE;
            HelpInputMode = FALSE;
            InputMode = FALSE;
            if (!CommEnabled)
                GamePaused = FALSE;
            SetRedrawScreen(pp);
            }
            
        if (KEY_PRESSED(KEYSC_SPACE) || KEY_PRESSED(KEYSC_ENTER) || KEY_PRESSED(KEYSC_PGDN) || KEY_PRESSED(KEYSC_DOWN) || KEY_PRESSED(KEYSC_RIGHT) || KEY_PRESSED(sc_kpad_3) || KEY_PRESSED(sc_kpad_2) || KEY_PRESSED(sc_kpad_6))    
            {
            KEY_PRESSED(KEYSC_SPACE) = KEY_PRESSED(KEYSC_ENTER) = 0;
            KEY_PRESSED(KEYSC_PGDN) = 0;
            KEY_PRESSED(KEYSC_DOWN) = 0;
            KEY_PRESSED(KEYSC_RIGHT) = 0;
            KEY_PRESSED(sc_kpad_3) = 0;
            KEY_PRESSED(sc_kpad_2) = 0;
            KEY_PRESSED(sc_kpad_6) = 0;
            
            HelpPage++;
            if (HelpPage >= (int)SIZ(HelpPagePic))
                // CTW MODIFICATION
                // "Oops! I did it again..."
                // HelpPage = SIZ(HelpPagePic) - 1;
                HelpPage = 0;
                // CTW MODIFICATION END
            }
            
        if (KEY_PRESSED(KEYSC_PGUP) || KEY_PRESSED(KEYSC_UP) || KEY_PRESSED(KEYSC_LEFT) || KEY_PRESSED(sc_kpad_9) || KEY_PRESSED(sc_kpad_8) || KEY_PRESSED(sc_kpad_4))    
            {
            KEY_PRESSED(KEYSC_PGUP) = 0;
            KEY_PRESSED(KEYSC_UP) = 0;
            KEY_PRESSED(KEYSC_LEFT) = 0;
            KEY_PRESSED(sc_kpad_8) = 0;
            KEY_PRESSED(sc_kpad_9) = 0;
            KEY_PRESSED(sc_kpad_4) = 0;
            
            HelpPage--;
            if (HelpPage < 0)
                // CTW MODIFICATION
                // "Played with the logic, got lost in the game..."
                HelpPage = SIZ(HelpPagePic) - 1;
                // CTW MODIFICATION END
            }
        }
    }    

short MirrorDelay;    
    
VOID
getinput(SW_PACKET *loc)
    {
    BOOL found = FALSE;
    int i;
    PLAYERp pp = Player + myconnectindex;
    PLAYERp newpp = Player + myconnectindex;
    int pnum = myconnectindex;
    int inv_hotkey = 0;

#define TURBOTURNTIME (120/8)
#define NORMALTURN   (12+6)
#define RUNTURN      (28)
#define PREAMBLETURN 3
#define NORMALKEYMOVE 35
#define MAXVEL       ((NORMALKEYMOVE*2)+10)
#define MAXSVEL      ((NORMALKEYMOVE*2)+10)
#define MAXANGVEL    100
#define SET_LOC_KEY(loc, sync_num, key_test) SET(loc, ((!!(key_test)) << (sync_num)))

    ControlInfo info;
    boolean running;
    int32 turnamount;
    static int32 turnheldtime;
    int32 keymove;
    int32 momx, momy;
    int aimvel;

    extern BOOL MenuButtonAutoRun;
    extern BOOL MenuButtonAutoAim;
    
    if (Prediction && CommEnabled) 
        {
        newpp = ppp;
        }
    
    CONTROL_GetInput(&info);

    info.dz = (info.dz * move_scale)>>8;
    info.dyaw = (info.dyaw * turn_scale)>>8;

    // reset all syncbits
    loc->bits = 0;
    svel = vel = angvel = aimvel = 0;

    // MAKE SURE THIS WILL GET SET
    SET_LOC_KEY(loc->bits, SK_QUIT_GAME, MultiPlayQuitFlag);
    
    PauseKey(pp);
    
    if (PauseKeySet)
        return;

    if (!MenuInputMode && !UsingMenus)
        {
        GetMessageInput(pp);
        GetConInput(pp);
        GetHelpInput(pp);
        }

    // MAP KEY
    if (BUTTON(gamefunc_Map))
        {
        CONTROL_ClearButton(gamefunc_Map);
        
        // Init follow coords
        Follow_posx = pp->posx;
        Follow_posy = pp->posy;

        if (dimensionmode == 3)
            dimensionmode = 5;
        else
        if (dimensionmode == 5)
            dimensionmode = 6;
        else
            {
            MirrorDelay = 1;
            dimensionmode = 3;
            SetFragBar(pp);    
            ScrollMode2D = FALSE;
            SetRedrawScreen(pp);
            }
        }
    
    // Toggle follow map mode on/off
    if (dimensionmode == 5 || dimensionmode == 6)    
        {
        if (BUTTON(gamefunc_Map_Follow_Mode) && !BUTTONHELD(gamefunc_Map_Follow_Mode))
            {
            ScrollMode2D = !ScrollMode2D;
            Follow_posx = pp->posx;
            Follow_posy = pp->posy;
            }
        }

    // If in 2D follow mode, scroll around using glob vars
    // Tried calling this in domovethings, but key response it too poor, skips key presses
    // Note: ScrollMode2D = Follow mode, so this get called only during follow mode
    if (ScrollMode2D && pp == Player + myconnectindex && !Prediction)
        MoveScrollMode2D(Player + myconnectindex);
        
    // !JIM! Added UsingMenus so that you don't move at all while using menus    
    if (MenuInputMode || UsingMenus || ScrollMode2D || InputMode)
        return;
        
    SET_LOC_KEY(loc->bits, SK_SPACE_BAR, !!KEY_PRESSED(KEYSC_SPACE));
        
    running = BUTTON(gamefunc_Run) || TEST(pp->Flags, PF_LOCK_RUN);
        
    if (BUTTON(gamefunc_Strafe) && !pp->sop)
        svel = -info.dyaw >> 6;
    else
        {
        if (info.dyaw > 0)
            angvel = labs((-info.dyaw) >> 8);
        else    
            angvel = info.dyaw >> 8;
        }
    
    svel -= info.dx;

            if (gs.MouseAimingType == 1) // while held
                {
                if (BUTTON(gamefunc_Mouse_Aiming))
                    {
                    SET(pp->Flags, PF_MOUSE_AIMING_ON);
                    gs.MouseAimingOn = TRUE;

                    if (info.dz > 0)
                        // a negative value shifted down will always return -1 for the smallest value
                        // this allows sensitivity in the up direction
                        aimvel = labs((-info.dz) >> 10);
                    else
                        aimvel = info.dz >> 10;
         
                    aimvel = min(127, aimvel);
                    aimvel = max(-128, aimvel);
                    
                    if (gs.MouseInvert)
                        aimvel = -aimvel;
                    }
                else
                    {
                    if (TEST(pp->Flags, PF_MOUSE_AIMING_ON))
                        {
                        SET_LOC_KEY(loc->bits, SK_LOOK_UP, TRUE);
                        RESET(pp->Flags, PF_MOUSE_AIMING_ON);
                        gs.MouseAimingOn = FALSE;
                        }

                    aimvel = 0;
                    vel = -info.dz >> 6;
                    }
                }
            else
            if (gs.MouseAimingType == 0) // togglable button
                {
                if (BUTTON(gamefunc_Mouse_Aiming) && !BUTTONHELD(gamefunc_Mouse_Aiming))
                    {
                    FLIP(pp->Flags, PF_MOUSE_AIMING_ON);
                    gs.MouseAimingOn = !gs.MouseAimingOn;
                    if (!TEST(pp->Flags, PF_MOUSE_AIMING_ON))
                        {
                        SET_LOC_KEY(loc->bits, SK_LOOK_UP, TRUE);
                        PutStringInfo(pp, "Mouse Aiming Off");
                        }
                    else    
                        {
                        PutStringInfo(pp, "Mouse Aiming On");
                        }
                    }

                if (TEST(pp->Flags, PF_MOUSE_AIMING_ON))
                    {
                    if (info.dz > 0)
                        aimvel = labs((-info.dz) >> 10);
                    else
                        aimvel = info.dz >> 10;
         
                    aimvel = min(127, aimvel);
                    aimvel = max(-128, aimvel);
                    
                    if (gs.MouseInvert)
                        aimvel = -aimvel;
                    }
                else
                    {
                    aimvel = 0;
                    vel = -info.dz >> 6;
                    }
                }

    
    if (running)
        {
        if (pp->sop_control)
            turnamount = RUNTURN * 3;
        else
            turnamount = RUNTURN;
            
        keymove = NORMALKEYMOVE << 1;
        }
    else
        {
        if (pp->sop_control)
            turnamount = NORMALTURN * 3;
        else
            turnamount = NORMALTURN;
            
        keymove = NORMALKEYMOVE;
        }

    if (BUTTON(gamefunc_Strafe) && !pp->sop)
        {
        if (BUTTON(gamefunc_Turn_Left))
            svel -= -keymove;
        if (BUTTON(gamefunc_Turn_Right))
            svel -= keymove;
        }
    else
        {
        if (BUTTON(gamefunc_Turn_Left))
            {
            turnheldtime += synctics;
            if (turnheldtime >= TURBOTURNTIME)
                angvel -= turnamount;
            else
                angvel -= PREAMBLETURN;
            }
        else
                if (BUTTON(gamefunc_Turn_Right))
            {
            turnheldtime += synctics;
            if (turnheldtime >= TURBOTURNTIME)
                angvel += turnamount;
            else
                angvel += PREAMBLETURN;
            }
        else
            {
            turnheldtime = 0;
            }
        }
        
    if (BUTTON(gamefunc_Strafe_Left) && !pp->sop)
        svel += keymove;

    if (BUTTON(gamefunc_Strafe_Right) && !pp->sop)
        svel += -keymove;
        
    if (BUTTON(gamefunc_Move_Forward))
        {
        vel += keymove;
        //DSPRINTF(ds,"vel key %d",vel);
        //DebugWriteString(ds);
        }
    else
        {
        //DSPRINTF(ds,"vel %d",vel);
        //DebugWriteString(ds);
        }

    if (BUTTON(gamefunc_Move_Backward))
        vel += -keymove;

    
    if (vel < -MAXVEL)
        vel = -MAXVEL;
    if (vel > MAXVEL)
        vel = MAXVEL;
    if (svel < -MAXSVEL)
        svel = -MAXSVEL;
    if (svel > MAXSVEL)
        svel = MAXSVEL;
    if (angvel < -MAXANGVEL)
        angvel = -MAXANGVEL;
    if (angvel > MAXANGVEL)
        angvel = MAXANGVEL;
    
    momx = mulscale(vel, sintable[NORM_ANGLE(newpp->pang + 512)], 9);
    momy = mulscale(vel, sintable[NORM_ANGLE(newpp->pang)], 9);

    momx += mulscale(svel, sintable[NORM_ANGLE(newpp->pang)], 9);
    momy += mulscale(svel, sintable[NORM_ANGLE(newpp->pang + 1536)], 9);

    loc->vel = momx;
    loc->svel = momy;
    loc->angvel = angvel;
    loc->aimvel = aimvel;

    if (MenuButtonAutoRun)
        {
        MenuButtonAutoRun = FALSE;
        if ((!!TEST(pp->Flags, PF_LOCK_RUN)) != gs.AutoRun)
            SET_LOC_KEY(loc->bits, SK_RUN_LOCK, TRUE);
        }
        
    SET_LOC_KEY(loc->bits, SK_RUN_LOCK, BUTTON(gamefunc_AutoRun));
    
    if (!CommEnabled)
        {
        if (MenuButtonAutoAim)
            {
            MenuButtonAutoAim = FALSE;
            if ((!!TEST(pp->Flags, PF_AUTO_AIM)) != gs.AutoAim)
                SET_LOC_KEY(loc->bits, SK_AUTO_AIM, TRUE);
            }    
        }    
    else    
    if (KEY_PRESSED(sc_Pause))
        {
        SET_LOC_KEY(loc->bits, SK_PAUSE, KEY_PRESSED(sc_Pause));
        KEY_PRESSED(sc_Pause) = 0;
        }
    
    SET_LOC_KEY(loc->bits, SK_CENTER_VIEW, BUTTON(gamefunc_Center_View));
    
    SET_LOC_KEY(loc->bits, SK_RUN, BUTTON(gamefunc_Run));
    SET_LOC_KEY(loc->bits, SK_SHOOT, BUTTON(gamefunc_Fire));

    // actually snap
    SET_LOC_KEY(loc->bits, SK_SNAP_UP, BUTTON(gamefunc_Aim_Up));
    SET_LOC_KEY(loc->bits, SK_SNAP_DOWN, BUTTON(gamefunc_Aim_Down));

    // actually just look
    SET_LOC_KEY(loc->bits, SK_LOOK_UP, BUTTON(gamefunc_Look_Up));
    SET_LOC_KEY(loc->bits, SK_LOOK_DOWN, BUTTON(gamefunc_Look_Down));
    
    
    for (i = 0; i < MAX_WEAPONS_KEYS; i++)
        {
        if (BUTTON(gamefunc_Weapon_1 + i))
            {
            SET(loc->bits, i + 1);
            break;
            }
        }
    
    if (BUTTON(gamefunc_Next_Weapon))
        {
        USERp u = User[pp->PlayerSprite];
        short next_weapon = u->WeaponNum + 1;
        short start_weapon;
        
        CONTROL_ClearButton(gamefunc_Next_Weapon);
                
        start_weapon = u->WeaponNum + 1;

        if (u->WeaponNum == WPN_SWORD)    
            start_weapon = WPN_STAR;

        if (u->WeaponNum == WPN_FIST)    
            {
            next_weapon = 14;
            }
        else    
            {    
            next_weapon = -1;
            for (i = start_weapon; TRUE; i++)
                {
                if (i >= MAX_WEAPONS_KEYS)
                    {
                    next_weapon = 13;
                    break;
                    }
                    
                if (TEST(pp->WpnFlags, BIT(i)) && pp->WpnAmmo[i])
                    {
                    next_weapon = i;
                    break;
                    }
                }
            }    
        
        SET(loc->bits, next_weapon + 1);
        }
        
        
    if (BUTTON(gamefunc_Previous_Weapon))
        {
        USERp u = User[pp->PlayerSprite];
        short prev_weapon = u->WeaponNum - 1;
        short start_weapon;

        CONTROL_ClearButton(gamefunc_Previous_Weapon);

        start_weapon = u->WeaponNum - 1;
        
        if (u->WeaponNum == WPN_SWORD)    
            {
            prev_weapon = 13;
            }
        else    
        if (u->WeaponNum == WPN_STAR)    
            {
            prev_weapon = 14;
            }
        else    
            {
            prev_weapon = -1;
            for (i = start_weapon; TRUE; i--)
                {
                if (i <= -1)
                    i = WPN_HEART;

                if (TEST(pp->WpnFlags, BIT(i)) && pp->WpnAmmo[i])
                    {
                    prev_weapon = i;
                    break;
                    }
                }
            }    
        
        SET(loc->bits, prev_weapon + 1);
        }
    
    inv_hotkey = 0;
    if (BUTTON(gamefunc_Med_Kit))
        inv_hotkey = INVENTORY_MEDKIT+1;
    if (BUTTON(gamefunc_Smoke_Bomb))
        inv_hotkey = INVENTORY_CLOAK+1;
    if (BUTTON(gamefunc_Night_Vision))
        inv_hotkey = INVENTORY_NIGHT_VISION+1;
    if (BUTTON(gamefunc_Gas_Bomb))
        inv_hotkey = INVENTORY_CHEMBOMB+1;
    if (BUTTON(gamefunc_Flash_Bomb) && dimensionmode == 3)
        inv_hotkey = INVENTORY_FLASHBOMB+1;
    if (BUTTON(gamefunc_Caltrops))
        inv_hotkey = INVENTORY_CALTROPS+1;
    
    SET(loc->bits, inv_hotkey<<SK_INV_HOTKEY_BIT0);
        
    SET_LOC_KEY(loc->bits, SK_INV_USE, BUTTON(gamefunc_Inventory));

    SET_LOC_KEY(loc->bits, SK_OPERATE, BUTTON(gamefunc_Open));
    SET_LOC_KEY(loc->bits, SK_JUMP, BUTTON(gamefunc_Jump));
    SET_LOC_KEY(loc->bits, SK_CRAWL, BUTTON(gamefunc_Crouch));

    SET_LOC_KEY(loc->bits, SK_TURN_180, BUTTON(gamefunc_TurnAround));

    SET_LOC_KEY(loc->bits, SK_INV_LEFT, BUTTON(gamefunc_Inventory_Left));
    SET_LOC_KEY(loc->bits, SK_INV_RIGHT, BUTTON(gamefunc_Inventory_Right));
    
    SET_LOC_KEY(loc->bits, SK_HIDE_WEAPON, BUTTON(gamefunc_Holster_Weapon));

    // need BUTTON
    SET_LOC_KEY(loc->bits, SK_CRAWL_LOCK, KEY_PRESSED(KEYSC_NUM));
    
    if (gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
        {
        if (BUTTON(gamefunc_See_Co_Op_View))
            {
            CONTROL_ClearButton(gamefunc_See_Co_Op_View);

            screenpeek = connectpoint2[screenpeek];

            if (screenpeek < 0)
                screenpeek = connecthead;
    
            if (dimensionmode != 2 && screenpeek == myconnectindex)
                {
            // JBF: figure out what's going on here
                COVERsetbrightness(gs.Brightness,&palette_data[0][0]);
                memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
                DoPlayerDivePalette(pp);  // Check Dive again
                DoPlayerNightVisionPalette(pp);  // Check Night Vision again
                } else
                {
                PLAYERp tp = Player+screenpeek;

                if (tp->FadeAmt<=0)
                    memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
                else    
                    memcpy(pp->temp_pal, tp->temp_pal, sizeof(tp->temp_pal));
                COVERsetbrightness(gs.Brightness,&palette_data[0][0]);
                DoPlayerDivePalette(tp);  
                DoPlayerNightVisionPalette(tp);
                }
            }
        }   
    
#if DEBUG
    DebugKeys(pp);

    if (!CommEnabled)                   // Single player only keys
        SinglePlayInput(pp);
#endif

    FunctionKeys(pp);

    if (BUTTON(gamefunc_Toggle_Crosshair))
        {
        CONTROL_ClearButton(gamefunc_Toggle_Crosshair);
        pToggleCrosshair(pp);
        }
    }

#define MAP_WHITE_SECTOR    (LT_GREY + 2)
#define MAP_RED_SECTOR      (RED + 6)
#define MAP_FLOOR_SPRITE    (RED + 8)
#define MAP_ENEMY           (RED + 10)
#define MAP_SPRITE          (FIRE + 8)
#define MAP_PLAYER          (GREEN + 6)

#define MAP_BLOCK_SPRITE    (DK_BLUE + 6)

void drawoverheadmap(int cposx, int cposy, int czoom, short cang)
    {
    int i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
    int dax, day, cosang, sinang, xspan, yspan, sprx, spry;
    int xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
    int xvect, yvect, xvect2, yvect2;
    char col;
    walltype *wal, *wal2;
    spritetype *spr;
    short p;
    static int pspr_ndx[8]={0,0,0,0,0,0,0,0};
    BOOL sprisplayer = FALSE;
    short txt_x, txt_y;
    
    // draw location text
    if (gs.BorderNum <= BORDER_BAR-1)    
        {
        txt_x = 7;
        txt_y = 168;
        } else
        {
        txt_x = 7;
        txt_y = 147;
        }
        
    if (ScrollMode2D)
        {
        minigametext(txt_x,txt_y-7,"Follow Mode",0,2+8);
        }
    
    if (UserMapName[0])
        sprintf(ds,"%s",UserMapName);
    else    
        sprintf(ds,"%s",LevelInfo[Level].Description);
        
    minigametext(txt_x,txt_y,ds,0,2+8);

    //////////////////////////////////

    xvect = sintable[(2048 - cang) & 2047] * czoom;
    yvect = sintable[(1536 - cang) & 2047] * czoom;
    xvect2 = mulscale(xvect, yxaspect, 16);
    yvect2 = mulscale(yvect, yxaspect, 16);
    
    // Draw red lines
    for (i = 0; i < numsectors; i++)
        {
        startwall = sector[i].wallptr;
        endwall = sector[i].wallptr + sector[i].wallnum - 1;

        z1 = sector[i].ceilingz;
        z2 = sector[i].floorz;

        for (j = startwall, wal = &wall[startwall]; j <= endwall; j++, wal++)
            {
            k = wal->nextwall;
            if (k < 0)
                continue;

            if ((show2dwall[j >> 3] & (1 << (j & 7))) == 0)
                continue;
            if ((k > j) && ((show2dwall[k >> 3] & (1 << (k & 7))) > 0))
                continue;

            if (sector[wal->nextsector].ceilingz == z1)
                if (sector[wal->nextsector].floorz == z2)
                    if (((wal->cstat | wall[wal->nextwall].cstat) & (16 + 32)) == 0)
                        continue;

            col = 152;

            //if (dimensionmode == 2)
            if (dimensionmode == 6)
                {
                if (sector[i].floorz != sector[i].ceilingz)
                    if (sector[wal->nextsector].floorz != sector[wal->nextsector].ceilingz)
                        if (((wal->cstat | wall[wal->nextwall].cstat) & (16 + 32)) == 0)
                            if (sector[i].floorz == sector[wal->nextsector].floorz)
                                continue;
                if (sector[i].floorpicnum != sector[wal->nextsector].floorpicnum)
                    continue;
                if (sector[i].floorshade != sector[wal->nextsector].floorshade)
                    continue;
                col = 12;
                }

            ox = wal->x - cposx;
            oy = wal->y - cposy;
            x1 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
            y1 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

            wal2 = &wall[wal->point2];
            ox = wal2->x - cposx;
            oy = wal2->y - cposy;
            x2 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
            y2 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

            drawline256(x1 + (xdim << 11), y1 + (ydim << 11), x2 + (xdim << 11), y2 + (ydim << 11), col);
            }
        }

    // Draw sprites
    k = Player[screenpeek].PlayerSprite;
    for (i = 0; i < numsectors; i++)
        for (j = headspritesect[i]; j >= 0; j = nextspritesect[j])
        {
            for(p=connecthead;p >= 0;p=connectpoint2[p])
            {
                if(Player[p].PlayerSprite == j)
                    {
                    if(sprite[Player[p].PlayerSprite].xvel > 16)
                        pspr_ndx[myconnectindex] = ((totalclock>>4)&3);
                    sprisplayer = TRUE;
                    
                    goto SHOWSPRITE;
                    }
            }
            if ((show2dsprite[j >> 3] & (1 << (j & 7))) > 0)
                {
                SHOWSPRITE:
                spr = &sprite[j];
                
                col = 56;
                if ((spr->cstat & 1) > 0)
                    col = 248;
                if (j == k)
                    col = 31;

                sprx = spr->x;
                spry = spr->y;

                k = spr->statnum;
                if ((k >= 1) && (k <= 8) && (k != 2))   // Interpolate moving
                    {
                    sprx = sprite[j].x;
                    spry = sprite[j].y;
                    }

                switch (spr->cstat & 48)
                    {
                case 0:  // Regular sprite
                    if(Player[p].PlayerSprite == j)
                    {
                    ox = sprx - cposx;
                    oy = spry - cposy;
                    x1 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                    y1 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                    if (dimensionmode == 5 && (gNet.MultiGameType != MULTI_GAME_COMMBAT || j == Player[screenpeek].PlayerSprite))
                        {
                        ox = (sintable[(spr->ang + 512) & 2047] >> 7);
                        oy = (sintable[(spr->ang) & 2047] >> 7);
                        x2 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                        y2 = mulscale(oy, xvect, 16) + mulscale(ox, yvect, 16);

                        if (j == Player[screenpeek].PlayerSprite)
                            {
                            x2 = 0L;
                            y2 = -(czoom << 5);
                            }

                        x3 = mulscale(x2, yxaspect, 16);
                        y3 = mulscale(y2, yxaspect, 16);

                        drawline256(x1 - x2 + (xdim << 11), y1 - y3 + (ydim << 11),
                            x1 + x2 + (xdim << 11), y1 + y3 + (ydim << 11), col);
                        drawline256(x1 - y2 + (xdim << 11), y1 + x3 + (ydim << 11),
                            x1 + x2 + (xdim << 11), y1 + y3 + (ydim << 11), col);
                        drawline256(x1 + y2 + (xdim << 11), y1 - x3 + (ydim << 11),
                            x1 + x2 + (xdim << 11), y1 + y3 + (ydim << 11), col);
                        }
                    else
                        {
                        if (((gotsector[i >> 3] & (1 << (i & 7))) > 0) && (czoom > 192))
                            {
                            daang = (spr->ang - cang) & 2047;
                            if (j == Player[screenpeek].PlayerSprite)
                                {
                                x1 = 0;
                                //y1 = (yxaspect << 2);
                                y1 = 0;
                                daang = 0;
                                }
                                
                            // Special case tiles
                            if (spr->picnum == 3123) break;
                            
                            if(sprisplayer)    
                                {
                                if (gNet.MultiGameType != MULTI_GAME_COMMBAT || j == Player[screenpeek].PlayerSprite)
                                    rotatesprite((x1 << 4) + (xdim << 15), (y1 << 4) + (ydim << 15), mulscale(czoom * (spr->yrepeat), yxaspect, 16), daang, 1196+pspr_ndx[myconnectindex], spr->shade, spr->pal, (spr->cstat & 2) >> 1, windowx1, windowy1, windowx2, windowy2);
                                }    
                            else    
                                rotatesprite((x1 << 4) + (xdim << 15), (y1 << 4) + (ydim << 15), mulscale(czoom * (spr->yrepeat), yxaspect, 16), daang, spr->picnum, spr->shade, spr->pal, (spr->cstat & 2) >> 1, windowx1, windowy1, windowx2, windowy2);
                            }
                        }
                    }
                    break;
                case 16: // Rotated sprite
                    x1 = sprx;
                    y1 = spry;
                    tilenum = spr->picnum;
                    xoff = (int) ((signed char) ((picanm[tilenum] >> 8) & 255)) + ((int) spr->xoffset);
                    if ((spr->cstat & 4) > 0)
                        xoff = -xoff;
                    k = spr->ang;
                    l = spr->xrepeat;
                    dax = sintable[k & 2047] * l;
                    day = sintable[(k + 1536) & 2047] * l;
                    l = tilesizx[tilenum];
                    k = (l >> 1) + xoff;
                    x1 -= mulscale(dax, k, 16);
                    x2 = x1 + mulscale(dax, l, 16);
                    y1 -= mulscale(day, k, 16);
                    y2 = y1 + mulscale(day, l, 16);

                    ox = x1 - cposx;
                    oy = y1 - cposy;
                    x1 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                    y1 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                    ox = x2 - cposx;
                    oy = y2 - cposy;
                    x2 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                    y2 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                    drawline256(x1 + (xdim << 11), y1 + (ydim << 11),
                        x2 + (xdim << 11), y2 + (ydim << 11), col);

                    break;
                case 32:    // Floor sprite
                    if (dimensionmode == 5)
                        {
                        tilenum = spr->picnum;
                        xoff = (int) ((signed char) ((picanm[tilenum] >> 8) & 255)) + ((int) spr->xoffset);
                        yoff = (int) ((signed char) ((picanm[tilenum] >> 16) & 255)) + ((int) spr->yoffset);
                        if ((spr->cstat & 4) > 0)
                            xoff = -xoff;
                        if ((spr->cstat & 8) > 0)
                            yoff = -yoff;

                        k = spr->ang;
                        cosang = sintable[(k + 512) & 2047];
                        sinang = sintable[k];
                        xspan = tilesizx[tilenum];
                        xrepeat = spr->xrepeat;
                        yspan = tilesizy[tilenum];
                        yrepeat = spr->yrepeat;

                        dax = ((xspan >> 1) + xoff) * xrepeat;
                        day = ((yspan >> 1) + yoff) * yrepeat;
                        x1 = sprx + mulscale(sinang, dax, 16) + mulscale(cosang, day, 16);
                        y1 = spry + mulscale(sinang, day, 16) - mulscale(cosang, dax, 16);
                        l = xspan * xrepeat;
                        x2 = x1 - mulscale(sinang, l, 16);
                        y2 = y1 + mulscale(cosang, l, 16);
                        l = yspan * yrepeat;
                        k = -mulscale(cosang, l, 16);
                        x3 = x2 + k;
                        x4 = x1 + k;
                        k = -mulscale(sinang, l, 16);
                        y3 = y2 + k;
                        y4 = y1 + k;

                        ox = x1 - cposx;
                        oy = y1 - cposy;
                        x1 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                        y1 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                        ox = x2 - cposx;
                        oy = y2 - cposy;
                        x2 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                        y2 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                        ox = x3 - cposx;
                        oy = y3 - cposy;
                        x3 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                        y3 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                        ox = x4 - cposx;
                        oy = y4 - cposy;
                        x4 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
                        y4 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

                        drawline256(x1 + (xdim << 11), y1 + (ydim << 11),
                            x2 + (xdim << 11), y2 + (ydim << 11), col);

                        drawline256(x2 + (xdim << 11), y2 + (ydim << 11),
                            x3 + (xdim << 11), y3 + (ydim << 11), col);

                        drawline256(x3 + (xdim << 11), y3 + (ydim << 11),
                            x4 + (xdim << 11), y4 + (ydim << 11), col);

                        drawline256(x4 + (xdim << 11), y4 + (ydim << 11),
                            x1 + (xdim << 11), y1 + (ydim << 11), col);

                        }
                    break;
                    }
                }
        }
    // Draw white lines
    for (i = 0; i < numsectors; i++)
        {
        startwall = sector[i].wallptr;
        endwall = sector[i].wallptr + sector[i].wallnum - 1;

        for (j = startwall, wal = &wall[startwall]; j <= endwall; j++, wal++)
            {
            if (wal->nextwall >= 0)
                continue;

            if ((show2dwall[j >> 3] & (1 << (j & 7))) == 0)
                continue;

            if (tilesizx[wal->picnum] == 0)
                continue;
            if (tilesizy[wal->picnum] == 0)
                continue;

            ox = wal->x - cposx;
            oy = wal->y - cposy;
            x1 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
            y1 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

            wal2 = &wall[wal->point2];
            ox = wal2->x - cposx;
            oy = wal2->y - cposy;
            x2 = mulscale(ox, xvect, 16) - mulscale(oy, yvect, 16);
            y2 = mulscale(oy, xvect2, 16) + mulscale(ox, yvect2, 16);

            drawline256(x1 + (xdim << 11), y1 + (ydim << 11), x2 + (xdim << 11), y2 + (ydim << 11), 24);
            }
        }

    }
    
extern int tilefileoffs[MAXTILES];//offset into the
extern char tilefilenum[MAXTILES];//0-11

#if 0
loadtile (short tilenume)
{
    char *ptr;
        int i;
        char zerochar = 0;

        if (walsiz[tilenume] <= 0)
           return;

    i = tilefilenum[tilenume];
    if (i != artfilnum)
            {
        if (artfil != -1)
                    kclose(artfil);
        artfilnum = i;
        artfilplc = 0L;

        artfilename[7] = (i%10)+48;
        artfilename[6] = ((i/10)%10)+48;
        artfilename[5] = ((i/100)%10)+48;
        artfil = kopen4load(artfilename);
        faketimerhandler();
            }

    if (waloff[tilenume] == 0)
            allocache(&waloff[tilenume],walsiz[tilenume],&zerochar);

    if (artfilplc != tilefileoffs[tilenume])
    {
        klseek(artfil,tilefileoffs[tilenume]-artfilplc,SEEK_CUR);
        faketimerhandler();
    }

    ptr = (char *)waloff[tilenume];
    kread(artfil,ptr,walsiz[tilenume]);
    faketimerhandler();
    artfilplc = tilefileoffs[tilenume]+walsiz[tilenume];
}
#endif

#if RANDOM_DEBUG
int 
RandomRange(int range, char *file, unsigned line)
    {
    ULONG rand_num;
    ULONG value;
    extern FILE * fout_err;
    extern ULONG MoveThingsCount;
    
    if (RandomPrint && !Prediction)
        { 
        sprintf(ds,"mtc %d, %s, line %d, %d",MoveThingsCount,file,line,randomseed);
        DebugWriteString(ds);
        }
    
    if (range <= 0)
        return(0);
    
    rand_num = krand2();
    
    if (rand_num == 65535U)
        rand_num--;
    
    // shift values to give more precision
    value = (rand_num << 14) / ((65535UL << 14) / range);
    
    if (value >= range)
        value = range - 1;
        
    return(value);
    }
#else
int 
RandomRange(int range)
    {
    ULONG rand_num;
    ULONG value;
    
    if (range <= 0)
        return(0);
    
    rand_num = RANDOM();
    
    if (rand_num == 65535U)
        rand_num--;
    
    // shift values to give more precision
    value = (rand_num << 14) / ((65535UL << 14) / range);
    
    if (value >= (ULONG)range)
        value = range - 1;
        
    return(value);
    }
#endif    

int 
StdRandomRange(int range)
    {
    ULONG rand_num;
    ULONG value;
    
    if (range <= 0)
        return(0);
    
    rand_num = STD_RANDOM();
    
    if (rand_num == RAND_MAX)
        rand_num--;
    
    // shift values to give more precision
#if (RAND_MAX > 0x7fff)
    value = rand_num / (((int)RAND_MAX) / range);
#else
    value = (rand_num << 14) / ((((int)RAND_MAX) << 14) / range);
#endif
    
    if (value >= (ULONG)range)
        value = range - 1;
        
    return(value);
    }

#include "saveable.h"

static saveable_data saveable_build_data[] = {
    SAVE_DATA(sector),
    SAVE_DATA(sprite),
    SAVE_DATA(wall)
};

saveable_module saveable_build = {
    // code
    NULL,
    0,

    // data
    saveable_build_data,
    NUM_SAVEABLE_ITEMS(saveable_build_data)
};

// vim:ts=4:sw=4:expandtab:
