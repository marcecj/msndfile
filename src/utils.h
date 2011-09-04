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
} FMT_TABLE;

/* function to get a value from a look-up table */
int lookup_val(const FMT_TABLE *array, const char *name);

/* function that gets the information on a file from the args pointer and
 * transfers it to the sf_file_info struct */
void get_file_info(SF_INFO* sf_file_info, char* sf_in_fname, const mxArray const* args);

#endif /* __UTILS_H__ */
