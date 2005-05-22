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

#if 0
// CD.C  Jim Norwood

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <i86.h>
#include <dos.h>
#include <conio.h>
#include "mytypes.h"
#include "build.h"
#include "proto.h"
#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"
#include "tags.h"
#include "player.h"
#include "lists.h"
#include "warp.h"
#include "quake.h"

#include "mathutil.h"
#include "function.h"
#include "control.h"
#include "trigger.h"

#include "savedef.h"
#include "def.h"
#include "menus.h"
#include "net.h"
#include "pal.h"


#define ADDRESS_MODE_HSG        0
#define ADDRESS_MODE_RED_BOOK   1

#define STATUS_ERROR_BIT        0x8000
#define STATUS_BUSY_BIT         0x0200
#define STATUS_DONE_BIT         0x0100
#define STATUS_ERROR_MASK       0x00ff

#define ERROR_WRITE_PROTECT     0
#define ERROR_UNKNOWN_UNIT      1
#define ERROR_DRIVE_NOT_READY   2
#define ERROR_UNKNOWN_COMMAND   3
#define ERROR_CRC_ERROR         4
#define ERROR_BAD_REQUEST_LEN   5
#define ERROR_SEEK_ERROR        6
#define ERROR_UNKNOWN_MEDIA     7
#define ERROR_SECTOR_NOT_FOUND  8
#define ERROR_OUT_OF_PAPER      9
#define ERROR_WRITE_FAULT       10
#define ERROR_READ_FAULT        11
#define ERROR_GENERAL_FAILURE   12
#define ERROR_RESERVED_13       13
#define ERROR_RESERVED_14       14
#define ERROR_BAD_DISK_CHANGE   15

#define COMMAND_READ            3
#define COMMAND_WRITE           12
#define COMMAND_PLAY_AUDIO      132
#define COMMAND_STOP_AUDIO      133
#define COMMAND_RESUME_AUDIO    136

#define READ_REQUEST_AUDIO_CHANNEL_INFO         4
#define READ_REQUEST_DEVICE_STATUS              6
#define READ_REQUEST_MEDIA_CHANGE               9
#define READ_REQUEST_AUDIO_DISK_INFO            10
#define READ_REQUEST_AUDIO_TRACK_INFO           11
#define READ_REQUEST_AUDIO_STATUS               15

#define WRITE_REQUEST_EJECT                     0
#define WRITE_REQUEST_RESET                     2
#define WRITE_REQUEST_AUDIO_CHANNEL_INFO        3

#define STATUS_DOOR_OPEN                        0x00000001
#define STATUS_DOOR_UNLOCKED                    0x00000002
#define STATUS_RAW_SUPPORT                      0x00000004
#define STATUS_READ_WRITE                       0x00000008
#define STATUS_AUDIO_SUPPORT                    0x00000010
#define STATUS_INTERLEAVE_SUPPORT               0x00000020
#define STATUS_BIT_6_RESERVED                   0x00000040
#define STATUS_PREFETCH_SUPPORT                 0x00000080
#define STATUS_AUDIO_MANIPLUATION_SUPPORT       0x00000100
#define STATUS_RED_BOOK_ADDRESS_SUPPORT         0x00000200

#define MEDIA_NOT_CHANGED       1
#define MEDIA_STATUS_UNKNOWN    0
#define MEDIA_CHANGED          -1

#define AUDIO_CONTROL_MASK              0xd0
#define AUDIO_CONTROL_DATA_TRACK        0x40
#define AUDIO_CONTROL_AUDIO_2_TRACK     0x00
#define AUDIO_CONTROL_AUDIO_2P_TRACK    0x10
#define AUDIO_CONTROL_AUDIO_4_TRACK     0x80
#define AUDIO_CONTROL_AUDIO_4P_TRACK    0x90

#define AUDIO_STATUS_PAUSED             0x0001

#pragma pack(1)

