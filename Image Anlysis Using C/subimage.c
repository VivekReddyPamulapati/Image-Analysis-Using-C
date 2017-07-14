/*
	subimage
	Written as part of code clinic: C by lynda

	Compares all JPEG image files in the current directory for subimages. A "subimage" is a cropped
	version of the original image. Both images must have the same resolution.
	
	This code requires installation of the libjpeg C language library. Detailed info on libjpeg
	can be found here: https://github.com/Windower/libjpeg/blob/master/libjpeg.txt
*/

#include "subimage.h"

int main(int argc, char *argv[])
{
	DIR *dir1,*dir2;
	struct dirent *source_file,*target_file;
	struct stat source_stat,target_stat;
	struct image original_image,potential_duplicate;
	int r1,r2,r3,matches;

	/* check for and process the --help switch */
	if( argc > 1 && strcmp(argv[1],"--help") == 0)
	{
		puts("subimage\nWritten by Dan Gookin, 2015\n");
		puts("Processes the current directory for images that are cropped");
		puts("versions of other images. Only JPEG images are compared.");
		return(1);
	}

	/* set global variables */
	reduction_factor = 5;					/* scale value for reducing image, i.e., 1/5 */
	mismatch_percentage	= 10;				/* one scanline can have up to 10% mismatched bytes and be okay */
	variation = 24;							/* amount, plus or minus, a byte comparison can be off and still be okay */

	dir1 = opendir(".");					/* use current directory */
	if(dir1 == NULL)
	{
		fprintf(stderr,"Unable to read directory.\n");
		exit(1);
	}
	/* read the directory and hunt for image files */
	while((source_file = readdir(dir1)) != NULL)
	{
		stat(source_file->d_name,&source_stat);
		/* Skip any directories */
		if( S_ISDIR(source_stat.st_mode))
			continue;
		/* Skip any dot-prefix files */
		if( source_file->d_name[0] == '.')
			continue;
		/* Initialize structure values before the call */
		original_image.raw = (unsigned char *)malloc(1);
		original_image.filename = source_file->d_name;
		original_image.width = original_image.height = original_image.bytes_per_pixel = 0;
		printf("Source: %s",original_image.filename);
		r1 = read_jpeg(&original_image);		/* call to libjpeg library to read in an image */
		/* A return value of 0 is a match */
		if( r1==0 )
		{
			matches = 0;
			printf("\nTarget: ");
			dir2 = opendir(".");
			/* Read every other file in the directory, looking for a match */
			while((target_file = readdir(dir2)) != NULL)
			{
				stat(target_file->d_name,&target_stat);
				/* Skip over directories */
				if( S_ISDIR(target_stat.st_mode))
					continue;
				/* Skip dot-prefix files */
				if( target_file->d_name[0] != '.')
				{
					/* And don't compare to the source file */
					if( strcmp(source_file->d_name,target_file->d_name) != 0)
					{
						potential_duplicate.raw = (unsigned char *)malloc(1);	/* do this before calling */
						potential_duplicate.filename = target_file->d_name;
						potential_duplicate.width = potential_duplicate.height = potential_duplicate.bytes_per_pixel = 0;
						r2 = read_jpeg(&potential_duplicate);
						/* if r2 == 0 then a jpeg file has been opened */
						if( r2 == 0)
						{
							/* compare the files */
							r3 = compare(&original_image,&potential_duplicate);
							if( r3 )
							{
								/* `matches` variable used to track multiple matches */
								if(matches)
									printf(", ");
								printf("%s *match",potential_duplicate.filename);
								matches++;
							}
						}
						free(potential_duplicate.raw);
					}
					/* no 'else' here; just ignore any invalid files */
				}
			}
			free(original_image.raw);
			if(matches == 0)
				printf("No matches");
			putchar('\n');
		}
		else
		{
			/* non-Jpeg or unreadable file encountered. Display error message */
			switch(r1)
			{
				case 1:
					printf(" (can't open)");
					break;
				case 2:
					printf(" (non-JPEG)");
					break;
				default:
					printf(" (error)");
			}
			putchar('\n');
		}
		putchar('\n');
	}
	
	/* clean-up */
	closedir(dir1);
	closedir(dir2);

	return(0);
}

