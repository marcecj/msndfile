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
    SF_INFO*         sf_file_info  = NULL;
    SNDFILE*         sf_input_file = NULL;
    AUDIO_FILE_INFO* file_info     = NULL;

    int         sndfile_err; // libsndfile error status
    int         num_chns;
    const int   cmd_size = (nrhs > 0 ? mxGetN(prhs[0])+1 : 0); // length of the command
    const int   str_size = (nrhs > 1 ? mxGetN(prhs[1])+1 : 0); // length of the input file name
    int         cmd_id = -1;
    char        *cmd_str;
    char        *sf_in_fname=NULL; // input file name
    sf_count_t  num_frames=0, processed_frames=0;

    mexAtExit(&clear_static_vars);

    if( nrhs < 1 || !mxIsChar(prhs[0]))
        mexErrMsgTxt("Missing argument: you need to pass a command (either 'open', 'read', or 'close').");

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
    else if( strcmp(cmd_str, "closeall") == 0 )
        cmd_id = CMD_CLOSEALL;
    free(cmd_str);

    if( cmd_id == -1 )
        mexErrMsgTxt("Unknown command.");

    if( nrhs > 1 && mxIsChar(prhs[1]) && cmd_id != CMD_CLOSEALL )
    {
        /* get input filename */
        sf_in_fname = (char*)calloc(str_size, sizeof(char));
        if( sf_in_fname == NULL ) {
            mexErrMsgTxt("calloc error!");
        }
        mxGetString(prhs[1], sf_in_fname, str_size);
    }
    else if( cmd_id != CMD_CLOSEALL )
        mexErrMsgTxt("Missing argument: you need to pass a file name.");

    if( cmd_id == CMD_OPEN )
    {
        if( lookup_file_info(file_list, sf_in_fname) != NULL ) {
            free(sf_in_fname);
            mexErrMsgTxt("File already open!");
        }

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

            if( !mxIsStruct(prhs[2]) ) {
                free(sf_in_fname);
                free(sf_file_info);
                sf_file_info = NULL;
                mexErrMsgTxt("The second argument has to be a struct! (see help text)");
            }

            get_file_info(sf_file_info, sf_in_fname, prhs[2]);
        }

        sf_input_file = sf_open(sf_in_fname, SFM_READ, sf_file_info);

        if( sf_input_file == NULL ) {
            free(sf_file_info);
            sf_file_info = NULL;
            mexErrMsgTxt("Could not open audio file.");
        }

        file_info = create_file_info(sf_in_fname, sf_file_info, sf_input_file);
        file_list = store_file_info(file_list, file_info);
    }
    else if( cmd_id == CMD_CLOSE )
    {
        if( lookup_file_info(file_list, sf_in_fname) != NULL )
            file_list = remove_file_info(file_list, sf_in_fname);
    }
    else if( cmd_id == CMD_CLOSEALL )
    {
        destroy_file_list(file_list);
        file_list = NULL;
    }
    else if( cmd_id == CMD_READ )
    {
        bool        do_transpose=true;
        double*     temp_array;

        /*
         * allocate the strings corresponding to the names of the major formats,
         * format subtypes and the endianness as per the libsndfile documentation
         */
        file_info = lookup_file_info(file_list, sf_in_fname);

        if( file_info == NULL )
            mexErrMsgTxt("File not open!");

        if( nrhs < 3 )
            mexErrMsgTxt("Missing argument: no range specified!");

        if( !mxIsEmpty(prhs[2]) && mxIsDouble(prhs[2]))
        {
            const double *start_end_idx = mxGetPr(prhs[2]);
            const int    range_size     = mxGetN(prhs[2]);

            if( range_size == 2 ) {
                num_frames = (sf_count_t)(start_end_idx[1] - start_end_idx[0] + 1);

                if( sf_seek(file_info->file, start_end_idx[0]-1, SEEK_SET) < 0 )
                    mexErrMsgTxt("Invalid range!");
            }
            else if( range_size == 1 )
                num_frames = (sf_count_t)(start_end_idx[0]);
            else
                mexErrMsgTxt("Range can be a row vector with 1 or 2 elements.");
        }
        else
            num_frames = file_info->info->frames;

        if( nrhs == 4 && mxIsLogicalScalar(prhs[3]) ) {
            do_transpose = *mxGetPr(prhs[3]);
        }

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
        processed_frames = sf_readf_double(file_info->file, temp_array, num_frames);
        if( processed_frames == 0 ) {
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
        sndfile_err = sf_error(file_info->file);
        if( sndfile_err != SF_ERR_NO_ERROR ) {
            mexWarnMsgTxt("libsndfile error!");
            mexErrMsgTxt(sf_error_number(sndfile_err));
        }
    }

    /* free memory */
    free(sf_in_fname);
}
