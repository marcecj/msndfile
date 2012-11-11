#include <string.h>
#include <sndfile.h>
#include <mex.h>
#include "read_utils.h"

/*
 * functions used to generate a valid file name
 */

/* returns the number of simple formats + RAW */
unsigned int get_num_formats()
{
    int num_formats;

    sf_command(0, SFC_GET_SIMPLE_FORMAT_COUNT, &num_formats, sizeof(int));

    /* SFC_GET_SIMPLE_FORMAT returns the highest valid format ID (i.e., 0 <=
     * format <= ID) , so increment by 1 to get a standard C count (0 <= format
     * < ID); furthermore, RAW is missing, as it is not a simple format, but we
     * handle it anyway, so another +1 */
    return num_formats+2;
}

/* returns a list of file extensions to simple formats + RAW */
char** get_format_extensions()
{
    unsigned int i;
    const unsigned int num_formats = get_num_formats();
    char** file_exts = (char**)malloc(num_formats*sizeof(char*));
    SF_FORMAT_INFO format_info;

    /* handle the libsndfile simple formats */
    for( i = 0; i < num_formats-1; i++ ) {
        format_info.format = i;

        sf_command(0, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof(SF_FORMAT_INFO));

        file_exts[i] = (char*)malloc((strlen(format_info.extension)+1)*sizeof(char));
        file_exts[i] = strcpy(file_exts[i], format_info.extension);
    }

    /* RAW is not a simple format, but we want to handle it */
    file_exts[num_formats-1] = (char*)malloc(4*sizeof(char));
    file_exts[num_formats-1] = strcpy(file_exts[i], "raw");

    return file_exts;
}

/* helper function for gen_filename(): return whether a file extension was
 * already checked */
unsigned int ext_already_checked(char** extensions, const char* ext, const unsigned int num_ext)
{
    unsigned int i;
    for( i = 0; i < num_ext; i++ )
        if( strcmp(extensions[i], ext) == 0 )
            return 1;

    return 0;
}

/* function to get a valid file name; for wavread() compatibility, if the file
 * name does not have a suffix, file_name+".wav" is attempted, and if that
 * fails, NULL is returned */
char* gen_filename(char* fname)
{
    const size_t N                 = strlen(fname);
    const unsigned int num_formats = get_num_formats();
    char** file_exts               = NULL;
    char** read_exts               = NULL;
    FILE* audio_file               = NULL;
    unsigned int num_read_exts     = 0;
    unsigned int num_files         = 0; /* file name ambiguity if num_files>1 */
    unsigned int i;

    /* if the file name (probably) has a suffix, the file name is OK */
    if( strrchr(fname, '.') != NULL )
        goto get_filename_cleanup;

    /*
     * For each file type extension, append it to the original file name and
     * check if such a file exists.  In the case of multiple candidates, default
     * to WAV.  If no WAV file exists, return NULL.
     */

    file_exts = get_format_extensions();

    for( i = 0; i < num_formats; i++ ) {
        char* tmp_fname      = NULL;
        const char* cur_ext  = file_exts[i];
        const size_t ext_len = strlen(cur_ext)+1; /* '.' + extension */
        const size_t new_len = N+ext_len+1;

        /* get_format_extensions() returns duplicate entries, so check here if
         * the extension has already been tried */
        if( ext_already_checked(read_exts, cur_ext, num_read_exts) )
            continue;

        /* append the current extension to the list of checked extensions */
        read_exts = (char**)realloc(read_exts, (num_read_exts+1)*sizeof(char*));
        read_exts[num_read_exts] = (char*)malloc(ext_len*sizeof(char));
        read_exts[num_read_exts] = strcpy(read_exts[num_read_exts], cur_ext);
        num_read_exts++;

        /* copy the original N chars from fname into tmp_fname */
        tmp_fname = (char*)calloc(new_len, sizeof(char));
        tmp_fname = strncpy(tmp_fname, fname, N);

        /* append the file type extension */
        tmp_fname = strcat(tmp_fname, ".");
        tmp_fname = strcat(tmp_fname, cur_ext);

        /* try to open the file; continue with next extension on failure */
        if( (audio_file = fopen(tmp_fname, "r")) == NULL ) {
            free(tmp_fname);
            continue;
        }

        /*  overwrite the original file name */
        fclose(audio_file); /* close temporary file */
        num_files++;
        fname = (char*)mxRealloc(fname, new_len*sizeof(char));
        fname = strcpy(fname, tmp_fname);
        free(tmp_fname);

        /* break as soon as a WAV file is found */
        if( strcmp(cur_ext, "wav") == 0 )
            break;
    }

get_filename_cleanup:
    if( read_exts ) {
        for( i = 0; i < num_read_exts; i++ )
            free(read_exts[i]);
        free(read_exts);
    }

    if( file_exts ) {
        for( i = 0; i < num_formats; i++ )
            free(file_exts[i]);
        free(file_exts);
    }

    /* multiple candidates were found, but no WAV file */
    if( num_files > 1 && strcmp(&fname[strlen(fname)-3], "wav") != 0 ) {
        mxFree(fname);
        fname = NULL;
    }

    if( num_files > 1 && fname != NULL ) {
        const char msg_fmt[] = "Defaulted to file name \"%s\".";
        const size_t msg_len = strlen(fname) + strlen(msg_fmt) - 1;
        char* message = (char*)malloc(msg_len*sizeof(char));

        sprintf(message, msg_fmt, fname);
        mexWarnMsgIdAndTxt("msndfile:read:ambiguousname", message);

        free(message);
    }

    return fname;
}

