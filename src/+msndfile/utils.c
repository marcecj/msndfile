#include <stdint.h>
#include <string.h>
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

    tmp_ptr = mxGetField(args, 0, "samplerate" );
    if( tmp_ptr != NULL )
        sf_file_info->samplerate = (int)*mxGetPr(tmp_ptr);
    else {
        free(sf_in_fname);
        mexErrMsgTxt("Field 'samplerate' not set.");
    }

    tmp_ptr = mxGetField(args, 0, "channels" );
    if( tmp_ptr != NULL )
        sf_file_info->channels = (int)*mxGetPr(tmp_ptr);
    else {
        free(sf_in_fname);
        mexErrMsgTxt("Field 'channels' not set.");
    }

    /*
     * get the format information
     */

    /* format name should be set to RAW when reading RAW files */
    tmp_ptr = mxGetField(args, 0, "format" );
    if( tmp_ptr != NULL )
        mxGetString(tmp_ptr, maj_fmt_name, FMT_STR_SIZE);

    tmp_ptr = mxGetField(args, 0, "sampleformat" );
    if( tmp_ptr != NULL )
        mxGetString(tmp_ptr, sub_fmt_name, FMT_STR_SIZE);
    else {
        free(sf_in_fname);
        mexErrMsgTxt("Field 'sampleformat' not set.");
    }

    /* endianness_name does not need to be set */
    tmp_ptr = mxGetField(args, 0, "endianness" );
    if( tmp_ptr != NULL )
        mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);

    sf_file_info->format = lookup_val(&maj_fmts, maj_fmt_name)
        | lookup_val(&sub_fmts, sub_fmt_name)
        | lookup_val(&endianness_types, endianness_name);

    /* check format for validity */
    if( !sf_format_check(sf_file_info) ) {
        mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
        free(sf_in_fname);
        mexErrMsgTxt("Invalid format specified.");
    }
}

/* check the fmt argument and return true or false */
int get_fmt(const char* const args)
{
    int do_read_raw = 0;

    if( !strcmp(args, "native") )
        do_read_raw = 1;
    else if( !strcmp(args, "double") )
        do_read_raw = 0;
    else
        mexWarnMsgTxt("Bad 'fmt' argument: defaulting to 'double'.");

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
