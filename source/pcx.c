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

// PCX.C  by Jim Norwood
// PCX file reader.

// I N C L U D E S ////////////////////////////////////////////////////////////////////////////

#if 0
// JBF: not used at the moment

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include "mytypes.h"
//#include "cmdlib.h"
//#include "vga.h"
#include "pcx.h"

// G L O B A L S //////////////////////////////////////////////////////////////////////////////
void TerminateGame(void);

static struct
    {
    char manufacturer;                  // This is always 10.
    char version;                       // 0-Ver 2.5 Paintbrush,
    // 2-Ver 2.8 with palette.
    // 3-Ver 2.8 use default
    // palette, 5-Ver 3.0 or
    // better of Paintbrush.
    char encoding;                      // This is always 1, meaning
    // RLE encoding.
    char bits_per_pixel;                // This is the bits per pixel
    short x, y;                         // This is upper left of image
    short width, height;                // Size of image.
    short horz_res;                     // # pixels in x direction
    short vert_res;                     // # pixels in y direction
    char ega_palette[48];               // EGA palette, ignore it.
    char reserved;                      // Nothing.
    char num_color_planes;              // # of planes in image.
    short bytes_per_line;               // This is the number of bytes
    // per single horizontal line.
    short palette_type;                 // Ignore this too.
    char padding[58];                   // Used for structure alignment, fat.
    } pcx;


void PcxError(char *err_msg)
    {
    TerminateGame();
    printf("%s",err_msg);
    exit(0);
    }

// CTW MODIFICATION ENTIRE FILE
// Converted all "false" and "true" to "FALSE" and "TRUE"
// CTW MODIFICATION ENTIRE FILE END

boolean 
DisplayPCX(char file_name[])
    {
    FILE *fname;
    unsigned char ch, buffer[320];
    int cnt, i, j, k, m, pass, col, row, index = 0, destoff;
    char *screen;
    unsigned char *file_buf;
    unsigned char palette_data[256][3];

    //screen = (char *) VGAMEM;
    screen = (char *) 0xa0000;

    if ((file_buf = (unsigned char *) malloc(65536)) == NULL)
        PcxError("\nOut of memory in restore_screen!\n");

    if ((fname = fopen(file_name, "rb")) == NULL)
        {
        PcxError("\nCan't find file.\n");
        return (0);
        }
    else
        {
        fread(&pcx, 1, 128, fname);
        if (pcx.manufacturer != 0x0A ||
            (pcx.horz_res != 320) || (pcx.vert_res != 200))
            {
            printf("manufacturer = %d\n xres = %d\n yres = %d\n",
                pcx.manufacturer, pcx.horz_res, pcx.vert_res);
            PcxError("Not a valid pcx file or .pcx is not 320x200x256.\n");
            fclose(fname);
            return (FALSE);
            }
        }

    fseek(fname, -769L, SEEK_END);
    ch = fgetc(fname);
    if (ch == 0x0C)
        {
        fread(file_buf, 768, 1, fname);

        // Read in palette data
        for (i = 0, k = 0; i < 256; i++)
            for (j = 0; j < 3; j++)
                palette_data[i][j] = file_buf[k++] >> 2;

        SetPaletteToVESA(&palette_data[0][0]);

        fseek(fname, 128L, SEEK_SET);
        
        if (index == 0)
            fread(file_buf, 65536, 1, fname);
            
        for (row = pcx.y; row <= pcx.height; row++)
            {
            for (col = pcx.x; col <= pcx.width; col++)
                {
                next_char(ch, index);
                
                if ((ch & 0xC0) != 0xC0)
                    pass = 1;
                else
                    {
                    pass = ch & 0x3F;
                    next_char(ch, index);
                    }
                    
                for (m = 0; m < pass; m++)
                    buffer[col++] = ch;
                col--;
                }

            destoff = row * pcx.bytes_per_line + pcx.x;

            for (cnt = 0; cnt < pcx.bytes_per_line; cnt++)
                screen[destoff + cnt] = (char) buffer[cnt];
            }
                
        fclose(fname);
        return (TRUE);
        }
    else
        {
        fclose(fname);
        PcxError("\nNot a 320x200x256 color pcx file.\n");
        return (FALSE);
        }
    }
#endif
