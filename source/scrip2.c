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

// scriplib.c
#include "build.h"
#include "compat.h"
#include "cache1d.h"

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"

#include "parse.h"
#include "jsector.h"
#include "parent.h"

#define PATHSEPERATOR   '\\'

//#define COMPUTE_TOTALS    1

extern int nextvoxid;	// JBF: in game.c

ParentalStruct aVoxelArray[MAXTILES];


/*
=============================================================================

                        ABNORMAL TERMINATION

=============================================================================
*/
#ifdef RENDERTYPEWIN
void Error (char *error, ...)
{
    va_list argptr;

    va_start (argptr,error);
    vprintf (error,argptr);
    va_end (argptr);
    printf ("\n");
    exit (1);
}
#else
void Error(char *,...);
#endif


/*
=============================================================================

                        PARSING STUFF

=============================================================================
*/

char    token[MAXTOKEN];
char    *scriptbuffer,*script_p,*scriptend_p;
int     grabbed;
int     scriptline;
BOOL    endofscript;
BOOL    tokenready;                     // only TRUE if UnGetToken was just called

/*
==============
=
= LoadScriptFile
=
==============
*/

BOOL LoadScriptFile (char *filename)
{
    long size, readsize;
    int fp;


    if((fp=kopen4load(filename,0)) == -1)
    {
        // If there's no script file, forget it.
        return(FALSE);
    }

    size = kfilelength(fp);    

    scriptbuffer = (char *)AllocMem(size);

    ASSERT(scriptbuffer != NULL);

    readsize = kread(fp, scriptbuffer, size);

    kclose(fp);

    ASSERT(readsize == size);


    // Convert filebuffer to all upper case
    //strupr(scriptbuffer);

    script_p = scriptbuffer;
    scriptend_p = script_p + size;
    scriptline = 1;
    endofscript = FALSE;
    tokenready = FALSE;
    return(TRUE);
}


/*
==============
=
= UnGetToken
=
= Signals that the current token was not used, and should be reported
= for the next GetToken.  Note that

GetToken (TRUE);
UnGetToken ();
GetToken (FALSE);

= could cross a line boundary.
=
==============
*/

void UnGetToken (void)
{
    tokenready = TRUE;
}


/*
==============
=
= GetToken
=
==============
*/

void GetToken (BOOL crossline)
{
    char    *token_p;

    if (tokenready)                         // is a token already waiting?
    {
        tokenready = FALSE;
        return;
    }

    if (script_p >= scriptend_p)
    {
        if (!crossline)
            Error ("Line %i is incomplete\n",scriptline);
        endofscript = TRUE;
        return;
    }

//
// skip space
//
skipspace:
    while (*script_p <= 32)
    {
        if (script_p >= scriptend_p)
        {
            if (!crossline)
                Error ("Line %i is incomplete\n",scriptline);
            endofscript = TRUE;
            return;
        }
        if (*script_p++ == '\n')
        {
            if (!crossline)
                Error ("Line %i is incomplete\n",scriptline);
            scriptline++;
        }
    }

    if (script_p >= scriptend_p)
    {
        if (!crossline)
            Error ("Line %i is incomplete\n",scriptline);
        endofscript = TRUE;
        return;
    }

    if (*script_p == '#')   // # is comment field
    {
        if (!crossline)
            Error ("Line %i is incomplete\n",scriptline);
        while (*script_p++ != '\n')
            if (script_p >= scriptend_p)
            {
                endofscript = TRUE;
                return;
            }
        goto skipspace;
    }

//
// copy token
//
    token_p = token;

    while ( *script_p > 32 && *script_p != '#')
    {
        *token_p++ = *script_p++;
        if (script_p == scriptend_p)
            break;
        ASSERT (token_p != &token[MAXTOKEN])
//          Error ("Token too large on line %i\n",scriptline);
    }

    *token_p = 0;
}


