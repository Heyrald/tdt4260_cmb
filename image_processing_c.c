#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "ppm.h"

// Image from:
// http://7-themes.com/6971875-funny-flowers-pictures.html

typedef struct
{
	double red, green, blue;
} AccuratePixel;

typedef struct
{
	int x, y;
	AccuratePixel *data;
} AccurateImage;

// Convert ppm to high precision format.
AccurateImage *convertToAccurateImage(PPMImage *image)
{
	// Make a copy
	AccurateImage *imageAccurate;
	imageAccurate = (AccurateImage *)malloc(sizeof(AccurateImage));
	imageAccurate->data = (AccuratePixel *)malloc(image->x * image->y * sizeof(AccuratePixel));
	for (int i = 0; i < image->x * image->y; i++)
	{
		imageAccurate->data[i].red = (double)image->data[i].red;
		imageAccurate->data[i].green = (double)image->data[i].green;
		imageAccurate->data[i].blue = (double)image->data[i].blue;
	}
	imageAccurate->x = image->x;
	imageAccurate->y = image->y;

	return imageAccurate;
}

// blur one color channel
void blurIteration(AccurateImage *imageOut, AccurateImage *imageIn, int size)
{
	;
	;
	// Iterate over each pixel
	#pragma omp parallel for 
	for (int senterY = 0; senterY < imageIn->y; senterY++)
	{
		int outerY = imageOut->x * senterY;
		double sum[3] ={0, 0, 0};
		int countIncluded = 0;
		//Start up the left side
		for (int y = -size; y <= size; y++) {
			int currentY = senterY + y;
			if (currentY < 0 || currentY >= imageIn->y)
				continue;
			int innerY = currentY * imageIn->x;
			for (int x = 0; x < size; x++){
				int offsetOfThePixel = (innerY + x);
				sum[0] += imageIn->data[offsetOfThePixel].red;
				sum[1] += imageIn->data[offsetOfThePixel].green;
				sum[2] += imageIn->data[offsetOfThePixel].blue;
				countIncluded++;
			}
		}

		for (int senterX = 0; senterX < imageIn->x; senterX++)
		{
			for (int y = -size; y <= size; y++)
			{
				int currentY = senterY + y;
				if (currentY >= 0 && currentY < imageIn->y){
					int innerY = currentY * imageIn->x;
					int frontX = senterX + size;
					int backX = senterX - size - 1;

					if (frontX < imageIn->x) {
						int offsetOfThePixel = (innerY + frontX);
						sum[0] += imageIn->data[offsetOfThePixel].red;
						sum[1] += imageIn->data[offsetOfThePixel].green;
						sum[2] += imageIn->data[offsetOfThePixel].blue;
						countIncluded++;
					}

					if (backX >= 0) {
						int offsetOfThePixel = (innerY + backX);
						sum[0] -= imageIn->data[offsetOfThePixel].red;
						sum[1] -= imageIn->data[offsetOfThePixel].green;
						sum[2] -= imageIn->data[offsetOfThePixel].blue;
						countIncluded--;
					}
				}
			}

			// Update the output image
			int offsetOfThePixel = (outerY + senterX);
			imageOut->data[offsetOfThePixel].red = sum[0] / countIncluded;
			imageOut->data[offsetOfThePixel].green = sum[1] / countIncluded;
			imageOut->data[offsetOfThePixel].blue = sum[2] / countIncluded;
		}
	}
}