struct playAudioRequest
    {
    char addressingMode;
    int startLocation;
    int sectors;
    };

struct readRequest
    {
    char mediaDescriptor;
    short bufferOffset;
    short bufferSegment;
    short length;
    short startSector;
    int volumeID;
    };

struct writeRequest
    {
    char mediaDescriptor;
    short bufferOffset;
    short bufferSegment;
    short length;
    short startSector;
    int volumeID;
    };

struct cd_request
    {
    char headerLength;
    char unit;
    char command;
    short status;
    char reserved[8];
    union
        {
        struct playAudioRequest playAudio;
        struct readRequest read;
        struct writeRequest write;
        } x;
    };


struct audioChannelInfo_s
    {
    char code;
    char channel0input;
    char channel0volume;
    char channel1input;
    char channel1volume;
    char channel2input;
    char channel2volume;
    char channel3input;
    char channel3volume;
    };

struct deviceStatus_s
    {
    char code;
    int status;
    };

struct mediaChange_s
    {
    char code;
    char status;
    };

struct audioDiskInfo_s
    {
    char code;
    char lowTrack;
    char highTrack;
    int leadOutStart;
    };

struct audioTrackInfo_s
    {
    char code;
    char track;
    int start;
    char control;
    };

struct audioStatus_s
    {
    char code;
    short status;
    int PRstartLocation;
    int PRendLocation;
    };

struct reset_s
    {
    char code;
    };

union readInfo_u
    {
    struct audioChannelInfo_s audioChannelInfo;
    struct deviceStatus_s deviceStatus;
    struct mediaChange_s mediaChange;
    struct audioDiskInfo_s audioDiskInfo;
    struct audioTrackInfo_s audioTrackInfo;
    struct audioStatus_s audioStatus;
    struct reset_s reset;
    };

#pragma pack()

#define MAXIMUM_TRACKS                  32

typedef struct
    {
    int start;
    int length;
    BOOL isData;
    } track_info;

typedef struct
    {
    BOOL valid;
    int leadOutAddress;
    track_info track[MAXIMUM_TRACKS];
    BYTE lowTrack;
    BYTE highTrack;
    } cd_info;

static struct cd_request far *cdRequest;
static union  readInfo_u far *readInfo;
static cd_info cd;

static BOOL playing = FALSE;
static BOOL wasPlaying = FALSE;
static BOOL mediaCheck = FALSE;
BOOL initialized = FALSE;
BOOL enabled = TRUE;
BOOL cdvalid = FALSE;
static BOOL playLooping = FALSE;
static short cdRequestSegment;
static short cdRequestOffset;
static short readInfoSegment;
static short readInfoOffset;
static BYTE remap[256];
static BYTE cdrom;
BYTE playTrack;
static BYTE cdvolume;

// My Stuff ////////////////////////////////////////////////////////////////
// Never play track 1, that's the game.
BYTE RedBookSong[40] =
    {
    2,4,9,12,10, // Title and ShareWare levels
    5,6,8,11,12,5,10,4,6,9,7,10,8,7,9,10,11,5, // Registered levels
    11,8,7,13,5,6,  // Deathmatch levels    
    13 // Fight boss
    };

union  REGS  regs;
struct SREGS sregs;

char far *memory;   // My real mode memory buffer used to hold the CD requestor structure.

// Always make sure regs and sregs contain valid values unless you enjoy segment violations
#define dos_int86(intrpt) (int386x(intrpt, &regs, &regs, &sregs))

// Allocates real mode memory in the first megabyte of memory

static struct rminfo {     
    long EDI;     
    long ESI;     
    long EBP;     
    long reserved_by_system;     
    long EBX;     
    long EDX;     
    long ECX;     
    long EAX;     
    short flags;     
    short ES,DS,FS,GS,IP,CS,SP,SS;
     
} RMI;

short selector;     
short segment; 
int interrupt_no=0x31; // Interrupt 0x31 service 0x0300 is used for passing interrupts to real mode

