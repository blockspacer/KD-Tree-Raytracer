#pragma once

#include "fb/lang/ICharacterOutputReceiver.h"
#include "fb/lang/Types.h"

FB_DECLARE0(StringRef)

FB_PACKAGE1(lang)

class PrintfHandler
{
public:
	PrintfHandler();
	~PrintfHandler();

	/* Adds a CharacterOutputReceiver. If maximum number of receivers is already added, returns false, otherwise returns true. */
	bool addCharacterOutputReceiver(ICharacterOutputReceiver* receiver);
	/* Removes CharacterOutputReceiver. Returns true, if given receiver is found, otherwise returns false. */
	bool removeCharacterOutputReceiver(ICharacterOutputReceiver* receiver);

	void doPrintf(const char *fmt, ...);

	/* Returns singleton PrintFHandler. Should be called before entering multi-threaded mode to avoid unnecessary
	* trouble. */
	static PrintfHandler& getPrintfHandler();

private:
	/* Let's do this as simply as possible to avoid introducing dependencies (fb_printf is used e.g. in fb_assert, so everything depends on this) */
	static const SizeType maxReceivers = 4;
	ICharacterOutputReceiver* receivers[maxReceivers];
	SizeType numReceivers;
};


class BasicCharacterOutputReceiver : public ICharacterOutputReceiver
{
public:
	virtual void write(const StringRef &str);
};


FB_END_PACKAGE1()