/*
	Read (decompress) a jpeg image and store information in the image structure
*/
int read_jpeg(struct image *j)
{
	struct my_error_mgr jerr;
	struct jpeg_decompress_struct dcinfo;
	FILE *input;
	int i,width,height,bytes_per_pixel;
	unsigned char **row;
	
	input = fopen(j->filename,"rb");
	if(input == NULL)
	{
		return(1);
	}

	/* set our own exit routine */
	dcinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if( setjmp(jerr.setjump_buffer) )
	{
		return(2);			/* The long jump returns here if there is an error */
	}
	/* initialize jpeg structures */
	jpeg_create_decompress(&dcinfo);
	
	jpeg_stdio_src(&dcinfo,input);
	jpeg_read_header(&dcinfo,TRUE);

	/* re-size image to 1/REDUCTION_FACTOR (20%) size and convert to grayscale */
	dcinfo.scale_num = 1;
	dcinfo.scale_denom = reduction_factor;
	dcinfo.out_color_space = JCS_GRAYSCALE;
	
	jpeg_start_decompress(&dcinfo);

	width = dcinfo.output_width;
	height = dcinfo.output_height;
	bytes_per_pixel = dcinfo.output_components;
		
	/* Create buffer for the uncompressed data */
	j->raw = (unsigned char *)realloc(j->raw,bytes_per_pixel*width*height);
	if( j->raw == NULL)
	{
		fprintf(stderr,"Out of memory: Unable to allocate buffer.\n");
		exit(1);
	}
	
	/* reference buffer by row pointer */
	row = (unsigned char **)malloc(sizeof(unsigned char *)*height);
	for(i=0;i<height;i++)
		row[i] = j->raw+(i*width*bytes_per_pixel);

	/* loop to read each scanline */
	while(dcinfo.output_scanline < height)
	{
		jpeg_read_scanlines(&dcinfo,&row[dcinfo.output_scanline],1);
	}
	/* raw image is stored in the buffer j->raw */

	/* cleanup and close */
	j->width = width;
	j->height = height;
	j->bytes_per_pixel = bytes_per_pixel;
	jpeg_finish_decompress(&dcinfo);
	jpeg_destroy_decompress(&dcinfo);
	fclose(input);
	
	return(0);
}

/*
	If libjpeg handles the error, then the program stops. This setjump/longjump routine
	handles any errors so that the program doesn't die prematurely
*/
void my_error_exit(j_common_ptr dcinfo)
{
	my_error_ptr myerr = (my_error_ptr) dcinfo->err;
/* Suppress JPEG error output message / un-comment to re-enable
	(*dcinfo->err->output_message) (dcinfo);
*/
	longjmp(myerr->setjump_buffer,1);
}

/*
	Scan all possibile positions in the main image for the subimage.
	Start at coordinate 0,0 (UL corner) and scan for a match
*/
int compare(struct image *org, struct image *dup)
{
	int line,line_depth,offset,offset_width;
	unsigned char *original,*duplicate;
	int verify,row_count;
	
	/* original image cannot be shorter than the duplicate */
	line_depth = org->height - dup->height;
	if( line_depth < 0)
		return(0);
	/* original image cannot be narrower than the duplicate */
	offset_width = org->width - dup->width;
	if( offset_width < 0 )
		return(0);
	
	/* begin scan */
	for(line=0; line<=line_depth; line++)
	{
		for(offset=0; offset<=offset_width; offset++)
		{
			original = org->raw+(line*org->width)+offset;
			duplicate = dup->raw;
			verify = 1;
			row_count = 0;
			while(verify)
			{
				verify = check_row(original,duplicate,dup->width);
				original += org->width;
				duplicate += dup->width;
				row_count++;
				if( row_count == dup->height)
					break;
			}
			if( verify == 1)
				return(1);
		}
	}
	return(0);
}

/*
	Given a starting position in the original image, scan to see whether
	a row in the subimage matches. The comparison is "fuzzy" to account
	for the JPEG lossy compression.
*/
int check_row(unsigned char *a, unsigned char *b, int length)
{
	int i,calc,mismatch,threshold;
	
	calc = mismatch = 0;
	threshold = length/mismatch_percentage;
	for(i=0; i<length; i++)
	{
		calc = *(a+i) - *(b+i);
		if( calc <= -variation || calc >= variation)
			mismatch++;
		if(mismatch > threshold)
			return(0);
	}
	return(1);
}
