#ifndef FB_LANG_CALLSTACKLISTENER_H
#define FB_LANG_CALLSTACKLISTENER_H

FB_PACKAGE1(lang)

/// Listener has to be thread safe!
class CallStackListener
{
public:
	virtual ~CallStackListener() {}

	/// @className can be null
	virtual void pushFunction(const char *className, const char *functionName) = 0;
	virtual void popFunction(const char *className, const char *functionName) = 0;
};

void addStackListener(CallStackListener *listener);
void removeStackListener(CallStackListener *listener);

FB_END_PACKAGE1()

#endif
