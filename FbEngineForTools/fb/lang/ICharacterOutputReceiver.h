#pragma once

#include "fb/lang/Declaration.h"

FB_DECLARE0(StringRef)

FB_PACKAGE1(lang)

/* Interface to allow piping printf output to different places. */
class ICharacterOutputReceiver
{
public:
	virtual ~ICharacterOutputReceiver() {}
	virtual void write(const StringRef &str) = 0;
};

FB_END_PACKAGE1()
