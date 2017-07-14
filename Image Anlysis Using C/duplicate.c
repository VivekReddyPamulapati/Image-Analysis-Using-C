/*
   duplicate
   Written as part of code clinic: C course by Lynda
   
   Test program for the libjpeg library. Trying to see whether I can
   read in and spit out a JPEG file. Ensuring that I get it correct.

   Specficially uses the LIBERTY.JPG image at 444x987
*/
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

int main()
{
	FILE *original,*copy;
	char *input_file = "liberty.jpg";
	char *output_file = "liberty-duplicate.jpg";
	struct jpeg_decompress_struct dcinfo;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr dcjerr;
	struct jpeg_error_mgr cjerr;
	unsigned char *raw_image;
	JSAMPROW row_pointer[1];
	unsigned long location,row_stride;
	int i;

/* Read the file LIBERTY.JPG, 444x987 */
	original = fopen(input_file,"rb");
	if(!original)
	{
		fprintf(stderr,"Unable to read image file.\n");
		exit(1);
	}
	printf("File '%s' read\n",input_file);

	dcinfo.err = jpeg_std_error(&dcjerr);
	jpeg_create_decompress(&dcinfo);
	jpeg_stdio_src(&dcinfo,original);
	jpeg_read_header(&dcinfo,TRUE);
	
	jpeg_start_decompress(&dcinfo);
	
	/* Create buffers for the uncompressed data */
	raw_image = (unsigned char *)malloc(dcinfo.output_width*dcinfo.output_height*dcinfo.num_components);
	row_pointer[0] = (unsigned char *)malloc(dcinfo.output_width*dcinfo.num_components);
	
	/* loop to read each scanline */
	location = 0;
	while(dcinfo.output_scanline < dcinfo.output_height)
	{
		jpeg_read_scanlines(&dcinfo,row_pointer,1);
		for(i=0;i<dcinfo.image_width*dcinfo.num_components;i++)
		{
			*(raw_image+location) = row_pointer[0][i];
			location++;
		}
	}

	/* raw image is stored in the buffer raw_image */
	/* cleanup and close */
	jpeg_finish_decompress(&dcinfo);
	jpeg_destroy_decompress(&dcinfo);
	free(row_pointer[0]);
	fclose(original);

/* Compress & Save */
/* Raw data remains in the raw_image buffer. */

	copy = fopen(output_file,"wb");
	if(!copy)
	{
		fprintf(stderr,"Unable to create output image file.\n");
		exit(1);
	}

	cinfo.err = jpeg_std_error(&cjerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo,copy);

	cinfo.image_width = 444;
	cinfo.image_height = 987;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	/* Begin compression */
	jpeg_start_compress(&cinfo,TRUE);
	/* write one scanline at a time */
	while(cinfo.next_scanline < cinfo.image_height)
	{
		row_stride=cinfo.next_scanline*cinfo.image_width*cinfo.input_components;
		row_pointer[0] = (raw_image+row_stride);
		jpeg_write_scanlines(&cinfo,row_pointer,1);
	}

	/* clean-up */
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(copy);
	printf("Output file '%s' created\n",output_file);

	return(0);
}

