#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include "stdint_compat.h"
#endif
#include <string.h>
#include <stdio.h>
#include <mex.h>
#include <sndfile.h>
#include "utils.h"
#include "format_tables.h"

/*
 * libsndfile format look-up functions
 */

/* the max length of a format string (9 for "IMA_ADPCM" & "VOX_ADPCM") + \0 */
#define FMT_STR_SIZE 10

/* function to get a value from a look-up table */
int lookup_val(const FMT_TABLE *const array, const char *const name)
{
    int i;
    for(i = 0; i < array->size; i++)
        if( strcmp(name, array->table[i].name) == 0 )
            return array->table[i].number;

    return 0;
}

/*
 * misc functions
 */

/* returns the number of simple formats + RAW */
unsigned int get_num_formats()
{
    int num_formats;

    sf_command(0, SFC_GET_SIMPLE_FORMAT_COUNT, &num_formats, sizeof(int));

    /* SFC_GET_SIMPLE_FORMAT returns the highest valid format ID (i.e., 0 <=
     * format <= ID) , so increment by 1 to get a standard C count (0 <= format
     * < ID); furthermore, RAW is missing, as it is not a simple format, but we
     * handle it anyway, so another +1 */
    return num_formats+2;
}

/* returns a list of file extensions to simple formats + RAW */
char** get_format_extensions()
{
    int i;
    const int num_formats = get_num_formats();
    char** file_exts = (char**)malloc(num_formats*sizeof(char*));
    SF_FORMAT_INFO format_info;

    /* handle the libsndfile simple formats */
    for( i = 0; i < num_formats-1; i++ ) {
        format_info.format = i;

        sf_command(0, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof(SF_FORMAT_INFO));

        file_exts[i] = (char*)malloc((strlen(format_info.extension)+1)*sizeof(char));
        file_exts[i] = strcpy(file_exts[i], format_info.extension);
    }

    /* RAW is not a simple format, but we want to handle it */
    file_exts[num_formats-1] = (char*)malloc(4*sizeof(char));
    file_exts[num_formats-1] = strcpy(file_exts[i], "raw");

    return file_exts;
}

/* helper function for gen_filename(): return whether a file extension was
 * already checked */
unsigned int ext_already_checked(char** extensions, const char* ext, const unsigned int num_ext)
{
    size_t i;
    for( i = 0; i < num_ext; i++ )
        if( strcmp(extensions[i], ext) == 0 )
            return 1;

    return 0;
}

/* function to get a valid file name; for wavread() compatibility, if the file
 * name does not have a suffix, file_name+".wav" is attempted, and if that
 * fails, NULL is returned */
