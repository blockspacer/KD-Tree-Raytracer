#pragma once


FB_PACKAGE1(util)

// Return needed destination size buffer size in bytes.
uint32_t getBase64EncodedSizeInBytes(uint32_t sourceSizeInBytes);
uint32_t getBase64DecodedSizeInBytes(const void *sourceBuffer, uint32_t sourceSizeInBytes);
// Return amount of bytes written to destination buffer
uint32_t base64Encode(const void *sourceBuffer, uint32_t sourceSizeInBytes, void *destinationBuffer, uint32_t destinationSizeInBytes);
uint32_t base64Decode(const void *sourceBuffer, uint32_t sourceSizeInBytes, void *destinationBuffer, uint32_t destinationSizeInBytes);

// Encode straight to string which is the main use case anyway
void base64Encode(const void *sourceBuffer, uint32_t sourceSizeInBytes, HeapString &dstStr);

// Decode string to vector container
template<typename StringType, typename VectorType>
void base64Decode(const StringType &sourceString, VectorType &destVector)
{
	destVector.resize(util::getBase64DecodedSizeInBytes(sourceString.getPointer(), (uint32_t) sourceString.getLength()));
	FB_UNUSED_NAMED_VAR(uint32_t, resultInBytes) = util::base64Decode(sourceString.getPointer(), (uint32_t) sourceString.getLength(), destVector.getPointer(), destVector.getSize());
	fb_assert(resultInBytes <= destVector.getSize());
}

FB_END_PACKAGE1()
