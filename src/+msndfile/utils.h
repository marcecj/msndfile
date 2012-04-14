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
int lookup_val(const FMT_TABLE *const array, const char *const name);

/*
 * misc functions
 */

/* returns the number of simple formats + RAW */
unsigned int get_num_formats();

/* returns a list of file extensions to simple formats + RAW */
char** get_format_extensions();

/* helper function for gen_filename(): return whether a file extension was
 * already checked */
unsigned int ext_already_checked(char** extensions, const char* ext, const unsigned int num_ext);

/* function to get a valid file name; for wavread() compatibility, if the file
 * name does not have a suffix, file_name+".wav" is attempted, and if that
 * fails, NULL is returned */
char* gen_filename(char* fname);

/*
 * return a transposed version of "input" as "output"
 *
 * TODO: maybe do an in-place transpose? Files already open in about 2/3 of
 * the time of Matlab's wavread(), so some additional time complexity
 * probably won't hurt much.
 */
void transpose_data(void* output, void* input, int num_frames, int num_chns, mxClassID class_id);

/* get information about a file from an args pointer and transfer it to an
 * SF_INFO struct */
void get_file_info(SF_INFO* sf_file_info, char* sf_in_fname, const mxArray *const  args);

/* get the number of frames to be read and check for valid ranges */
int get_num_frames(const SF_INFO* const sf_file_info, SNDFILE* sf_input_file, const mxArray *const arg);

/* check the fmt argument and return true or false */
int get_fmt(const char* const args);

/* get the mxClassID corresponding to a format subtype */
mxClassID get_class_id(SF_INFO* sf_file_info);

#endif /* __UTILS_H__ */