/*
==============
=
= TokenAvailable
=
= Returns true if there is another token on the line
=
==============
*/

BOOL TokenAvailable (void)
{
    char    *search_p;

    search_p = script_p;

    if (search_p >= scriptend_p)
        return FALSE;

    while ( *search_p <= 32)
    {
        if (*search_p == '\n')
            return FALSE;
        search_p++;
        if (search_p == scriptend_p)
            return FALSE;

    }

    if (*search_p == '#')
        return FALSE;

    return TRUE;
}


// Load all the voxel files using swvoxfil.txt script file
// Script file format:

//          # - Comment
//          spritenumber (in artfile), voxel number, filename
//          Ex. 1803 0 medkit2.kvx
//              1804 1 shotgun.kvx 
//              etc....

void LoadKVXFromScript( char *filename )
{
    long lNumber=0,lTile=0; // lNumber is the voxel no. and lTile is the editart tile being
                            // replaced.
    char *sName;            // KVS file being loaded in.

    int grabbed=0;          // Number of lines parsed

    sName = (char *)AllocMem(256);    // Up to 256 bytes for path
    ASSERT(sName != NULL);

    // zero out the array memory with -1's for pics not being voxelized
    memset(&aVoxelArray[0],-1,sizeof(struct TILE_INFO_TYPE)*MAXTILES);
    for(grabbed = 0; grabbed < MAXTILES; grabbed++)
    {
        aVoxelArray[grabbed].Voxel = -1;
        aVoxelArray[grabbed].Parental = -1;
    }

    grabbed = 0;

    // Load the file
    if(!LoadScriptFile(filename))
        ASSERT(TRUE==FALSE);

    do {
        GetToken (TRUE);    // Crossing a line boundary on the end of line to first token
                            // of a new line is permitted (and expected)
        if (endofscript)
            break;

        lTile = atol(token);

        GetToken(FALSE);
        lNumber = atol(token);

        GetToken(FALSE);
        strcpy(sName,token);            // Copy the whole token as a file name and path

        // Load the voxel file into memory
        if (!qloadkvx(lNumber,sName)) {
            // Store the sprite and voxel numbers for later use
            aVoxelArray[lTile].Voxel = lNumber; // Voxel num
	}

	if (lNumber >= nextvoxid)	// JBF: so voxels in the def file append to the list
		nextvoxid = lNumber + 1;
	
        grabbed++;
        ASSERT(grabbed < MAXSPRITES);

    } while (script_p < scriptend_p);

        FreeMem(scriptbuffer);
    FreeMem(sName);
    script_p = NULL;
}

// Load in info for all Parental lock tile targets
//          # - Comment
//          tilenumber (in artfile), replacement tile offset (if any)
//          Ex. 1803 -1       -1 = No tile replacement
//              1804 2000 
//              etc....
void LoadPLockFromScript( char *filename )
{
    long lNumber=0,lTile=0; // lNumber is the voxel no. and lTile is the editart tile being
                            // replaced.
    char *sName;            // KVS file being loaded in.

        int grabbed=0;          // Number of lines parsed

    sName = (char *)AllocMem(256);    // Up to 256 bytes for path
    ASSERT(sName != NULL);

        // Load the file
    if(!LoadScriptFile(filename))
        ASSERT(TRUE==FALSE);

    do {
        GetToken (TRUE);    // Crossing a line boundary on the end of line to first token
                            // of a new line is permitted (and expected)
        if (endofscript)
            break;

        lTile = atoi(token);

        GetToken(FALSE);
        lNumber = atoi(token);

        // Store the sprite and voxel numbers for later use
        aVoxelArray[lTile].Parental = lNumber;  // Replacement to tile, -1 for none

        grabbed++;
        ASSERT(grabbed < MAXSPRITES);

    } while (script_p < scriptend_p);

    FreeMem(scriptbuffer);
    FreeMem(sName);
    script_p = NULL;
}


