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
#include "baselayer.h"
#include "mmulti.h"

#include "keys.h"
#include "game.h"
#include "tags.h"
#include "names2.h"
#include "net.h"
#include "pal.h"
#include "demo.h"

#include "weapon.h"
#include "text.h"
#include "menus.h"


/*
SYNC BUG NOTES:

    1.  Look at Prediction code first when player movement is involved.  If the
    prediction code changes any variable not in the Player or Players
    Sprite/User structure that effects movement then there will be a Out Of Sync
    problem.

    EXAMPLE:  Prediction player was updating a sop->drive_oangvel making it
    invalid for movement.  Look at DoPlayerBoatTurn for comment.

    2.  Changing movement variables in the draw code.  Because the draw code is
    called at a variable rate this don't work.  This includes using RANDOM_RANGE
    or RANDOM_P2 in the draw code.  This updates the random seed variable and is
    only for movement code.  Use STD_RANDOM_RANGE for draw code.

    3.  Plain old bugs such as using uninitialized local variables.

*/

//#undef MAXSYNCBYTES
//#define MAXSYNCBYTES 16
BYTE tempbuf[576], packbuf[576];
int PlayClock;
extern BOOL PauseKeySet;
extern char CommPlayerName[32];

gNET gNet;
extern short PlayerQuitMenuLevel;

#define TIMERUPDATESIZ 32

//SW_PACKET fsync;

//Local multiplayer variables
// should move this to a local scope of faketimerhandler - do it when able to test
SW_PACKET loc;

//SW_PACKET oloc;

BOOL ready2send = 0;

BOOL CommEnabled = FALSE;
BYTE CommPlayers = 0;
int movefifoplc, movefifosendplc;//, movefifoend[MAX_SW_PLAYERS];
unsigned int MoveThingsCount;

//int myminlag[MAX_SW_PLAYERS];
int mymaxlag, otherminlag, bufferjitter = 1;
extern CHAR sync_first[MAXSYNCBYTES][60];
extern int sync_found;

//
// Tic Duplication - so you can move multiple times per packet
//
typedef struct
    {
    LONG vel;
    LONG svel;
    LONG angvel;
    LONG aimvel;
    LONG bits;
    }SW_AVERAGE_PACKET;

int MovesPerPacket = 1;
SW_AVERAGE_PACKET AveragePacket;

 // GAME.C sync state variables
BYTE syncstat[MAXSYNCBYTES];
//int syncvalhead[MAX_SW_PLAYERS];
int syncvaltail, syncvaltottail;
void GetSyncInfoFromPacket(BYTEp packbuf, int packbufleng, int *j, int otherconnectindex);

// when you set totalclock to 0 also set this one
int ototalclock;
int smoothratio;
int save_totalclock;

// must start out as 0

BOOL GamePaused = FALSE;
BOOL NetBroadcastMode = TRUE;
BOOL NetModeOverride = FALSE;


void netsendpacket(int ind, BYTEp buf, int len)
{
	BYTE bbuf[ sizeof(packbuf) + sizeof(PACKET_PROXY) ];
	PACKET_PROXYp prx = (PACKET_PROXYp)bbuf;

	// send via master if in M/S mode and we are not the master, and the recipient is not the master and not ourselves
	if (!NetBroadcastMode && myconnectindex != connecthead && ind != myconnectindex && ind != connecthead) {
		if ((unsigned)len > sizeof(packbuf)) {
			debugprintf("netsendpacket(): packet length > %d!\n",(int)sizeof(packbuf));
			len = sizeof(packbuf);
		}

		prx->PacketType = PACKET_TYPE_PROXY;
		prx->PlayerIndex = (BYTE)ind;
		memcpy(&prx[1], buf, len);	// &prx[1] == (char*)prx + sizeof(PACKET_PROXY)
		len += sizeof(PACKET_PROXY);

		sendpacket(connecthead, bbuf, len);
		return;
	}

	sendpacket(ind, buf, len);
}

void netbroadcastpacket(BYTEp buf, int len)
{
	int i;
	BYTE bbuf[ sizeof(packbuf) + sizeof(PACKET_PROXY) ];
	PACKET_PROXYp prx = (PACKET_PROXYp)bbuf;

	// broadcast via master if in M/S mode and we are not the master
	if (!NetBroadcastMode && myconnectindex != connecthead) {
		if ((unsigned)len > sizeof(packbuf)) {
			debugprintf("netbroadcastpacket(): packet length > %d!\n",(int)sizeof(packbuf));
			len = sizeof(packbuf);
		}

		prx->PacketType = PACKET_TYPE_PROXY;
		prx->PlayerIndex = (BYTE)(-1);
		memcpy(&prx[1], buf, len);
		len += sizeof(PACKET_PROXY);

		sendpacket(connecthead, bbuf, len);
		return;
	}

	TRAVERSE_CONNECT(i)
	{
		if (i == myconnectindex) continue;
		sendpacket(i, buf, len);
	}
}

