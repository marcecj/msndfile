/*
 * Copyright (C) 2010-2015 Marc Joliet
 *
 * Full license information can be found in the file LICENSE in the top-level
 * directory of the source repository.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <uchar.h>
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
int lookup_val(const FMT_TABLE *const restrict array, const char *const restrict name)
{
    for(int i = 0; i < array->size; i++)
        if( strcmp(name, array->table[i].name) == 0 )
            return array->table[i].number;

    return 0;
}

/*
 * misc functions
 */

/*
 * return a transposed version of "input" as "output"
 *
 * TODO: maybe do an in-place transpose? Files already open in about 2/3 of
 * the time of Matlab's wavread(), so some additional time complexity
 * probably won't hurt much.
 */
#define TRANSPOSE_DATA(out, in, m, n, T) { \
    for( int i=0; i<m; i++ ) \
        for( int j=0; j<n; j++ ) \
            ((T*)out)[i+j*m] = ((T*)in)[i*n+j]; \
}
void transpose_data(const void* restrict output, const void* restrict input, const int num_frames, const int num_chns, const mxClassID class_id)
{
    switch ( class_id ) {
        case mxINT8_CLASS:
            TRANSPOSE_DATA(output, input, num_frames, num_chns, int8_t);
            break;
        case mxUINT8_CLASS:
            TRANSPOSE_DATA(output, input, num_frames, num_chns, uint8_t);
            break;
        case mxINT16_CLASS:
            TRANSPOSE_DATA(output, input, num_frames, num_chns, int16_t);
            break;
        case mxINT32_CLASS:
            TRANSPOSE_DATA(output, input, num_frames, num_chns, int32_t);
            break;
        case mxSINGLE_CLASS:
            TRANSPOSE_DATA(output, input, num_frames, num_chns, float);
            break;
        default:
            TRANSPOSE_DATA(output, input, num_frames, num_chns, double);
            break;
    }
}

/* get information about a file from an args pointer and transfer it to an
 * SF_INFO struct */
void get_file_info(SF_INFO* restrict sf_file_info, char* restrict sf_in_fname, const mxArray *const restrict args)
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
int get_num_frames(const SF_INFO* const restrict sf_file_info, SNDFILE* restrict sf_input_file, const mxArray *const restrict arg)
{
    const double * const start_end_idx = mxGetPr(arg);
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
int get_fmt(const char* const restrict args)
{
    int do_read_raw = -1;

    if( strcmp(args, "native") == 0 )
        do_read_raw = 1;
    else if( strcmp(args, "double") == 0 )
        do_read_raw = 0;

    return do_read_raw;
}

/* get the mxClassID corresponding to a format subtype */
mxClassID get_class_id(const SF_INFO* const restrict sf_file_info)
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
