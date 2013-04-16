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

#define RECT_CLIP 1

long MultiClipMove(PLAYERp pp, long z, long floor_dist);
short MultiClipTurn(PLAYERp pp, short new_ang, long z, long floor_dist);
long testquadinsect(long *point_num, long *qx, long *qy, short sectnum);
long RectClipMove(PLAYERp pp, long *qx, long *qy);
long testpointinquad(long x, long y, long *qx, long *qy);
//short RectClipTurn(PLAYERp pp, short new_ang, long z, long floor_dist, long *qx, long *qy);
short RectClipTurn(PLAYERp pp, short new_ang, long *qx, long *qy, long *ox, long *oy);