int netgetpacket(int *ind, BYTEp buf)
{
	int i;
	int len;
	PACKET_PROXYp prx;

	len = getpacket(ind, buf);
	if ((unsigned)len < sizeof(PACKET_PROXY) || buf[0] != PACKET_TYPE_PROXY) {
		return len;
	}

	prx = (PACKET_PROXYp)buf;

	if (myconnectindex == connecthead) {
		// I am the master

		if (prx->PlayerIndex == (BYTE)(-1)) {
			// broadcast

			// Rewrite the player index to be the sender's connection number
			prx->PlayerIndex = (BYTE)*ind;

			// Transmit to all the other players except ourselves and the sender
			TRAVERSE_CONNECT(i) {
				if (i == myconnectindex || i == *ind) continue;
				sendpacket(i, buf, len);
			}

			// Return the packet payload to the caller
			len -= sizeof(PACKET_PROXY);
			memmove(buf, &prx[1], len);
			return len;
		} else {
			// proxy send to a specific player

			i = prx->PlayerIndex;

			// Rewrite the player index to be the sender's connection number
			prx->PlayerIndex = (BYTE)*ind;

			// Transmit to the intended recipient
			if (i == myconnectindex) {
				len -= sizeof(PACKET_PROXY);
				memmove(buf, &prx[1], len);
				return len;
			}

            if (Player[i].IsDisconnected) return 0;
			sendpacket(i, buf, len);
			return 0;	// nothing for us to do
		}
	} else if (*ind == connecthead) {
		// I am a slave, and the proxy message came from the master
		*ind = prx->PlayerIndex;
		len -= sizeof(PACKET_PROXY);
		memmove(buf, &prx[1], len);
		return len;
	} else {
		debugprintf("netgetpacket(): proxy from peer %d\n",*ind);
	}
	return 0;
}


int EncodeBits(SW_PACKET *pak, SW_PACKET *old_pak, BYTEp buf)
    {
    BYTEp base_ptr = buf;
    unsigned i;

    // skipping the bits field sync test fake byte (Ed. Ken)
    *buf = 0;
    buf++;

    if (pak->vel != old_pak->vel)
        {
        *((short*)buf) = pak->vel;
        buf += sizeof(pak->vel);
        SET(*base_ptr, BIT(0));
        }

    if (pak->svel != old_pak->svel)
        {
        *((short*)buf) = pak->svel;
        buf += sizeof(pak->svel);
        SET(*base_ptr, BIT(1));
        }

    if (pak->angvel != old_pak->angvel)
        {
        *((char *)buf) = pak->angvel;
        buf += sizeof(pak->angvel);
        SET(*base_ptr, BIT(2));
        }

    if (pak->aimvel != old_pak->aimvel)
        {
        *((char*)buf) = pak->aimvel;
        buf += sizeof(pak->aimvel);
        SET(*base_ptr, BIT(3));
        }

    //won't work if > 4 bytes
    for(i = 0; i < sizeof(pak->bits); i++)
        {
        if (TEST(pak->bits ^ old_pak->bits, 0xff<<(i<<3)))
            {
            *buf = (pak->bits>>(i<<3));
            buf++;
            SET(*base_ptr, BIT(i+4));
            }
        }

    return(int)(buf - base_ptr);
    }

int DecodeBits(SW_PACKET *pak, SW_PACKET *old_pak, BYTEp buf)
    {
    BYTEp base_ptr = buf;
    unsigned i;

    // skipping the bits field sync test fake byte (Ed. Ken)
    buf++;

    *pak = *old_pak;

    if (TEST(*base_ptr, BIT(0)))
        {
        pak->vel = *(short*)buf;
        buf += sizeof(pak->vel);
        }

    if (TEST(*base_ptr, BIT(1)))
        {
        pak->svel = *(short*)buf;
        buf += sizeof(pak->svel);
        }

    if (TEST(*base_ptr, BIT(2)))
        {
        pak->angvel = *(char*)buf;
        buf += sizeof(pak->angvel);
        }

    if (TEST(*base_ptr, BIT(3)))
        {
        pak->aimvel = *(char*)buf;
        buf += sizeof(pak->aimvel);
        }

    //won't work if > 4 bytes
    for(i = 0; i < sizeof(pak->bits); i++)
        {
        if (TEST(*base_ptr, BIT(i+4)))
            {
            RESET(pak->bits, 0xff<<(i<<3));
            SET(pak->bits, ((int)(*buf))<<(i<<3));
            buf++;
            }
        }

    return(int)( buf - base_ptr );
    }

VOID
PauseGame(VOID)
    {
    if (PauseKeySet)
        return;

    if (DemoPlaying || DemoRecording)
        return;

    if (GamePaused)
        return;

    if (CommPlayers < 2)
        GamePaused = TRUE;
    }

VOID
ResumeGame(VOID)
    {
    if (PauseKeySet)
        return;

    if (DemoPlaying || DemoRecording)
        return;

    if (CommPlayers < 2)
        GamePaused = FALSE;
    }

VOID
PauseAction(VOID)
    {
    ready2send = 0;
    save_totalclock = totalclock;
    }

VOID
ResumeAction(VOID)
    {
    ready2send = 1;
    totalclock = save_totalclock;
    }

VOID
SendMessage(short pnum, char *text)
    {
    if (!CommEnabled)
        return;

    tempbuf[0] = PACKET_TYPE_MESSAGE;
    strcpy((char *)&tempbuf[1], text);
    netsendpacket(pnum, tempbuf, (int)strlen(text) + 2);
    }


