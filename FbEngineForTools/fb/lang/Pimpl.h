#pragma once

#include "fb/lang/ScopedPointer.h"

// A small macro just to make PIMPLed class headers less polluted.

// Weird, class declaration has be to public on MSC if using custom memory operators in Impl
#define FB_PIMPL \
	public: \
		class Impl; \
	private: \
		ScopedPointer<Impl> impl;
