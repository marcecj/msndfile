#include <string.h>
#include <mex.h>
#include <sndfile.h>
#include "msndfile.h"

/*
 * This is a simple mex-File using libsndfile for reading in audio files
 *
 * TODO: this needs more testing
 */

SNDFILE* sf_input_file;

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
    const int   str_size = mxGetN(prhs[0])+1; // length of the input file name
    char        *sf_in_fname; // input file name
    sf_count_t  num_frames, processed_frames=0;
    double      *data, *output, *fs;
    SF_INFO     *sf_file_info;
    // the three OR-ed components of the "format" field in sf_file_info
    char        *maj_fmt_name, *sub_fmt_name, *endianness_name;

    mexAtExit(&clear_memory);

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    /* get input filename */
    sf_in_fname = (char*)mxCalloc(str_size, sizeof(char));
    if( sf_in_fname == NULL )
        mexErrMsgTxt("mxCalloc error!");
    mxGetString(prhs[0], sf_in_fname, str_size);

    /*
     * allocate the strings corresponding to the names of the major formats,
     * format subtypes and the endianness as per the libsndfile documentation
     */

    maj_fmt_name    = (char*)mxCalloc(20, sizeof(char));
    sub_fmt_name    = (char*)mxCalloc(20, sizeof(char));
    endianness_name = (char*)mxCalloc(20, sizeof(char));

    if( maj_fmt_name == NULL ||
            sub_fmt_name == NULL ||
            endianness_name == NULL )
        mexErrMsgTxt("mxCalloc error!"); 

    /* initialize sf_file_info struct pointer */
    sf_file_info = (SF_INFO*)mxMalloc(sizeof(SF_INFO));
    if( sf_file_info == NULL )
        mexErrMsgTxt("Could not allocate SF_INFO* instance");

    if( nrhs < 2 )
        /* "format" needs to be set to 0 before a file is opened for reading,
         * unless the file is a RAW file */
        sf_file_info->format = 0;
    else
    {
        /* handle RAW files */
        if( mxIsStruct(prhs[1]) )
        {
            mxArray *tmp_ptr; /* a temporary array */

            /* 
             * get the sample rate and the number of channels
             */

            tmp_ptr = mxGetField(prhs[1], 0, "samplerate" );
            if( tmp_ptr != NULL )
                sf_file_info->samplerate = (int)*mxGetPr(tmp_ptr);
            else
                mexErrMsgTxt("Field 'samplerate' not set.");

            tmp_ptr = mxGetField(prhs[1], 0, "channels" );
            if( tmp_ptr != NULL )
                sf_file_info->channels = (int)*mxGetPr(tmp_ptr);
            else
                mexErrMsgTxt("Field 'channels' not set.");

            /*
             * get the format information
             */

            /* format name should be set to RAW when reading RAW files */
            tmp_ptr = mxGetField(prhs[1], 0, "format" );
            if( tmp_ptr != NULL )
                mxGetString(tmp_ptr, maj_fmt_name, mxGetN(tmp_ptr)+1);
            else
                maj_fmt_name = "RAW";

            tmp_ptr = mxGetField(prhs[1], 0, "sampleformat" );
            if( tmp_ptr != NULL )
                mxGetString(tmp_ptr, sub_fmt_name, mxGetN(tmp_ptr)+1);
            else
                mexErrMsgTxt("Field 'sampleformat' not set.");

            /* endianness_name does not need to be set */
            tmp_ptr = mxGetField(prhs[1], 0, "endianness" );
            if( tmp_ptr != NULL )
                mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);
            else
                endianness_name = "FILE";

            /* sf_file_info->format = lookup_val(&maj_fmts, maj_fmt_name) | \ */
            sf_file_info->format = SF_FORMAT_RAW | \
                                   lookup_val(&sub_fmts, sub_fmt_name) | \
                                   lookup_val(&endianness_types, endianness_name);
        }
        else
            mexErrMsgTxt("The second argument has to be a struct! (see help text)");
    }

    /* open sound file */
    if( nrhs > 1 && !sf_format_check(sf_file_info) ) {
        mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
        mexErrMsgTxt("Invalid format specified.");
    }
    else 
        sf_input_file = sf_open(sf_in_fname, SFM_READ, sf_file_info);

    if( sf_input_file == NULL )
        mexErrMsgTxt("Could not open audio file.");

    num_frames = sf_file_info->frames;

    /* initialise Matlab output array */
    num_chns = sf_file_info->channels;
    plhs[0]      = mxCreateDoubleMatrix((int)sf_file_info->frames, num_chns, mxREAL);
    output       = mxGetPr(plhs[0]);
    /* data read via libsndfile */
    data         = (double*)mxCalloc((int)sf_file_info->frames*num_chns,sizeof(double));

    /* read the entire file in one go */
    processed_frames = sf_readf_double(sf_input_file, data, num_frames);
    if( processed_frames <= 0 )
        mexErrMsgTxt("Error reading frames from input file: 0 frames read!");

    /*
     * transpose returned data
     *
     * TODO: maybe do an in-place transpose? Files already open in about 2/3 of
     * the time of Matlab's wavread(), so some additional time complexity
     * probably won't hurt much.
     */
    for( i=0; i<num_frames; i+=num_chns ) {
        int j;
        for( j=0; j<num_chns; j++ )
            output[i+j*num_frames] = data[i*num_chns+j];
    }

    /* rudimentary way of dealing with libsndfile errors */
    sndfile_err = sf_error(sf_input_file);
    if( sndfile_err != SF_ERR_NO_ERROR ) {
        mexWarnMsgTxt("libsndfile error!");
        mexErrMsgTxt(sf_error_number(sndfile_err));
    }

    /* return sampling rate if requested */
    if( nlhs > 1 ) {
        plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
        fs      = mxGetPr(plhs[1]);
        *fs     = (double)sf_file_info->samplerate;
    }

    if( sf_input_file != NULL ) {
        if( !sf_close(sf_input_file) )
            /* sf_close() doesn't set the pointer to NULL, and Matlab doesn't
             * like that (it prints "too many files open" errors), even though
             * this pointer is overwritten every call anyway */
            sf_input_file = NULL;
        else
            mexWarnMsgTxt("libsndfile could not close the file!");
    }

}
