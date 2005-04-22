#ifndef SAVEABLE_H
#define SAVEABLE_H

typedef void * saveable_code;

typedef struct {
	void *base;
	unsigned long size;
} saveable_data;

typedef struct {
	saveable_code *code;
	unsigned long numcode;
	
	saveable_data *data;
	unsigned long numdata;
} saveable_module;

#define SAVE_CODE(s) (void*)s
#define SAVE_DATA(s) { (void*)&s, sizeof(s) }

#define NUM_SAVEABLE_ITEMS(x) (sizeof(x)/sizeof(x[0]))

typedef struct {
	unsigned long module;
	unsigned long index;
} savedcodesym;

typedef struct {
	unsigned long module;
	unsigned long index;
	unsigned long offset;
} saveddatasym;

void Saveable_Init(void);

int Saveable_FindCodeSym(void *ptr, savedcodesym *sym);
int Saveable_FindDataSym(void *ptr, saveddatasym *sym);

int Saveable_RestoreCodeSym(savedcodesym *sym, void **ptr);
int Saveable_RestoreDataSym(saveddatasym *sym, void **ptr);

#endif
