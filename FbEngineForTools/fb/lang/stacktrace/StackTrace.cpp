#include "Precompiled.h"
#include "StackTrace.h"

FB_PACKAGE1(lang)

void debugDumpCallStack(const CallStackCaptureBase &capture, const char *prefix)
{
#if FB_BUILD != FB_FINAL_RELEASE
	const lang::CallStackFrameCapture *frames = capture.getCapturedFrames();
	for (uint32_t i = 0; i < capture.numCapturedFrames; i++)
	{
		lang::StackFrame f;
		resolveCallStackFrame(f, frames[i]);
		FB_PRINTF("%s%s:%d - %s + 0x%x\n", prefix, f.file, f.line, f.function, f.displacement);
	}
#endif
}

FB_END_PACKAGE1()
