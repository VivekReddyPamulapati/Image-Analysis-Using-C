/*
   shrink
  Written as part of code clinic: C by lynda

   Yet another test program, this one to ensure that I can read in a
   color image, reset it to grayscale, and generate a 1/4-size duplicate
   as output.
*/
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

int main()
{
	FILE *original,*copy;
	char *input_file = "liberty.jpg";
	char *output_file = "liberty-shrink.jpg";
	struct jpeg_decompress_struct dcinfo;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr dcjerr;
	struct jpeg_error_mgr cjerr;
	unsigned char *raw_image;
	unsigned char **row;
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
	dcinfo.scale_num = 1;
	dcinfo.scale_denom = 4;
	dcinfo.out_color_space = JCS_GRAYSCALE;
	
	jpeg_start_decompress(&dcinfo);

	width = dcinfo.output_width;
	height = dcinfo.output_height;
	bytes_per_pixel = dcinfo.output_components;
		
	/* Create buffers for the uncompressed data */
	raw_image = (unsigned char *)malloc(bytes_per_pixel*width*height);
	row = (unsigned char **)malloc(sizeof(unsigned char *)*height);
	for(i=0;i<height;i++)
		row[i] = raw_image+(i*width*bytes_per_pixel);

	/* loop to read each scanline */
	while(dcinfo.output_scanline < height)
	{
		jpeg_read_scanlines(&dcinfo,&row[dcinfo.output_scanline],1);
	}

	/* raw image is stored in the buffer raw_image */
	/* cleanup and close */
	jpeg_finish_decompress(&dcinfo);
	jpeg_destroy_decompress(&dcinfo);
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
		jpeg_write_scanlines(&cinfo,&row[cinfo.next_scanline],1);
	}

	/* clean-up */
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(copy);
	printf("Output file '%s' created\n",output_file);

	return(0);
}

