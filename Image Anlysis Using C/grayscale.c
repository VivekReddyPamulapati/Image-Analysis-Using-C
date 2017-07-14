/*
	grayscale
	Written as part of code clinic: C by lynda

	Test program to ensure that I can read in a color image and generate
	a grayscale image as output.

	Test file is LIBERTY.JPG
*/
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

int main()
{
	FILE *original,*copy;
	char *input_file = "liberty.jpg";
	char *output_file = "liberty-grayscale.jpg";
	struct jpeg_decompress_struct dcinfo;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr dcjerr;
	struct jpeg_error_mgr cjerr;
	unsigned char *raw_image;
	JSAMPROW row_pointer[1];
	unsigned long location,row_stride;
	int i,width,height,bytes_per_pixel;

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

	/* re-size */
	dcinfo.out_color_space = JCS_GRAYSCALE;
		
	jpeg_start_decompress(&dcinfo);

	width = dcinfo.output_width;
	height = dcinfo.output_height;
	bytes_per_pixel = dcinfo.output_components;
		
	/* Create buffers for the uncompressed data */
	raw_image = (unsigned char *)malloc(bytes_per_pixel*width*height);
	row_pointer[0] = (unsigned char *)malloc(width*dcinfo.num_components);

	/* loop to read each scanline */
	location = 0;
	while(dcinfo.output_scanline < height)
	{
		jpeg_read_scanlines(&dcinfo,row_pointer,1);
		for(i=0;i<width*dcinfo.output_components;i++)
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

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = JCS_GRAYSCALE;

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

