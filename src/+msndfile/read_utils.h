/*
 * Copyright (C) 2010-2013 Marc Joliet
 *
 * Full license information can be found in the file LICENSE in the top-level
 * directory of the source repository.
 */

#ifndef __READ_UTILS_H__
#define __READ_UTILS_H__

#include <sndfile.h>

/*
 * functions used to generate a valid file name
 */

/* returns the number of simple formats + RAW */
unsigned int get_num_formats();

/* returns a list of file extensions to simple formats + RAW */
char** get_format_extensions();

/* helper function for gen_filename(): return whether a file extension was
 * already checked */
unsigned int ext_already_checked(char* restrict * restrict extensions, const char* const restrict ext, const unsigned int num_ext);

/* function to get a valid file name; for wavread() compatibility, if the file
 * name does not have a suffix, file_name+".wav" is attempted, and if that
 * fails, NULL is returned */
char* gen_filename(char* fname);

/*
 * functions needed for the return values
 */

/* get the number of bits of an audio file */
short get_bits(const SF_INFO* const restrict sf_file_info);

/*
 * functions used to generate the opts output argument
 */

/* create an opts structure a la wavread() */
void get_opts(const SF_INFO* const restrict sf_file_info, SNDFILE* const restrict sf_input_file, mxArray* restrict opts);

/* The value of SF_STR_GENRE is a bit of a jump from the previous element of the
 * enum, which makes it difficult to us the SF_STR_* values as indices.  This
 * function works around this difficulty by manually checking them and returning
 * appropriate values. */
int sf_str_to_index(const int i);

/* generate a value for the wFormatTag field based on the format subtype. */
int get_wformattag(const SF_INFO* const restrict sf_file_info);

#endif /* __READ_UTILS_H__ */
