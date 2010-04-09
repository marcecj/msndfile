#include <string.h>
#include <mex.h>
#include <sndfile.h>

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

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    int         i; // counter in for-loops
    int         sndfile_error;
    int         num_channels; // temporary hack for Gentoo
    const int   str_size = mxGetN(prhs[0])+1;
    char        *sf_input_fname;
    sf_count_t  num_frames, processed_frames;
    double      *data, *output, *fs;
    SF_INFO     *sf_file_info;

    mexAtExit(&clear_memory);

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    /* get input filename string */
    sf_input_fname = (char*)mxCalloc(str_size, sizeof(char));
    if( !sf_input_fname )
        mexErrMsgTxt("mxCalloc error!");
    mxGetString(prhs[0], sf_input_fname, str_size);

    /* initiate sf_file_info struct pointer */
    sf_file_info = (SF_INFO*)mxMalloc(sizeof(SF_INFO));
    if( !sf_file_info )
        mexErrMsgTxt("Could not allocate SF_INFO* instance");
    /* "format" needs to be set to 0 before a file is opened for reading */
    sf_file_info->format = 0;

    /* open sound file */
    sf_input_file = sf_open(sf_input_fname, SFM_READ, sf_file_info);
    if( !sf_input_file )
        mexErrMsgTxt("Could not open audio file.");

    num_frames = sf_file_info->frames;

    /* set channels to 2 due to a bug in Gentoos 32 bit libsndfile */
    /* num_channels = 2; */
    num_channels = sf_file_info->channels;
    plhs[0]      = mxCreateDoubleMatrix((int)sf_file_info->frames, num_channels, mxREAL);
    output       = mxGetPr(plhs[0]);
    data         = (double*)mxCalloc((int)sf_file_info->frames*num_channels,sizeof(double));

    /* read the entire file in one go */
    processed_frames = sf_readf_double(sf_input_file, data, num_frames);

    /* transpose returned data */
    for( i=0; i<num_frames; i+=num_channels )
    {
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