//
// Allocate Real Mode memory under 1 Meg from DPMI
//
void dos_getmemory(short size)
{
//  int interrupt_no=0x31;         

    /* DPMI call 100h allocates DOS memory */     
    memset(&sregs,0,sizeof(sregs));     
    regs.w.ax=0x0100;     
    regs.w.bx=(size>>4)+1;     // Number of 16 byte blocks being requested
    int386x( interrupt_no, &regs, &regs, &sregs);     

    segment=regs.w.ax;  
    
    ASSERT(segment != 0x07 && segment != 0x08);
    
    if(segment == 0x07 || segment == 0x08) return;   

    selector=regs.w.dx;     
        
}

// Prototypes
VOID PutStringInfo(PLAYERp pp, char *string);

///////////////////////////////////////////////////////////////////////////

static int 
RedBookToSector(int rb)
    {
    BYTE minute;
    BYTE second;
    BYTE frame;

    minute = (rb >> 16) & 0xff;
    second = (rb >> 8) & 0xff;
    frame = rb & 0xff;
    return minute * 60 * 75 + second * 75 + frame;
    }


static void 
CDAudio_Reset(void)
    {
    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_WRITE;
    cdRequest->status = 0;

    cdRequest->x.write.mediaDescriptor = 0;
    cdRequest->x.write.bufferOffset = readInfoOffset;
    cdRequest->x.write.bufferSegment = readInfoSegment;
    cdRequest->x.write.length = sizeof(struct reset_s);
    cdRequest->x.write.startSector = 0;
    cdRequest->x.write.volumeID = 0;

    readInfo->reset.code = WRITE_REQUEST_RESET;


    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );
    }


void 
CDAudio_Eject(void)
    {
    if (playing)
        CDAudio_Stop();

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_WRITE;
    cdRequest->status = 0;

    cdRequest->x.write.mediaDescriptor = 0;
    cdRequest->x.write.bufferOffset = readInfoOffset;
    cdRequest->x.write.bufferSegment = readInfoSegment;
    cdRequest->x.write.length = sizeof(struct reset_s);
    cdRequest->x.write.startSector = 0;
    cdRequest->x.write.volumeID = 0;

    readInfo->reset.code = WRITE_REQUEST_EJECT;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x1510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );
    }


static int 
CDAudio_GetAudioTrackInfo(BYTE track, int *start)
    {
    BYTE control;

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_READ;
    cdRequest->status = 0;

    cdRequest->x.read.mediaDescriptor = 0;
    cdRequest->x.read.bufferOffset = readInfoOffset;
    cdRequest->x.read.bufferSegment = readInfoSegment;
    cdRequest->x.read.length = sizeof(struct audioTrackInfo_s);
    cdRequest->x.read.startSector = 0;
    cdRequest->x.read.volumeID = 0;

    readInfo->audioTrackInfo.code = READ_REQUEST_AUDIO_TRACK_INFO;
    readInfo->audioTrackInfo.track = track;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    if (cdRequest->status & STATUS_ERROR_BIT)
        {
        sprintf(ds,"CDAudio_GetAudioTrackInfo %04x\n", cdRequest->status & 0xffff);
        return -1;
        }

    *start = readInfo->audioTrackInfo.start;
    control = readInfo->audioTrackInfo.control & AUDIO_CONTROL_MASK;
    return (control & AUDIO_CONTROL_DATA_TRACK);
    }