VOID
InitNetPlayerOptions(VOID)
    {
    PLAYERp pp = Player + myconnectindex;
    PACKET_OPTIONS p;

    // if you don't have a name :(
    if (!CommPlayerName[0])
        sprintf(CommPlayerName, "PLAYER %d", myconnectindex + 1);

    Bstrupr(CommPlayerName);
    strcpy(pp->PlayerName, CommPlayerName);

    // myconnectindex palette
    pp->TeamColor = gs.NetColor;
    pp->SpriteP->pal = PALETTE_PLAYER0 + pp->TeamColor;
    User[pp->SpriteP - sprite]->spal = pp->SpriteP->pal;

    if (CommEnabled)
        {
        memset(&p, 0, sizeof(p));
        p.PacketType = PACKET_TYPE_PLAYER_OPTIONS;
        p.AutoRun = gs.AutoRun;
        p.MouseAimingOn = gs.MouseAimingOn;
        p.Color = gs.NetColor;
        strcpy(p.PlayerName, CommPlayerName);

        netbroadcastpacket((BYTEp)(&p), sizeof(p));
        }
    }

VOID
SendMulitNameChange(char *new_name, int new_color)
    {
    PLAYERp pp = Player + myconnectindex;
    PACKET_NAME_CHANGE p;

    if (new_name)
        {
        strncpy(CommPlayerName, new_name, sizeof(CommPlayerName)-1);
        CommPlayerName[sizeof(CommPlayerName)-1] = 0;
        Bstrupr(CommPlayerName);
        strcpy(pp->PlayerName, CommPlayerName);
        SetRedrawScreen(pp);
        }

    if (new_color >= 0)
        {
        gs.NetColor = new_color;
        pp->TeamColor = gs.NetColor;
        if (pp->SpriteP)
            {
            pp->SpriteP->pal = PALETTE_PLAYER0 + pp->TeamColor;
            User[pp->SpriteP - sprite]->spal = pp->SpriteP->pal;
            }
        }

    if (CommEnabled)
        {
        memset(&p, 0, sizeof(p));
        p.PacketType = PACKET_TYPE_NAME_CHANGE;
        strcpy(p.PlayerName, CommPlayerName);
        p.Color = gs.NetColor;

        netbroadcastpacket((BYTEp)(&p), sizeof(p));
        }
    }

VOID
SendVersion(int version)
    {
    PLAYERp pp = Player + myconnectindex;
    PACKET_VERSION p;

    if (!CommEnabled)
        return;

    pp->PlayerVersion = version;

    p.PacketType = PACKET_TYPE_VERSION;
    p.Version = version;
    netbroadcastpacket((BYTEp)(&p), sizeof(p));
    }

VOID
CheckVersion(int GameVersion)
    {
    short pnum;
    #define VERSION_MSG "You cannot play with different versions!"

    if (!CommEnabled)
        return;

    TRAVERSE_CONNECT(pnum)
        {
        if (pnum != myconnectindex)
            {
            if (GameVersion != Player[pnum].PlayerVersion)
                {
                buildprintf("CheckVersion(): player %d has version %d, expecting %d\n",
                        pnum, Player[pnum].PlayerVersion, GameVersion);

                adduserquote(VERSION_MSG);
                adduserquote(VERSION_MSG);
                adduserquote(VERSION_MSG);
                adduserquote(VERSION_MSG);
                adduserquote(VERSION_MSG);
                adduserquote(VERSION_MSG);

                if (!Player[pnum].PlayerVersion)
                    {
                    SendMessage(pnum, VERSION_MSG);
                    SendMessage(pnum, VERSION_MSG);
                    SendMessage(pnum, VERSION_MSG);
                    SendMessage(pnum, VERSION_MSG);
                    SendMessage(pnum, VERSION_MSG);
                    SendMessage(pnum, VERSION_MSG);
                    }
                }
            }
        }
    }

VOID
Connect(VOID)
    {
    if (CommEnabled)
        {
        screenpeek = myconnectindex;
        }

    //InitTimingVars();                   // resettiming();
    }

int wfe_Clock;
BOOL (*wfe_ExitCallback)(VOID);

void
waitforeverybody(void)
    {
    int i, size = 1;

    if (!CommEnabled)
        return;

    tempbuf[0] = PACKET_TYPE_PLAYER_READY;
#ifdef DEBUG
    tempbuf[1] = Player[myconnectindex].playerreadyflag + 1;
    size++;
#endif
    netbroadcastpacket(tempbuf, size);

    #if 0
    TRAVERSE_CONNECT(i)
        {
        if (i != myconnectindex)
            {
            DSPRINTF(ds,"Ready packet sent to %d", i);
            DebugWriteString(ds);
            }
        }
    #endif

    //KEY_PRESSED(KEYSC_ESC) = FALSE;
    Player[myconnectindex].playerreadyflag++;

    while (TRUE)
        {
        if (PlayerQuitMenuLevel >= 0)
            {
            //DSPRINTF(ds,"%d, Player Quit Menu Level %d", myconnectindex, PlayerQuitMenuLevel);
            //DebugWriteString(ds);

            MenuCommPlayerQuit(PlayerQuitMenuLevel);
            PlayerQuitMenuLevel = -1;
            }

		handleevents();
        getpackets();

        if (quitevent || (wfe_ExitCallback && wfe_ExitCallback()))
            {
            // allow exit
            //if (KEY_PRESSED(KEYSC_ESC))
                {
                tempbuf[0] = PACKET_TYPE_MENU_LEVEL_QUIT;
                netbroadcastpacket(tempbuf, 1);

                TerminateGame();
                exit(0);
                }
            }

        #if 0
        TRAVERSE_CONNECT(i)
            {
            DSPRINTF(ds,"myindex %d, myready %d, Player %d, Ready %d", myconnectindex, Player[myconnectindex].playerreadyflag, i, Player[i].playerreadyflag);
            DebugWriteString(ds);
            }
        #endif

        TRAVERSE_CONNECT(i)
            {
            if (Player[i].playerreadyflag < Player[myconnectindex].playerreadyflag)
                break;
            }

        if (i < 0)
            {
            return;
            }
        }
    }


