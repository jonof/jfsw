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

// PCX.H by Jim Norwood
// Contains header information for the pcx reader.

#ifndef _PCX_H

        #define _PCX_H        

// D E F I N E S //////////////////////////////////////////////////////////////////////////////

// Explanation of PCX bit fields.
// OR these together to add in each operation you want
#define PCX_RPAL        0x01                    // Read palette but don't set it
#define PCX_SPAL        0x02                    // Read and set palette
#define PCX_INFO        0x04                    // Display pcx header info
#define PCX_DISP        0x08                    // Blit image data to VGA memory
#define PCX_BUFF        0x10                    // Store image data to screen_buff

// M A C R O S ////////////////////////////////////////////////////////////////////////////////

// Macro which which indexes data
#define next_char(ch, index) {  ch = file_buf[index];   \
                                index++;}

// S T R U C T U R E S ////////////////////////////////////////////////////////////////////////      

/*
typedef struct RGB_color_typ
{
        unsigned char red;
        unsigned char green;
        unsigned char blue;
} RGB_color, *RGB_color_ptr;               
*/

// P R O T O T Y P E S //////////////////////////////////////////////////////////////////////////

boolean  restore_screen(char file_name[], char opcode);

// E X T E R N S ////////////////////////////////////////////////////////////////////////////////


#endif
