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
#include "compat.h"
#include "cache1d.h"
#include "symutil.h"

SYM_TABLEp SymTableCode = NULL;
SYM_TABLEp SymTableData = NULL;

long SymCountCode = 0;
long SymCountData = 0;

long SymDataSegNum = 3;
long SymCodeSegNum = 1;
char SymDataSegPrefix[16] = "0003:";
char SymCodeSegPrefix[16] = "0001:";

// thats what these default to for some reason
long SymDataSegOffset = 0;
long SymCodeSegOffset = 3;

void *CallocMem(int size, int num);

long filesize( FILE *fp )
  {
    long save_pos, size_of_file;

    save_pos = ftell( fp );
    fseek( fp, 0L, SEEK_END );
    size_of_file = ftell( fp );
    fseek( fp, save_pos, SEEK_SET );
    return( size_of_file );
  }
    

int _nullarea = 0;	// XXX: This all needs to be reevaluated!
int __begtext = 0;


////////////////////////////////////////////////////////////////////////////////    
//
//  Get unrelocated address
//    
////////////////////////////////////////////////////////////////////////////////    
    
unsigned long SymDataPtrToOffset(char *ptr)
    {
    return((unsigned long)(ptr - ((char *)(&_nullarea)) + SymDataSegOffset));
    }

unsigned long SymCodePtrToOffset(char *ptr)
    {
    return((unsigned long)(ptr - ((char *)(&__begtext)) + SymCodeSegOffset));
    }

////////////////////////////////////////////////////////////////////////////////    
//
//  Get pointers from symbol offsets
//    
////////////////////////////////////////////////////////////////////////////////    
    
void * SymOffsetToDataPtr(long sym_offset)
    {
    return(((char *)(&_nullarea)) + sym_offset - SymDataSegOffset);
    }

void * SymOffsetToCodePtr(long sym_offset)
    {
    return(((char *)(&__begtext)) + sym_offset - SymCodeSegOffset);
    }
    
////////////////////////////////////////////////////////////////////////////////    
//
//  Search Sym Table by offset
//    
////////////////////////////////////////////////////////////////////////////////    
    
// might be used for bsearch
int LongCompare(long *key, long *base)
    {    
    return(*key - *base);
    }                          
                              
SYM_TABLEp SearchSymTableByOffset(SYM_TABLEp SymTable, int count, unsigned long key_sym_offset, unsigned long *offset_from_symbol)
    {    
    short ndx;
    
    *offset_from_symbol = 0;
    
    // should convert this linear search to a binary search
    for (ndx = 0; ndx < count; ndx++)
        {
        if (SymTable[ndx].Offset == key_sym_offset)
            return(&SymTable[ndx]);
        else
        if (SymTable[ndx].Offset > key_sym_offset)
            {
            // the address is not exactly equal to the symbol
            // back up the index one - we passed it
            ndx--;
            
            // return the offset from the symbol also
            *offset_from_symbol = key_sym_offset - SymTable[ndx].Offset;
            return(&SymTable[ndx]);
            }
        }    
    
    return(NULL);
    }                          
    
////////////////////////////////////////////////////////////////////////////////    
//
//  Search Sym Table by symbol NAME
//    
////////////////////////////////////////////////////////////////////////////////    
    
SYM_TABLEp SearchSymTableByName(SYM_TABLEp SymTable, int count, char *key_sym_name)
    {    
    short i;
    
    // should convert this linear search to a binary search
    for (i = 0; i < count; i++)
        {
        if (strcmp(SymTable[i].Name, key_sym_name) == 0)
            return(&SymTable[i]);
        }    
    
    return(NULL);
        
    }                          
    
////////////////////////////////////////////////////////////////////////////////    
//
//  Load & Save SymTable
//    
////////////////////////////////////////////////////////////////////////////////    
    
#if 1    
void LoadSymTable(char *name, SYM_TABLEp *SymTable, long *count)
    {
    long handle;
    long size;

    handle = kopen4load(name, 0);

    if (handle == -1)
        {
        *count = -1;
        return;
        }

    size = kfilelength(handle);
    
    if (*SymTable == NULL)
        *SymTable = CallocMem(size,1);
    
    kread(handle, *SymTable, size);
    
    kclose(handle);
    
    if (strcmp((*SymTable)[0].Name, "__nullarea") == 0)
        SymDataSegOffset = (*SymTable)[0].Offset;
    else    
        SymCodeSegOffset = (*SymTable)[0].Offset;
        
    *count = size/sizeof(SYM_TABLE);
    }
#endif    

#if 0
void LoadSymTable(char *name, SYM_TABLEp SymTable, long *count)
    {
    long handle;
    long size;

    handle = kopen4load(name, 0);

    if (handle == -1)
        {
        *count = -1;
        return;
        }

    size = kfilelength(handle);
    
    kread(handle, SymTable, size);
    
    kclose(handle);
    
    if (strcmp(SymTable[0].Name, "__nullarea") == 0)
        SymDataSegOffset = SymTable[0].Offset;
    else    
        SymCodeSegOffset = SymTable[0].Offset;
        
    *count = size/sizeof(SYM_TABLE);
    }
#endif    
    
#if 0    
void LoadSymTable(char *name, SYM_TABLEp SymTable, long *count)
    {
    FILE *fin;
    long ndx = 0;
    
    fin = fopen(name, "r");
    
    if (!fin)
        {
        *count = -1;
        return;
        }

    while (fscanf(fin, "%s %x\n", SymTable[ndx].Name, &SymTable[ndx].Offset) != EOF)
        {
        ndx++;
        }    
        
    fclose(fin);
    
    if (strcmp(SymTable[0].Name, "__nullarea") == 0)
        SymDataSegOffset = SymTable[0].Offset;
    else    
        SymCodeSegOffset = SymTable[0].Offset;
    
    *count = ndx;
    }
#endif
    
