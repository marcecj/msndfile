#ifndef __MSNDFILE_H__
#define __MSNDFILE_H__

#include <sndfile.h>

/* sizes of the lookup tables */
/*
 * const int major_formats_size = 21;
 * const int sub_formats_size  = 22;
 * const int endianness_types_size =  4;
 */
/* a simple lookup table */
typedef struct {
    char* name;
    int number;
} const KEY_VAL;
typedef struct {
    KEY_VAL *table;
    int size;
} const LOOKUP_TABLE;

/* major formats */
KEY_VAL major_formats_names[] = {
    { "WAV"   , SF_FORMAT_WAV, },
    { "AIFF"  , SF_FORMAT_AIFF, },
    { "AU"    , SF_FORMAT_AU, },
    { "RAW"   , SF_FORMAT_RAW, },
    { "PAF"   , SF_FORMAT_PAF, },
    { "SVX"   , SF_FORMAT_SVX, },
    { "NIST"  , SF_FORMAT_NIST, },
    { "VOC"   , SF_FORMAT_VOC, },
    { "IRCAM" , SF_FORMAT_IRCAM, },
    { "W64"   , SF_FORMAT_W64, },
    { "MAT4"  , SF_FORMAT_MAT4, },
    { "MAT5"  , SF_FORMAT_MAT5, },
    { "PVF"   , SF_FORMAT_PVF, },
    { "XI"    , SF_FORMAT_XI, },
    { "HTK"   , SF_FORMAT_HTK, },
    { "SDS"   , SF_FORMAT_SDS, },
    { "AVR"   , SF_FORMAT_AVR, },
    { "WAVEX" , SF_FORMAT_WAVEX, },
    { "SD2"   , SF_FORMAT_SD2, },
    { "FLAC"  , SF_FORMAT_FLAC, },
    { "CAF"   , SF_FORMAT_CAF, },
};
const LOOKUP_TABLE const major_formats = {major_formats_names, 21};

/* sample formats */
KEY_VAL const sub_formats_names[] = {
    { "PCM_S8", SF_FORMAT_PCM_S8, },
    { "PCM_16", SF_FORMAT_PCM_16, },
    { "PCM_24", SF_FORMAT_PCM_24, },
    { "PCM_32", SF_FORMAT_PCM_32, },
    { "PCM_U8", SF_FORMAT_PCM_U8, },
    { "FLOAT", SF_FORMAT_FLOAT, },
    { "DOUBLE", SF_FORMAT_DOUBLE, },
    { "ULAW", SF_FORMAT_ULAW, },
    { "ALAW", SF_FORMAT_ALAW, },
    { "IMA_ADPCM", SF_FORMAT_IMA_ADPCM, },
    { "MS_ADPCM", SF_FORMAT_MS_ADPCM, },
    { "GSM610", SF_FORMAT_GSM610, },
    { "VOX_ADPCM", SF_FORMAT_VOX_ADPCM, },
    { "G721_32", SF_FORMAT_G721_32, },
    { "G723_24", SF_FORMAT_G723_24, },
    { "G723_40", SF_FORMAT_G723_40, },
    { "DWVW_12", SF_FORMAT_DWVW_12, },
    { "DWVW_16", SF_FORMAT_DWVW_16, },
    { "DWVW_24", SF_FORMAT_DWVW_24, },
    { "DWVW_N", SF_FORMAT_DWVW_N, },
    { "DPCM_8", SF_FORMAT_DPCM_8, },
    { "DPCM_16", SF_FORMAT_DPCM_16, },
};
const LOOKUP_TABLE const sub_formats = {sub_formats_names, 22};

/* endianness options. */
KEY_VAL const endianness_types_names[] = {
    { "FILE", SF_ENDIAN_FILE, },
    { "LITTLE", SF_ENDIAN_LITTLE, },
    { "BIG", SF_ENDIAN_BIG, },
    { "CPU", SF_ENDIAN_CPU, },
};
const LOOKUP_TABLE const endianness_types = {endianness_types_names, 4};

/* function to get a value from a look-up table */
int get_val(const LOOKUP_TABLE *array, const char *name);

#endif /* __MSNDFILE_H__ */
