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
#include "osdfuncs.h"
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
#include "gamedefs.h"
#include "config.h"

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

#include "startwin.h"
#include "version.h"

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
8, // panel scale
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

int CommandSetup = FALSE;

const char *GameEditionName = "Unknown edition";
int GameVariant = GRPFILE_GAME_REG;
BOOL UseDarts = FALSE;

char UserMapName[80]="", buffer[80], ch;
char LevelName[20];

BYTE DebugPrintColor = 255;

int krandcount;

extern void SetupOSDCommands(void); // osdcmds.c

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
    (void)ptr;
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
    int rv;

    rv = loadboard(filename, SW_SHAREWARE ? 1 : 0, &Player[0].posx, &Player[0].posy, &Player[0].posz, &Player[0].pang, &Player[0].cursectnum);
    if (rv < 0)
        TerminateGame();
    switch (rv)
        {
        case -1:
            wm_msgbox(NULL, "Level not found: %s", filename);
            break;

        case -2:
            wm_msgbox(NULL, "Level not compatible: %s", filename);
            break;
        }
    if (rv < 0)
        exit(0);
    }

VOID
LoadImages(char *filename)
    {
    if (loadpics(filename, 32*1048576) == -1)
        {
        TerminateGame();
        wm_msgbox(NULL, "Art not found. Please check your GRP file.");
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
        wm_msgbox(NULL,"To play a Network game with more than 4 players you must purchase "
            "the full version.  Read the Ordering Info screens for details.");
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


void COVERsetbrightness(int bright, unsigned char *pal)
    {
    setbrightness(bright, pal, 0);
    }


static int netsuccess = 0;

int nextvoxid = 0;  // JBF
static const char *deffile = "sw.def";

VOID
InitGame(VOID)
    {
    extern int MovesPerPacket;
    //void *ReserveMem=NULL;
    int i;
    extern void (*ASS_MessageOutputString)(const char *);

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

    inittimer(120, NULL);

    CON_InitConsole();  // Init console command list

    ////DSPRINTF(ds,"%s, %d",__FILE__,__LINE__);   MONO_PRINT(ds);

    memcpy(palette_data,palette,768);
    InitPalette();

    // sets numplayers, connecthead, connectpoint2, myconnectindex
    if (netsuccess) {
        buildputs("Waiting for players...\n");
        while (initmultiplayerscycle()) {
            handleevents();
            if (quitevent) {
                return;
            }
        }
    } else {
        initsingleplayers();
    }

    initsynccrc();

    // code to duplicate packets
    if (numplayers > 4 && MovesPerPacket == 1)
        {
        MovesPerPacket = 2;
        }

    MultiSharewareCheck();

    CommPlayers = numplayers;
    OrigCommPlayers = CommPlayers;
    if (numplayers > 1)
        {
        CommEnabled = TRUE;
        if(!BotMode)
            gNet.MultiGameType = MULTI_GAME_COMMBAT;
        else
            gNet.MultiGameType = MULTI_GAME_AI_BOTS;
        NetBroadcastMode = (networkmode == MMULTI_MODE_P2P);
        }

    LoadDemoRun();

    // LoadImages will now proceed to steal all the remaining heap space
    buildputs("Loading sound and graphics...\n");
    LoadImages("tiles000.art");

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
        LoadLevel("$dozer.map");
        SetupPreCache();
        DoTheCache();
        }
    else
        {
        LoadLevel(UserMapName);
        SetupPreCache();
        DoTheCache();
        }

    Set_GameMode();
    GraphicsMode = TRUE;
    SetupAspectRatio();

    COVERsetbrightness(gs.Brightness,&palette_data[0][0]);

    ASS_MessageOutputString = buildputs;    // Send JFAudioLib messages to the JFBuild console.
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
LEVEL_INFO LevelInfoWanton[MAX_LEVELS_REG+2] =
    {
    {"title.map",      "theme.mid", " ", " ", " "  },
    {"$bullet.map",    "e1l01.mid", "Seppuku Station", "0 : 55", "5 : 00"  },
    {"$dozer.map",     "e1l03.mid", "Zilla Construction", "4 : 59", "8 : 00"  },
    {"$shrine.map",    "e1l02.mid", "Master Leep's Temple", "3 : 16", "10 : 00"  },
    {"$woods.map",     "e1l04.mid", "Dark Woods of the Serpent", "7 : 06", "16 : 00"  },
    {"$whirl.map",     "yokoha03.mid", "Chinatown", "5 : 30", "10 : 00"   },
    {"$tank.map",      "nippon34.mid", "Monastary", "1 : 46", "4 : 00"   },
    {"$boat.map",      "execut11.mid", "Trolly Yard", "1 : 56", "4 : 00"   },
    {"$garden.map",    "execut11.mid", "Resturant", "1 : 06", "2 : 00"   },
    {"$outpost.map",   "sanai.mid",    "Skyscraper", "1 : 23", "3 : 00"      },
    {"$hidtemp.map",   "kotec2.mid",   "Airplane", "2 : 05", "4 : 10"     },
    {"$plax1.map",     "kotec2.mid",   "Military Base", "6 : 32", "12 : 00"     },
    {"$bath.map",      "yokoha03.mid", "Train", "10 : 00", "10 : 00"   },
    {"$airport.map",   "nippon34.mid", "Auto Factory", "2 : 59", "6 : 00"   },
    {"$refiner.map",   "kotoki12.mid", "Crude Oil", "2 : 40", "5 : 00"   },
    {"$newmine.map",   "hoshia02.mid", "Coolie Mines", "2 : 48", "6 : 00"   },
    {"$subbase.map",   "hoshia02.mid", "Subpen 7", "2 : 02", "4 : 00"   },
    {"$rock.map",      "kotoki12.mid", "The Great Escape", "3 : 18", "6 : 00"   },
    {"$yamato.map",    "sanai.mid",    "Floating Fortress", "11 : 38", "20 : 00"      },
    {"$seabase.map",   "kotec2.mid",   "Water Torture", "5 : 07", "10 : 00"     },
    {"$volcano.map",   "kotec2.mid",   "Skyline", "9 : 15", "20 : 00"     },
    {"$shore.map",     "kotec2.mid",   "Redwood Forest", "3 : 58", "8 : 00"     },
    {"$auto.map",      "kotec2.mid",   "The Docks", "4 : 07", "8 : 00"     },
    {"tank.map",       "kotec2.mid",   "Waterfight (DM only)", "10 : 00", "10 : 00"     },
    {"$dmwoods.map",   "kotec2.mid",   "Wanton DM 1 (DM only)", "10 : 00", "10 : 00"     },
    {"$dmshrin.map",   "kotec2.mid",   "Wanton DM 2 (DM only)", "10 : 00", "10 : 00"     },
    {"$rush.map",      "kotec2.mid",   "Lo Wang Rally (DM only)", "10 : 00", "10 : 00"     },
    {"shotgun.map",    "kotec2.mid",   "Wanton CTF (CTF)", "10 : 00", "10 : 00"     },
    {"$dmdrop.map",    "kotec2.mid",   "Killing Fields (CTF)", "10 : 00", "10 : 00"     },
    {NULL, NULL, NULL, NULL, NULL}
    };

char EpisodeNames[2][MAX_EPISODE_NAME_LEN+2] = {
    "^Enter the Wang",
    "^Code of Honor"
};
char EpisodeNamesWanton[2][MAX_EPISODE_NAME_LEN+2] = {
    "^Enter the Wang",
    "^Wanton Destr"
};
char EpisodeSubtitles[2][MAX_EPISODE_SUBTITLE_LEN+1] = {
    "Four levels (Shareware Version)",
    "Eighteen levels (Full Version Only)"
};
char EpisodeSubtitlesWanton[2][MAX_EPISODE_SUBTITLE_LEN+1] = {
    "Four levels (Shareware Version)",
    "Twelve levels (Full Version Only)"
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
    short j;

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

    if (!DemoMode)
        {
        char windowtitle[256];

        if (UserMapName[0])
            Bsnprintf(windowtitle, sizeof(windowtitle), "User map: %s - %s", UserMapName, GameEditionName);
        else
            Bsnprintf(windowtitle, sizeof(windowtitle), "%s - %s", LevelInfo[Level].Description, GameEditionName);

        windowtitle[sizeof(windowtitle)-1] = 0;
        wm_setwindowtitle(windowtitle);
        }

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

    for (stat = STAT_PLAYER0; stat < STAT_PLAYER0 + CommPlayers; stat++)
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
                ForceMenus = TRUE;
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
    int fin;
    unsigned char pal[PAL_SIZE];
    UserInput uinfo = { FALSE, FALSE, dir_None };


    //DSPRINTF(ds,"LogoLevel...");
    //MONO_PRINT(ds);

    // start music at logo
    PlaySong(LevelInfo[0].SongName, RedBookSong[0], TRUE, TRUE);

    //DSPRINTF(ds,"After music stuff...");
    //MONO_PRINT(ds);

    // PreCache Anim
    LoadAnm(0);

    if ((fin = kopen4load("3drealms.pal", 0)) != -1)
        {
        kread(fin, pal, PAL_SIZE);
        kclose(fin);
        setbrightness(gs.Brightness, pal, 2);
        }
    //DSPRINTF(ds,"Just read in 3drealms.pal...");
    //MONO_PRINT(ds);

    //FadeOut(0, 0);
    ready2send = 0;
    totalclock = 0;
    ototalclock = 0;

    //DSPRINTF(ds,"About to display 3drealms pic...");
    //MONO_PRINT(ds);

    //FadeIn(0, 3);

    ResetKeys();
    while (TRUE)
        {
        clearallviews(0);
        rotatesprite(0, 0, RS_SCALE, 0, THREED_REALMS_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        nextpage();

        handleevents();
        flushpackets();

        CONTROL_GetUserInput(&uinfo);
        CONTROL_ClearUserInput(&uinfo);
        if (quitevent) break;

        // taken from top of faketimerhandler
        // limits checks to max of 40 times a second
        if (totalclock >= ototalclock + synctics)
            {
            ototalclock += synctics;
            }

        if (totalclock > 5*120 || KeyPressed() || uinfo.button0 || uinfo.button1)
            {
            break;
            }
        }

    clearallviews(0);
    nextpage();
    setbrightness(gs.Brightness, &palette_data[0][0], 2);

    // put up a blank screen while loading

    //DSPRINTF(ds,"End of LogoLevel...");
    //MONO_PRINT(ds);

    }

VOID
CreditsLevel(VOID)
    {
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
    clearallviews(0);
    nextpage();

    // Lo Wang feel like singing!
    handle = PlaySound(DIGI_JG95012,&zero,&zero,&zero,v3df_none);

    if (handle > 0)
        while(FX_SoundActive(handle))
            handleevents();

    // try 14 then 2 then quit
    if (!PlaySong(NULL, 14, FALSE, TRUE))
        {
        if (!PlaySong(NULL, 2, FALSE, TRUE))
            {
            handle = PlaySound(DIGI_NOLIKEMUSIC,&zero,&zero,&zero,v3df_none);
            if (handle > 0)
                while(FX_SoundActive(handle))
                    handleevents();
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
        handleevents();
        flushpackets();

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
    clearallviews(0);
    nextpage();
    ResetKeys();
    StopSong();
    }


VOID
SybexScreen(VOID)
    {
    UserInput uinfo;

    if (!SW_SHAREWARE) return;

    if (CommEnabled)
        return;

    rotatesprite(0, 0, RS_SCALE, 0, 5261, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    nextpage();

    CONTROL_GetUserInput(&uinfo);
    CONTROL_ClearUserInput(&uinfo);
    ResetKeys();
    while (TRUE)
        {
        handleevents();
        CONTROL_GetUserInput(&uinfo);
        CONTROL_ClearUserInput(&uinfo);
        if (KeyPressed() || quitevent || uinfo.button0 || uinfo.button1)
            break;
        }
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
    clearallviews(0);
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

    clearallviews(0);
    nextpage();
    SetPaletteToVESA(backup_pal);

    // put up a blank screen while loading
    clearallviews(0);
    nextpage();

    ready2send = bakready2send;
    totalclock = bak;
    }
*/
// CTW REMOVED END

VOID
TitleLevel(VOID)
    {
    //int fin;
    //unsigned char pal[PAL_SIZE];
    unsigned char tempbuf[256];
    unsigned char *palook_bak = palookup[0];
    int i;

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    palookup[0] = tempbuf;

    clearallviews(0);
    nextpage();

//    if ((fin = kopen4load("title.pal", 0)) != -1)
//        {
//        kread(fin, pal, PAL_SIZE);
//        kclose(fin);
//        SetPaletteToVESA(pal);
//        }

//    clearallviews(0);
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
        flushpackets();
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

//    clearallviews(0);
//    nextpage();
    //SetPaletteToVESA(backup_pal);

    // put up a blank screen while loading
//    clearallviews(0);
//    nextpage();
    }


VOID DrawMenuLevelScreen(VOID)
    {
    flushperms();
    clearallviews(0);
    rotatesprite(0, 0, RS_SCALE, 0, TITLE_PIC, 20, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    }

VOID DrawStatScreen(VOID)
    {
    flushperms();
    clearallviews(0);
    rotatesprite(0, 0, RS_SCALE, 0, STAT_SCREEN_PIC, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
    }

VOID DrawLoadLevelScreen(VOID)
    {
    flushperms();
    clearallviews(0);
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
    extern BOOL MNU_StartNetGame(void);
    extern int totalclocklock;
    short w,h;

    DSPRINTF(ds,"MenuLevel...");
    MONO_PRINT(ds);

    wm_setwindowtitle(GameEditionName);

    if (gs.MusicOn)
        {
        PlaySong(LevelInfo[0].SongName, RedBookSong[0], TRUE, FALSE);
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
    if (!CommEnabled && CommPlayers <= 1 && !FinishAnim && !NoDemoStartup)
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

    clearallviews(0);
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

    if (SW_SHAREWARE)
        {
        // go to ordering menu only if shareware
        if (FinishAnim)
            {
            ForceMenus = TRUE;
            ControlPanelType = ct_ordermenu;
            FinishAnim = 0;
            }
        }
    else
        {
        FinishAnim = 0;
        }

    while (TRUE)
        {
        handleevents();
        OSD_DispatchQueued();

        if (quitevent)
            {
            if (CommPlayers >= 2)
                MultiPlayQuitFlag = TRUE;
            else
                QuitFlag = TRUE;
            }

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
            ForceMenus = TRUE;
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
    clearallviews(0);
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

    (void)level_name;

    DrawLoadLevelScreen();

    if (DemoMode)
        sprintf(ds,"DEMO");
    else
        sprintf(ds,"ENTERING");

    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 170, ds,1,16);

    if (UserMapName[0])
        snprintf(ds, sizeof(ds), "%s",UserMapName);
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
    (void)SpriteNum;
    PlaySound(DIGI_PLAYERYELL3, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    return(0);
    }

int BonusKickSound(short SpriteNum)
    {
    PLAYERp pp = Player + myconnectindex;
    (void)SpriteNum;
    PlaySound(DIGI_PLAYERYELL2, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    return(0);
    }

int BonusGrabSound(short SpriteNum)
    {
    PLAYERp pp = Player + myconnectindex;
    (void)SpriteNum;
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
    UserInput uinfo = { FALSE, FALSE, dir_None };

    (void)pp;

    if(Level < 0) Level = 0;

	flushperms();
    clearallviews(0);
    nextpage();

    KB_ClearKeysDown();

    totalclock = ototalclock = 0;

    if (gs.MusicOn)
        {
        PlaySong(voc[DIGI_ENDLEV].name, 3, TRUE, TRUE);
        }

    // special case code because I don't care any more!
    if (FinishAnim)
        {
        rotatesprite(0, 0, RS_SCALE, 0, 5120, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        rotatesprite(158<<16, 86<<16, RS_SCALE, 0, State->Pic, 0, 0, TITLE_ROT_FLAGS, 0, 0, xdim - 1, ydim - 1);
        nextpage();
        FadeIn(0,0);
        }

    BonusDone = FALSE;
    while (!BonusDone)
        {
        handleevents();
        flushpackets();

        // taken from top of faketimerhandler
        if (totalclock < ototalclock + synctics)
            {
            continue;
            }
        ototalclock += synctics;

        CONTROL_GetUserInput(&uinfo);
        CONTROL_ClearUserInput(&uinfo);
        if (KEY_PRESSED(KEYSC_SPACE) || KEY_PRESSED(KEYSC_ENTER) || uinfo.button0 || uinfo.button1)
            {
            if (State >= s_BonusRest && State < &s_BonusRest[SIZ(s_BonusRest)])
                {
                State = s_BonusAnim[STD_RANDOM_RANGE(SIZ(s_BonusAnim))];
                Tics = 0;
                }
            }

        gStateControl(&State, &Tics);
		clearallviews(0);
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
    extern BOOL FinishedLevel;
    extern int PlayClock;
    extern short LevelSecrets;
    extern short TotalKillable;
    short w,h;

    short rows,cols,i,j;
    PLAYERp pp = NULL;
    int x,y;
    short death_total[MAX_SW_PLAYERS_REG];
    short kills[MAX_SW_PLAYERS_REG];
    short pal;

    UserInput uinfo = { FALSE, FALSE, dir_None };

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

    KB_ClearKeysDown();

    if (gs.MusicOn)
        {
        PlaySong(voc[DIGI_ENDLEV].name, 3, TRUE, TRUE);
        }

    while (TRUE)
        {
        handleevents();
        flushpackets();
        if (quitevent) break;

        CONTROL_GetUserInput(&uinfo);
        CONTROL_ClearUserInput(&uinfo);
        if (KEY_PRESSED(KEYSC_SPACE) || KEY_PRESSED(KEYSC_ENTER) || uinfo.button0 || uinfo.button1)
            {
            break;
            }

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

    wm_setwindowtitle(GameEditionName);

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
    wm_msgbox(NULL, "%s", ds);
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
        wm_msgbox(NULL, "%s", msg);
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
    (void)str;
    (void)format;
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
         TRAVERSE_CONNECT(pnum)
            {
            if (movefifoplc == Player[pnum].movefifoend)
                break;
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
    int track;
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

    if (Level == 0)
        {
        track = RedBookSong[4+RANDOM_RANGE(10)];
        }
    else
        {
        track = RedBookSong[Level];
        }
    PlaySong(LevelSong, track, TRUE, TRUE);


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
    InitRunLevel();

    FX_SetVolume(gs.SoundVolume);
    SetSongVolume(gs.MusicVolume);

    //waitforeverybody();
    ready2send = 1;

    while (TRUE)
        {
        handleevents();
        OSD_DispatchQueued();

        if (quitevent)
            {
            if (CommPlayers >= 2)
                MultiPlayQuitFlag = TRUE;
            else
                QuitFlag = TRUE;
            }

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

VOID AlphaMessage(VOID)
    {
    if (SW_SHAREWARE)
        buildputs("SHADOW WARRIOR(tm) (Shareware Version)\n");
    else
    if (GameVariant == GRPFILE_GAME_WD)
        buildputs("\"Wanton Destruction\" developed by Sunstorm Interactive (1997)\n");
    else
    if (GameVariant == GRPFILE_GAME_TD)
        buildputs("\"Twin Dragon\" developed by Level Infinity and Wylde Productions (1998)\n");
    else
        buildputs("SHADOW WARRIOR(tm)\n");
    buildputs("Copyright (c) 1997 3D Realms Entertainment\n");
    }

typedef struct
{
char    registered;
char    *arg_fmt;
char    *arg_descr;
}CLI_ARG;

#if DEBUG
CLI_ARG cli_dbg_arg[] =
{
{0, "-coop#",               "Single Player Cooperative Mode"        },
{0, "-commbat#",            "Single Player Commbat Mode"            },
{0, "-demosyncrecord",      "Demo sync record"                      },
{0, "-demosynctest",        "Demo sync test"                        },
{0, "-cam",                 "Camera test mode"                      },
{0, "-debugactor",          "No Actors"                             },
{0, "-debuganim",           "No Anims"                              },
{0, "-debugso",             "No Sector Objects"                     },
{0, "-debugsector",         "No Sector Movement"                    },
{0, "-debugpanel",          "No Panel"                              },
{0, "-mono",                "Mono"                                  },
{0, "-allsync",             "Enable full sync testing"              },
};
#endif


CLI_ARG cli_arg[] =
{
{0, "-?",                   "This help message"                     },
#if DEBUG
{0, "-debug",               "Debug Help Options"                    },
#endif
{1, "-map [mapname]",       "Load a map"                            },
{1, "-nocd<audio>",         "No CD Red Book Audio"                  },
{0, "-name [playername]",   "Player Name"                           },
{0, "-s#",                  "Skill (1-4)"                           },
{0, "-f#",                  "Packet Duplication - 2, 4, 8"          },
{0, "-nopred<ict>",         "Disable Net Prediction Method"         },
{0, "-level#",              "Start at level# (Shareware: 1-4, full version 1-28)" },
{0, "-dr[filename.dmo]",    "Demo record. NOTE: Must use -level# with this option." },
{0, "-dp[filename.dmo]",    "Demo playback. NOTE: Must use -level# with this option." },
{0, "-monst<ers>",          "No Monsters"                           },
{0, "-nodemo",              "No demos on game startup"              },
{0, "-nometers",            "Don't show air or boss meter bars in game"},
{0, "-movescale [sc]",      "Adjust movement scale: 256 = 1 unit"   },
{0, "-turnscale [sc]",      "Adjust turning scale: 256 = 1 unit"    },
{0, "-extcompat",           "Controller compatibility mode (with Duke 3D)"},
{1, "-g[filename.grp]",     "Load an extra GRP or ZIP file"         },
{1, "-h[filename.def]",     "Use filename.def instead of SW.DEF"    },
{0, "-setup",               "Displays the configuration dialogue box"},
{0, "-nosetup",             "Prevents display of the configuration dialogue box"},

#if 0 //def NET_MODE_MASTER_SLAVE
{0, "-broad<cast>",         "Broadcast network method (default)"    },
{0, "-master<slave>",       "Master/Slave network method"           },
#endif
};

/*
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
*/


int DetectShareware(void)
    {
    #define DOS_SCREEN_NAME_SW  "SHADSW.BIN"
    #define DOS_SCREEN_NAME_REG "SWREG.BIN"

    int h;

    h = kopen4load(DOS_SCREEN_NAME_SW,1);
    if (h >= 0)
        {
        GameVariant = GRPFILE_GAME_SHARE;
        kclose(h);
        return 0;
        }

    h = kopen4load(DOS_SCREEN_NAME_REG,1);
    if (h >= 0)
        {
        GameVariant = GRPFILE_GAME_REG;
        kclose(h);
        return 0;
        }

    return 1;   // heavens knows what this is...
    }


void CommandLineHelp(CLI_ARG *args, int numargs)
    {
    int i;

    const char *usagefmt = "Usage: %s [options]\n";
    const char *optionsfmt = "options:  (%s)\n\n";
#ifdef _WIN32
    const char *argfmt = "%s\t%s\n";
    const char *strargv0 = "sw";
    const char *stropts = "'/' may be used instead of '-', <> text is optional";
#else
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
    const char *argfmt = " %s  \u2014  %s\n";
    #else
    const char *argfmt = " %s  --  %s\n";
    #endif
    const char *strargv0 = _buildargv[0];
    const char *stropts = "<> text is optional";
#endif

    char *str = NULL, *strapp;
    int len;

    len = snprintf(NULL, 0, usagefmt, strargv0);
    len += snprintf(NULL, 0, optionsfmt, stropts);
    for (i=0;i < numargs;i++)
        {
        if (!args[i].arg_fmt || args[i].registered > SW_REGISTERED)
            continue;
        len += snprintf(NULL, 0, argfmt, args[i].arg_fmt, args[i].arg_descr);
        }

    if (len < 0 || !(str = (char *)malloc(len + 1)))
        {
        wm_msgbox("Shadow Warrior Help", "Command line help not available");
        return;
        }

    strapp = str;
    strapp += sprintf(strapp, usagefmt, strargv0);
    strapp += sprintf(strapp, optionsfmt, stropts);
    for (i=0;i < numargs;i++)
        {
        if (!args[i].arg_fmt || args[i].registered > SW_REGISTERED)
            continue;
        strapp += sprintf(strapp, argfmt, args[i].arg_fmt, args[i].arg_descr);
        }

        wm_msgbox("Shadow Warrior Help", "%s", str);
        free(str);
    }


#if defined RENDERTYPEWIN || (defined RENDERTYPESDL && (defined __APPLE__ || defined HAVE_GTK))
# define HAVE_STARTWIN
#endif

int app_main(int argc, char const * const argv[])
    {
    int i;
    extern int MovesPerPacket;
    VOID DoSector(VOID);
    VOID gameinput(VOID);
    int cnt = 0;
    int netparam = 0, endnetparam = 0;
    int configloaded = 0;
    struct grpfile const *gamegrp = NULL;
    char grpfile[BMAX_PATH+1] = "sw.grp";

#ifndef HAVE_STARTWIN
    (void)configloaded;
#endif
#ifdef RENDERTYPEWIN
    if (win_checkinstance()) {
        if (!wm_ynbox("Shadow Warrior","Another Build game is currently running. "
                    "Do you wish to continue starting this copy?"))
            return 0;
    }
#endif

#if defined(DATADIR)
    {
        const char *datadir = DATADIR;
        if (datadir && datadir[0]) {
            addsearchpath(datadir);
        }
    }
#endif

    {
        char *supportdir = Bgetsupportdir(TRUE);
        char *appdir = Bgetappdir();
        char dirpath[BMAX_PATH+1];

        // the OSX app bundle, or on Windows the directory where the EXE was launched
        if (appdir) {
            addsearchpath(appdir);
            free(appdir);
        }

        // the global support files directory
        if (supportdir) {
            Bsnprintf(dirpath, sizeof(dirpath), "%s/JFShadowWarrior", supportdir);
            addsearchpath(dirpath);
            free(supportdir);
        }
    }

    // default behaviour is to write to the user profile directory, but
    // creating a 'user_profiles_disabled' file in the current working
    // directory where the game was launched makes the installation
    // "portable" by writing into the working directory
    if (access("user_profiles_disabled", F_OK) == 0) {
        char cwd[BMAX_PATH+1];
        if (getcwd(cwd, sizeof(cwd))) {
            addsearchpath(cwd);
        }
    } else {
        char *supportdir;
        char dirpath[BMAX_PATH+1];
        int asperr;

        if ((supportdir = Bgetsupportdir(FALSE))) {
#if defined(_WIN32) || defined(__APPLE__)
            const char *confdir = "JFShadowWarrior";
#else
            const char *confdir = ".jfsw";
#endif
            Bsnprintf(dirpath, sizeof(dirpath), "%s/%s", supportdir, confdir);
            asperr = addsearchpath(dirpath);
            if (asperr == -2) {
                if (Bmkdir(dirpath, S_IRWXU) == 0) {
                    asperr = addsearchpath(dirpath);
                } else {
                    asperr = -1;
                }
            }
            if (asperr == 0) {
                chdir(dirpath);
            }
            free(supportdir);
        }
    }

    buildsetlogfile("sw.log");

    OSD_SetFunctions(
        NULL, NULL, NULL, NULL, NULL,
        osdfunc_clearbackground, NULL, osdfunc_onshowosd
    );
    SetupOSDCommands();

    wm_setapptitle("JFShadowWarrior");
    buildprintf("\nJFShadowWarrior\n"
        "Based on Shadow Warrior by 3D Realms Entertainment.\n"
        "Additional improvements by Jonathon Fowler (http://www.jonof.id.au) and other contributors.\n"
        "See GPL.TXT for license terms.\n\n"
        "Version %s.\nBuilt %s %s.\n", game_version, game_date, game_time);

    for (i=1;i<argc;i++) {
        char const *arg = argv[i];

#ifdef _WIN32
        if (*arg != '-' && *arg != '/') continue;
#else
        if (*arg != '-') continue;
#endif

        arg++;
        if (!Bstrcasecmp(arg, "setup"))
            {
            CommandSetup = 1;
            }
        else
        if (!Bstrcasecmp(arg, "nosetup"))
            {
            CommandSetup = -1;
            }
        else
        if (!Bstrcasecmp(arg, "net"))
            {
            netparam = ++i;
            for (; i<argc; i++)
                if (!strcmp(argv[i], "--")) break;
            endnetparam = i;
            }
        else
        if (!Bstrcasecmp(arg, "?"))
            {
            CommandLineHelp(cli_arg, SIZ(cli_arg));
            return(0);
            }
#if DEBUG
        else
        if (!Bstrcasecmp(arg, "debug"))
            {
            CommandLineHelp(cli_dbg_arg, SIZ(cli_dbg_arg));
            return(0);
            }
#endif
    }

    if (preinitengine()) {
       wm_msgbox("Build Engine Initialisation Error",
               "There was a problem initialising the Build engine: %s", engineerrstr);
       exit(1);
    }

    configloaded = CONFIG_ReadSetup();
    if (getenv("SWGRP")) {
        strncpy(grpfile, getenv("SWGRP"), BMAX_PATH);
    }

    ScanGroups();
    gamegrp = IdentifyGroup(grpfile);

    if (netparam) { // -net parameter on command line.
        netsuccess = initmultiplayersparms(endnetparam - netparam, &argv[netparam]);
    }

#ifdef HAVE_STARTWIN
    {
        struct startwin_settings settings;

        memset(&settings, 0, sizeof(settings));
        settings.fullscreen = ScreenMode;
        settings.xdim3d = ScreenWidth;
        settings.ydim3d = ScreenHeight;
        settings.bpp3d = ScreenBPP;
        settings.forcesetup = ForceSetup;
        settings.usemouse = UseMouse;
        settings.usejoy = UseJoystick;
        settings.samplerate = MixRate;
        settings.bitspersample = NumBits;
        settings.channels = NumChannels;
        settings.selectedgrp = gamegrp;
        settings.netoverride = netparam > 0;

        if (configloaded < 0 || (ForceSetup && CommandSetup == 0) || (CommandSetup > 0)) {
            if (startwin_run(&settings) == STARTWIN_CANCEL) {
                uninitengine();
                exit(0);
            }
        }

        ScreenMode = settings.fullscreen;
        ScreenWidth = settings.xdim3d;
        ScreenHeight = settings.ydim3d;
        ScreenBPP = settings.bpp3d;
        ForceSetup = settings.forcesetup;
        UseMouse = settings.usemouse;
        UseJoystick = settings.usejoy;
        MixRate = settings.samplerate;
        NumBits = settings.bitspersample;
        NumChannels = settings.channels;
        gamegrp = settings.selectedgrp;

        if (!netparam) {
            char modeparm[8];
            const char *parmarr[3] = { modeparm, NULL, NULL };
            int parmc = 0;

            if (settings.joinhost) {
                strcpy(modeparm, "-nm");
                parmarr[1] = settings.joinhost;
                parmc = 2;
            } else if (settings.numplayers > 1 && settings.numplayers <= MAXPLAYERS) {
                sprintf(modeparm, "-nm:%d", settings.numplayers);
                parmc = 1;
            }

            if (parmc > 0) {
                netsuccess = initmultiplayersparms(parmc, parmarr);
            }

            if (settings.joinhost) {
                free(settings.joinhost);
            }
        }
    }
#endif

    if (gamegrp)
        {
        if (gamegrp->game & GRPFILE_ADDON)
            {
            struct grpfile const *fg = NULL;
            for (fg = GroupsFound(); fg; fg = fg->next)
                if (fg->game == GRPFILE_GAME_REG)
                    {
                    buildprintf("GRP file: %s\n", fg->name);
                    initgroupfile(fg->name);
                    break;
                    }
            if (!fg)
                {
                wm_msgbox(NULL, "Registered game data was not found and is required for addons.");
                swexit(0);
                }
            }

        Bstrcpy(grpfile, gamegrp->name);
        GameEditionName = gamegrp->ref->name;  // Points to static data, so won't be lost in FreeGroups().
        GameVariant = gamegrp->game & ~GRPFILE_ADDON;
        }

    buildprintf("GRP file: %s\n", grpfile);
    initgroupfile(grpfile);
    if (!gamegrp && !DetectShareware()) {
        if (SW_SHAREWARE) {
            buildputs("Detected shareware GRP\n");
        } else {
            buildputs("Detected registered GRP\n");
        }
    }

    buildputs("\n");
    if (GameVariant == GRPFILE_GAME_WD)
        {
        memcpy(LevelInfo, LevelInfoWanton, sizeof(LevelInfo));
        memcpy(EpisodeNames, EpisodeNamesWanton, sizeof(EpisodeNames));
        memcpy(EpisodeSubtitles, EpisodeSubtitlesWanton, sizeof(EpisodeSubtitles));
        }
    else
    if (SW_SHAREWARE)
        GameVersion++;
    AlphaMessage();
    buildputs("\n");

    FreeGroups();

    for (i = 0; i < MAX_SW_PLAYERS; i++)
        INITLIST(&Player[i].PanelSpriteList);

    DebugOperate = TRUE;

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

#ifdef _WIN32
        if (*arg != '/' && *arg != '-') continue;
#else
        if (*arg != '-') continue;
#endif

        // Store arg in command line array!
        CON_StoreArg(arg);
        arg++;

        if (Bstrcasecmp(arg, "net") == 0)
            {
            for (; i<argc; i++)
                if (!strcmp(argv[i], "--")) break;
            }
        else
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
        if (Bstrncasecmp(arg, "short",5) == 0)
            {
            ShortGameMode = TRUE;
            }
        else
        if (Bstrncasecmp(arg, "nodemo",6) == 0)
            {
            NoDemoStartup = TRUE;
            }
#if DEBUG
        else
        if (Bstrncasecmp(arg, "allsync",3) == 0)
            {
            NumSyncBytes = MAXSYNCBYTES;
            }
#endif
        else
        if (Bstrncasecmp(arg, "name",4) == 0)
            {
            if (cnt <= argc-2)
                {
                strncpy(CommPlayerName, argv[++cnt], SIZ(CommPlayerName)-1);
                CommPlayerName[SIZ(CommPlayerName)-1] = '\0';
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
        if (Bstrncasecmp(arg, "nometers", 8) == 0)
            {
            NoMeters = TRUE;
            }
        else
#if DEBUG
        if (Bstrncasecmp(arg, "commbat", 7) == 0)
            {
            if (strlen(arg) > 7)
                {
                FakeMultiNumPlayers = atoi(&arg[7]);
                gNet.MultiGameType = MULTI_GAME_COMMBAT;
                }
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
        if (Bstrncasecmp(arg, "bots", 4) == 0)
            {
            if (strlen(arg) > 4)
                {
                FakeMultiNumPlayers = atoi(&arg[4]);
                printf("Adding %d BOT(s) to the game!\n",FakeMultiNumPlayers);
                gNet.MultiGameType = MULTI_GAME_AI_BOTS;
                BotMode = TRUE;
                }
            }
        else
        if (Bstrncasecmp(arg, "ddr", 3) == 0)
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
#endif
        if (Bstrncasecmp(arg, "dr", 2) == 0)
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
        if (Bstrncasecmp(arg, "dp", 2) == 0)
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
#if DEBUG
        else
        if (Bstrncasecmp(arg, "cam",3) == 0)
            {
            CameraTestMode = TRUE;
            }
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
        if (Bstrncasecmp(arg, "map", 3) == 0)
            {
            int fil;

            strcpy(UserMapName, argv[++cnt]);
            if (strchr(UserMapName, '.') == 0)
                strcat(UserMapName, ".map");

            if ((fil = kopen4load(UserMapName,0)) == -1)
                {
                kclose(fil);
                wm_msgbox(NULL, "ERROR: Could not find user map %s!",UserMapName);
                swexit(0);
                }
            else
                kclose(fil);
            }

        else
        if (Bstrncasecmp(arg, "g", 1) == 0)
            {
            if (strlen(arg) > 1)
                {
                if (initgroupfile(arg+1) >= 0)
                    buildprintf("Added %s\n", arg+1);
                }
            }
        else
        if (Bstrncasecmp(arg, "h", 1) == 0)
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

        CommPlayers++;
        numplayers++;
        }

    }

VOID
BotPlayerInsert(PLAYERp pp)
    {
    PLAYERp npp = Player + numplayers;

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

        CommPlayers++;
        numplayers++;
        }

//    SetFragBar(pp);
    }

VOID
ManualPlayerDelete(PLAYERp cur_pp)
    {
    short i, nexti;
    USERp u;
    PLAYERp pp;

    (void)cur_pp;

    if (numplayers > 1)
        {
        CommPlayers--;
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
        NextScreenPeek();

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

    if ((kp = KeyPressedRange(&KEY_PRESSED(KEYSC_1), &KEY_PRESSED(KEYSC_9))) && CommPlayers > 1)
        {
        short save_myconnectindex;

        save_myconnectindex = myconnectindex;

        myconnectindex = (intptr_t)kp - (intptr_t)(&KEY_PRESSED(KEYSC_1));

        if (myconnectindex >= CommPlayers)
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


    if (CommPlayers <= 1)
        {
        // F2 save menu
        if (KEY_PRESSED(KEYSC_F2))
            {
            KEY_PRESSED(KEYSC_F2) = 0;
            if (!TEST(pp->Flags, PF_DEAD))
                {
                ForceMenus = TRUE;
                ControlPanelType = ct_savemenu;
                }
            }

        // F3 load menu
        if (KEY_PRESSED(KEYSC_F3))
            {
            KEY_PRESSED(KEYSC_F3) = 0;
            if (!TEST(pp->Flags, PF_DEAD))
                {
                ForceMenus = TRUE;
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
                ForceMenus = TRUE;
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
                    ForceMenus = TRUE;
                    ControlPanelType = ct_quickloadmenu;
                    }
                }
            }

        }


    // F4 sound menu
    if (KEY_PRESSED(KEYSC_F4))
        {
        KEY_PRESSED(KEYSC_F4) = 0;
        ForceMenus = TRUE;
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
        ForceMenus = TRUE;
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
                ForceMenus = TRUE;
                ControlPanelType = ct_quickloadmenu;
                }
            }
        }
    }



VOID GetMessageInput(PLAYERp pp)
    {
    int pnum = myconnectindex;
    extern signed char MNU_InputSmallString(char *, short);
    extern signed char MNU_InputString(char *, short);
    static BOOL TeamSendAll;
    #define TEAM_MENU "A - Send to ALL,  T - Send to TEAM"
    static char HoldMessageInputString[256];
    int i;
    extern BOOL IsCommand(char *str);

    if (!MessageInputMode && !ConInputMode)
        {
        if (BUTTON(gamefunc_SendMessage))
            {
            CONTROL_ClearButton(gamefunc_SendMessage);
            KB_FlushKeyboardQueue();
            MessageInputMode = TRUE;
            InputMode = TRUE;
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
                    snprintf(ds,sizeof(ds),"%s: %s",pp->PlayerName,MessageInputString);

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
    extern signed char MNU_InputSmallString(char *, short);
    extern signed char MNU_InputString(char *, short);

    (void)pp;

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
                    CON_ConMessage("%s", MessageInputString);
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
int MouseYAxisMode = -1;

VOID
getinput(SW_PACKET *loc)
    {
    int i;
    PLAYERp pp = Player + myconnectindex;
    PLAYERp newpp = Player + myconnectindex;
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
    int mouseaxis;

    extern BOOL MenuButtonAutoRun;
    extern BOOL MenuButtonAutoAim;

    if (Prediction && CommEnabled)
        {
        newpp = ppp;
        }

    // reset all syncbits
    loc->bits = 0;
    svel = vel = angvel = aimvel = 0;

    // MAKE SURE THIS WILL GET SET
    SET_LOC_KEY(loc->bits, SK_QUIT_GAME, MultiPlayQuitFlag);

    if (gs.MouseAimingType == 1) // while held
        {
        if (BUTTON(gamefunc_Mouse_Aiming))
            {
            SET(pp->Flags, PF_MOUSE_AIMING_ON);
            gs.MouseAimingOn = TRUE;
            }
        else
            {
            if (TEST(pp->Flags, PF_MOUSE_AIMING_ON))
                {
                SET_LOC_KEY(loc->bits, SK_LOOK_UP, TRUE);
                RESET(pp->Flags, PF_MOUSE_AIMING_ON);
                gs.MouseAimingOn = FALSE;
                }
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
        }

    if (TEST(pp->Flags, PF_MOUSE_AIMING_ON))
        {
        mouseaxis = analog_lookingupanddown;
        }
    else
        {
        mouseaxis = MouseAnalogAxes[1];
        }
    if (mouseaxis != MouseYAxisMode)
        {
        CONTROL_MapAnalogAxis(1, mouseaxis, controldevice_mouse);
        MouseYAxisMode = mouseaxis;
        }

    CONTROL_GetInput(&info);

    info.dz = (info.dz * move_scale)>>8;
    info.dyaw = (info.dyaw * turn_scale)>>8;

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

    SET_LOC_KEY(loc->bits, SK_SPACE_BAR, ((!!KEY_PRESSED(KEYSC_SPACE)) | BUTTON(gamefunc_Open)));

    running = BUTTON(gamefunc_Run) || TEST(pp->Flags, PF_LOCK_RUN);

    if (BUTTON(gamefunc_Strafe) && !pp->sop)
        svel = -info.dyaw;
    else
        {
        if (info.dyaw > 0)
            angvel = abs((-info.dyaw));
        else
            angvel = info.dyaw;
        }

    aimvel = info.dpitch;
    aimvel = min(127, aimvel);
    aimvel = max(-128, aimvel);
    if (gs.MouseInvert)
        aimvel = -aimvel;

    svel -= info.dx;
    vel = -info.dz;

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
            NextScreenPeek();

            if (dimensionmode != 2 && screenpeek == myconnectindex)
                {
            // JBF: figure out what's going on here
                COVERsetbrightness(gs.Brightness,&palette_data[0][0]);
                memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
                DoPlayerDivePalette(pp);  // Check Dive again
                DoPlayerNightVisionPalette(pp);  // Check Night Vision again
                }
            else
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
    unsigned char col;
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
        snprintf(ds, sizeof(ds), "%s",UserMapName);
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
            TRAVERSE_CONNECT(p)
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

void
NextScreenPeek(void)
    {
    int startpeek = screenpeek;
    do
        {
        screenpeek = connectpoint2[screenpeek];
        if (screenpeek < 0)
            screenpeek = connecthead;
        if (!Player[screenpeek].IsDisconnected)
            {
            CON_Message("Viewing %s", Player[screenpeek].PlayerName);
            break;
            }
        }
    while (screenpeek != startpeek);
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
