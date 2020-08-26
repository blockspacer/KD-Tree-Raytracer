#ifndef FB_LANG_CONTAINERINTERFACE_H
#define FB_LANG_CONTAINERINTERFACE_H

FB_PACKAGE1(lang)

template <typename T, size_t N>
inline SizeType getSize(T (&v)[N])
{
	return SizeType(N);
}

template <typename T>
inline SizeType getSize(T &container)
{
	return container.getSize();
}

template <typename T, size_t N>
inline T *getBegin(T (&v)[N])
{
	return v;
}

template <typename T>
inline SizeType getBegin(T &container)
{
	return container.getBegin();
}

template <typename T, size_t N>
inline T *getEnd(T (&v)[N])
{
	return v + N;
}

template <typename T>
inline SizeType getEnd(T &container)
{
	return container.getEnd();
}

FB_END_PACKAGE1()

#endif
