#if defined(_MSC_VER) && _MSC_VER <= 1900
// VS2015 fix
//  warning C4577: 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed. Specify /EHsc
#pragma warning(disable:4577)
#endif
#include "Precompiled.h"