BOOL MyCommPlayerQuit(void)
{
    PLAYERp pp;
    short i;
    extern BOOL QuitFlag;
    short found = FALSE;

    TRAVERSE_CONNECT(i)
        {
        if (TEST_SYNC_KEY(Player + i, SK_QUIT_GAME))
            {
            found = TRUE;

            if (i != myconnectindex)
                {
                sprintf(ds,"%s has quit the game.",Player[i].PlayerName);
                adduserquote(ds);
                }
            if (i == connecthead && !NetBroadcastMode)
                QuitFlag = TRUE;
            }
        }

    if (found)
        {
        TRAVERSE_CONNECT(i)
            {
            pp = Player + i;

            if (!TEST_SYNC_KEY(pp, SK_QUIT_GAME))
                continue;

            SET(pp->SpriteP->cstat, CSTAT_SPRITE_INVISIBLE);
            RESET(pp->SpriteP->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN|CSTAT_SPRITE_BLOCK_MISSILE);
            InitBloodSpray(pp->PlayerSprite,TRUE,-2);
            InitBloodSpray(pp->PlayerSprite,FALSE,-2);
            pp->SpriteP->ang = NORM_ANGLE(pp->SpriteP->ang + 1024);
            InitBloodSpray(pp->PlayerSprite,FALSE,-1);
            InitBloodSpray(pp->PlayerSprite,TRUE,-1);

            // if I get my own messages get out to DOS QUICKLY!!!
            if (i == myconnectindex)
                {
                QuitFlag = TRUE;
                ready2send = 0;
                return(TRUE);
                }

            DSPRINTF(ds,"MyCommPlayerQuit %d", i);
            DebugWriteString(ds);

            pp->IsDisconnected = TRUE;

            CommPlayers--;

            // for COOP mode
            if (screenpeek == i)
                NextScreenPeek();
            }
        }

    return(FALSE);
}

BOOL MenuCommPlayerQuit(short quit_player)
{
    PLAYERp pp;
    short i;
    extern BOOL QuitFlag;
    short pnum;

    // tell everyone else you left the game
    TRAVERSE_CONNECT(pnum)
        {
        if (pnum != quit_player)
            {
            sprintf(ds,"%s has quit the game.",Player[myconnectindex].PlayerName);
            SendMessage(pnum, ds);
            }
        }

    TRAVERSE_CONNECT(i)
        {
        pp = Player + i;

        // have to reorder the connect list
        if (i != quit_player)
            continue;

        DSPRINTF(ds,"MenuPlayerQuit %d", quit_player);
        DebugWriteString(ds);

        pp->IsDisconnected = TRUE;

        CommPlayers--;

        // for COOP mode
        if (screenpeek == i)
            NextScreenPeek();
        }

    return(FALSE);
}

VOID ErrorCorrectionQuit(VOID)
    {
    int oldtotalclock;
    short j;

    if (CommPlayers > 1)
        {
        for (j = 0; j < MAX_SW_PLAYERS; j++)
            {
            oldtotalclock = totalclock;
            while (totalclock < oldtotalclock + synctics)
                {
                handleevents();
                getpackets();
                }

            tempbuf[0] = PACKET_TYPE_NULL_PACKET;
            netbroadcastpacket(tempbuf, 1);
            }
        }
    }

VOID
InitNetVars(VOID)
    {
    short pnum;
    PLAYERp pp;

    memset(&loc, 0, sizeof(loc));

    TRAVERSE_CONNECT(pnum)
        {
        pp = Player + pnum;
        pp->movefifoend = 0;
        Player[pnum].syncvalhead = 0;
        memset(pp->inputfifo,0,sizeof(pp->inputfifo));
        }
    movefifoplc = 0;
    movefifosendplc = 0;
    syncvaltail = 0;
    syncvaltottail = 0;
    predictmovefifoplc = 0;

    memset(&syncstat, 0, sizeof(syncstat));
    syncstate = 0;
    memset(sync_first, 0, sizeof(sync_first));
    sync_found = FALSE;

    TRAVERSE_CONNECT(pnum)
        {
        Player[pnum].myminlag = 0;
        }

    otherminlag = mymaxlag = 0;
    }

VOID
InitTimingVars(VOID)
    {
    PlayClock = 0;

    // resettiming();
    totalsynctics = 0;
    totalclock = 0;
    ototalclock = 0;
    randomseed = 17L;

    MoveSkip8 = 2;
    MoveSkip2 = 0;
    MoveSkip4 = 1;                      // start slightly offset so these
                                        // don't move the same
                                        // as the Skip2's
    MoveThingsCount = 0;

    }


void
AddSyncInfoToPacket(int *j)
    {
    int sb;
	int count = 0;

    // sync testing
    while (Player[myconnectindex].syncvalhead != syncvaltail && count++ < 4)
        {
        for (sb = 0; sb < NumSyncBytes; sb++)
            packbuf[(*j)++] = Player[myconnectindex].syncval[syncvaltail & (SYNCFIFOSIZ - 1)][sb];

        syncvaltail++;
        }
    }

