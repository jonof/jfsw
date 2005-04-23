#include "saveable.h"
#include "stdlib.h"

#define maxModules 40

static saveable_module *saveablemodules[maxModules];
static int nummodules = 0;

void Saveable_Init(void)
{
	static int inited = 0;

	if (inited) return;

#define module(x) \
	extern saveable_module saveable_ ##x ; \
	saveablemodules[nummodules++] = &saveable_ ##x

	module(actor);
	module(ai);
	module(build);
	module(bunny);
	module(coolg);
	module(coolie);
	module(eel);
	module(girlninj);
	module(goro);
	module(hornet);
	module(jweapon);
	module(lava);
	module(miscactr);
	module(morph);
	module(ninja);
	module(panel);
	module(player);
	module(quake);
	module(ripper);
	module(ripper2);
	module(rotator);
	module(serp);
	module(skel);
	module(skull);
	module(slidor);
	module(spike);
	module(sprite);
	module(sumo);
	module(track);
	module(vator);
	module(wallmove);
	module(weapon);
	module(zilla);
	module(zombie);

	inited=1;
}

int Saveable_FindCodeSym(void *ptr, savedcodesym *sym)
{
	unsigned int m,i;

	if (!ptr) {
		sym->module = 0;	// module 0 is the "null module" for null pointers
		sym->index  = 0;
		return 0;
	}

	for (m=0; m<nummodules; m++) {
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

	if (!ptr) {
		sym->module = 0;
		sym->index  = 0;
		sym->offset = 0;
		return 0;
	}

	for (m=0; m<nummodules; m++) {
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

int Saveable_RestoreCodeSym(savedcodesym *sym, void **ptr)
{
	if (sym->module == 0) {
		*ptr = NULL;
		return 0;
	}

	if (sym->module > nummodules) return -1;
	if (sym->index  >= saveablemodules[ sym->module-1 ]->numcode) return -1;

	*ptr = saveablemodules[ sym->module-1 ]->code[ sym->index ];

	return 0;
}

int Saveable_RestoreDataSym(saveddatasym *sym, void **ptr)
{
	if (sym->module == 0) {
		*ptr = NULL;
		return 0;
	}

	if (sym->module > nummodules) return -1;
	if (sym->index  >= saveablemodules[ sym->module-1 ]->numdata) return -1;
	if (sym->offset >= saveablemodules[ sym->module-1 ]->data[ sym->index ].size) return -1;

	*ptr = (void*)((unsigned long)saveablemodules[ sym->module-1 ]->data[ sym->index ].base + sym->offset);

	return 0;
}
