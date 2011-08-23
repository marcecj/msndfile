#include <mex.h>
#include <sndfile.h>
#include <string.h>

SNDFILE* sf_input_file;

void clear_memory(void)
{
    if( !sf_input_file )
        sf_close(sf_input_file);
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    int i; // counter in for-loops
    int str_size;
    char* sf_input_fname;
    int num_frames;
    double* data;
    int processed_frames;
    int sndfile_error;
    double* fs;
    SF_INFO* sf_file_info;

    mexAtExit(&clear_memory);

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to at least pass an array or a file name.");

    /* all this just to get to a string... */
    str_size = mxGetN(prhs[0])+1;
    sf_input_fname = (char*)mxCalloc(str_size, sizeof(char));
    if( sf_input_fname == NULL )
        mexErrMsgTxt("mxCalloc error!");
    mxGetString(prhs[0], sf_input_fname, str_size);

    /* initiate sf_file_info struct pointer */
    sf_file_info = (SF_INFO*)mxMalloc(sizeof(SF_INFO));
    if( !sf_file_info )
        mexErrMsgTxt("could not allocate SF_INFO* instance");
    /* format needs to be set to 0 before a file is opened */
    sf_file_info->format = 0;

    /* open sound file */
    mexPrintf("Opening file...\n");
    sf_input_file = sf_open(sf_input_fname, SFM_READ, sf_file_info);
    if( !sf_input_file )
        mexErrMsgTxt("could not open sound file");
    mexPrintf("Opened file.\n");

    num_frames = (int)sf_file_info->frames;

    /*
     * set channels to 2 due to a bug in Gentoos 32 bit libsndfile
     */
    plhs[0]      = mxCreateDoubleMatrix(sf_file_info->channels, (int)sf_file_info->frames, mxREAL);
    /* plhs[0]      = mxCreateDoubleMatrix(2, (int)sf_file_info->frames, mxREAL); */
    data = mxGetPr(plhs[0]);

    /* TODO: need to correct/test this */

    /* read in the entire file */
    processed_frames = (int)sf_readf_double(sf_input_file, data, num_frames);

    if( processed_frames < num_frames && sf_file_info->seekable )
        processed_frames = (int)sf_seek(sf_input_file, processed_frames, SEEK_CUR);

    sndfile_error = sf_error(sf_input_file);
    if( sndfile_error != SF_ERR_NO_ERROR ) {
        mexWarnMsgTxt("libsndfile error!");
        mexErrMsgTxt(sf_error_number(sndfile_error));
    }

    if( nlhs > 1 ) {
        plhs[1]    = mxCreateDoubleMatrix(1, 1, mxREAL);
        fs = mxGetPr(plhs[1]);
        *fs = (double)sf_file_info->samplerate;
    }
}
