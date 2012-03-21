#include <string.h>
#include <mex.h>
#include <matrix.h>
#include <sndfile.h>
#include "utils.h"
#include "read_utils.h"

/*
 * This is a simple mex-File using libsndfile for reading in audio files
 *
 * TODO: this needs more testing
 */

static SNDFILE* sf_input_file=NULL;

/* function for clearing memory after Matlab ends */
void clear_memory(void)
{
    if( sf_input_file != NULL )
        sf_close(sf_input_file);
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    int         sndfile_err; /* libsndfile error status */
    int         num_chns;
    const int   str_size = (nrhs > 0 ? mxGetN(prhs[0])+1 : 0); /* length of the input file name */
    char        *sf_in_fname; /* input file name */
    sf_count_t  num_frames=0;
    double      *data, *output;
    SF_INFO     *sf_file_info;
    int         do_read_raw = 0;
    mxClassID   class_id = mxDOUBLE_CLASS;

    mexAtExit(&clear_memory);

    /* If a file was not closed properly last run, attempt to close it
     * again.  If it still fails, abort. */
    if( sf_input_file != NULL ) {
        if( !sf_close(sf_input_file) )
            sf_input_file = NULL;
        else
            mexErrMsgTxt("There was still a file open that could not be closed!");
    }

    if( nrhs < 1 )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    /* get input filename */
    sf_in_fname = (char*)calloc(str_size, sizeof(char));
    if( sf_in_fname == NULL ) {
        free(sf_in_fname);
        mexErrMsgTxt("calloc error!");
    }
    mxGetString(prhs[0], sf_in_fname, str_size);

    /* initialize sf_file_info struct pointer */
    sf_file_info = (SF_INFO*)malloc(sizeof(SF_INFO));
    if( sf_file_info == NULL ) {
        free(sf_in_fname);
        mexErrMsgTxt("Could not allocate SF_INFO* instance");
    }

    /* "format" needs to be set to 0 before a file is opened for reading,
     * unless the file is a RAW file */
    sf_file_info->format = 0;

    /* handle the fourth input argument */
    if( nrhs >= 4 && !mxIsEmpty(prhs[3]) )
    {
        if( !mxIsStruct(prhs[3]) ) {
            free(sf_in_fname);
            free(sf_file_info);
            mexErrMsgTxt("The fourth argument has to be a struct! (see help text)");
        }

        get_file_info(sf_file_info, sf_in_fname, prhs[3]);
    }

    /* handle the third input argument */
    if( nrhs >= 3 && !mxIsEmpty(prhs[2]) )
    {
        const short fmt_len = mxGetN(prhs[2])+1;
        char* fmt = (char*)malloc(fmt_len*sizeof(char));

        if( !mxIsChar(prhs[2]) ) {
            free(sf_in_fname);
            free(sf_file_info);
            mexErrMsgTxt("The third argument has to be a string! (see help text)");
        }

        if( mxGetString(prhs[2], fmt, fmt_len) == 1 ) {
            free(sf_in_fname);
            free(sf_file_info);
            free(fmt);
            mexErrMsgTxt("Error getting 'fmt' string.");
        }

        do_read_raw = get_fmt(fmt);
    }

    sf_input_file = sf_open(sf_in_fname, SFM_READ, sf_file_info);
    free(sf_in_fname);

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
        const short cmd_size = mxGetN(prhs[1])+1;

        char *cmd_str = (char*)malloc(cmd_size*sizeof(char));
        if( cmd_str == NULL )
            mexErrMsgTxt("malloc error!");

        if( mxGetString(prhs[1], cmd_str, cmd_size) == 1 ) {
            free(cmd_str);
            mexErrMsgTxt("Error getting command string.");
        }

        if( strcmp(cmd_str, "size") == 0 ) {
            double *dims;

            plhs[0] = mxCreateDoubleMatrix(1, 2, mxREAL);
            dims    = mxGetPr(plhs[0]);

            dims[0] = (double)(sf_file_info->frames);
            dims[1] = (double)(sf_file_info->channels);

            /* Skip everything else and close the SF_INFO file */
            free(cmd_str);
            goto return_to_matlab;
        } else if( !strcmp(cmd_str, "double") || !strcmp(cmd_str, "native") ) {
            do_read_raw = get_fmt(cmd_str);
        } else {
            free(cmd_str);
            mexErrMsgTxt("Unknown command.");
        }

        free(cmd_str);
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
    if( do_read_raw )
        class_id = get_class_id(sf_file_info);
    plhs[0]  = mxCreateNumericMatrix((int)num_frames, num_chns, class_id, mxREAL);
    output   = (double*)mxGetPr(plhs[0]);

    /* read the entire file in one go
     *
     * If we want the native file type (do_read_raw), then we need to use
     * sf_read_raw() and pass it the number of *bytes* to be read.*/
    data = (double*)malloc((int)num_frames*num_chns*sizeof(double));
    if( do_read_raw )
    {
        const size_t nbytes = num_frames*num_chns*get_bits(sf_file_info)/8;
        if( sf_read_raw(sf_input_file, data, nbytes) <= 0 ) {
            free(data);
            free(sf_file_info);
            mexErrMsgTxt("Error reading bytes from input file: 0 bytes read!");
        }
    }
    else
    {
        if( sf_readf_double(sf_input_file, data, num_frames) <= 0 ) {
            free(data);
            free(sf_file_info);
            mexErrMsgTxt("Error reading frames from input file: 0 frames read!");
        }
    }

    /* transpose returned data */
    transpose_data(output, data, num_frames, num_chns, class_id);
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

    if( nlhs > 2 ) {
        double *nbits;

        plhs[2] = mxCreateDoubleMatrix(1, 1, mxREAL);
        nbits   = mxGetPr(plhs[2]);

        *nbits = (double)get_bits(sf_file_info);
    }

    if( nlhs > 3 ) {
        const mwSize ndims[] = {1, 1};
        const char* fields[] = {"fmt",};

        plhs[3] = mxCreateStructArray(1, ndims, 1, fields);

        get_opts(sf_file_info, sf_input_file, plhs[3]);
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

    /* free memory */
    free(sf_file_info);
}
