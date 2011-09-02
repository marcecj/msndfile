#include <string.h>
#include "utils.h"

/* function to get a value from a look-up table */
int lookup_val(const LOOKUP_TABLE *array, const char *name)
{
    int i;
    for(i = 0; i < array->size; i++) {
        if( strcmp(name, array->table[i].name) == 0 )
            return array->table[i].number;
    }

	return 0;
}
