#include <sndfile.h>
#include <mex.h>
#include "read_utils.h"

/*
 * functions used to generate the opts output argument
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
    short info_count = SF_STR_LAST-SF_STR_FIRST+1;

    double fmt_data[6];

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

        if (info_data != NULL)
        {
            const int j = sf_str_to_index(i);
            mxArray *info_array = mxCreateString(info_data);

            mxAddField(info, info_fields[j]);
            mxSetField(info, 0, info_fields[j], info_array);
        }
        else
            --info_count;
    }

    /* only add the info field if it is non-empty */
    if (info_count > 0) {
        mxAddField(opts, "info");
        mxSetField(opts, 0, "info", info);
    }
}

/* The value of SF_STR_GENRE is a bit of a jump from the previous element of the
 * enum, which makes it difficult to us the SF_STR_* values as indices.  This
 * function works around this difficulty by manually checking them and returning
 * appropriate values. */
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
