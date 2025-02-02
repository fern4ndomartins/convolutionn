#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image.h"
#include "../stb/stb_image_write.h"

void get_gaussian_kernel(float **kernel, int i, int j, float sigma) 
{
	int x,y;
	float sum = 0.0;
	for (int a=0; a<i; a++) 
	{
		kernel[a] = (float*)malloc(sizeof(float)*j);
		for (int b=0; b<j; b++) 
		{
			x = a - (i/2);
			y = b - (j/2);
			kernel[a][b] = 1/(2*M_PI*sigma*sigma)*exp((double)(-(x*x + y*y)/2.0*sigma*sigma));
			sum+=kernel[a][b];
		}
	}
	// normalizing the gaussian kernel
	for (int a=0; a<i; a++) 
	{
		for (int b=0; b<j; b++) 
		{
			x = a - (i/2);
			y = b - (j/2);
			kernel[a][b] = kernel[a][b]/sum; 
		}
		//printf("%f %f %f %f %f\n", kernel[a][0], kernel[a][1], kernel[a][2], kernel[a][3], kernel[a][4]);
	}
}
float ***allocate_3d_array(int width, int height, int channels) 
{
	float ***array = (float***)malloc(sizeof(float**)*width);
	for (int w=0; w<width; w++) 
	{
		array[w] = (float**)malloc(sizeof(float*)*height);
		for (int h=0; h<height; h++) 
		{
			array[w][h] = (float*)malloc(sizeof(float)*channels);
		}
	}
	return array;
} 

void free_3d_array(float ***array, int height, int width) 
{
	for (int w=0; w<width; w++) 
	{
		for (int h=0;h<height;h++)
		{
			free(array[w][h]);
		}
		free(array[w]);
	}
	free(array);
}

void convolution(float *** pixels, float ** kernel, int width, int height, int channels, float ***bluredimage) 
{
	for (int w=0; w<width; w++) 
	{
		for (int h=0; h<height; h++) 
		{
			for (int c=0; c<channels; c++) 
			{
      				bluredimage[w][h][c] = 0.0;
				for (int k=0; k<5; k++) 
				{
					for (int l=0; l<5; l++) 
					{
						if (w+(l-2)<0) continue;
						if (h+(k-2)<0) continue;
						if (w+(l-2)>=width) continue;
						if (h+(k-2)>=height) continue;

						bluredimage[w][h][c] += kernel[k][l]*pixels[w+(l-2)][h+(k-2)][c]; 
						//bluredimage pixels[w][h][c]; 
					}	
				}
			}
		}
	}

}

float *** process_image(const char * filename, int *width, int *height, int *channels) 
{
	unsigned char *image = stbi_load(filename, width, height, channels, 3);
	if (!image) 
	{
		printf("error");
	}
	float *** pixels = allocate_3d_array(*width, *height, *channels);	
	for (int w=0; w<*width; w++) 
	{
		for (int h=0; h<*height; h++) 
		{
			for (int c=0; c<*channels; c++) 
			{
				pixels[w][h][c] = (float)image[(h * (*width) + w) * (*channels) + c];
			}
		}
	}	
	free(image);
	return pixels;
}

void generate_image(float ***bluredimage, int width, int height, int channels, unsigned char *rawbluredimage) 
{
	rawbluredimage = (unsigned char*)malloc(sizeof(unsigned char)*width*height*channels);
	for (int w=0; w<width; w++) 
	{
		for (int h=0; h<height; h++) 
		{
			for (int c=0; c<channels; c++) 
			{
				float pixel = bluredimage[w][h][c];
				if (pixel < 0) pixel = 0;
                		if (pixel > 255) pixel = 255;
		                rawbluredimage[(h * width + w) * channels + c] = (unsigned char)pixel;
			}
		}
	}
	stbi_write_jpg("output.jpg", width, height, channels, rawbluredimage, 100);
	free(rawbluredimage);
}

int main(int argc, char * argv[]) 
{
	int blurintensity = 1;
	printf("Type blur intensity: ");
	scanf("%d", &blurintensity);
 	int width,height,channels;
	float **kernel = (float **)malloc(5*sizeof(float*));
	float ***pixels = process_image(argv[1], &width, &height, &channels);
	float ***bluredimage = allocate_3d_array(width, height, channels);
	unsigned char *rawbluredimage;
	get_gaussian_kernel(kernel, 5, 5, 1.0);	
	for (int i=0;i<blurintensity;i++) 
	{
		convolution(pixels, kernel, width, height, channels, bluredimage);
		convolution(bluredimage, kernel, width, height, channels, pixels);
	}
	convolution(pixels, kernel, width, height, channels, bluredimage);
	generate_image(bluredimage, width, height, channels, rawbluredimage);

	for (int i=0; i<5;i++) {
		free(kernel[i]);
	}
	free(kernel); 

	free_3d_array(pixels, height, width);
	free_3d_array(bluredimage, height, width);

	return 0;
}
