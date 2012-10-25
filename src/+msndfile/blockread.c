#include <errno.h>
#include <string.h>
#include <mex.h>
#include <sndfile.h>
#include "utils.h"
#include "audio_files.h"

/*
 * This is a simple mex-File using libsndfile for reading in audio files
 */

enum {
    CMD_OPEN=0,
    CMD_READ,
    CMD_SEEK,
    CMD_TELL,
    CMD_CLOSE,
    CMD_CLOSEALL
};

static AUDIO_FILES* file_list=NULL;

void clear_static_vars()
{
    destroy_file_list(file_list);
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    const int   cmd_size = (nrhs > 0 ? mxGetN(prhs[0])+1 : 0); /* length of the command */
    const int   str_size = (nrhs > 1 ? mxGetN(prhs[1])+1 : 0); /* length of the input file name */
    int         cmd_id = -1;
    char        *cmd_str;
    char        *sf_in_fname=NULL; /* input file name */

    mexAtExit(&clear_static_vars);

    if( nrhs < 1 || !mxIsChar(prhs[0]))
        mexErrMsgIdAndTxt("msndfile:argerror", "Missing argument: you need to pass a command (see help).");

    if( mxIsEmpty(prhs[0]) || !mxIsChar(prhs[0]))
        mexErrMsgIdAndTxt("msndfile:argerror", "Argument error: command may not be empty.");

    if( (cmd_str = (char*)malloc(cmd_size*sizeof(char))) == NULL )
        mexErrMsgIdAndTxt("msndfile:argerror", strerror(errno));

    if( mxGetString(prhs[0], cmd_str, cmd_size) == 1 ) {
        free(cmd_str);
        mexErrMsgIdAndTxt("msndfile:argerror", "Error getting command string.");
    }

    if(      strcmp(cmd_str, "open") == 0 )     cmd_id = CMD_OPEN;
    else if( strcmp(cmd_str, "read") == 0 )     cmd_id = CMD_READ;
    else if( strcmp(cmd_str, "seek") == 0 )     cmd_id = CMD_SEEK;
    else if( strcmp(cmd_str, "tell") == 0 )     cmd_id = CMD_TELL;
    else if( strcmp(cmd_str, "close") == 0 )    cmd_id = CMD_CLOSE;
    else if( strcmp(cmd_str, "closeall") == 0 ) cmd_id = CMD_CLOSEALL;
    free(cmd_str);

    if( cmd_id == -1 )
        mexErrMsgIdAndTxt("msndfile:argerror", "Unknown command.");

    if( nrhs > 1 && mxIsChar(prhs[1]) && cmd_id != CMD_CLOSEALL )
    {
        /* get input filename */
        sf_in_fname = (char*)calloc(str_size, sizeof(char));
        if( !sf_in_fname )
            mexErrMsgIdAndTxt("msndfile:system", strerror(errno));

        mxGetString(prhs[1], sf_in_fname, str_size);
    }
    else if( cmd_id != CMD_CLOSEALL )
        mexErrMsgIdAndTxt("msndfile:argerror", "Missing argument: you need to pass a file name.");

    if( cmd_id == CMD_OPEN )
    {
        AUDIO_FILE_INFO* file_info = NULL;
        SNDFILE* sf_input_file     = NULL;
        SF_INFO* sf_file_info      = NULL;

        if( lookup_file_info(file_list, sf_in_fname) != NULL ) {
            free(sf_in_fname);
            mexErrMsgIdAndTxt("msndfile:argerror", "File already open!");
        }

        /* initialize sf_file_info struct pointer */
        if( (sf_file_info = (SF_INFO*)malloc(sizeof(SF_INFO))) == NULL ) {
            free(sf_in_fname);
            mexErrMsgIdAndTxt("msndfile:system", strerror(errno));
        }

        /* "format" needs to be set to 0 before a file is opened for reading,
         * unless the file is a RAW file */
        sf_file_info->format = 0;
        if( nrhs >= 3 )
        {
            /* handle RAW files */
            if( !mxIsStruct(prhs[2]) ) {
                free(sf_in_fname);
                free(sf_file_info);
                mexErrMsgIdAndTxt("msndfile:argerror", "The second argument has to be a struct! (see help text)");
            }

            get_file_info(sf_file_info, sf_in_fname, prhs[2]);
        }

        if( (sf_input_file = sf_open(sf_in_fname, SFM_READ, sf_file_info)) == NULL ) {
            free(sf_file_info);
            mexErrMsgIdAndTxt("msndfile:sndfile", sf_strerror(sf_input_file));
        }

        file_info = create_file_info(sf_in_fname, sf_file_info, sf_input_file);
        file_list = store_file_info(file_list, file_info);
    }
    else if( cmd_id == CMD_SEEK )
    {
        AUDIO_FILE_INFO* file_info = NULL;
        double *seek_idx;

        if( (file_info = lookup_file_info(file_list, sf_in_fname)) == NULL )
            mexErrMsgIdAndTxt("msndfile:filenotopen", "File not open!");

        if( nrhs < 3 )
            mexErrMsgIdAndTxt("msndfile:argerror", "Missing argument: no frame index specified!");

        if( mxIsEmpty(prhs[2]) && !mxIsDouble(prhs[2]))
            mexErrMsgIdAndTxt("msndfile:argerror", "Frame index is empty!");

        seek_idx = mxGetPr(prhs[2]);

        if( seek_idx[0] > file_info->info->frames
                || sf_seek(file_info->file, seek_idx[0]-1, SEEK_SET) == -1 )
            mexErrMsgIdAndTxt("msndfile:argerror", "Invalid frame index!");
    }
    else if( cmd_id == CMD_TELL )
    {
        AUDIO_FILE_INFO* file_info = NULL;
        double *cur_pos;

        if( (file_info = lookup_file_info(file_list, sf_in_fname)) == NULL )
            mexErrMsgIdAndTxt("msndfile:filenotopen", "File not open!");

        plhs[0] = mxCreateDoubleMatrix((int)1, 1, mxREAL);
        cur_pos = mxGetPr(plhs[0]);

        if( (*cur_pos = sf_seek(file_info->file, 0, SEEK_CUR)) == -1 )
            mexErrMsgIdAndTxt("msndfile:sndfile", sf_error_number(*cur_pos));

        (*cur_pos)++;
    }
    else if( cmd_id == CMD_CLOSE )
    {
        file_list = remove_file_info(file_list, sf_in_fname);
    }
    else if( cmd_id == CMD_CLOSEALL )
    {
        destroy_file_list(file_list);
        file_list = NULL;
    }
    else if( cmd_id == CMD_READ )
    {
        AUDIO_FILE_INFO* file_info = NULL;
        bool        do_transpose=true;
        double*     temp_array;
        int         num_chns;
        sf_count_t  num_frames=0;
        int         sndfile_err; /* libsndfile error status */

        if( (file_info = lookup_file_info(file_list, sf_in_fname)) == NULL )
            mexErrMsgIdAndTxt("msndfile:filenotopen", "File not open!");

        if( nrhs < 3 )
            mexErrMsgIdAndTxt("msndfile:argerror", "Missing argument: no range specified!");

        if( !mxIsEmpty(prhs[2]) && mxIsDouble(prhs[2]))
            num_frames = get_num_frames(file_info->info, file_info->file, prhs[2]);
        else
            num_frames = file_info->info->frames;

        if( nrhs == 4 && mxIsLogicalScalar(prhs[3]) )
            do_transpose = *mxGetPr(prhs[3]);

        /* initialise Matlab output array */
        num_chns = file_info->info->channels;

        if( do_transpose ) {
            plhs[0]    = mxCreateDoubleMatrix((int)num_frames, num_chns, mxREAL);
            temp_array = (double*)malloc((int)num_frames*num_chns*sizeof(double));
        } else {
            plhs[0]    = mxCreateDoubleMatrix(num_chns, (int)num_frames, mxREAL);
            temp_array = mxGetPr(plhs[0]);
        }

        /* read the entire file in one go */
        if( sf_readf_double(file_info->file, temp_array, num_frames) == 0 ) {
            if( do_transpose )
                free(temp_array);
            mexErrMsgTxt("Error reading frames from input file: 0 frames read!");
        }

        /*
         * transpose returned data
         */
        if( do_transpose )
        {
            double* output = mxGetPr(plhs[0]);

            int i;
            for( i=0; i<num_frames; i++ ) {
                int j;
                for( j=0; j<num_chns; j++ )
                    output[i+j*num_frames] = temp_array[i*num_chns+j];
            }

            free(temp_array);
        }

        /* rudimentary way of dealing with libsndfile errors */
        if( (sndfile_err = sf_error(file_info->file)) != SF_ERR_NO_ERROR )
            mexErrMsgIdAndTxt("msndfile:sndfile", sf_error_number(sndfile_err));
    }

    /* free memory */
    free(sf_in_fname);
}
