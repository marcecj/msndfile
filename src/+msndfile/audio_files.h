#ifndef __AUDIO_FILE_LOOKUP_H__
#define __AUDIO_FILE_LOOKUP_H__

#include <sndfile.h>

/* a simple lookup table that associates the name of an open file with
 * information about it stored in SF_INFO and its SNDFILE pointer */
typedef struct {
    char* name;
    SF_INFO* info;
    SNDFILE* file;
} AUDIO_FILE_INFO;

typedef struct {
    AUDIO_FILE_INFO **files;
    int num_files;
} AUDIO_FILES;

/*
 * audio file info look-up functions
 */

/* create an AUDIO_FILE_INFO struct */
AUDIO_FILE_INFO* create_file_info(const char *const name, SF_INFO* file_info, SNDFILE* file);

/* add an AUDIO_FILE_INFO structure to an AUDIO_FILES look-up table */
AUDIO_FILES* store_file_info(AUDIO_FILES* array, AUDIO_FILE_INFO* file_info);

/* Get an AUDIO_FILE_INFO structure from an AUDIO_FILES look-up table.  Returns
 * NULL if the file is not open. */
AUDIO_FILE_INFO* lookup_file_info(const AUDIO_FILES *const array, const char *const name);

/* remove an AUDIO_FILE_INFO structure from an AUDIO_FILES look-up table */
AUDIO_FILES* remove_file_info(AUDIO_FILES *array, const char *const name);

/* deallocate an AUDIO_FILE_INFO structure */
AUDIO_FILE_INFO* destroy_file_info(AUDIO_FILE_INFO* file_info);

/* deallocate an AUDIO_FILES look-up table */
void destroy_file_list(AUDIO_FILES* array);

#endif /* __AUDIO_FILE_LOOKUP_H__ */
