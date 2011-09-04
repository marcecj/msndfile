#include <string.h>
#include <mex.h>
#include "utils.h"
#include "format_tables.h"

/* the max length of a format string (9 for "IMA_ADPCM" & "VOX_ADPCM") + \0 */
#define FMT_STR_SIZE 10

/* function to get a value from a look-up table */
int lookup_val(const FMT_TABLE *array, const char *name)
{
    int i;
    for(i = 0; i < array->size; i++) {
        if( strcmp(name, array->table[i].name) == 0 )
            return array->table[i].number;
    }

	return 0;
}

/* function that gets the information on a file from the args pointer and
 * transfers it to the sf_file_info struct */
void get_file_info(SF_INFO* sf_file_info, char* sf_in_fname, const mxArray const* args)
{
	/* a temporary array */
	mxArray *tmp_ptr = NULL;

	/* the three OR-ed components of the "format" field in sf_file_info */
	char maj_fmt_name[FMT_STR_SIZE] = "RAW";
	char sub_fmt_name[FMT_STR_SIZE];
	char endianness_name[FMT_STR_SIZE] = "FILE";

	/*
	 * get the sample rate and the number of channels
	 */

	tmp_ptr = mxGetField(args, 0, "samplerate" );
	if( tmp_ptr != NULL )
		sf_file_info->samplerate = (int)*mxGetPr(tmp_ptr);
	else {
		free(sf_in_fname);
		free(sf_file_info);
		mexErrMsgTxt("Field 'samplerate' not set.");
	}

	tmp_ptr = mxGetField(args, 0, "channels" );
	if( tmp_ptr != NULL )
		sf_file_info->channels = (int)*mxGetPr(tmp_ptr);
	else {
		free(sf_in_fname);
		free(sf_file_info);
		mexErrMsgTxt("Field 'channels' not set.");
	}

	/*
	 * get the format information
	 */

	/* format name should be set to RAW when reading RAW files */
	tmp_ptr = mxGetField(args, 0, "format" );
	if( tmp_ptr != NULL )
		mxGetString(tmp_ptr, maj_fmt_name, FMT_STR_SIZE);

	tmp_ptr = mxGetField(args, 0, "sampleformat" );
	if( tmp_ptr != NULL )
		mxGetString(tmp_ptr, sub_fmt_name, FMT_STR_SIZE);
	else {
		free(sf_in_fname);
		free(sf_file_info);
		mexErrMsgTxt("Field 'sampleformat' not set.");
	}

	/* endianness_name does not need to be set */
	tmp_ptr = mxGetField(args, 0, "endianness" );
	if( tmp_ptr != NULL )
		mxGetString(tmp_ptr, endianness_name, mxGetN(tmp_ptr)+1);

	/* sf_file_info->format = lookup_val(&maj_fmts, maj_fmt_name) | \ */
	sf_file_info->format = SF_FORMAT_RAW
		| lookup_val(&sub_fmts, sub_fmt_name)
		| lookup_val(&endianness_types, endianness_name);

	/* check format for validity */
	if( !sf_format_check(sf_file_info) ) {
		mexPrintf("Format '%x' invalid.\n", sf_file_info->format);
		free(sf_in_fname);
		free(sf_file_info);
		mexErrMsgTxt("Invalid format specified.");
	}
}
