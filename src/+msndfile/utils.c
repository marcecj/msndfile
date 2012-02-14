#include <string.h>
#include <mex.h>
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
 * audio file info look-up functions
 */

/* create an AUDIO_FILE_INFO struct */
AUDIO_FILE_INFO* create_file_info(const char *const name, SF_INFO* sf_file_info, SNDFILE* file)
{
    const int name_len = strlen(name);

    AUDIO_FILE_INFO* file_info =
        (AUDIO_FILE_INFO*)malloc(sizeof(AUDIO_FILE_INFO));

    /* file_info.info = (SF_INFO*)malloc(sizeof(SF_INFO)); */
    /* file_info.info = memcpy(file_info.info, sf_file_info, sizeof(sf_file_info)); */
    file_info->info = sf_file_info;
    file_info->file = file;

    file_info->name = (char*)calloc((name_len+1), sizeof(char));
    file_info->name = strcpy(file_info->name, name);

    return file_info;
}

/* add an AUDIO_FILE_INFO structure to an AUDIO_FILES look-up table */
AUDIO_FILES* store_file_info(AUDIO_FILES *array, AUDIO_FILE_INFO *file_info)
{
    if( array == NULL ) {
        array = (AUDIO_FILES*)malloc(sizeof(AUDIO_FILES));
        if( array == NULL )
            return NULL;
        array->num_files = 0;
        array->files = (AUDIO_FILE_INFO**)malloc(sizeof(AUDIO_FILE_INFO*));
    }

    if( lookup_file_info(array, file_info->name) == NULL ) {
        /* append the file name */
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

    if( array == NULL )
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

    while( strcmp(name, array->files[i]->name) != 0 )
        i++;

    if( i < array->num_files )
    {
        array->files[i] = destroy_file_info(array->files[i]);

        /* replace the deleted element with the last one */
        array->files[i] = array->files[array->num_files-1];

        array->files = (AUDIO_FILE_INFO**)realloc(array->files, (--array->num_files)*sizeof(AUDIO_FILE_INFO*));
        if( array->num_files < 1 )
            array->files = (AUDIO_FILE_INFO**)malloc(sizeof(AUDIO_FILE_INFO*));
    }
    else
        mexWarnMsgTxt("File not open.");

    return array;
}

/* deallocate an AUDIO_FILE_INFO structure */
AUDIO_FILE_INFO* destroy_file_info(AUDIO_FILE_INFO* file_info)
{
    if( file_info == NULL )
        mexWarnMsgTxt("File already removed! This is odd.");

    free(file_info->name);
    free(file_info->info);

    /* TODO: what to do here? */
    if( !sf_close(file_info->file) )
        file_info->file = NULL;
    else
        mexWarnMsgTxt("libsndfile could not close the file!");

    free(file_info);

    return file_info;
}

/* deallocate an AUDIO_FILES look-up table */
void destroy_file_list(AUDIO_FILES* array)
{
    if( array != NULL ) {
        int i=0;
        for( i = 0; i < array->num_files; i++ )
            array->files[i] = destroy_file_info(array->files[i]);
    }
    free(array);
}

/*
 * misc functions
 */

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
        free(sf_file_info);
        mexErrMsgTxt("Field 'samplerate' not set.");
    }

    tmp_ptr = mxGetField(args, 0, "channels" );
    if( tmp_ptr != NULL )
        sf_file_info->channels = (int)*mxGetPr(tmp_ptr);
    else {
        free(sf_in_fname);
        free(sf_file_info);
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
        free(sf_file_info);
        mexErrMsgTxt("Field 'sampleformat' not set.");
    }

    /* endianness_name does not need to be set */
    tmp_ptr = mxGetField(args, 0, "endianness" );
    if( tmp_ptr != NULL )
        mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);

    /* sf_file_info->format = lookup_val(&maj_fmts, maj_fmt_name) | \ */
    sf_file_info->format = SF_FORMAT_RAW
        | lookup_val(&sub_fmts, sub_fmt_name)
        | lookup_val(&endianness_types, endianness_name);

    /* check format for validity */
    if( !sf_format_check(sf_file_info) ) {
        mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
        free(sf_in_fname);
        free(sf_file_info);
        mexErrMsgTxt("Invalid format specified.");
    }
}

/* get the number of bits of an audio file */
short get_bits(SF_INFO* sf_file_info)
{
    /* a best effort attempt to get the number of bits of an audio file */
    short bits = 0;
    switch(sf_file_info->format & SF_FORMAT_SUBMASK)
    {
        case SF_FORMAT_PCM_S8:
        case SF_FORMAT_PCM_U8:
        case SF_FORMAT_DPCM_8:
            bits =  8; break;
        case SF_FORMAT_DWVW_12:
            bits = 12; break;
        case SF_FORMAT_PCM_16:
        case SF_FORMAT_DPCM_16:
        case SF_FORMAT_DWVW_16:
            bits = 16; break;
        case SF_FORMAT_PCM_24:
        case SF_FORMAT_DWVW_24:
            bits = 24; break;
        case SF_FORMAT_PCM_32:
        case SF_FORMAT_FLOAT:
            bits = 32; break;
        case SF_FORMAT_DOUBLE:
            bits = 64; break;
    }
    return bits;
}

/* create an opts structure a la wavread() */
void get_opts(SF_INFO* sf_file_info, SNDFILE* sf_input_file, mxArray* opts)
{
    int i;
    const double *opts_ptr     = mxGetPr(opts);
    const short nbits          = get_bits(sf_file_info);
    const short num_fmt_fields = 6;
    double *fmt_data           = (double*)malloc(num_fmt_fields*sizeof(double));
    mxArray **mx_data          = (mxArray**)malloc(num_fmt_fields*sizeof(mxArray*));
    const char* fmt_fields[] = {
        "wFormatTag",
        "nChannels",
        "nSamplesPerSec",
        "nAvgBytesPerSec",
        "nBlockAlign",
        "nBitsPerSample"
    };
    const mwSize ndims[]   = {1, 1};
    mxArray *fmt           = mxCreateStructArray(1, ndims, 6, fmt_fields);

    /*
     * set fmt field
     */

    fmt_data[0] = (double)sf_file_info->format;
    fmt_data[1] = (double)sf_file_info->channels;
    fmt_data[2] = (double)sf_file_info->samplerate;
    fmt_data[3] = (double)(sf_file_info->samplerate*(nbits/8)*sf_file_info->channels);
    fmt_data[4] = 0.f;
    fmt_data[5] = (double)nbits;

    for( i = 0; i < num_fmt_fields; i++ ) {
        mx_data[i] = mxCreateDoubleScalar(fmt_data[i]);
        mxSetField(fmt, 0, fmt_fields[i], mxCreateDoubleScalar(fmt_data[i]));
    }

    mxSetField(opts, 0, "fmt", fmt);

    /*
     * TODO: set info field
     */

    sf_get_string(sf_input_file, SF_STR_TITLE);

    /*
     * free pointers
     */

    if( fmt_data != NULL ) free(fmt_data);
    if( mx_data != NULL )  free(mx_data);
}