// Perform the final step, and return it as ppm.
PPMImage *imageDifference(AccurateImage *imageInSmall, AccurateImage *imageInLarge)
{
	PPMImage *imageOut;
	imageOut = (PPMImage *)malloc(sizeof(PPMImage));
	imageOut->data = (PPMPixel *)malloc(imageInSmall->x * imageInSmall->y * sizeof(PPMPixel));

	imageOut->x = imageInSmall->x;
	imageOut->y = imageInSmall->y;

	for (int i = 0; i < imageInSmall->x * imageInSmall->y; i++)
	{
		double value = (imageInLarge->data[i].red - imageInSmall->data[i].red);
		if (value > 255)
			imageOut->data[i].red = 255;
		else if (value < -1.0)
		{
			value = 257.0 + value;
			if (value > 255)
				imageOut->data[i].red = 255;
			else
				imageOut->data[i].red = floor(value);
		}
		else if (value > -1.0 && value < 0.0)
		{
			imageOut->data[i].red = 0;
		}
		else
		{
			imageOut->data[i].red = floor(value);
		}

		value = (imageInLarge->data[i].green - imageInSmall->data[i].green);
		if (value > 255)
			imageOut->data[i].green = 255;
		else if (value < -1.0)
		{
			value = 257.0 + value;
			if (value > 255)
				imageOut->data[i].green = 255;
			else
				imageOut->data[i].green = floor(value);
		}
		else if (value > -1.0 && value < 0.0)
		{
			imageOut->data[i].green = 0;
		}
		else
		{
			imageOut->data[i].green = floor(value);
		}

		value = (imageInLarge->data[i].blue - imageInSmall->data[i].blue);
		if (value > 255)
			imageOut->data[i].blue = 255;
		else if (value < -1.0)
		{
			value = 257.0 + value;
			if (value > 255)
				imageOut->data[i].blue = 255;
			else
				imageOut->data[i].blue = floor(value);
		}
		else if (value > -1.0 && value < 0.0)
		{
			imageOut->data[i].blue = 0;
		}
		else
		{
			imageOut->data[i].blue = floor(value);
		}
	}
	return imageOut;
}

int main(int argc, char** argv)
{
    // read image
    PPMImage *image;
    // select where to read the image from
    if(argc > 1) {
        // from file for debugging (with argument)
        image = readPPM("flower.ppm");
    } else {
        // from stdin for cmb
        image = readStreamPPM(stdin);
    }

	AccurateImage *imageAccurate1_tiny = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_tiny = convertToAccurateImage(image);

	// Process the tiny case:
	int size = 2;
	blurIteration(imageAccurate2_tiny, imageAccurate1_tiny, size);
	blurIteration(imageAccurate1_tiny, imageAccurate2_tiny, size);
	blurIteration(imageAccurate2_tiny, imageAccurate1_tiny, size);
	blurIteration(imageAccurate1_tiny, imageAccurate2_tiny, size);
	blurIteration(imageAccurate2_tiny, imageAccurate1_tiny, size);

	AccurateImage *imageAccurate1_small = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_small = convertToAccurateImage(image);

	// Process the small case:
	size = 3;
	blurIteration(imageAccurate2_small, imageAccurate1_small, size);
	blurIteration(imageAccurate1_small, imageAccurate2_small, size);
	blurIteration(imageAccurate2_small, imageAccurate1_small, size);
	blurIteration(imageAccurate1_small, imageAccurate2_small, size);
	blurIteration(imageAccurate2_small, imageAccurate1_small, size);

	AccurateImage *imageAccurate1_medium = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_medium = convertToAccurateImage(image);

	// Process the medium case:
	size = 5;
	blurIteration(imageAccurate2_medium, imageAccurate1_medium, size);
	blurIteration(imageAccurate1_medium, imageAccurate2_medium, size);
	blurIteration(imageAccurate2_medium, imageAccurate1_medium, size);
	blurIteration(imageAccurate1_medium, imageAccurate2_medium, size);
	blurIteration(imageAccurate2_medium, imageAccurate1_medium, size);

	AccurateImage *imageAccurate1_large = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_large = convertToAccurateImage(image);

	// Do each color channel
	size = 8;
	blurIteration(imageAccurate2_large, imageAccurate1_large, size);
	blurIteration(imageAccurate1_large, imageAccurate2_large, size);
	blurIteration(imageAccurate2_large, imageAccurate1_large, size);
	blurIteration(imageAccurate1_large, imageAccurate2_large, size);
	blurIteration(imageAccurate2_large, imageAccurate1_large, size);


	// Save the images.
	// calculate difference
	PPMImage *final_tiny = imageDifference(imageAccurate2_tiny, imageAccurate2_small);
    PPMImage *final_small = imageDifference(imageAccurate2_small, imageAccurate2_medium);
    PPMImage *final_medium = imageDifference(imageAccurate2_medium, imageAccurate2_large);
	
	// Save the images.
    if(argc > 1) {
        writePPM("flower_tiny.ppm", final_tiny);
        writePPM("flower_small.ppm", final_small);
        writePPM("flower_medium.ppm", final_medium);
    } else {
        writeStreamPPM(stdout, final_tiny);
        writeStreamPPM(stdout, final_small);
        writeStreamPPM(stdout, final_medium);
    }
}
