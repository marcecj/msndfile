#include <string.h>
#include <mex.h>
#include <sndfile.h>
#include "utils.h"

/*
 * This is a simple mex-File using libsndfile for reading in audio files
 *
 * TODO: this needs more testing
 */

enum {
    CMD_OPEN=0,
    CMD_READ,
    CMD_CLOSE
};

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    static SNDFILE* sf_input_file=NULL;
    static SF_INFO* sf_file_info=NULL;

    int         i; // counter in for-loops
    int         sndfile_err; // libsndfile error status
    int         num_chns;
    const int   cmd_size = (nrhs > 0 ? mxGetN(prhs[0])+1 : 0); // length of the command
    const int   str_size = (nrhs > 1 ? mxGetN(prhs[1])+1 : 0); // length of the input file name
    int         cmd_id = -1;
    char        *cmd_str;
    char        *sf_in_fname; // input file name
    sf_count_t  num_frames=0, processed_frames=0;
    double      *data, *output;

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to pass a command (either 'open', 'read', or 'close').");

    if( nrhs < 2 )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    /* get input filename */
    sf_in_fname = (char*)calloc(str_size, sizeof(char));
    if( sf_in_fname == NULL ) {
        mexErrMsgTxt("calloc error!");
    }
    mxGetString(prhs[1], sf_in_fname, str_size);

    /*
     * If the second argument is 'size', then only return the dimensions of the
     * signal.
     */
    if( mxIsEmpty(prhs[0]) || !mxIsChar(prhs[0]))
        mexErrMsgTxt("Argument error: command may not be empty.");

    cmd_str = (char*)malloc(cmd_size*sizeof(char));
    if( cmd_str == NULL )
        mexErrMsgTxt("malloc error!");

    if( mxGetString(prhs[0], cmd_str, cmd_size) == 1 ) {
        free(cmd_str);
        mexErrMsgTxt("Error getting command string.");
    }

    if( strcmp(cmd_str, "open") == 0 )
        cmd_id = CMD_OPEN;
    else if( strcmp(cmd_str, "read") == 0 )
        cmd_id = CMD_READ;
    else if( strcmp(cmd_str, "close") == 0 )
        cmd_id = CMD_CLOSE;
    else {
        free(cmd_str);
        mexErrMsgTxt("Unknown command.");
    }
    free(cmd_str);

    if( cmd_id == CMD_OPEN )
    {
        /* initialize sf_file_info struct pointer */
        sf_file_info = (SF_INFO*)malloc(sizeof(SF_INFO));
        if( sf_file_info == NULL ) {
            free(sf_in_fname);
            mexErrMsgTxt("Could not allocate SF_INFO* instance");
        }

        if( nrhs < 4 )
            /* "format" needs to be set to 0 before a file is opened for reading,
             * unless the file is a RAW file */
            sf_file_info->format = 0;
        else
        {
            /*
             * handle RAW files
             */

            if( !mxIsStruct(prhs[2]) ) {
                free(sf_in_fname);
                free(sf_file_info);
                mexErrMsgTxt("The second argument has to be a struct! (see help text)");
            }

            get_raw_info(sf_file_info, sf_in_fname, prhs[2]);
        }

        /* open sound file */
        if( nrhs > 2 && !sf_format_check(sf_file_info) ) {
            mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
            free(sf_in_fname);
            free(sf_file_info);
            mexErrMsgTxt("Invalid format specified.");
        }

        sf_input_file = sf_open(sf_in_fname, SFM_READ, sf_file_info);
        free(sf_in_fname);

        if( sf_input_file == NULL ) {
            free(sf_file_info);
            mexErrMsgTxt("Could not open audio file.");
        }
    }
    else if( cmd_id == CMD_CLOSE )
    {
        if( sf_input_file != NULL )
        {
            if( !sf_close(sf_input_file) )
                sf_input_file = NULL;
            else
                mexWarnMsgTxt("libsndfile could not close the file!");
        }

        if( sf_file_info != NULL )
            free(sf_file_info);
    }
    else if( cmd_id == CMD_READ )
    {
        /*
         * allocate the strings corresponding to the names of the major formats,
         * format subtypes and the endianness as per the libsndfile documentation
         */
        if( sf_input_file == NULL )
            mexErrMsgTxt("No file open!");

        if( !mxIsEmpty(prhs[2]) && mxIsDouble(prhs[2]))
        {
            double *start_end_idx = mxGetPr(prhs[2]);
            int    range_size     = mxGetN(prhs[2]);

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
        data     = (double*)calloc((int)num_frames*num_chns,sizeof(double));

        /* read the entire file in one go */
        processed_frames = sf_readf_double(sf_input_file, data, num_frames);
        if( processed_frames == 0 ) {
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
    }
}
