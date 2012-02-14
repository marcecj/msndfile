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
    const short nbits      = get_bits(sf_file_info);
    const mwSize ndims[]   = {1, 1};

    const char* fmt_fields[] = {
        /* supporting the WAVE format field is... difficult when using a
         * higher-level library like libsndfile.  The sndfile-info utility
         * apparently reads the file itself to get the info. */
        /* "wFormatTag", */
        "nChannels",
        "nSamplesPerSec",
        "nAvgBytesPerSec",
        "nBlockAlign",
        "nBitsPerSample"
    };

    const char* info_fields[] = {
        "inam", /* Title */
        "icpy",
        "isft",
        "iart", /* Artist */
        "icom",
        "icrd", /* Date */
        "ialb",
        "ilic",
        "inum",
        "igen"
    };

    const short num_fmt_fields  = sizeof(fmt_fields)/sizeof(char*);
    const short num_info_fields = SF_STR_LAST-SF_STR_FIRST+1;

    double *fmt_data     = (double*)malloc(num_fmt_fields*sizeof(double));
    char **info_data     = (char**)calloc(num_info_fields,sizeof(char*));

    mxArray **fmt_array  = (mxArray**)malloc(num_fmt_fields*sizeof(mxArray*));
    mxArray **info_array = (mxArray**)malloc(num_info_fields*sizeof(mxArray*));

    mxArray *fmt           = mxCreateStructArray(1, ndims, num_fmt_fields, fmt_fields);
    mxArray *info          = mxCreateStructArray(1, ndims, 0, NULL);

    /*
     * set fmt field
     */

    /* fmt_data[0] = 0; */
    fmt_data[0] = (double)sf_file_info->channels;
    fmt_data[1] = (double)sf_file_info->samplerate;
    fmt_data[2] = (double)(sf_file_info->samplerate*(nbits/8)*sf_file_info->channels);
    fmt_data[3] = (double)(sf_file_info->channels*nbits/8); /* see wavread() */
    fmt_data[4] = (double)nbits;

    for( i = 0; i < num_fmt_fields; i++ ) {
        fmt_array[i] = mxCreateDoubleScalar(fmt_data[i]);
        mxSetField(fmt, 0, fmt_fields[i], mxCreateDoubleScalar(fmt_data[i]));
    }

    mxSetField(opts, 0, "fmt", fmt);

    /*
     * set info field
     */

    for( i = 0; i < num_info_fields; i++ )
    {
        info_data[i] = sf_get_string(sf_input_file, i+SF_STR_FIRST);
        if (info_data[i] != NULL)
        {
            info_array[i] = mxCreateString(info_data[i]);
            mxAddField(info, info_fields[i]);
            mxSetField(info, 0, info_fields[i], info_array[i]);
        }
    }

    mxAddField(opts, "info");
    mxSetField(opts, 0, "info", info);

    /*
     * free pointers
     */

    if( fmt_data != NULL )   free(fmt_data);
    if( fmt_array != NULL )  free(fmt_array);
    if( info_data != NULL )  free(info_data);
    if( info_array != NULL ) free(info_array);
}