static int 
CDAudio_GetAudioDiskInfo(void)
    {
    int n;

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_READ;
    cdRequest->status = 0;

    cdRequest->x.read.mediaDescriptor = 0;
    cdRequest->x.read.bufferOffset = readInfoOffset;
    cdRequest->x.read.bufferSegment = readInfoSegment;
    cdRequest->x.read.length = sizeof(struct audioDiskInfo_s);
    cdRequest->x.read.startSector = 0;
    cdRequest->x.read.volumeID = 0;

    readInfo->audioDiskInfo.code = READ_REQUEST_AUDIO_DISK_INFO;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x1510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );


    if (cdRequest->status & STATUS_ERROR_BIT)
        {
        //sprintf(ds,"CDAudio_GetAudioDiskInfo %04x\n", cdRequest->status & 0xffff);
        return -1;
        }

    cd.valid = TRUE;
    cdvalid = TRUE;
    cd.lowTrack = readInfo->audioDiskInfo.lowTrack;
    cd.highTrack = readInfo->audioDiskInfo.highTrack;
    cd.leadOutAddress = readInfo->audioDiskInfo.leadOutStart;

    for (n = cd.lowTrack; n <= cd.highTrack && n < MAXIMUM_TRACKS; n++)
        {
        ASSERT(n >= 0 && n < MAXIMUM_TRACKS);
        
        cd.track[n].isData = CDAudio_GetAudioTrackInfo(n, &cd.track[n].start);
        if (n > cd.lowTrack)
            {
            cd.track[n - 1].length = RedBookToSector(cd.track[n].start) - RedBookToSector(cd.track[n - 1].start);
            if (n == cd.highTrack)
                cd.track[n].length = RedBookToSector(cd.leadOutAddress) - RedBookToSector(cd.track[n].start);
            }
        }


    return 0;
    }


static int 
CDAudio_GetAudioStatus(void)
    {
    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_READ;
    cdRequest->status = 0;

    cdRequest->x.read.mediaDescriptor = 0;
    cdRequest->x.read.bufferOffset = readInfoOffset;
    cdRequest->x.read.bufferSegment = readInfoSegment;
    cdRequest->x.read.length = sizeof(struct audioStatus_s);
    cdRequest->x.read.startSector = 0;
    cdRequest->x.read.volumeID = 0;

    readInfo->audioDiskInfo.code = READ_REQUEST_AUDIO_STATUS;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    if (cdRequest->status & STATUS_ERROR_BIT)
        return -1;
    return 0;
    }


static int 
CDAudio_MediaChange(void)
    {
    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_READ;
    cdRequest->status = 0;

    cdRequest->x.read.mediaDescriptor = 0;
    cdRequest->x.read.bufferOffset = readInfoOffset;
    cdRequest->x.read.bufferSegment = readInfoSegment;
    cdRequest->x.read.length = sizeof(struct mediaChange_s);
    cdRequest->x.read.startSector = 0;
    cdRequest->x.read.volumeID = 0;

    readInfo->mediaChange.code = READ_REQUEST_MEDIA_CHANGE;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    return readInfo->mediaChange.status;
    }


BYTE 
CDAudio_GetVolume(void)
    {
    return cdvolume;
    }


// we set the volume to 0 first and then to the desired volume
// some cd-rom drivers seem to need it done this way
void 
CDAudio_SetVolume(BYTE volume)
    {
    if (!initialized || !enabled)
        return;

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_WRITE;
    cdRequest->status = 0;

    cdRequest->x.read.mediaDescriptor = 0;
    cdRequest->x.read.bufferOffset = readInfoOffset;
    cdRequest->x.read.bufferSegment = readInfoSegment;
    cdRequest->x.read.length = sizeof(struct audioChannelInfo_s);
    cdRequest->x.read.startSector = 0;
    cdRequest->x.read.volumeID = 0;

    readInfo->audioChannelInfo.code = WRITE_REQUEST_AUDIO_CHANNEL_INFO;
    readInfo->audioChannelInfo.channel0input = 0;
    readInfo->audioChannelInfo.channel0volume = 0;
    readInfo->audioChannelInfo.channel1input = 1;
    readInfo->audioChannelInfo.channel1volume = 0;
    readInfo->audioChannelInfo.channel2input = 2;
    readInfo->audioChannelInfo.channel2volume = 0;
    readInfo->audioChannelInfo.channel3input = 3;
    readInfo->audioChannelInfo.channel3volume = 0;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x1510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );


    readInfo->audioChannelInfo.channel0volume = volume;
    readInfo->audioChannelInfo.channel1volume = volume;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x1510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );


    cdvolume = volume;
    }


