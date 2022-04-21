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
	/*printf("Start 1\n");
	int divisor = size + 1;
	for (int y = 0; y < imageIn->y; y++)
	{	
		int yLevel = y * imageOut->x;
		printf("%d", y);

		printf("Help?");
		//Add the three outermost squares to the sum.

		double sum = 0;

		for(int tempX = 0; tempX <= size; tempX++)
		{
			printf("Here %d?\n", tempX + yLevel);
			switch (colourType)
			{
			case 0:
				sum += imageIn->data[tempX + yLevel].red;
				break;
			case 1:
				sum += imageIn->data[tempX + yLevel].green;
				break;
			case 2:
				sum += imageIn->data[tempX + yLevel].blue;
				break;
			default:
				break;
			}
			printf("Sum: %f Ever here?\n", sum);
		}
		printf("Error here?");
		switch (colourType)
		{
		case 0:
			imageOut->data[0 + yLevel].red = sum / divisor;
			break;
		case 1:
			imageOut->data[0 + yLevel].green = sum / divisor;
			break;
		case 2:
			imageOut->data[0 + yLevel].blue = sum / divisor;
			break;
		default:
			break;
		}

		//Add the two last cells.
		for (int tempX = size + 1; tempX <= 2 * size + 1; tempX++)
		{
			divisor++;
			switch (colourType)
			{
			case 0:
				sum += imageIn->data[tempX + yLevel].red;
				imageOut->data[tempX - size].red = sum / divisor;
				break;
			case 1:
				sum += imageIn->data[tempX + yLevel].green;
				imageOut->data[tempX - size + yLevel].green = sum / divisor;
				break;
			case 2:
				sum += imageIn->data[tempX + yLevel].blue;
				imageOut->data[tempX - size + yLevel].blue = sum / divisor;
				break;
			default:
				break;
			}
		}

		//Go through every intermediate cell.
		for (int x = size; x < imageIn->x - size; x++)
		{
		switch (colourType)
			{
			case 0:
				sum -= imageIn->data[x - size - 1 + yLevel].red;
				sum += imageIn->data[x + size + yLevel].red;
				imageOut->data[x + yLevel].red = sum / divisor;
				break;
			case 1:
				sum -= imageIn->data[x - size - 1 + yLevel].green;
				sum += imageIn->data[x + size + yLevel].green;
				imageOut->data[x + yLevel].green = sum / divisor;
				break;
			case 2:
				sum -= imageIn->data[x - size - 1 + yLevel].blue;
				sum += imageIn->data[x + size + yLevel].blue;
				imageOut->data[x + yLevel].blue = sum / divisor;
				break;
			default:
				break;
			}
		}
	}*/

	// Iterate over each pixel
	//#pragma omp parallel for
	for (int senterY = 0; senterY < imageIn->y; senterY++)
	{
		int outerY = imageOut->x * senterY;
		for (int senterX = 0; senterX < imageIn->x; senterX++)
		{
			// For each pixel we compute the magic number
			double sum[3] = {0, 0, 0};

			int countIncluded = 0;
			for (int y = -size; y <= size; y++)
			{
				int currentY = senterY + y;

				// Check if we are outside the bounds
				if (currentY < 0 || currentY >= imageIn->y)
					continue;

				int innerY = currentY * imageIn->x;

				for (int x = -size; x <= size; x++)
				{
					int currentX = senterX + x;

					if (currentX < 0 || currentX >= imageIn->x)
						continue;

					// Now we can begin
					int offsetOfThePixel = (innerY + currentX);
					sum[0] += imageIn->data[offsetOfThePixel].red;
					sum[1] += imageIn->data[offsetOfThePixel].green;
					sum[2] += imageIn->data[offsetOfThePixel].blue;

					// Keep track of how many values we have included
					countIncluded++;
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

int main()
{
	PPMImage *image;
	image = readPPM("flower.ppm");

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
	PPMImage *final_tiny = imageDifference(imageAccurate2_tiny, imageAccurate2_small);
	writePPM("flower_tiny.ppm", final_tiny);

	PPMImage *final_small = imageDifference(imageAccurate2_small, imageAccurate2_medium);
	writePPM("flower_small.ppm", final_small);

	PPMImage *final_medium = imageDifference(imageAccurate2_medium, imageAccurate2_large);
	writePPM("flower_medium.ppm", final_medium);
}
