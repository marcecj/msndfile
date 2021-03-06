/*
 * Copyright (C) 2010-2015 Marc Joliet
 *
 * Full license information can be found in the file LICENSE in the top-level
 * directory of the source repository.
 */

#include <errno.h>
#include <string.h>
#include <uchar.h>
#include <mex.h>
#include <matrix.h>
#include <sndfile.h>
#include "utils.h"
#include "read_utils.h"

/*
 * This is a simple mex-File using libsndfile for reading in audio files
 */

static SNDFILE* sf_input_file=NULL;

/* function for clearing memory after Matlab ends */
void clear_memory(void)
{
    if( sf_input_file )
        sf_close(sf_input_file);
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    /* "format" needs to be set to 0 before a file is opened for reading,
     * unless the file is a RAW file */
    SF_INFO     sf_file_info = { .format = 0 };
    sf_count_t  num_frames=0;
    int         do_read_raw = 0;
    mxClassID   class_id = mxDOUBLE_CLASS;

    mexAtExit(&clear_memory);

    /* If a file was not closed properly last run, attempt to close it
     * again.  If it still fails, abort. */
    if( sf_input_file != NULL ) {
        int status;
        if( (status = sf_close(sf_input_file)) == 0 )
            sf_input_file = NULL;
        else
            mexErrMsgIdAndTxt("msndfile:sndfile", sf_error_number(status));
    }

    if( nrhs < 1 )
        mexErrMsgIdAndTxt("msndfile:argerror",
                          "Missing argument: you need to pass a file name.");

    /* get input filename */
    char *sf_in_fname = mxArrayToString(prhs[0]);
    if( (sf_in_fname = gen_filename(sf_in_fname)) == NULL )
        mexErrMsgIdAndTxt("msndfile:read:ambiguousname",
                          "No file extension specified and no WAV file found.");

    /* handle the fourth input argument */
    if( nrhs >= 4 && !mxIsEmpty(prhs[3]) )
    {
        if( !mxIsStruct(prhs[3]) ) {
            mxFree(sf_in_fname);
            mexErrMsgIdAndTxt("msndfile:argerror",
                              "The fourth argument has to be a struct! (see help text)");
        }

        get_file_info(&sf_file_info, sf_in_fname, prhs[3]);
    }

    /* handle the third input argument */
    if( nrhs >= 3 && !mxIsEmpty(prhs[2]) )
    {
        const short fmt_len = mxGetN(prhs[2])+1;
        char fmt[fmt_len];

        if( !mxIsChar(prhs[2]) ) {
            mxFree(sf_in_fname);
            mexErrMsgIdAndTxt("msndfile:argerror",
                              "The third argument has to be a string! (see help text)");
        }

        if( mxGetString(prhs[2], fmt, fmt_len) == 1 ) {
            mxFree(sf_in_fname);
            mexErrMsgIdAndTxt("msndfile:argerror",
                              "Error getting 'fmt' string.");
        }

        do_read_raw = get_fmt(fmt);
        if( do_read_raw == -1 )
            mexErrMsgIdAndTxt("msndfile:argerror", "Bad 'fmt' argument.");
    }

    sf_input_file = sf_open(sf_in_fname, SFM_READ, &sf_file_info);
    mxFree(sf_in_fname);

    if( sf_input_file == NULL )
        mexErrMsgIdAndTxt("msndfile:sndfile", sf_strerror(sf_input_file));

    /*
     * If the second argument is 'size', then only return the dimensions of the
     * signal.
     */
    if( nrhs > 1
            && !mxIsEmpty(prhs[1])
            && mxIsChar(prhs[1]))
    {
        const short cmd_size = mxGetN(prhs[1])+1;
        char cmd_str[cmd_size];

        if( mxGetString(prhs[1], cmd_str, cmd_size) == 1 )
            mexErrMsgIdAndTxt("msndfile:argerror",
                              "Error getting command string.");

        if( strcmp(cmd_str, "size") == 0 ) {
            plhs[0]      = mxCreateDoubleMatrix(1, 2, mxREAL);
            double *dims = mxGetPr(plhs[0]);

            dims[0] = (double)(sf_file_info.frames);
            dims[1] = (double)(sf_file_info.channels);

            /* Skip everything else and close the SF_INFO file */
            goto return_to_matlab;
        } else if( !strcmp(cmd_str, "double") || !strcmp(cmd_str, "native") )
            do_read_raw = get_fmt(cmd_str);
        else
            mexErrMsgIdAndTxt("msndfile:argerror", "Unknown command.");
    }

    if( nrhs > 1
            && !mxIsEmpty(prhs[1])
            && mxIsDouble(prhs[1]))
        num_frames = get_num_frames(&sf_file_info, sf_input_file, prhs[1]);
    else
        num_frames = sf_file_info.frames;

    /* initialise Matlab output array */
    const int num_chns = sf_file_info.channels;
    if( do_read_raw )
        class_id = get_class_id(&sf_file_info);
    plhs[0]  = mxCreateNumericMatrix((int)num_frames, num_chns, class_id, mxREAL);
    const double* output = (double*)mxGetPr(plhs[0]);

    /* read the entire file in one go
     *
     * If we want the native file type (do_read_raw), then we need to use
     * sf_read_raw() and pass it the number of *bytes* to be read.
     *
     * NOTE: Matlab 2010a returns the whole file when num_frames == 0, but warns
     * that in the future, an empty matrix will be returned. This implements
     * that future behaviour. */
    double *data = (double*)malloc((int)num_frames*num_chns*sizeof(double));
    if( data == NULL )
        mexErrMsgIdAndTxt("msndfile:system", strerror(errno));

    if( do_read_raw )
    {
        const size_t nbytes = num_frames*num_chns*get_bits(&sf_file_info)/8;
        if( sf_read_raw(sf_input_file, data, nbytes) == 0 )
            mexWarnMsgIdAndTxt("msndfile:sndfile",
                              "Error reading frames from input file: 0 frames read!");
    }
    else
    {
        if( sf_readf_double(sf_input_file, data, num_frames) == 0 )
            mexWarnMsgIdAndTxt("msndfile:sndfile",
                              "Error reading frames from input file: 0 frames read!");
    }

    /* transpose returned data */
    transpose_data(output, data, num_frames, num_chns, class_id);
    free(data);

    /* rudimentary way of dealing with libsndfile errors */
    if( sf_error(sf_input_file) != SF_ERR_NO_ERROR )
        mexErrMsgIdAndTxt("msndfile:sndfile", sf_strerror(sf_input_file));

return_to_matlab:

    /* return sampling rate if requested */
    if( nlhs > 1 ) {
        plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
        double *fs = mxGetPr(plhs[1]);

        *fs = (double)sf_file_info.samplerate;
    }

    /* return bit rate if requested */
    if( nlhs > 2 ) {
        plhs[2] = mxCreateDoubleMatrix(1, 1, mxREAL);
        double *nbits = mxGetPr(plhs[2]);

        *nbits = (double)get_bits(&sf_file_info);
    }

    /* return fmt struct if requested */
    if( nlhs > 3 ) {
        const mwSize ndims[] = {1, 1};
        const char* fields[] = {"fmt",};

        plhs[3] = mxCreateStructArray(1, ndims, 1, fields);

        get_opts(&sf_file_info, sf_input_file, plhs[3]);
    }

    if( sf_input_file ) {
        int status;
        if( (status = sf_close(sf_input_file)) == 0 )
            /* sf_close() doesn't set the pointer to NULL, and Matlab doesn't
             * like that (it prints "too many files open" errors), even though
             * this pointer is overwritten every call anyway */
            sf_input_file = NULL;
        else
            mexErrMsgIdAndTxt("msndfile:sndfile", sf_error_number(status));
    }
}
