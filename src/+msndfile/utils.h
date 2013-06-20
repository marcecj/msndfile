/*
 * Copyright (C) 2010-2013 Marc Joliet
 *
 * Full license information can be found in the file LICENSE in the top-level
 * directory of the source repository.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <mex.h>
#include <sndfile.h>

/* a simple lookup table that associates a format string with a format id */
typedef struct {
    char* name;
    int number;
} KEY_VAL;

typedef struct {
    const KEY_VAL *table;
    int size;
} FMT_TABLE;

/*
 * libsndfile format look-up functions
 */

/* get a value from a look-up table */
int lookup_val(const FMT_TABLE *const restrict array, const char *const restrict name);

/*
 * misc functions
 */

/*
 * return a transposed version of "input" as "output"
 *
 * TODO: maybe do an in-place transpose? Files already open in about 2/3 of
 * the time of Matlab's wavread(), so some additional time complexity
 * probably won't hurt much.
 */
void transpose_data(const void* restrict output, const void* restrict input, const int num_frames, const int num_chns, const mxClassID class_id);

/* get information about a file from an args pointer and transfer it to an
 * SF_INFO struct */
void get_file_info(SF_INFO* restrict sf_file_info, char* restrict sf_in_fname, const mxArray *const  restrict args);

/* get the number of frames to be read and check for valid ranges */
int get_num_frames(const SF_INFO* const restrict sf_file_info, SNDFILE* restrict sf_input_file, const mxArray *const restrict arg);

/* check the fmt argument and return true or false */
int get_fmt(const char* const restrict args);

/* get the mxClassID corresponding to a format subtype */
mxClassID get_class_id(const SF_INFO* restrict const sf_file_info);

#endif /* __UTILS_H__ */
