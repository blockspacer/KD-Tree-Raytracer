#ifndef FB_SYS_VERSIONCONTROL_DEFAULTVERSIONCONTROL_H
#define FB_SYS_VERSIONCONTROL_DEFAULTVERSIONCONTROL_H

// "Unnecessary" include is intentional, user of this file shouldn't have to 
// manually include the actual implementation
#include "fb/sys/versioncontrol/SVN.h"

FB_PACKAGE2(sys, versioncontrol)

typedef SVN DefaultVersionControl;

FB_END_PACKAGE2()

#endif
