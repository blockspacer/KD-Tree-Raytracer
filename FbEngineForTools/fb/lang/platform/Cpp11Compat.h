#pragma once

// A few C++11 compatibility macros for older crappy compilers (a.k.a. GHS)

// Supported features from C++11 (though, don't rely on getting errors/warnings from these with old compilers)
// - final keyword
// - override keyword

// (enum class would be nice, but might look ugly, and mostly unnecessary due to FB_ENUM)
// (macro for "for each" would be nice, but unfortunately difficult to nicely support)
// (auto would be nice, but cannot be done via macro-magic)
// (member initialization at headers would be nice, but not possible via macro-magic)

#include "Compiler.h"

#if (FB_COMPILER_SUPPORTS_CPP11 == FB_TRUE)
	// nothing here, assuming proper compiler support
#else
	#define override 
	#define final
#endif