void 
CDAudio_Play(BYTE track, BOOL looping)
    {
    BYTE volume;

    //if (!gs.PlayCD)
    //    return;
    
    if (!initialized || !enabled)
            return;

    if (!cd.valid)
        return;

    track = remap[track];

    if (playing)
        {
        if (playTrack == track)
            return;
        CDAudio_Stop();
        }

    playLooping = looping;

    if (track < cd.lowTrack || track > cd.highTrack)
        {
        sprintf(ds, "CDAudio_Play: Bad track number %u.  Defaulting to track 2.\n", track);
        CON_Message(ds);
        CDAudio_Play(2, TRUE); // Default to track 2
        return;
        }

    playTrack = track;

    if (cd.track[track].isData)
        {
        sprintf(ds, "CDAudio_Play: Can not play data.\n", track);
        CON_Message( ds);
        return;
        }

    volume = (BYTE)gs.MusicVolume;

    CDAudio_SetVolume(volume);

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_PLAY_AUDIO;
    cdRequest->status = 0;

    cdRequest->x.playAudio.addressingMode = ADDRESS_MODE_RED_BOOK;
    cdRequest->x.playAudio.startLocation = cd.track[track].start;
    cdRequest->x.playAudio.sectors = cd.track[track].length;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    if (cdRequest->status & STATUS_ERROR_BIT)
        {
        sprintf(ds, "CDAudio_Play: track %u failed\n", track);
        CON_Message( ds);
        cd.valid = FALSE;
        cdvalid = FALSE;
        playing = FALSE;
        return;
        }

    playing = TRUE;
    }


void 
CDAudio_Stop(void)
    {
    if (!initialized || !enabled)
        return;

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_STOP_AUDIO;
    cdRequest->status = 0;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    wasPlaying = playing;
    playing = FALSE;
    }


void 
CDAudio_Resume(void)
    {
    if (!initialized || !enabled)
        return;

    if (!cd.valid)
        return;

    if (!wasPlaying)
        return;

    cdRequest->headerLength = 13;
    cdRequest->unit = 0;
    cdRequest->command = COMMAND_RESUME_AUDIO;
    cdRequest->status = 0;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001510; 
    RMI.ECX=cdrom;
    RMI.ES =segment;     
    RMI.EBX=cdRequestOffset;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    playing = TRUE;
    }


