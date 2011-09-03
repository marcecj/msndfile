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

/* function to get the information on a RAW file from the args pointer and
 * transfer it to the sf_file_info struct */
void get_raw_info(SF_INFO* sf_file_info, char* sf_in_fname, const mxArray const* args);

#endif /* __UTILS_H__ */
