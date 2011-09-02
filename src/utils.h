#ifndef __UTILS_H__
#define __UTILS_H__

#include <sndfile.h>

/* a simple lookup table */
typedef struct {
    char* name;
    int number;
} KEY_VAL;

typedef struct {
    const KEY_VAL *table;
    int size;
} LOOKUP_TABLE;

/* function to get a value from a look-up table */
int lookup_val(const LOOKUP_TABLE *array, const char *name);

#endif /* __UTILS_H__ */