static void 
CD_f(void)
    {
    int ret;
    int n;
    int startAddress;
    char base[80],command[80];
    SHORT op2=0,op3=0;

    // Looking for two operators for cd stuff, "cd" and then one of the below commands
    if (sscanf(MessageInputString,"%s %s %d %d",base,command,&op2,&op3) < 2)
        return;

    strlwr(command);    // Make sure operator is all lower case

    if (!strcmp(command, "on"))
        {
        enabled = TRUE;
        CDAudio_Init();

        sprintf(ds, "CD AUDIO is now ON" );
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "off"))
        {
        if (playing)
            CDAudio_Stop();
        enabled = FALSE;

        sprintf(ds, "CD AUDIO is now OFF");
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "reset"))
        {
        enabled = TRUE;
        if (playing)
            CDAudio_Stop();
        for (n = 0; n < 256; n++)
            remap[n] = n;
        CDAudio_Reset();
        CDAudio_GetAudioDiskInfo();

        sprintf(ds, "CD AUDIO has been reset");
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "remap"))
        {
        if (op2 <= 0)
            {
            for (n = 1; n < 256; n++)
                if (remap[n] != n)
                {
                    CON_Message("  %u -> %u\n", n, remap[n]);
                }
            return;
            } 
            else
            {
            remap[op2] = op3;
            sprintf(ds, "Remapped track %d to track %d\n",op2,op3);
            CON_Message( ds);
            }
        return;
        }

    if (!cd.valid)
        {
        sprintf(ds, "No CD in player\n");
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "play"))
        {
        CDAudio_Play(op2, FALSE);
        if(playing)
            sprintf(ds, "Playing track %d once\n",op2);
        else
            sprintf(ds, "Could not play track %d\n",op2);
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "loop"))
        {
        CDAudio_Play(op2, TRUE);
        if(playing)
            sprintf(ds, "Looping track %d\n",op2);
        else
            sprintf(ds, "Could not play track %d\n",op2);
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "stop"))
        {
        CDAudio_Stop();
        if(enabled)
            sprintf(ds, "Current audio track stopped\n");
        else
            sprintf(ds, "CD is not enabled!\n");
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "resume"))
        {
        CDAudio_Resume();
        if(enabled)
            sprintf(ds, "Current audio track resumed\n");
        else
            sprintf(ds, "CD is not enabled!\n");
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "eject"))
        {
        CDAudio_Eject();
        cd.valid = FALSE;
        cdvalid = FALSE;
        sprintf(ds, "CD ejected!\n");
        CON_Message( ds);
        return;
        }

    if (!strcmp(command, "info"))
        {
        CON_Message("%u tracks\n", cd.highTrack - cd.lowTrack + 1);
        CON_ConMessage("%u tracks\n", cd.highTrack - cd.lowTrack + 1);
//        for (n = cd.lowTrack; n <= cd.highTrack; n++)
//            {
//            ret = CDAudio_GetAudioTrackInfo(n, &startAddress);
//            printf("Track %2u: %s at %2u:%02u\n", n, ret ? "data " : "music", (startAddress >> 16) & 0xff, (startAddress >> 8) & 0xff);
//            }
        if (playing)
        {
            CON_Message("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
            CON_ConMessage("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
        } else
        {
            CON_Message("CD is not currently playing.");
            CON_ConMessage("CD is not currently playing.");
        }
        CON_Message("Volume is %u\n", cdvolume);
        CON_ConMessage("Volume is %u\n", cdvolume);
        CDAudio_MediaChange();
        CON_Message("Status %04x\n", cdRequest->status & 0xffff);
        CON_ConMessage("Status %04x\n", cdRequest->status & 0xffff);
        return;
        }
    }

char
CDAudio_CheckEnable(void)
    {
    if (!initialized)
        {
        if (!CDAudio_GetAudioDiskInfo())
            {
            enabled = TRUE;
            CDAudio_Init();
            return(TRUE);
            }
        }
    return(FALSE);
    }
    
void 
CDAudio_Update(void)
    {
    int ret;
    unsigned char newVolume;


    if (!initialized || !enabled)
        return;

        // CTW NOTE/ADDITION
        // Ok, cdRequest->status is bogus and never returns anything but 0100.
        // This makes it completely useless for detecting when the song has completed.
        // So, instead I'm just going to let them go for roughly 200 seconds and then
        // the existing logic acts like the song has finished. This gets rid of the
        // current stuttering music bug that exists even in the registered version.
        // I'm sure I could have found a solution by reading up on Redbook Audio 
        // quicker than it took me to write this comment, but fuck it.
        if ((totalclock - lastUpdate) < 20000)
        return;
        // CTW NOTE/ADDITION END

        if ((totalclock - lastUpdate) < 30)
        return;
        lastUpdate = totalclock;

    if (mediaCheck)
        {
        static long lastCheck;

                if ((totalclock - lastCheck) < 150)
            return;
                lastCheck = totalclock;

        ret = CDAudio_MediaChange();
        if (ret == MEDIA_CHANGED)
            {
            sprintf(ds,"CDAudio: media changed\n");
            if(!InGame)
                sprintf(ds,"%s");
            else
                {
                CON_Message( ds);
                }

            playing = FALSE;
            wasPlaying = FALSE;
            cd.valid = FALSE;
            cdvalid = FALSE;
            CDAudio_GetAudioDiskInfo();
            return;
            }
        }

    newVolume = (BYTE)gs.MusicVolume;
    if (newVolume != cdvolume)
        {
        CDAudio_SetVolume(newVolume);
        }

    if (playing)
        {
        CDAudio_GetAudioStatus();

        if (cdRequest->status & STATUS_ERROR_BIT)
            {
            cdvalid = FALSE;
            playing = FALSE;    
            return;
            }
            
        if ((cdRequest->status & STATUS_BUSY_BIT) == 0)
            {
            playing = FALSE;
            if (playLooping)
                {
                CDAudio_Play(playTrack, TRUE);
                }
            }
        }
    }


BOOL 
CDAudio_Playing(void)
    {
    return playing;
    }


int 
CDAudio_Init(void)
    {
    int n;

    if(!enabled) return -1;

    if (CON_CheckParm("-nocd"))
        return -1;

    if (CON_CheckParm("-cdmediacheck"))
        mediaCheck = TRUE;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x00001500; 
    RMI.EBX=0;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

      if (RMI.EBX == 0)
        {
        sprintf(ds,
            "MSCDEX not loaded, music is\n"
            "disabled.  Use \"-nocdaudio\" if you\n"
            "wish to avoid this message in the\n"
            "future.  See README.TXT for help.\n"
            );
        if(!InGame)
            sprintf(ds,"%s");
        else {
            sprintf(ds, "MSCDEX not loaded, music is disabled!\n");
            CON_Message( ds);
        }
        return -1;
        }
    if (RMI.EBX > 1)
    {
        sprintf(ds,"CDAudio_Init: First CD-ROM drive will be used\n");
        if(!InGame)
            sprintf(ds,"%s");
        else
            {
            CON_Message( ds);
            }
    }

    cdrom = RMI.ECX;

    /* Set up real-mode call structure */
    memset(&RMI,0,sizeof(RMI));
    RMI.EAX=0x0000150c; 
    RMI.EBX=0;

    //Use DPMI call 300h to issue the DOS interrupt
    regs.w.ax = 0x0300;
    regs.h.bl = 0x2f;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es   = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x( interrupt_no, &regs, &regs, &sregs );

    if (RMI.EBX == 0)    
        {
        sprintf(ds,
            "MSCDEX version 2.00 or later\n"
            "required for music. See README.TXT\n"
            "for help.\n"
            );
        if(!InGame)
            sprintf(ds,"%s");
        else {
            sprintf(ds,"MSCDEX version 2.00 or later required!\n");
            CON_Message( ds);
        }

        sprintf(ds,"CDAudio_Init: MSCDEX version 2.00 or later required.\n");
        if(!InGame)
            sprintf(ds,"%s");
        else
            {
            CON_Message( ds);
            }

        return -1;

        }else
        {
            sprintf(ds,"CDAudio_Init: MSCDEX version %d.%d is loaded.\n",
                (unsigned int)(RMI.EBX>>8),(unsigned int)(RMI.EBX & 0xff));
            if(!InGame)
                sprintf(ds,"%s");
            else
                {
                CON_Message( ds);
                }
        }


    dos_getmemory(sizeof(struct cd_request) + sizeof(union readInfo_u));
    memory = MK_FP(selector,0);

    if (memory == NULL)
        {
        sprintf(ds,"CDAudio_Init: Unable to allocate low memory.\n");
        if(!InGame)
            sprintf(ds,"%s");
        else
            {
            CON_Message( ds);
            }
        return -1;
        }

    cdRequest = (struct cd_request far *)memory;
    cdRequestSegment = segment;
    cdRequestOffset = FP_OFF(memory);

    readInfo = (union readInfo_u far *)(memory + sizeof(struct cd_request));
    readInfoSegment = segment;
    readInfoOffset = FP_OFF(memory + sizeof(struct cd_request));

    for (n = 0; n < 256; n++)
        remap[n] = n;
    initialized = TRUE;

    CDAudio_SetVolume(gs.MusicVolume);
    if (CDAudio_GetAudioDiskInfo())
        {
        sprintf(ds, "CDAudio_Init: No CD in player.\n");
        if(!InGame)
            sprintf(ds,"%s");
        else
            {
            CON_Message( ds);
            }

        enabled = FALSE;
        cdvalid = FALSE;
        }

    CON_AddCommand("cd", CD_f);
    if(enabled)
        sprintf(ds, "CDAudio_Init: CD Audio Initialized\n");
    else
        sprintf(ds, "CDAudio_Init: CD Audio initializion failed!\n");

    if(!InGame)
        sprintf(ds,"%s");
    else
        {
        CON_Message( ds);
        }

    return 0;
    }


void 
CDAudio_Shutdown(void)
    {
    if (!initialized)
        return;
    CDAudio_Stop();
    }

#else
#include "mytypes.h"
#include "compat.h"

#ifdef RENDERTYPEWIN
#include "cda.h"
static int loopmode=0, playmode = CDA_Stopped;
#endif
extern long totalclock;

BOOL enabled = TRUE;
BOOL cdvalid = FALSE;
BYTE playTrack;
long lastUpdate=0;
// Never play track 1, that's the game.
BYTE RedBookSong[40] =
    {
    2,4,9,12,10, // Title and ShareWare levels
    5,6,8,11,12,5,10,4,6,9,7,10,8,7,9,10,11,5, // Registered levels
    11,8,7,13,5,6,  // Deathmatch levels    
    13 // Fight boss
    };
void CDAudio_Eject(void)
{
}
BYTE CDAudio_GetVolume(void )
{
	return 0;
}
void CDAudio_SetVolume(BYTE volume)
{
}
void CDAudio_Play(BYTE track,BOOL looping)
{
#ifdef RENDERTYPEWIN
	if (cda_numtracks <= 0) return;
	
	track--;	// my CDA code is based at 0
	if ((unsigned)track >= (unsigned)cda_numtracks) return;

	loopmode = looping;

	if (cda_play(cda_trackinfo[track].start, cda_trackinfo[track].start + cda_trackinfo[track].length - 1)) return;

	playmode = CDA_Playing;
	playTrack = track;
#endif
}
void CDAudio_Stop(void )
{
#ifdef RENDERTYPEWIN
	cda_pause(1);
	playmode = CDA_Paused;
#endif
}
void CDAudio_Resume(void )
{
#ifdef RENDERTYPEWIN
	cda_pause(0);
	playmode = CDA_Playing;
#endif
}
void CDAudio_Update(void )
{
	if (totalclock-lastUpdate < 30) return;
	lastUpdate = totalclock;

#ifdef RENDERTYPEWIN
	if (playmode == CDA_Playing) {
		switch (cda_getstatus()) {
			case CDA_Stopped:
				cda_play(cda_trackinfo[playTrack].start,
					cda_trackinfo[playTrack].start + cda_trackinfo[playTrack].length - 1);
				break;
			case CDA_NotReady:
				playmode = CDA_Stopped;
				break;
			default: break;
		}
	}
#endif
}
BOOL CDAudio_Playing(void )
{
#ifdef RENDERTYPEWIN
	if (cda_getstatus() == CDA_Playing) return 1;
#endif
	return 0;
}
int CDAudio_Init(void )
{
#ifdef RENDERTYPEWIN
	if (cda_opendevice(-1)) return -1;

	if (cda_getstatus() != CDA_NotReady)
		cda_querydisc();

	return 0;
#else
	return -1;
#endif
}
void CDAudio_Shutdown(void )
{
#ifdef RENDERTYPEWIN
	cda_stop();
	cda_closedevice();
#endif
}
#endif