/*
 * Here begins JonoF's modding enhancement stuff
 */

#include "scriptfile.h"

enum {
	CM_MAP,
	CM_EPISODE,
	CM_TITLE,
	CM_FILENAME,
	CM_SONG,
	CM_CDATRACK,
	CM_BESTTIME,
	CM_PARTIME,
	CM_SUBTITLE,
	CM_SKILL,
	CM_TEXT,
	CM_COOKIE,
	CM_GOTKEY,
	CM_NEEDKEY,
	CM_SECRET,
	CM_QUIT,
};

static const struct {
	char *str;
	int tokn;
} cm_tokens[] = {
	{ "map",         CM_MAP      },
	{ "level",       CM_MAP      },
	{ "episode",     CM_EPISODE  },
	{ "filename",    CM_FILENAME },
	{ "file",        CM_FILENAME },
	{ "fn",          CM_FILENAME },
	{ "levelname",   CM_FILENAME },
	{ "song",        CM_SONG     },
	{ "music",       CM_SONG     },
	{ "songname",    CM_SONG     },
	{ "title",       CM_TITLE    },
	{ "name",        CM_TITLE    },
	{ "description", CM_TITLE    },
	{ "besttime",    CM_BESTTIME },
	{ "partime",     CM_PARTIME  },
	{ "cdatrack",    CM_CDATRACK },
	{ "cdtrack",     CM_CDATRACK },
	{ "subtitle",    CM_SUBTITLE },
	{ "skill",       CM_SKILL    },
	{ "text",        CM_TEXT     },
	{ "cookie",      CM_COOKIE   },
	{ "fortune",     CM_COOKIE   },
	{ "gotkey",      CM_GOTKEY   },
	{ "needkey",     CM_NEEDKEY  },
	{ "secret",      CM_SECRET   },
	{ "quit",        CM_QUIT     },
};
#define cm_numtokens (sizeof(cm_tokens)/sizeof(cm_tokens[0]))

static int cm_transtok(const char *tok)
{
	int i;

	for (i=0; i<cm_numtokens; i++) {
		if (!Bstrcasecmp(tok, cm_tokens[i].str))
			return cm_tokens[i].tokn;
	}

	return -1;
}

// Load custom map and episode information
//   level # {
//      title    "Map Name"
//      filename "filename.map" 
//      song     "filename.mid"
//      cdatrack n
//      besttime secs
//      partime  secs
//   }
//
//   episode # {
//      title    "Episode Name"
//      subtitle "Caption text"
//   }
//
//   skill # {
//      name "Tiny grasshopper"
//   }
//
//   text {
//      fortune {
//      	"You never going to score."
//      	"26-31-43-82-16-29"
//      	"Sorry, you no win this time, try again."
//      }
//      gotkey {
//      	"Got the RED key!"
//      	"Got the BLUE key!"
//      	"Got the GREEN key!"
//      }
//      needkey {
//      	"You need a RED key for this door."
//      	"You need a BLUE key for this door."
//      	"You need a GREEN key for this door."
//      }
//      secret  "You found a secret area!"
//      quit    "PRESS (Y) TO QUIT, (N) TO FIGHT ON."
//   }

static LEVEL_INFO custommaps[MAX_LEVELS_REG];
static char *customfortune[MAX_FORTUNES];
static char *customkeymsg[MAX_KEYS];
static CHARp customkeydoormsg[MAX_KEYS];

