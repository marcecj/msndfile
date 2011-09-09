#include <string.h>
#include <mex.h>
#include <sndfile.h>
#include "msndfile.h"

/*
 * This is a simple mex-File using libsndfile for reading in audio files
 *
 * TODO: this needs more testing
 */

SNDFILE* sf_input_file=NULL;

/* the max length of a format string (9 for "IMA_ADPCM" & "VOX_ADPCM") + \0 */
#define FMT_STR_SIZE 10

/* function for clearing memory after Matlab ends */
void clear_memory(void)
{
    if( sf_input_file != NULL )
        sf_close(sf_input_file);
}

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

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    int         i; // counter in for-loops
    int         sndfile_err; // libsndfile error status
    int         num_chns;
    const int   str_size = (nrhs > 0 ? mxGetN(prhs[0])+1 : 0); // length of the input file name
    char        *sf_in_fname; // input file name
    sf_count_t  num_frames=0, processed_frames=0;
    double      *data, *output;
    SF_INFO     *sf_file_info;

    mexAtExit(&clear_memory);

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    /* get input filename */
    sf_in_fname = (char*)calloc(str_size, sizeof(char));
    if( sf_in_fname == NULL ) {
        free(sf_in_fname);
        mexErrMsgTxt("calloc error!");
    }
    mxGetString(prhs[0], sf_in_fname, str_size);

    /*
     * allocate the strings corresponding to the names of the major formats,
     * format subtypes and the endianness as per the libsndfile documentation
     */

    /* initialize sf_file_info struct pointer */
    sf_file_info = (SF_INFO*)malloc(sizeof(SF_INFO));
    if( sf_file_info == NULL ) {
        free(sf_in_fname);
        mexErrMsgTxt("Could not allocate SF_INFO* instance");
    }

    if( nrhs < 3 )
        /* "format" needs to be set to 0 before a file is opened for reading,
         * unless the file is a RAW file */
        sf_file_info->format = 0;
    else
    {
        /*
         * handle RAW files
         */

        /* a temporary array */
        mxArray *tmp_ptr = NULL;

        /* the three OR-ed components of the "format" field in sf_file_info */
        char maj_fmt_name[FMT_STR_SIZE] = "RAW";
        char sub_fmt_name[FMT_STR_SIZE];
        char endianness_name[FMT_STR_SIZE] = "FILE";

        if( !mxIsStruct(prhs[2]) ) {
            free(sf_in_fname);
            free(sf_file_info);
            mexErrMsgTxt("The second argument has to be a struct! (see help text)");
        }

        /*
         * get the sample rate and the number of channels
         */

        tmp_ptr = mxGetField(prhs[2], 0, "samplerate" );
        if( tmp_ptr != NULL )
            sf_file_info->samplerate = (int)*mxGetPr(tmp_ptr);
        else {
            free(sf_in_fname);
            free(sf_file_info);
            mexErrMsgTxt("Field 'samplerate' not set.");
        }

        tmp_ptr = mxGetField(prhs[2], 0, "channels" );
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
        tmp_ptr = mxGetField(prhs[2], 0, "format" );
        if( tmp_ptr != NULL )
            mxGetString(tmp_ptr, maj_fmt_name, FMT_STR_SIZE);

        tmp_ptr = mxGetField(prhs[2], 0, "sampleformat" );
        if( tmp_ptr != NULL )
            mxGetString(tmp_ptr, sub_fmt_name, FMT_STR_SIZE);
        else {
            free(sf_in_fname);
            free(sf_file_info);
            mexErrMsgTxt("Field 'sampleformat' not set.");
        }

        /* endianness_name does not need to be set */
        tmp_ptr = mxGetField(prhs[2], 0, "endianness" );
        if( tmp_ptr != NULL )
            mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);

        /* sf_file_info->format = lookup_val(&maj_fmts, maj_fmt_name) | \ */
        sf_file_info->format = SF_FORMAT_RAW
                                | lookup_val(&sub_fmts, sub_fmt_name)
                                | lookup_val(&endianness_types, endianness_name);
    }

    /* open sound file */
    if( nrhs > 2 && !sf_format_check(sf_file_info) ) {
        mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
        free(sf_in_fname);
        free(sf_file_info);
        mexErrMsgTxt("Invalid format specified.");
    }
    else
    {
        /* If a file was not closed properly last run, attempt to close it
         * again.  If it still fails, abort. */
        if( sf_input_file != NULL ) {
            if( !sf_close(sf_input_file) )
                sf_input_file = NULL;
            else {
                free(sf_in_fname);
                mexErrMsgTxt("There was still a file open that could not be closed!");
            }
        }
        sf_input_file = sf_open(sf_in_fname, SFM_READ, sf_file_info);
        free(sf_in_fname);
    }

    if( sf_input_file == NULL ) {
        free(sf_file_info);
        mexErrMsgTxt("Could not open audio file.");
    }

    /*
     * If the second argument is 'size', then only return the dimensions of the
     * signal.
     */
    if( nrhs > 1
            && !mxIsEmpty(prhs[1])
            && mxIsChar(prhs[1]))
    {
        char *cmd_str = (char*)malloc(4*sizeof(char));
        if( cmd_str == NULL )
            mexErrMsgTxt("malloc error!");

        if( mxGetString(prhs[1], cmd_str, mxGetN(prhs[1])+1) == 1 ) {
            free(cmd_str);
            mexErrMsgTxt("Error getting command string.");
        }

        if( strcmp(cmd_str, "size") == 0 ) {
            double *dims;

            plhs[0] = mxCreateDoubleMatrix(1, 2, mxREAL);
            dims    = mxGetPr(plhs[0]);

            dims[0] = (double)(sf_file_info->frames);
            dims[1] = (double)(sf_file_info->channels);
        }
        else {
            free(cmd_str);
            mexErrMsgTxt("Unknown command.");
        }

        /* Skip everything else and close the SF_INFO file */
        free(cmd_str);
        goto return_to_matlab;
    }

    if( nrhs > 1
            && !mxIsEmpty(prhs[1])
            && mxIsDouble(prhs[1]))
    {
        double *start_end_idx = mxGetPr(prhs[1]);
        int    range_size     = mxGetN(prhs[1]);

        if( range_size == 2 ) {
            num_frames = (sf_count_t)(start_end_idx[1] - start_end_idx[0] + 1);

            if( sf_seek(sf_input_file, start_end_idx[0]-1, SEEK_SET) < 0 )
                mexErrMsgTxt("Invalid range!");
        }
        else if( range_size == 1 )
            num_frames = (sf_count_t)(start_end_idx[0]);
        else
            mexErrMsgTxt("Range can be a row vector with 1 or 2 elements.");
    }
    else
        num_frames = sf_file_info->frames;

    /* initialise Matlab output array */
    num_chns = sf_file_info->channels;
    plhs[0]  = mxCreateDoubleMatrix((int)num_frames, num_chns, mxREAL);
    output   = mxGetPr(plhs[0]);
    /* data read via libsndfile */
    data     = (double*)malloc((int)num_frames*num_chns*sizeof(double));

    /* read the entire file in one go */
    processed_frames = sf_readf_double(sf_input_file, data, num_frames);
    if( processed_frames <= 0 ) {
        free(data);
        free(sf_file_info);
        mexErrMsgTxt("Error reading frames from input file: 0 frames read!");
    }

    /*
     * transpose returned data
     *
     * TODO: maybe do an in-place transpose? Files already open in about 2/3 of
     * the time of Matlab's wavread(), so some additional time complexity
     * probably won't hurt much.
     */
    for( i=0; i<num_frames; i++ ) {
        int j;
        for( j=0; j<num_chns; j++ )
            output[i+j*num_frames] = data[i*num_chns+j];
    }
    free(data);

    /* rudimentary way of dealing with libsndfile errors */
    sndfile_err = sf_error(sf_input_file);
    if( sndfile_err != SF_ERR_NO_ERROR ) {
        free(sf_file_info);
        mexWarnMsgTxt("libsndfile error!");
        mexErrMsgTxt(sf_error_number(sndfile_err));
    }

return_to_matlab:

    /* return sampling rate if requested */
    if( nlhs > 1 ) {
        double *fs;

        plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
        fs      = mxGetPr(plhs[1]);

        *fs = (double)sf_file_info->samplerate;
    }

    /* TODO: return the number of bits (requires checking the SF_INFO ->format field) */
    /*
     * if( nlhs > 2 ) {
     *     plhs[2] = mxCreateDoubleMatrix(1, 1, mxREAL);
     *     double *nbits   = mxGetPr(plhs[2]);
     *     *nbits  = 0;
     * }
     */

    if( sf_input_file != NULL ) {
        if( !sf_close(sf_input_file) )
            /* sf_close() doesn't set the pointer to NULL, and Matlab doesn't
             * like that (it prints "too many files open" errors), even though
             * this pointer is overwritten every call anyway */
            sf_input_file = NULL;
        else
            mexWarnMsgTxt("libsndfile could not close the file!");
    }

    /* free memory */
    free(sf_file_info);
}
