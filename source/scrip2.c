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
void Shutdown(void)
{
}
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


