/*
 * Copyright (C) 2010-2014 Marc Joliet
 *
 * Full license information can be found in the file LICENSE in the top-level
 * directory of the source repository.
 */

#include <string.h>
#include <stdlib.h>
#include <sndfile.h>
#include <mex.h>
#include "audio_files.h"

/*
 * audio file info look-up functions
 */

/* create an AUDIO_FILE_INFO struct */
AUDIO_FILE_INFO* create_file_info(const char *const restrict name, SF_INFO* const restrict sf_file_info, SNDFILE* const restrict file)
{
    AUDIO_FILE_INFO* file_info = (AUDIO_FILE_INFO*)malloc(sizeof(AUDIO_FILE_INFO));

    if(file_info == NULL)
        return NULL;

    file_info->info = sf_file_info;
    file_info->file = file;
    file_info->next = NULL;

    file_info->name = (char*)calloc(strlen(name)+1, sizeof(char));
    file_info->name = strcpy(file_info->name, name);

    return file_info;
}

/* append an AUDIO_FILE_INFO structure to an AUDIO_FILE_INFO linked list */
AUDIO_FILES* store_file_info(AUDIO_FILES* restrict file_list, AUDIO_FILE_INFO* const restrict file_info)
{
    AUDIO_FILE_INFO *ptr;

    /* allocate a file_list instance if it does not exist yet */
    if( file_list == NULL ) {
        if( (file_list = (AUDIO_FILES*)malloc(sizeof(AUDIO_FILES))) == NULL )
            return NULL;
        file_list->first = NULL;
    }

    /* if the list is empty set the file to the first element */
    if( (ptr = file_list->first) == NULL ) {
        file_list->first = file_info;
        return file_list;
    }

    /* if the file name is not stored yet, append it to the list */
    if( lookup_file_info(file_list, file_info->name) == NULL ) {
        while( ptr->next != NULL )
            ptr = ptr->next;
        ptr->next = file_info;
    }

    return file_list;
}

/* Get an AUDIO_FILE_INFO structure from an AUDIO_FILES linked list
 * Returns NULL if the file is not open. */
AUDIO_FILE_INFO* lookup_file_info(AUDIO_FILES* restrict file_list, const char *const restrict name)
{
    AUDIO_FILE_INFO *ptr;

    if( file_list == NULL )
        return NULL;

    for(ptr = file_list->first; ptr != NULL; ptr = ptr->next)
        if( strcmp(name, ptr->name) == 0 )
            break;

    return ptr;
}

/* remove an AUDIO_FILE_INFO structure from an AUDIO_FILES linked list */
int remove_file_info(AUDIO_FILES* restrict file_list, const char *const restrict name)
{
    AUDIO_FILE_INFO *ptr=NULL, *prev=NULL;

    /* there are no open files */
    if( file_list == NULL )
        return -1;

    for( ptr=file_list->first, prev=ptr; ptr != NULL; prev=ptr, ptr=ptr->next )
        if( strcmp(name, ptr->name) == 0 )
            break;

    /* the file is not open */
    if( ptr == NULL )
        return -1;

    prev->next = ptr->next;
    /* Special case: if the ptr is at the first element, we need to
     * update the pointer to the first element. */
    if( ptr == file_list->first )
        file_list->first = ptr->next;

    destroy_file_info(ptr);

    return 0;
}

/* deallocate an AUDIO_FILE_INFO structure */
void destroy_file_info(AUDIO_FILE_INFO* restrict file_info)
{
    if( file_info != NULL ) {
        int status;

        free(file_info->name);
        free(file_info->info);

        /* TODO: what to do here? */
        if( (status = sf_close(file_info->file)) == 0 )
            file_info->file = NULL;
        else {
            mexWarnMsgIdAndTxt("msndfile:sndfile", "libsndfile could not close the file!");
            mexWarnMsgIdAndTxt("msndfile:sndfile", sf_error_number(status));
        }

        free(file_info);
    } else
        mexWarnMsgTxt("File already removed! This is odd.");
}

/* deallocate an AUDIO_FILES linked list */
AUDIO_FILES* destroy_file_list(AUDIO_FILES* restrict file_list)
{
    if( file_list != NULL ) {
        AUDIO_FILE_INFO *ptr, *next;

        for( ptr = file_list->first; ptr != NULL; ptr=next ) {
            next = ptr->next;
            destroy_file_info(ptr);
        }

        free(file_list);
    }

    return NULL;
}