void
faketimerhandler(void)
    {
    int i, j, k;
    PLAYERp pp;
    void getinput(SW_PACKET *);
    extern BOOL BotMode;

    #if 0
    if (KEY_PRESSED(KEYSC_PERIOD))
        {
        extern unsigned int MoveThingsCount;
        MoveThingsCount++;
        MoveThingsCount--;
        return;
        }
    #endif

	sampletimer();
    if ((totalclock < ototalclock + synctics))
        return;

    if (!ready2send)
        return;

    ototalclock += synctics;

    getpackets();


// TENSW: this way we are guaranteed that the most advanced player is no more
// than 200 frames ahead of the most laggy. We're more healthy if the queue
// doesn't overflow, that's for sure.
    if (Player[myconnectindex].movefifoend - movefifoplc >= 100)
        return;

    getinput(&loc);

    AveragePacket.vel += loc.vel;
    AveragePacket.svel += loc.svel;
    AveragePacket.angvel += loc.angvel;
    AveragePacket.aimvel += loc.aimvel;
    SET(AveragePacket.bits, loc.bits);

    pp = Player + myconnectindex;

    if (pp->movefifoend & (MovesPerPacket-1))
        {
        memcpy(&pp->inputfifo[pp->movefifoend & (MOVEFIFOSIZ - 1)],
               &pp->inputfifo[(pp->movefifoend-1) & (MOVEFIFOSIZ - 1)],
               sizeof(SW_PACKET));

        pp->movefifoend++;
        return;
        }

    loc.vel = AveragePacket.vel / MovesPerPacket;
    loc.svel = AveragePacket.svel / MovesPerPacket;
    loc.angvel = AveragePacket.angvel / MovesPerPacket;
    loc.aimvel = AveragePacket.aimvel / MovesPerPacket;
    loc.bits = AveragePacket.bits;

    memset(&AveragePacket, 0, sizeof(AveragePacket));

    pp->inputfifo[Player[myconnectindex].movefifoend & (MOVEFIFOSIZ - 1)] = loc;
    pp->movefifoend++;

#if 0
//  AI Bot stuff
     if (CommPlayers > 1)
     {
        if (gNet.MultiGameType == MULTI_GAME_AI_BOTS)
        {
            TRAVERSE_CONNECT(i)
            {
                if(i != myconnectindex)
                {
                    if(BotMode && Player[i].IsAI == 1)  // Skip it if this player is not computer controlled!
                    {
                        computergetinput(i,&Player[i].inputfifo[Player[i].movefifoend&(MOVEFIFOSIZ-1)]);
                        Player[i].movefifoend++;
                    }
                }
            }
        }
     }
// AI Bot stuff
#endif

    if (!CommEnabled)
        {
        TRAVERSE_CONNECT(i)
            {
            if (i != myconnectindex)
                {
                if (BotMode && Player[i].IsAI == 1)
                {
                    computergetinput(i,&Player[i].inputfifo[Player[i].movefifoend&(MOVEFIFOSIZ-1)]);
                }
                else
                    memset(&Player[i].inputfifo[Player[i].movefifoend & (MOVEFIFOSIZ - 1)], 0, sizeof(Player[i].inputfifo[0]));
                Player[i].movefifoend++;
                }
            }
        return;
        }

    TRAVERSE_CONNECT(i)
        {
        if (i != myconnectindex)
            {
            k = (Player[myconnectindex].movefifoend - 1) - Player[i].movefifoend;
            Player[i].myminlag = min(Player[i].myminlag, k);
            mymaxlag = max(mymaxlag, k);
            }
        }

	if (((Player[myconnectindex].movefifoend - 1) & (TIMERUPDATESIZ - 1)) == 0)
        {
        i = mymaxlag - bufferjitter;
        mymaxlag = 0;
        if (i > 0)
            bufferjitter += ((2 + i) >> 2);
        else if (i < 0)
            bufferjitter -= ((2 - i) >> 2);
        }

    if (NetBroadcastMode)
        {
        packbuf[0] = PACKET_TYPE_BROADCAST;
        j = 1;

        if (((Player[myconnectindex].movefifoend - 1) & (TIMERUPDATESIZ - 1)) == 0)
            {
            if (myconnectindex == connecthead)
                {
                for (i = connectpoint2[connecthead]; i >= 0; i = connectpoint2[i])
                    {
                    if (Player[i].IsDisconnected)
                        continue;
                    packbuf[j++] = min(max(Player[i].myminlag, -128), 127);
                    }
                }
            else
                {
                i = Player[connecthead].myminlag - otherminlag;
                if (labs(i) > 2)
                    {
                    ////DSPRINTF(ds,"lag correction: %d,%d,%d",i,Player[connecthead].myminlag,otherminlag);
                    //MONO_PRINT(ds);

                    if (labs(i) > 8)
                        {
                        if (i < 0)
                            i++;
                        i >>= 1;
                        }
                    else
                        {
                        if (i < 0)
                            i = -1;
                        if (i > 0)
                            i = 1;
                        }
                    totalclock -= synctics * i;
                    otherminlag += i;
                    }
                }

            for (i = connecthead; i >= 0; i = connectpoint2[i])
                Player[i].myminlag = 0x7fffffff;
            }

        pp = Player + myconnectindex;

        #if !BIT_CODEC
        memcpy(&packbuf[j], &pp->inputfifo[(Player[myconnectindex].movefifoend - 1) & (MOVEFIFOSIZ - 1)], sizeof(SW_PACKET));
        j += sizeof(SW_PACKET);
        #else
        j += EncodeBits(&pp->inputfifo[(Player[myconnectindex].movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                   &pp->inputfifo[(Player[myconnectindex].movefifoend - 2) & (MOVEFIFOSIZ - 1)],
                   &packbuf[j]);
        #endif

        #if SYNC_TEST
        AddSyncInfoToPacket(&j);
        #endif

		netbroadcastpacket(packbuf, j);

        return;
        } // NetBroadcastMode

    // SLAVE CODE
    if (myconnectindex != connecthead)  // I am the Slave
        {
        if (((Player[myconnectindex].movefifoend - 1) & (TIMERUPDATESIZ - 1)) == 0)
            {
            i = Player[connecthead].myminlag - otherminlag;
            if (labs(i) > 2)
                {
                if (labs(i) > 8)
                    {
                    if (i < 0)
                        i++;
                    i >>= 1;
                    }
                else
                    {
                    if (i < 0)
                        i = -1;
                    if (i > 0)
                        i = 1;
                    }
                totalclock -= synctics * i;
                otherminlag += i;
                }

            for (i = connecthead; i >= 0; i = connectpoint2[i])
                {
                Player[i].myminlag = 0x7fffffff;
                }
            }
        packbuf[0] = PACKET_TYPE_SLAVE_TO_MASTER;
        j = 1;

        pp = Player + myconnectindex;
        #if !BIT_CODEC
        memcpy(&packbuf[j], &pp->inputfifo[(pp->movefifoend - 1) & (MOVEFIFOSIZ - 1)], sizeof(SW_PACKET));
        j += sizeof(SW_PACKET);
        #else
        j += EncodeBits(&pp->inputfifo[(pp->movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                   &pp->inputfifo[(pp->movefifoend - 2) & (MOVEFIFOSIZ - 1)],
                   &packbuf[j]);
        #endif

        #if SYNC_TEST
        AddSyncInfoToPacket(&j);
        #endif

        netsendpacket(connecthead, packbuf, j);
        return;
        }

    // This allows packet-resends
    //for (i = connecthead; i >= 0; i = connectpoint2[i])
    //    {
    //    if ((Player[i].movefifoend <= movefifosendplc))
    //        {
    //        packbuf[0] = 127;
    //        for (i = connectpoint2[connecthead]; i >= 0; i = connectpoint2[i])
    //            {
    //            sendpacket(i, packbuf, 1);
    //            }
    //        return;
    //        }
    //    }

    // I am MASTER...
    while (1)
        {
        TRAVERSE_CONNECT(i)
            {
            if ((Player[i].movefifoend <= movefifosendplc))
                return;
            }

        packbuf[0] = PACKET_TYPE_MASTER_TO_SLAVE;
        j = 1;

        // Fix timers and buffer/jitter value
        if ((movefifosendplc & (TIMERUPDATESIZ - 1)) == 0)
            {
            for (i = connectpoint2[connecthead]; i >= 0; i = connectpoint2[i])
                {
                if (Player[i].IsDisconnected)
                    continue;
                packbuf[j++] = min(max(Player[i].myminlag, -128), 127);
                }

            for (i = connecthead; i >= 0; i = connectpoint2[i])
                Player[i].myminlag = 0x7fffffff;
            }

        TRAVERSE_CONNECT(i)
            {
            pp = Player + i;

            #if !BIT_CODEC
            memcpy(&packbuf[j], &pp->inputfifo[movefifosendplc & (MOVEFIFOSIZ - 1)], sizeof(SW_PACKET));
            j += sizeof(SW_PACKET);
            #else
            j += EncodeBits(&pp->inputfifo[(movefifosendplc) & (MOVEFIFOSIZ - 1)],
                       &pp->inputfifo[(movefifosendplc - 1) & (MOVEFIFOSIZ - 1)],
                       &packbuf[j]);
            #endif
            //pp->movefifoend++;
            }

        #if SYNC_TEST
        AddSyncInfoToPacket(&j);
        #endif

        for (i = connectpoint2[connecthead]; i >= 0; i = connectpoint2[i])
            {
            if (Player[i].IsDisconnected)
                continue;
            netsendpacket(i, packbuf, j);
            /*
            pp = Player + i;
            if (TEST(pp->inputfifo[movefifosendplc & (MOVEFIFOSIZ - 1)].bits,QUITBIT)
               playerquitflag[i] = 1;
            */
            }

        movefifosendplc += MovesPerPacket;
        }
    }

VOID
checkmasterslaveswitch(VOID)
    {
    }

VOID
getpackets(VOID)
    {
    int otherconnectindex, packbufleng;
    int i, j, sb;
    PLAYERp pp;
    SW_PACKET tempinput;

    sampletimer();

    if (!CommEnabled)
        return;

    while ((packbufleng = netgetpacket(&otherconnectindex, packbuf)) > 0)
        {
        switch (packbuf[0])
            {
        case PACKET_TYPE_BROADCAST:
		case SERVER_GENERATED_BROADCAST:
            ////DSPRINTF(ds,"Receive Broadcast %d, ready2send %d",otherconnectindex, ready2send);
            //MONO_PRINT(ds);

            //ASSERT(ready2send);
            //if (!ready2send)
            //    break;

            j = 1;

            if ((Player[otherconnectindex].movefifoend & (TIMERUPDATESIZ - 1)) == 0)
                {
                if (otherconnectindex == connecthead)
                    {
                    for (i = connectpoint2[connecthead]; i >= 0; i = connectpoint2[i])
                        {
                        if (Player[i].IsDisconnected)
                            continue;
                        if (i == myconnectindex)
                            otherminlag = (int) ((signed char) packbuf[j]);
                        j++;
                        }
                    }
                }

            pp = Player + otherconnectindex;

            #if !BIT_CODEC
            memcpy(&pp->inputfifo[pp->movefifoend & (MOVEFIFOSIZ - 1)], &packbuf[j], sizeof(SW_PACKET));
            j += sizeof(SW_PACKET);
            #else
            j += DecodeBits(&pp->inputfifo[(pp->movefifoend) & (MOVEFIFOSIZ - 1)],
                       &pp->inputfifo[(pp->movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                       &packbuf[j]);
            #endif

            pp->movefifoend++;

            // Packet Duplication
            for (i = 1; i < MovesPerPacket; i++)
                {
                memcpy(
                    &pp->inputfifo[pp->movefifoend & (MOVEFIFOSIZ - 1)],
                    &pp->inputfifo[(pp->movefifoend-1) & (MOVEFIFOSIZ - 1)],
                    sizeof(SW_PACKET));

                pp->movefifoend++;
                }

            #if SYNC_TEST
            GetSyncInfoFromPacket(packbuf, packbufleng, &j, otherconnectindex);
            #endif

        //DSPRINTF(ds,"Receive packet size %d",j);
        //MONO_PRINT(ds);

            break;

        case PACKET_TYPE_MASTER_TO_SLAVE:
            // Here slave is receiving
            j = 1;

            if ((Player[otherconnectindex].movefifoend & (TIMERUPDATESIZ - 1)) == 0)
                {
                for (i = connectpoint2[connecthead]; i >= 0; i = connectpoint2[i])
                    {
                    if (Player[i].IsDisconnected)
                        continue;
                    if (i == myconnectindex)
                        otherminlag = (int) ((signed char) packbuf[j]);
                    j++;
                    }
                }

            TRAVERSE_CONNECT(i)
                {
                pp = Player + i;

                #if !BIT_CODEC
                if (i != myconnectindex)
                    {
                    memcpy(&pp->inputfifo[pp->movefifoend & (MOVEFIFOSIZ - 1)], &packbuf[j], sizeof(SW_PACKET));
                    pp->movefifoend++;
                    }
                j += sizeof(SW_PACKET);
                #else
                if (i == myconnectindex)
                    {
                    j += DecodeBits(&tempinput,
                                    &pp->inputfifo[(pp->movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                                    &packbuf[j]);
                    }
                else
                    {
                    j += DecodeBits(&pp->inputfifo[(pp->movefifoend) & (MOVEFIFOSIZ - 1)],
                                    &pp->inputfifo[(pp->movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                                    &packbuf[j]);
                    pp->movefifoend++;
                    }
                #endif
                }

            while (j < packbufleng)
                {
                for (i = connecthead; i >= 0; i = connectpoint2[i])
                    {
                    if (i != myconnectindex)
                        {
                        for (sb = 0; sb < NumSyncBytes; sb++)
                            {
                            Player[i].syncval[Player[i].syncvalhead & (SYNCFIFOSIZ - 1)][sb] = packbuf[j + sb];
                            }

                        Player[i].syncvalhead++;
                        }
                    }

                j += NumSyncBytes;
                }

            // #if SYNC_TEST   //This doesn't work right in this case
            // GetSyncInfoFromPacket(packbufleng, &j, otherconnectindex);
            // #endif

            for(i=connecthead;i>=0;i=connectpoint2[i])
                if (i != myconnectindex)
                    for(j=1;j<MovesPerPacket;j++)
                        {
                            pp = Player + i;
                            if (pp->IsDisconnected)
                                continue;

                            memcpy(
                                &pp->inputfifo[pp->movefifoend & (MOVEFIFOSIZ - 1)],
                                &pp->inputfifo[(pp->movefifoend-1) & (MOVEFIFOSIZ - 1)],
                                sizeof(SW_PACKET));

                            pp->movefifoend++;
                        }

            break;

        case PACKET_TYPE_SLAVE_TO_MASTER:
            // Here master is receiving
            pp = Player + otherconnectindex;
            j = 1;

            #if !BIT_CODEC
            memcpy(&pp->inputfifo[Player[otherconnectindex].movefifoend & (MOVEFIFOSIZ - 1)], &packbuf[j], sizeof(SW_PACKET));
            j += sizeof(SW_PACKET);
            #else
            j += DecodeBits(&pp->inputfifo[(Player[otherconnectindex].movefifoend) & (MOVEFIFOSIZ - 1)],
                       &pp->inputfifo[(Player[otherconnectindex].movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                       &packbuf[j]);
            #endif

            Player[otherconnectindex].movefifoend++;

            #if SYNC_TEST
            GetSyncInfoFromPacket(packbuf, packbufleng, &j, otherconnectindex);
            #endif

            // Tic duping
            for (i = 1; i < MovesPerPacket; i++)
                {
                memcpy(&pp->inputfifo[(Player[otherconnectindex].movefifoend) & (MOVEFIFOSIZ - 1)],
                       &pp->inputfifo[(Player[otherconnectindex].movefifoend - 1) & (MOVEFIFOSIZ - 1)],
                       sizeof(SW_PACKET));
                Player[otherconnectindex].movefifoend++;
                }

            break;

        case PACKET_TYPE_MESSAGE:
            {
            PLAYERp tp = Player + myconnectindex;

            pp = Player + otherconnectindex;

            PlaySound(DIGI_PMESSAGE,&tp->posx,&tp->posy,&tp->posz,v3df_dontpan);

            memcpy(ds,&packbuf[3],packbufleng-3);
            ds[packbufleng-3] = 0;
            //sprintf(ds, "%s",&packbuf[3]);
            adduserquote(ds);
            break;
            }

        case PACKET_TYPE_RTS:
            {
            PACKET_RTSp p;

            p = (void *)&packbuf[0];

            PlaySoundRTS(p->RTSnum);

            break;
            }


        case PACKET_TYPE_NEW_GAME:
            {
            extern BOOL NewGame, ShortGameMode, DemoInitOnce;
            PACKET_NEW_GAMEp p;
            extern short TimeLimitTable[];

            pp = Player + otherconnectindex;

            // Dukes New Game Packet
            //level_number //volume_number //player_skill //monsters_off //respawn_monsters
            //respawn_items //respawn_inventory //coop //marker //friendlyfire //boardname

            p = (void *)&packbuf[0];

            ready2send = 0;

            Level = p->Level;
            Skill = p->Skill;

            gNet.HurtTeammate = p->HurtTeammate;
            gNet.SpawnMarkers = p->SpawnMarkers;
            gNet.TeamPlay   = p->TeamPlay;
            gNet.AutoAim    = p->AutoAim;
            gNet.Nuke       = p->Nuke;
            gNet.KillLimit          = p->KillLimit*10;
            gNet.TimeLimit          = TimeLimitTable[p->TimeLimit]*60*120;

            if (ShortGameMode)
                {
                gNet.KillLimit /= 10;
                gNet.TimeLimit /= 2;
                }

            gNet.TimeLimitClock = gNet.TimeLimit;
            gNet.MultiGameType = p->GameType+1;

            // settings for No Respawn Commbat mode
            if (gNet.MultiGameType == MULTI_GAME_COMMBAT_NO_RESPAWN)
                {
                gNet.MultiGameType = MULTI_GAME_COMMBAT;
                gNet.NoRespawn = TRUE;
                }
            else
                {
                gNet.NoRespawn = FALSE;
                }

            ExitLevel = TRUE;
            NewGame = TRUE;
            // restart demo for multi-play mode
            DemoInitOnce = FALSE;
            ResetMenuInput();

            // send a dummy packet to see when it arrives
            //tempbuf[0] = PACKET_TYPE_DUMMY;
            //sendpacket(otherconnectindex, tempbuf, 1);

            ////DSPRINTF(ds,"Level %d, Skill %d, AutoAim %d",Level, Skill, gs.AutoAim);
            //MONO_PRINT(ds);

            break;
            }

        case PACKET_TYPE_DUMMY:
            ////DSPRINTF(ds,"Got Dummy Packet!!!");
            //MONO_PRINT(ds);
            break;

        case PACKET_TYPE_VERSION:
            {
            PACKET_VERSIONp p;

            pp = Player + otherconnectindex;
            p = (void *)&packbuf[0];

            pp->PlayerVersion = p->Version;
            break;
            }

        case PACKET_TYPE_PLAYER_OPTIONS:
            {
            PACKET_OPTIONSp p;

            pp = Player + otherconnectindex;
            p = (void *)&packbuf[0];

            // auto run
            if (p->AutoRun)
                SET(pp->Flags, PF_LOCK_RUN);
            else
                RESET(pp->Flags, PF_LOCK_RUN);

            // mouse aiming on
            if (p->MouseAimingOn)
                SET(pp->Flags, PF_MOUSE_AIMING_ON);
            else
                RESET(pp->Flags, PF_MOUSE_AIMING_ON);

            // palette
            pp->TeamColor = p->Color;
            pp->SpriteP->pal = PALETTE_PLAYER0 + pp->TeamColor;
            User[pp->SpriteP - sprite]->spal = pp->SpriteP->pal;

            // names
            strcpy(pp->PlayerName, p->PlayerName);

            break;
            }

        case PACKET_TYPE_NAME_CHANGE:
            {
            PACKET_NAME_CHANGEp p;
            pp = Player + otherconnectindex;
            p = (void *)&packbuf[0];

            // someone else has changed their name

            DSPRINTF(ds,"Recieved name: %s",p->PlayerName);
            MONO_PRINT(ds);

            strcpy(pp->PlayerName, p->PlayerName);
            SetRedrawScreen(Player+myconnectindex);

            // palette
            pp->TeamColor = p->Color;
            if (pp->SpriteP)
                {
                pp->SpriteP->pal = PALETTE_PLAYER0 + pp->TeamColor;
                User[pp->SpriteP - sprite]->spal = pp->SpriteP->pal;
                }
            break;
            }


        case PACKET_TYPE_MENU_LEVEL_QUIT:
            {
            PlayerQuitMenuLevel = otherconnectindex;
            break;
            }

        case PACKET_TYPE_PLAYER_READY:
            Player[otherconnectindex].playerreadyflag++;
			// It's important to return from getpackets() when a ready packet comes in and
			// you are inside waitforeverybody(). Otherwise multiple ready packets can come
			// in inside one waitforeverybody() which causes havoc if that w4e is protecting
			// an initialization step which (heh) for example resets the playerreadyflag count.
			return;
            //break;

        case PACKET_TYPE_DONT_USE:
            break;

        case PACKET_TYPE_NULL_PACKET:
            break;

	case PACKET_TYPE_PROXY:
	    buildputs("getpackets(): nested proxy packets!?\n");
	    break;

        default:
            DSPRINTF(ds,"Packet type unknown %d",packbuf[0]);
            MONO_PRINT(ds);
            }
        }
    }
