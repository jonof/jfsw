#include "saveable.h"
#include "stdlib.h"

extern saveable_module
	saveable_build,
	saveable_player,
	saveable_panel
	;

#define maxModules 100

static saveable_module *saveablemodules[maxModules];


void Saveable_Init(void)
{
	static int inited = 0;
	int di=0;

	if (inited) return;

#define module(x) saveablemodules[di++] = &saveable_ ##x

	module(build);
	module(player);
	module(panel);

	inited=1;
}

int Saveable_FindCodeSym(void *ptr, savedcodesym *sym)
{
	unsigned int m,i;

	Saveable_Init();
	
	if (!ptr) {
		sym->module = 0;	// module 0 is the "null module" for null pointers
		sym->index  = 0;
		return 0;
	}

	for (m=0; m<maxModules; m++) {
		if (!saveablemodules[m]) break;
		for (i=0; i<saveablemodules[m]->numcode; i++) {
			if (ptr != saveablemodules[m]->code[i]) continue;

			sym->module = 1+m;
			sym->index  = i;

			return 0;
		}
	}
			
	return -1;
}

int Saveable_FindDataSym(void *ptr, saveddatasym *sym)
{
	unsigned int m,i;

	Saveable_Init();
	
	if (!ptr) {
		sym->module = 0;
		sym->index  = 0;
		sym->offset = 0;
		return 0;
	}

	for (m=0; m<maxModules; m++) {
		if (!saveablemodules[m]) break;
		for (i=0; i<saveablemodules[m]->numdata; i++) {
			if (ptr < saveablemodules[m]->data[i].base) continue;
			if (ptr >= (void*)((unsigned long)saveablemodules[m]->data[i].base +
						saveablemodules[m]->data[i].size)) continue;

			sym->module = 1+m;
			sym->index  = i;
			sym->offset = (unsigned long)(ptr - saveablemodules[m]->data[i].base);

			return 0;
		}
	}
	return -1;
}


#include "build.h"

saveable_data saveable_build_data[] = {
	SAVE_DATA(sector),
	SAVE_DATA(sprite),
	SAVE_DATA(wall)
};

saveable_module saveable_build = {
	// code
	NULL,
	0,

	// data
	saveable_build_data,
	NUM_SAVEABLE_ITEMS(saveable_build_data)
};
