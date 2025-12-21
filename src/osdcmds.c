//-------------------------------------------------------------------------
/*
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
*/
//-------------------------------------------------------------------------

#include "build.h"
#include "osd.h"
#include "cache1d.h"
#include "game.h"
#include "demo.h"
#include "menus.h"

static int
osdcmd_restartvid(const osdfuncparm_t *parm)
    {
    extern BOOL RestartVideo;

    (void)parm;

    RestartVideo = TRUE;

    return OSDCMD_OK;
    }

static int
osdcmd_vidmode(const osdfuncparm_t *parm)
    {
    extern BOOL RestartVideo;
    extern int32 ScreenMode, ScreenDisplay, ScreenWidth, ScreenHeight, ScreenBPP;

    int newx = ScreenWidth, newy = ScreenHeight, newbpp = ScreenBPP;
    int newfs = ScreenMode, newdisp = ScreenDisplay;

    if (parm->numparms < 1 || parm->numparms > 5) return OSDCMD_SHOWHELP;

    switch (parm->numparms)
        {
        case 1:   // bpp switch
            newbpp = atoi(parm->parms[0]);
            break;
        case 2: // res switch
            newx = atoi(parm->parms[0]);
            newy = atoi(parm->parms[1]);
            break;
        case 3:   // res, bpp, fullscreen, display switch
        case 4:
        case 5:
            newx = atoi(parm->parms[0]);
            newy = atoi(parm->parms[1]);
            newbpp = atoi(parm->parms[2]);
            if (parm->numparms >= 4)
                newfs = (atoi(parm->parms[3]) != 0);
            if (parm->numparms == 5)
                newdisp = max(0,atoi(parm->parms[4]));
            break;
        }

    if (checkvideomode(&newx, &newy, newbpp, SETGAMEMODE_FULLSCREEN(newdisp, newfs), 0) >= 0)
        {
        RestartVideo = TRUE;
        ScreenMode = newfs;
        ScreenDisplay = newdisp;
        ScreenWidth = newx;
        ScreenHeight = newy;
        ScreenBPP = newbpp;
        }

    return OSDCMD_OK;
    }

static int
osdcmd_map(const osdfuncparm_t *parm)
{
    int i;
    char filename[BMAX_PATH];

    if (parm->numparms != 1) return OSDCMD_SHOWHELP;

    strcpy(filename, parm->parms[0]);
    if(strchr(filename,'.') == NULL)
        strcat(filename, ".map");

    if ((i = kopen4load(filename,0)) < 0)
        {
        buildprintf("map: file \"%s\" does not exist.\n", filename);
        return OSDCMD_OK;
        }
    kclose(i);

    if (DemoRecording || CommPlayers > 1)
        {
        buildprintf("map: no changing map in multiplayer.\n");
        return OSDCMD_OK;
        }
    else
    if (strlen(filename) > sizeof(UserMapName)-1)
        {
        buildprintf("map: filename is too long.\n");
        return OSDCMD_OK;
        }

    strcpy(UserMapName, filename);
    ExitLevel = TRUE;

    return OSDCMD_OK;
}

static int
osdcmd_quit(const osdfuncparm_t *parm)
{
    (void)parm;

    if (CommPlayers > 1)
        MultiPlayQuitFlag = TRUE;
    else
        QuitFlag = TRUE;

    return OSDCMD_OK;
}

static int
osdcmd_showfps(const osdfuncparm_t *parm)
{
    extern CHAR LocationInfo;   // Actually BOOL, but CHAR is the same size.

    if (parm->numparms < 1)
        LocationInfo = !(LocationInfo > 0);
    else
        LocationInfo = max(0, min(2, atoi(parm->parms[0])));
    return OSDCMD_OK;
}

void SetupOSDCommands(void)
{
    OSD_RegisterFunction("restartvid", "restartvid: reinitialise the video mode", osdcmd_restartvid);
    OSD_RegisterFunction("vidmode", "vidmode [xdim ydim] [bpp] [fullscreen]: change the video mode", osdcmd_vidmode);
    OSD_RegisterFunction("quit", "quit: exit the game immediately", osdcmd_quit);
    OSD_RegisterFunction("showfps", "showfps: show frame rate", osdcmd_showfps);

    if (SW_REGISTERED)
        {
        OSD_RegisterFunction("map","map <mapfile>: loads the given user map", osdcmd_map);
        }
}
