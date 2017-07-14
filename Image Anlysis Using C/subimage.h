/*
	subimage header file
	Written as part of code clinic: C by lynda
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <jpeglib.h>
#include <setjmp.h>

/* Global variables */

struct image {
	char *filename;
	unsigned char *raw;
	int width;
	int height;
	int bytes_per_pixel;
};

/* stolen from https://github.com/Windower/libjpeg/blob/master/example.c */
struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjump_buffer;
};
typedef struct my_error_mgr *my_error_ptr;

int reduction_factor;
int mismatch_percentage;
int variation;

/* Function prototypes */

int read_jpeg(struct image *j);
void my_error_exit(j_common_ptr cinfo);
int compare(struct image *org, struct image *dup);
int check_row(unsigned char *a, unsigned char *b, int length);
