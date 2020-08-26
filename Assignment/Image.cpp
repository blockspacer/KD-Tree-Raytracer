#include "Precompiled.h"
#include "Image.h"

#include "external/tgasaver/tga_saver.h"

FB_PACKAGE1(assignment)

/* These are off by one for some simple aa */
Image::Image(SizeType imageWidth, SizeType imageHeight)
	: width(imageWidth + 1)
	, height(imageHeight + 1)
{
	data.resize(width * height);
}

void Image::setPixelColor(SizeType x, SizeType y, const math::VC3 &color)
{
	fb_assert(x < width && y < height && "Invalid pixed position");
	data[y * width + x] = color;
}

void Image::writeToFile(const HeapString &fileName) const
{
	enum { R, G, B };
	/* These are off by one for some simple aa */
	SizeType heightCorrect = height - 1;
	SizeType widthCorrect = width - 1;
	char *imageData = new char[widthCorrect * heightCorrect * 3];
	for (SizeType y = 0; y < heightCorrect; ++y)
	{
		for (SizeType x = 0; x < widthCorrect; ++x)
		{
			/* Simple antialiasing scheme */
			math::VC3 pixel = data[y * width + x];
			pixel += data[y * width + (x + 1)];
			pixel += data[(y + 1) * width + (x + 1)];
			pixel += data[(y + 1) * width + x];
			pixel /= 4.0f;
			pixel[R] = FB_FMIN(pixel[R], 1.0f);
			pixel[G] = FB_FMIN(pixel[G], 1.0f);
			pixel[B] = FB_FMIN(pixel[B], 1.0f);

			fb_assert(pixel[R] >= 0.0 && pixel[G] >= 0.0 && pixel[B] >= 0.0 && "Negative color value in a pixel");
	
			pixel *= 255;
			SizeType baseIndex = (y * widthCorrect + x) * 3;
			imageData[baseIndex + 0] = char(pixel[B]);
			imageData[baseIndex + 1] = char(pixel[G]);
			imageData[baseIndex + 2] = char(pixel[R]);
		}
	}
	
	Write_Tga(fileName.getPointer(), (int)widthCorrect, (int)heightCorrect, imageData);
	delete[] imageData;
}

FB_END_PACKAGE1()
