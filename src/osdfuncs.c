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
#include "mytypes.h"
#include "keyboard.h"
#include "osdfuncs.h"

void osdfunc_onshowosd(int shown)
{
    extern BOOL GamePaused;
    if (!shown ^ !GamePaused)
        KB_KeyDown[sc_Pause] = 1;
}

#define BGTILE 4930
#define BGSHADE 12
#define BORDTILE 83   // BORDER_TILE
#define BORDSHADE 0
#define BITSTH 1+32+8+16    // high translucency
#define BITSTL 1+8+16   // low translucency
#define BITS 8+16+64        // solid
#define PALETTE 0
void osdfunc_clearbackground(int c, int r)
{
    int x, y, xsiz, ysiz, tx2, ty2;
    int daydim, bits;
    (void)c;

    if (!POLYMOST_RENDERMODE_POLYGL()) bits = BITS; else bits = BITSTL;

    daydim = r*14+4;

    xsiz = tilesizx[BGTILE]; tx2 = xdim/xsiz;
    ysiz = tilesizy[BGTILE]; ty2 = daydim/ysiz;

    for(x=0;x<=tx2;x++)
        for(y=0;y<=ty2;y++)
            rotatesprite(x*xsiz<<16,y*ysiz<<16,65536L,0,BGTILE,BGSHADE,PALETTE,bits,0,0,xdim,daydim);

     xsiz = tilesizy[BORDTILE]; tx2 = xdim/xsiz;
     ysiz = tilesizx[BORDTILE];

     for(x=0;x<=tx2;x++)
         rotatesprite(x*xsiz<<16,(daydim+ysiz+1)<<16,65536L,1536,BORDTILE,BORDSHADE,PALETTE,BITS,0,0,xdim,daydim+ysiz+1);
}
