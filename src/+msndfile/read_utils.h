#ifndef __READ_UTILS_H__
#define __READ_UTILS_H__

#include <sndfile.h>

/*
 * functions needed for the return values
 */

/* get the number of bits of an audio file */
short get_bits(SF_INFO* sf_file_info);

/* create an opts structure a la wavread() */
void get_opts(SF_INFO* sf_file_info, SNDFILE* sf_input_file, mxArray* opts);

/* The value of SF_STR_GENRE is a bit of a jump from the previous element of the
 * enum, which makes it difficult to us the SF_STR_* values as indices.  This
 * function works around this difficulty by manually checking them and returning
 * appropriate values. */
int sf_str_to_index(int i);

/* generate a value for the wFormatTag field based on the format subtype. */
int get_wformattag(SF_INFO* sf_file_info);

#endif /* __READ_UTILS_H__ */