char* gen_filename(char* fname)
{
    const size_t N           = strlen(fname);
    const size_t num_formats = get_num_formats();
    char** file_exts         = NULL;
    char** read_exts         = NULL;
    FILE* audio_file         = NULL;
    size_t num_read_exts     = 0;
    size_t num_files         = 0; /* file name ambiguity if num_files>1 */
    size_t i;

    /* if the file name (probably) has a suffix, the file name is OK */
    if( strrchr(fname, '.') != NULL )
        goto get_filename_cleanup;

    /*
     * For each file type extension, append it to the original file name and
     * check if such a file exists.  In the case of multiple candidates, default
     * to WAV.  If no WAV file exists, return NULL.
     */

    file_exts = get_format_extensions();

    for( i = 0; i < num_formats; i++ ) {
        char* tmp_fname      = NULL;
        const char* cur_ext  = file_exts[i];
        const size_t ext_len = strlen(cur_ext)+1; /* '.' + extension */
        const size_t new_len = N+ext_len+1;

        /* get_format_extensions() returns duplicate entries, so check here if
         * the extension has already been tried */
        if( ext_already_checked(read_exts, cur_ext, num_read_exts) )
            continue;

        /* append the current extension to the list of checked extensions */
        read_exts = (char**)realloc(read_exts, (num_read_exts+1)*sizeof(char*));
        read_exts[num_read_exts] = (char*)malloc(ext_len*sizeof(char));
        read_exts[num_read_exts] = strcpy(read_exts[num_read_exts], cur_ext);
        num_read_exts++;

        /* copy the original N chars from fname into tmp_fname */
        tmp_fname = (char*)calloc(new_len, sizeof(char));
        tmp_fname = strncpy(tmp_fname, fname, N);

        /* append the file type extension */
        tmp_fname = strcat(tmp_fname, ".");
        tmp_fname = strcat(tmp_fname, cur_ext);

        /* try to open the file; continue with next extension on failure */
        if( (audio_file = fopen(tmp_fname, "r")) == NULL ) {
            free(tmp_fname);
            continue;
        }

        /*  overwrite the original file name */
        fclose(audio_file); /* close temporary file */
        num_files++;
        fname = (char*)mxRealloc(fname, new_len*sizeof(char));
        fname = strcpy(fname, tmp_fname);
        free(tmp_fname);

        /* break as soon as a WAV file is found */
        if( strcmp(cur_ext, "wav") == 0 )
            break;
    }

get_filename_cleanup:
    if( read_exts ) {
        for( i = 0; i < num_read_exts; i++ )
            free(read_exts[i]);
        free(read_exts);
    }

    if( file_exts ) {
        for( i = 0; i < num_formats; i++ )
            free(file_exts[i]);
        free(file_exts);
    }

    /* multiple candidates were found, but no WAV file */
    if( num_files > 1 && strcmp(&fname[strlen(fname)-3], "wav") != 0 ) {
        mxFree(fname);
        fname = NULL;
    }

    if( num_files > 1 && fname != NULL ) {
        const char msg_fmt[] = "Defaulted to file name \"%s\".";
        const size_t msg_len = strlen(fname) + strlen(msg_fmt) - 1;
        char* message = (char*)malloc(msg_len*sizeof(char));

        sprintf(message, msg_fmt, fname);
        mexWarnMsgIdAndTxt("msndfile:read:ambiguousname", message);

        free(message);
    }

    return fname;
}

/*
 * return a transposed version of "input" as "output"
 *
 * TODO: maybe do an in-place transpose? Files already open in about 2/3 of
 * the time of Matlab's wavread(), so some additional time complexity
 * probably won't hurt much.
 */
void transpose_data(void* output, void* input, int num_frames, int num_chns, mxClassID class_id)
{
    int i; /* loop variable */

    /* transpose the data
     *
     * To transpose correctly, we need to cast both the input and the output.
     * Sadly I can't think of any other way to it than below without violating
     * ANSI C.  (In C99 I could probably just define pointers that point to cast
     * versions of input and output and use only one loop.)
     */
    switch ( class_id ) {
        case mxINT8_CLASS:
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    ((int8_t*)output)[i+j*num_frames] = ((int8_t*)input)[i*num_chns+j];
            }
            break;
        case mxUINT8_CLASS:
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    ((uint8_t*)output)[i+j*num_frames] = ((uint8_t*)input)[i*num_chns+j];
            }
            break;
        case mxINT16_CLASS:
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    ((int16_t*)output)[i+j*num_frames] = ((int16_t*)input)[i*num_chns+j];
            }
            break;
        case mxINT32_CLASS:
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    ((int32_t*)output)[i+j*num_frames] = ((int32_t*)input)[i*num_chns+j];
            }
            break;
        case mxSINGLE_CLASS:
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    ((float*)output)[i+j*num_frames] = ((float*)input)[i*num_chns+j];
            }
            break;
        default:
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    ((double*)output)[i+j*num_frames] = ((double*)input)[i*num_chns+j];
            }
            break;
    }
}

/* get information about a file from an args pointer and transfer it to an
 * SF_INFO struct */