/*
 * functions used to generate return values
 */

/* get the number of bits of an audio file */
short get_bits(SF_INFO* sf_file_info)
{
    /* a best effort attempt to get the number of bits of an audio file */
    short bits = 0;
    switch(sf_file_info->format & SF_FORMAT_SUBMASK)
    {
        case SF_FORMAT_PCM_S8:
        case SF_FORMAT_PCM_U8:
        case SF_FORMAT_DPCM_8:
            bits =  8; break;
        case SF_FORMAT_DWVW_12:
            bits = 12; break;
        case SF_FORMAT_PCM_16:
        case SF_FORMAT_DPCM_16:
        case SF_FORMAT_DWVW_16:
            bits = 16; break;
        case SF_FORMAT_PCM_24:
        case SF_FORMAT_DWVW_24:
            bits = 24; break;
        case SF_FORMAT_PCM_32:
        case SF_FORMAT_FLOAT:
            bits = 32; break;
        case SF_FORMAT_DOUBLE:
            bits = 64; break;
    }
    return bits;
}

/*
 * functions used to generate the opts output argument
 */

/* create an opts structure a la wavread() */
void get_opts(SF_INFO* sf_file_info, SNDFILE* sf_input_file, mxArray* opts)
{
    int i;
    const short nbits      = get_bits(sf_file_info);
    const mwSize ndims[]   = {1, 1};

    const char* fmt_fields[] = {
        "wFormatTag",
        "nChannels",
        "nSamplesPerSec",
        "nAvgBytesPerSec",
        "nBlockAlign",
        "nBitsPerSample"
    };

    const char* bext_fields[] = {
        "description",
        "originator",
        "originator_reference",
        "origination_date",
        "origination_time",
        "time_reference",
        "version",
        "umid",
        "coding_history"
    };


    /* see e.g. http://www.kk.iij4u.or.jp/~kondo/wave/mpidata.txt */
    const char* info_fields[] = {
        "inam", /* Title */
        "icop", /* Copyright */
        "isft", /* Software */
        "iart", /* Artist */
        "icmt", /* Comment */
        "icrd", /* Date */
        "ialb", /* Album */
        "ilic", /* License */
        "inum", /* Track number */
        "ignr", /* Genre */
    };

    const short num_fmt_fields  = sizeof(fmt_fields)/sizeof(char*);
    const short num_bext_fields = sizeof(bext_fields)/sizeof(char*);
    short info_count = SF_STR_LAST-SF_STR_FIRST+1;

    double fmt_data[6];
    SF_BROADCAST_INFO bwv_data;

    mxArray *fmt           = mxCreateStructArray(1, ndims, num_fmt_fields, fmt_fields);
    mxArray *info          = mxCreateStructArray(1, ndims, 0, NULL);

    /*
     * set fmt field
     */

    fmt_data[0] = (double)get_wformattag(sf_file_info);
    fmt_data[1] = (double)sf_file_info->channels;
    fmt_data[2] = (double)sf_file_info->samplerate;
    fmt_data[3] = (double)(sf_file_info->samplerate*(nbits/8)*sf_file_info->channels);
    fmt_data[4] = (double)(sf_file_info->channels*nbits/8); /* see wavread() */
    fmt_data[5] = (double)nbits;

    for( i = 0; i < num_fmt_fields; i++ )
        mxSetField(fmt, 0, fmt_fields[i], mxCreateDoubleScalar(fmt_data[i]));

    /* remove the wFormatTag field if the file is not a WAV file */
    if (fmt_data[0] == -1)
        mxRemoveField(fmt, 0);

    mxSetField(opts, 0, "fmt", fmt);

    /*
     * set info field
     */

    for( i = SF_STR_FIRST; i <= SF_STR_LAST; i++ )
    {
        const char* info_data = sf_get_string(sf_input_file, i);

        if( info_data != NULL )
        {
            const int j = sf_str_to_index(i);

            mxAddField(info, info_fields[j]);
            mxSetField(info, 0, info_fields[j], mxCreateString(info_data));
        }
        else
            --info_count;
    }

    /* only add the info field if it is non-empty */
    if (info_count > 0) {
        mxAddField(opts, "info");
        mxSetField(opts, 0, "info", info);
    }

    /*
     * set broadcast wave info
     */

    if( sf_command(sf_input_file, SFC_GET_BROADCAST_INFO, &bwv_data, sizeof(SF_BROADCAST_INFO)) == SF_TRUE )
    {
        mxArray *bext = mxCreateStructArray(1, ndims, num_bext_fields, bext_fields);
        const double time_ref_samples = 4294967296.l*bwv_data.time_reference_high + bwv_data.time_reference_low;

        /*
         * Each char[] field needs to be copied, because if a field is "full",
         * the lack of null byte leads to "overlapping" strings in opts.bext.
         * (calloc() and num_elements+1 account for the trailing null byte.)
         * */
        char* description          = (char*)calloc(sizeof(bwv_data.description)/sizeof(char)+1, sizeof(char));
        char* originator           = (char*)calloc(sizeof(bwv_data.originator)/sizeof(char)+1, sizeof(char));
        char* originator_reference = (char*)calloc(sizeof(bwv_data.originator_reference)/sizeof(char)+1, sizeof(char));
        char* origination_date     = (char*)calloc(sizeof(bwv_data.origination_date)/sizeof(char)+1, sizeof(char));
        char* origination_time     = (char*)calloc(sizeof(bwv_data.origination_time)/sizeof(char)+1, sizeof(char));
        char* umid                 = (char*)calloc(sizeof(bwv_data.umid)/sizeof(char)+1, sizeof(char));
        char* coding_history       = (char*)calloc(sizeof(bwv_data.coding_history)/sizeof(char)+1, sizeof(char));

        description          = (char*)memcpy(description          , bwv_data.description          , sizeof(bwv_data.description));
        originator           = (char*)memcpy(originator           , bwv_data.originator           , sizeof(bwv_data.originator));
        originator_reference = (char*)memcpy(originator_reference , bwv_data.originator_reference , sizeof(bwv_data.originator_reference));
        origination_date     = (char*)memcpy(origination_date     , bwv_data.origination_date     , sizeof(bwv_data.origination_date));
        origination_time     = (char*)memcpy(origination_time     , bwv_data.origination_time     , sizeof(bwv_data.origination_time));
        umid                 = (char*)memcpy(umid                 , bwv_data.umid                 , sizeof(bwv_data.umid));
        coding_history       = (char*)memcpy(coding_history       , bwv_data.coding_history       , bwv_data.coding_history_size);

        mxSetField(bext, 0, bext_fields[0], mxCreateString(description));
        mxSetField(bext, 0, bext_fields[1], mxCreateString(originator));
        mxSetField(bext, 0, bext_fields[2], mxCreateString(originator_reference));
        mxSetField(bext, 0, bext_fields[3], mxCreateString(origination_date));
        mxSetField(bext, 0, bext_fields[4], mxCreateString(origination_time));
        mxSetField(bext, 0, bext_fields[5], mxCreateDoubleScalar(time_ref_samples));
        mxSetField(bext, 0, bext_fields[6], mxCreateDoubleScalar(bwv_data.version));
        mxSetField(bext, 0, bext_fields[7], mxCreateString(umid));
        mxSetField(bext, 0, bext_fields[8], mxCreateString(coding_history));

        free(description);
        free(originator);
        free(originator_reference);
        free(origination_date);
        free(origination_time);
        free(umid);
        free(coding_history);

        mxAddField(opts, "bext");
        mxSetField(opts, 0, "bext", bext);
    }
}

