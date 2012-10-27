#include <string.h>
#include <stdlib.h>
#include <sndfile.h>
#include <mex.h>
#include "audio_files.h"

/*
 * audio file info look-up functions
 */

/* create an AUDIO_FILE_INFO struct */
AUDIO_FILE_INFO* create_file_info(const char *const name, SF_INFO* sf_file_info, SNDFILE* file)
{
    AUDIO_FILE_INFO* file_info;

    if((file_info = (AUDIO_FILE_INFO*)malloc(sizeof(AUDIO_FILE_INFO))) == NULL)
        return NULL;

    file_info->info = sf_file_info;
    file_info->file = file;

    file_info->name = (char*)calloc(strlen(name)+1, sizeof(char));
    file_info->name = strcpy(file_info->name, name);

    return file_info;
}

/* add an AUDIO_FILE_INFO structure to an AUDIO_FILES look-up table */
AUDIO_FILES* store_file_info(AUDIO_FILES *array, AUDIO_FILE_INFO *file_info)
{
    /* create a new AUDIO_FILES* array if it does not exist */
    if( !array ) {
        if( !(array = (AUDIO_FILES*)malloc(sizeof(AUDIO_FILES))) )
            return NULL;
        array->num_files = 0;
        array->files = (AUDIO_FILE_INFO**)malloc(sizeof(AUDIO_FILE_INFO*));
    }

    /* if the file name is not stored yet, append it to the array */
    if( !lookup_file_info(array, file_info->name) ) {
        if( array->num_files > 0 )
            array->files = (AUDIO_FILE_INFO**)realloc(array->files, (array->num_files+1)*sizeof(AUDIO_FILE_INFO*));
        array->num_files++;

        array->files[array->num_files-1] = file_info;
    }

    return array;
}

/* Get an AUDIO_FILE_INFO structure from an AUDIO_FILES look-up table.
 * Returns NULL if the file is not open. */
AUDIO_FILE_INFO* lookup_file_info(const AUDIO_FILES *const array, const char *const name)
{
    int i;

    if( !array )
        return NULL;

    for(i = 0; i < array->num_files; i++)
        if( strcmp(name, array->files[i]->name) == 0 )
            return array->files[i];

    return NULL;
}

/* remove an AUDIO_FILE_INFO structure from an AUDIO_FILES look-up table */
AUDIO_FILES* remove_file_info(AUDIO_FILES *array, const char *const name)
{
    int i=0;

    if( !array ) {
        mexWarnMsgIdAndTxt("blockread:filenotopen", "File not open.");
        return array;
    }

    while( i < array->num_files && strcmp(name, array->files[i]->name) != 0 )
        i++;

    if( i < array->num_files )
    {
        array->files[i] = destroy_file_info(array->files[i]);

        /* replace the deleted element with the last one */
        array->files[i] = array->files[array->num_files-1];

        /* resize the array */
        array->files = (AUDIO_FILE_INFO**)realloc(array->files, (--array->num_files)*sizeof(AUDIO_FILE_INFO*));

        /* if array->files now has zero elements, it will have an address of
         * NULL, so allocate it with one element */
        if( array->num_files < 1 )
            array->files = (AUDIO_FILE_INFO**)malloc(sizeof(AUDIO_FILE_INFO*));
    }
    else
        mexWarnMsgIdAndTxt("blockread:filenotopen", "File not open.");

    return array;
}

/* deallocate an AUDIO_FILE_INFO structure */
AUDIO_FILE_INFO* destroy_file_info(AUDIO_FILE_INFO* file_info)
{
    int status;

    if( !file_info )
        mexWarnMsgTxt("File already removed! This is odd.");

    free(file_info->name);
    free(file_info->info);

    /* TODO: what to do here? */
    if( (status = sf_close(file_info->file)) == 0 )
        file_info->file = NULL;
    else {
        mexWarnMsgTxt("libsndfile could not close the file! Deallocating structure anyway.");
        mexWarnMsgTxt(sf_error_number(status));
    }


    free(file_info);

    return file_info;
}

/* deallocate an AUDIO_FILES look-up table */
void destroy_file_list(AUDIO_FILES* array)
{
    if( array ) {
        int i;
        for( i = 0; i < array->num_files; i++ )
            array->files[i] = destroy_file_info(array->files[i]);
    }
    free(array);
}