void get_file_info(SF_INFO* sf_file_info, char* sf_in_fname, const mxArray *const args)
{
    /* a temporary array */
    mxArray *tmp_ptr = NULL;

    /* the three OR-ed components of the "format" field in sf_file_info */
    char maj_fmt_name[FMT_STR_SIZE] = "RAW";
    char sub_fmt_name[FMT_STR_SIZE];
    char endianness_name[FMT_STR_SIZE] = "FILE";

    /*
     * get the sample rate and the number of channels
     */

    if( (tmp_ptr = mxGetField(args, 0, "samplerate" )) != NULL )
        sf_file_info->samplerate = (int)*mxGetPr(tmp_ptr);
    else {
        mxFree(sf_in_fname);
        mexErrMsgIdAndTxt("msndfile:argerror", "Field 'samplerate' not set.");
    }

    if( (tmp_ptr = mxGetField(args, 0, "channels" )) != NULL )
        sf_file_info->channels = (int)*mxGetPr(tmp_ptr);
    else {
        mxFree(sf_in_fname);
        mexErrMsgIdAndTxt("msndfile:argerror", "Field 'channels' not set.");
    }

    /*
     * get the format information
     */

    /* format name should be set to RAW when reading RAW files */
    if( (tmp_ptr = mxGetField(args, 0, "format" )) != NULL )
        mxGetString(tmp_ptr, maj_fmt_name, FMT_STR_SIZE);

    if( (tmp_ptr = mxGetField(args, 0, "sampleformat" )) != NULL )
        mxGetString(tmp_ptr, sub_fmt_name, FMT_STR_SIZE);
    else {
        mxFree(sf_in_fname);
        mexErrMsgIdAndTxt("msndfile:argerror", "Field 'sampleformat' not set.");
    }

    /* endianness_name does not need to be set */
    if( (tmp_ptr = mxGetField(args, 0, "endianness" )) != NULL )
        mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);

    sf_file_info->format = lookup_val(&maj_fmts, maj_fmt_name)
        | lookup_val(&sub_fmts, sub_fmt_name)
        | lookup_val(&endianness_types, endianness_name);

    /* check format for validity */
    if( !sf_format_check(sf_file_info) ) {
        mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
        mxFree(sf_in_fname);
        mexErrMsgIdAndTxt("msndfile:argerror", "Invalid format specified.");
    }
}

/* get the number of frames to be read and check for valid ranges */
int get_num_frames(const SF_INFO* const sf_file_info, SNDFILE* sf_input_file, const mxArray *const arg)
{
    const double *start_end_idx = mxGetPr(arg);
    const int    range_size     = mxGetN(arg);
    int          num_frames     = 0;

    if( range_size == 2 ) {
        num_frames = (sf_count_t)(start_end_idx[1] - start_end_idx[0] + 1);

        if( sf_seek(sf_input_file, start_end_idx[0]-1, SEEK_SET) < 0
                || start_end_idx[1] > sf_file_info->frames )
            mexErrMsgIdAndTxt("msndfile:argerror", "Invalid range!");
    }
    else if( range_size == 1 ) {
        num_frames = (sf_count_t)(start_end_idx[0]);
        if( num_frames > sf_file_info->frames )
            mexErrMsgIdAndTxt("msndfile:argerror", "num_frames too large!");
    }
    else
        mexErrMsgIdAndTxt("msndfile:argerror",
                          "Range can be a row vector with 1 or 2 elements.");

    return num_frames;
}

/* check the fmt argument and return true or false */
int get_fmt(const char* const args)
{
    int do_read_raw = -1;

    if( strcmp(args, "native") == 0 )
        do_read_raw = 1;
    else if( strcmp(args, "double") == 0 )
        do_read_raw = 0;

    return do_read_raw;
}

/* get the mxClassID corresponding to a format subtype */
mxClassID get_class_id(SF_INFO* sf_file_info)
{
    /* TODO: What other formats should I add here? */
    switch( sf_file_info->format & SF_FORMAT_SUBMASK )
    {
        case SF_FORMAT_PCM_S8:
            return mxINT8_CLASS;
        case SF_FORMAT_PCM_16:
            return mxINT16_CLASS;
        case SF_FORMAT_PCM_24:
        case SF_FORMAT_PCM_32:
            return mxINT32_CLASS;
        case SF_FORMAT_PCM_U8:
            return mxUINT8_CLASS;
        case SF_FORMAT_FLOAT:
            return mxSINGLE_CLASS;
        case SF_FORMAT_DOUBLE:
            return mxDOUBLE_CLASS;
    }

    /* default to 32bit int */
    return mxINT32_CLASS;
}