/* The value of SF_STR_GENRE is a bit of a jump from the previous element of the
 * enum, which makes it difficult to calculate indices based on the SF_STR_*
 * values.  This function works around this difficulty by manually checking them
 * and returning appropriate values. */
int sf_str_to_index(int i)
{
    switch(i)
    {
        case SF_STR_TITLE:
        case SF_STR_COPYRIGHT:
        case SF_STR_SOFTWARE:
        case SF_STR_ARTIST:
        case SF_STR_COMMENT:
        case SF_STR_DATE:
        case SF_STR_ALBUM:
        case SF_STR_LICENSE:
        case SF_STR_TRACKNUMBER:
            return i - SF_STR_FIRST;
    }

    return SF_STR_TRACKNUMBER;
}

/* generate a value for the wFormatTag field based on the format subtype. */
int get_wformattag(SF_INFO* sf_file_info)
{
    /* TODO: maybe add exceptions for other WAV-like formats? */
    if( (sf_file_info->format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV)
        return -1;

    /* see e.g. http://www.sonicspot.com/guide/wavefiles.html#wavefileheader */
    switch(sf_file_info->format & SF_FORMAT_SUBMASK)
    {
        case SF_FORMAT_PCM_S8:
        case SF_FORMAT_PCM_U8:
        case SF_FORMAT_PCM_16:
        case SF_FORMAT_PCM_24:
        case SF_FORMAT_PCM_32:
            return 1;
        case SF_FORMAT_MS_ADPCM:
            return 2;
        case SF_FORMAT_ALAW:
            return 6;
        case SF_FORMAT_ULAW:
            return 7;
        case SF_FORMAT_IMA_ADPCM:
            return 17;
        case SF_FORMAT_G723_24:
        case SF_FORMAT_G723_40:
            return 20;
        case SF_FORMAT_GSM610:
            return 49;
        case SF_FORMAT_G721_32:
            return 64;
    }

    return 0;
}
