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

/* function for clearing memory after this program ends */
void clear_memory(void)
{
    if( !sf_input_file )
        sf_close(sf_input_file);
}

/* function to get a value from a look-up table */
int get_val(const lookup_table *array, const int array_size, const char *name)
{
	int i;
    for(i = 0; i < array_size; i++) {
        if( strcmp(name, array[i].name) == 0 )
            return array[i].number;
    }

	return 0;
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    int         i; // counter in for-loops
    int         sndfile_error; // libsndfile error status
    int         num_channels;
    const int   str_size = mxGetN(prhs[0])+1; // length of the input file name
    char        *sf_input_fname; // input file name
    sf_count_t  num_frames, processed_frames=0;
    double      *data, *output, *fs;
    SF_INFO     *sf_file_info;
    // the three OR-ed components of the "format" field in sf_file_info
    char        *major_format_name, *sub_format_name, *endianness_name;

    mexAtExit(&clear_memory);

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    /* get input filename */
    sf_input_fname = (char*)mxCalloc(str_size, sizeof(char));
    if( !sf_input_fname )
        mexErrMsgTxt("mxCalloc error!");
    mxGetString(prhs[0], sf_input_fname, str_size);

    /*
     * allocate the strings corresponding to the names of the major formats,
     * format subtypes and the endianness as per the libsndfile documentation
     */
    
    major_format_name = (char*)mxCalloc(20, sizeof(char));
    if( !major_format_name )
        mexErrMsgTxt("mxCalloc error!");

    sub_format_name = (char*)mxCalloc(20, sizeof(char));
    if( !sub_format_name )
        mexErrMsgTxt("mxCalloc error!");

    endianness_name = (char*)mxCalloc(20, sizeof(char));
    if( !endianness_name )
        mexErrMsgTxt("mxCalloc error!");

    /* initialize sf_file_info struct pointer */
    sf_file_info = (SF_INFO*)mxMalloc(sizeof(SF_INFO));
    if( !sf_file_info )
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

            tmp_ptr = mxGetField(prhs[1], 0, "format" );
            if( tmp_ptr != NULL )
                mxGetString(tmp_ptr, major_format_name, mxGetN(tmp_ptr)+1);
            else
                mexErrMsgTxt("Field 'format' not set.");

            tmp_ptr = mxGetField(prhs[1], 0, "sampleformat" );
            if( tmp_ptr != NULL )
                mxGetString(tmp_ptr, sub_format_name, mxGetN(tmp_ptr)+1);
            else
                mexErrMsgTxt("Field 'sampleformat' not set.");

            /* endianness_name does not need to be set */
            tmp_ptr = mxGetField(prhs[1], 0, "endianness" );
            if( tmp_ptr != NULL )
                mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);
            else
                endianness_name = "FILE";

            sf_file_info->format = get_val(major_formats, major_formats_size, major_format_name) | \
                                   get_val(sub_formats, sub_formats_size, sub_format_name) | \
                                   get_val(endianness_types, endianness_types_size, endianness_name);
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
        sf_input_file = sf_open(sf_input_fname, SFM_READ, sf_file_info);

    if( !sf_input_file )
        mexErrMsgTxt("Could not open audio file.");

    num_frames = sf_file_info->frames;

    /* initialise Matlab output array */
    num_channels = sf_file_info->channels;
    plhs[0]      = mxCreateDoubleMatrix((int)sf_file_info->frames, num_channels, mxREAL);
    output       = mxGetPr(plhs[0]);
    /* data read via libsndfile */
    data         = (double*)mxCalloc((int)sf_file_info->frames*num_channels,sizeof(double));

    /* read the entire file in one go */
    processed_frames = sf_readf_double(sf_input_file, data, num_frames);
    if( processed_frames <= 0 )
        mexErrMsgTxt("Error reading frames from input file: 0 frames read!");

    /*
     * transpose returned data
     *
     * TODO: maybe do an in-place transpose? Files already open almost twice as
     * fast compared to Matlab's built-in functions, so some additional time
     * complexity probably won't hurt much.
     */
    for( i=0; i<num_frames; i+=num_channels ) {
        int j;
        for( j=0; j<num_channels; j++ )
            output[i+j*num_frames] = data[i*num_channels+j];
    }

    /* rudimentary way of dealing with libsndfile errors */
    sndfile_error = sf_error(sf_input_file);
    if( sndfile_error != SF_ERR_NO_ERROR ) {
        mexWarnMsgTxt("libsndfile error!");
        mexErrMsgTxt(sf_error_number(sndfile_error));
    }

    /* return sampling rate if requested */
    if( nlhs > 1 ) {
        plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
        fs      = mxGetPr(plhs[1]);
        *fs     = (double)sf_file_info->samplerate;
    }
}
