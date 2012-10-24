#ifndef __AUDIO_FILE_LOOKUP_H__
#define __AUDIO_FILE_LOOKUP_H__

#include <sndfile.h>

/* a simple lookup table that associates the name of an open file with
 * information about it stored in SF_INFO and its SNDFILE pointer */
struct audio_file_info {
    char* name;
    SF_INFO* info;
    SNDFILE* file;
	struct audio_file_info* next;
};

typedef struct audio_file_info AUDIO_FILE_INFO;

typedef struct {
	AUDIO_FILE_INFO *first;
} AUDIO_FILES;

/*
 * audio file info look-up functions
 */

/* create an AUDIO_FILE_INFO struct */
AUDIO_FILE_INFO* create_file_info(const char *const name, SF_INFO* file_info, SNDFILE* file);

/* add an AUDIO_FILE_INFO structure to an AUDIO_FILES linked list */
AUDIO_FILES* store_file_info(AUDIO_FILES* file_list, AUDIO_FILE_INFO* file_info);

/* Get an AUDIO_FILE_INFO structure from an AUDIO_FILES linked list  Returns
 * NULL if the file is not open. */
AUDIO_FILE_INFO* lookup_file_info(AUDIO_FILES* file_list, const char *const name);

/* remove an AUDIO_FILE_INFO structure from an AUDIO_FILES linked list */
AUDIO_FILES* remove_file_info(AUDIO_FILES* file_list, const char *const name);

/* deallocate an AUDIO_FILE_INFO structure */
void destroy_file_info(AUDIO_FILE_INFO* file_info);

/* deallocate an AUDIO_FILES linked list */
AUDIO_FILES* destroy_file_list(AUDIO_FILES* file_list);

#endif /* __AUDIO_FILE_LOOKUP_H__ */
