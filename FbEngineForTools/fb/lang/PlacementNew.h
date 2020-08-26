#pragma once

// Templates used to avoid name conflict with vcruntime_new.h

template<typename T>
inline void* operator new(size_t s, T *ptr)
{
	return ptr;
}

template<typename T>
inline void operator delete(void *, T *)
{
	return;
}