// FIXME: yes, we are leaking memory here at the end of the program by not freeing anything
void LoadCustomInfoFromScript(char *filename)
{
	scriptfile *script;
	char *token;
	char *braceend;
	int curmap = -1;

	script = scriptfile_fromfile(filename);
	if (!script) return;

	while ((token = scriptfile_gettoken(script))) {
		switch (cm_transtok(token)) {
			case CM_MAP:
			{
				char *mapnumptr;
				if (scriptfile_getnumber(script, &curmap)) break; mapnumptr = script->ltextptr;
				if (scriptfile_getbraces(script, &braceend)) break;

				// first map file in LevelInfo[] is bogus, last map file is NULL
				if (curmap < 1 || curmap > MAX_LEVELS_REG) {
					initprintf("Error: map number %d not in range 1-%d on line %s:%d\n",
							curmap, MAX_LEVELS_REG, script->filename,
							scriptfile_getlinum(script,mapnumptr));
					script->textptr = braceend;
					break;
				}

				while (script->textptr < braceend) {
					if (!(token = scriptfile_gettoken(script))) break;
					if (token == braceend) break;
					switch (cm_transtok(token)) {
						case CM_FILENAME:
						{
							char *t;
							if (scriptfile_getstring(script, &t)) break;

							if (custommaps[curmap].LevelName) free(custommaps[curmap].LevelName);
							custommaps[curmap].LevelName = strdup(t);
							LevelInfo[curmap].LevelName = custommaps[curmap].LevelName;
							break;
						}
						case CM_SONG:
						{
							char *t;
							if (scriptfile_getstring(script, &t)) break;

							if (custommaps[curmap].SongName) free(custommaps[curmap].SongName);
							custommaps[curmap].SongName = strdup(t);
							LevelInfo[curmap].SongName = custommaps[curmap].SongName;
							break;
						}
						case CM_TITLE:
						{
							char *t;
							if (scriptfile_getstring(script, &t)) break;

							if (custommaps[curmap].Description) free(custommaps[curmap].Description);
							custommaps[curmap].Description = strdup(t);
							LevelInfo[curmap].Description = custommaps[curmap].Description;
							break;
						}
						case CM_BESTTIME:
						{
							int n;
							char s[10];
							if (scriptfile_getnumber(script, &n)) break;

							Bsnprintf(s, 10, "%d : %02d", n/60, n%60);
							if (custommaps[curmap].BestTime) free(custommaps[curmap].BestTime);
							custommaps[curmap].BestTime = strdup(s);
							LevelInfo[curmap].BestTime = custommaps[curmap].BestTime;
							break;
						}
						case CM_PARTIME:
						{
							int n;
							char s[10];
							if (scriptfile_getnumber(script, &n)) break;

							Bsnprintf(s, 10, "%d : %02d", n/60, n%60);
							if (custommaps[curmap].ParTime) free(custommaps[curmap].ParTime);
							custommaps[curmap].ParTime = strdup(s);
							LevelInfo[curmap].ParTime = custommaps[curmap].ParTime;
							break;
						}
						case CM_CDATRACK:
						{
							int n;
							if (scriptfile_getnumber(script, &n)) break;
							break;
						}
						default:
							initprintf("A Error on line %s:%d\n",
									script->filename,
									scriptfile_getlinum(script,script->ltextptr));
							break;
					}
				}
				break;
			}

			case CM_EPISODE:
			{
				char *epnumptr;
				if (scriptfile_getnumber(script, &curmap)) break; epnumptr = script->ltextptr;
				if (scriptfile_getbraces(script, &braceend)) break;

				// first map file in LevelInfo[] is bogus, last map file is NULL
				if (curmap < 1 || curmap > 2) {
					initprintf("Error: episode number %d not in range 1-2 on line %s:%d\n",
							curmap, script->filename,
							scriptfile_getlinum(script,epnumptr));
					script->textptr = braceend;
					break;
				}
				curmap--;

				while (script->textptr < braceend) {
					if (!(token = scriptfile_gettoken(script))) break;
					if (token == braceend) break;
					switch (cm_transtok(token)) {
						case CM_TITLE:
						{
							char *t;
							if (scriptfile_getstring(script, &t)) break;

							strncpy(&EpisodeNames[curmap][1], t, MAX_EPISODE_NAME_LEN);
							EpisodeNames[curmap][MAX_EPISODE_NAME_LEN+1] = 0;
							break;
						}
						case CM_SUBTITLE:
						{
							char *t;
							if (scriptfile_getstring(script, &t)) break;

							strncpy(EpisodeSubtitles[curmap], t, MAX_EPISODE_SUBTITLE_LEN);
							EpisodeSubtitles[curmap][MAX_EPISODE_SUBTITLE_LEN+1] = 0;
							break;
						}
						default:
							initprintf("Error on line %s:%d\n",
									script->filename,
									scriptfile_getlinum(script,script->ltextptr));
							break;
					}
				}
				break;
			}

			case CM_SKILL:
			{
				char *epnumptr;
				if (scriptfile_getnumber(script, &curmap)) break; epnumptr = script->ltextptr;
				if (scriptfile_getbraces(script, &braceend)) break;

				// first map file in LevelInfo[] is bogus, last map file is NULL
				if (curmap < 1 || curmap > 4) {
					initprintf("Error: skill number %d not in range 1-4 on line %s:%d\n",
							curmap, script->filename,
							scriptfile_getlinum(script,epnumptr));
					script->textptr = braceend;
					break;
				}
				curmap--;

				while (script->textptr < braceend) {
					if (!(token = scriptfile_gettoken(script))) break;
					if (token == braceend) break;
					switch (cm_transtok(token)) {
						case CM_TITLE:
						{
							char *t;
							if (scriptfile_getstring(script, &t)) break;

							strncpy(&SkillNames[curmap][1], t, MAX_SKILL_NAME_LEN);
							SkillNames[curmap][MAX_SKILL_NAME_LEN+1] = 0;
							break;
						}
						default:
							initprintf("Error on line %s:%d\n",
									script->filename,
									scriptfile_getlinum(script,script->ltextptr));
							break;
					}
				}
				break;
			}

			case CM_TEXT:
			{
				if (scriptfile_getbraces(script, &braceend)) break;

				while (script->textptr < braceend) {
					if (!(token = scriptfile_gettoken(script))) break;
					if (token == braceend) break;
					switch (cm_transtok(token)) {
						case CM_COOKIE:
						{
							char *t,*fortuneend;
							int fc = 0;

							if (scriptfile_getbraces(script, &fortuneend)) break;

							while (script->textptr < fortuneend) {
								if (scriptfile_getstring(script, &t)) break;

								if (fc == MAX_FORTUNES) continue;

								customfortune[fc] = strdup(t);
								if (customfortune[fc]) ReadFortune[fc] = customfortune[fc];
								fc++;
							}
							break;
						}
						case CM_GOTKEY:
						{
							char *t,*keyend;
							int fc = 0;

							if (scriptfile_getbraces(script, &keyend)) break;

							while (script->textptr < keyend) {
								if (scriptfile_getstring(script, &t)) break;

								if (fc == MAX_KEYS) continue;

								customkeymsg[fc] = strdup(t);
								if (customkeymsg[fc]) KeyMsg[fc] = customkeymsg[fc];
								fc++;
							}
							break;
						}
						case CM_NEEDKEY:
						{
							char *t,*keyend;
							int fc = 0;

							if (scriptfile_getbraces(script, &keyend)) break;

							while (script->textptr < keyend) {
								if (scriptfile_getstring(script, &t)) break;

								if (fc == MAX_KEYS) continue;

								customkeydoormsg[fc] = strdup(t);
								if (customkeydoormsg[fc]) KeyDoorMessage[fc] = customkeydoormsg[fc];
								fc++;
							}
							break;
						}
						case CM_SECRET:
						case CM_QUIT:
						default:
							initprintf("Error on line %s:%d\n",
									script->filename,
									scriptfile_getlinum(script,script->ltextptr));
							break;
					}
				}
				break;
			}

			default:
				initprintf("Error on line %s:%d\n",
						script->filename,
						scriptfile_getlinum(script,script->ltextptr));
				break;
		}
	}

	scriptfile_close(script);
}

