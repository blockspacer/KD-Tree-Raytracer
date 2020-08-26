#pragma once

FB_PACKAGE1(assignment)

class Image
{
public:
	Image() = delete;
	Image(SizeType imageWidth, SizeType imageHeight);

	void setPixelColor(SizeType x, SizeType y, const math::VC3 &color);
	void writeToFile(const HeapString &fileName) const;

private:
	PodVector<math::VC3> data;
	SizeType width;
	SizeType height;
};

FB_END_PACKAGE1()
