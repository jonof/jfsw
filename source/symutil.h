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

typedef struct
    {
    char Name[32];
    long Offset;
    }SYM_TABLE, *SYM_TABLEp;

extern int _nullarea;
extern int __begtext;

extern SYM_TABLEp SymTableCode;
extern SYM_TABLEp SymTableData;

extern long SymCountCode;
extern long SymCountData;

extern long SymDataSegNum;
extern long SymCodeSegNum;
extern char SymDataSegPrefix[];
extern char SymCodeSegPrefix[];

extern long SymDataSegOffset;
extern long SymCodeSegOffset;

long filesize( FILE *fp );
unsigned long SymDataPtrToOffset(char *ptr);
unsigned long SymCodePtrToOffset(char *ptr);
void * SymOffsetToDataPtr(long sym_offset);
void * SymOffsetToCodePtr(long sym_offset);
char * NextLine(char *ptr);
char * StartOfLine(char *base_ptr, char *ptr);
void FindDataSegPrefix(char *base_ptr);
void FindCodeSegPrefix(char *base_ptr);
long ParseMapFile(char *base_ptr, SYM_TABLEp SymTable, char *prefix);
SYM_TABLEp SearchSymTableByOffset(SYM_TABLEp SymTable, int count, unsigned long key_sym_offset, unsigned long *offset_from_symbol);
SYM_TABLEp SearchSymTableByName(SYM_TABLEp SymTable, int count, char *key_sym_name);
char * LoadMapFile(char *name, char *base_ptr);
void LoadSymTable(char *name, SYM_TABLEp *SymTable, long *count);
    

