#pragma once

// Make sure we don't mess up final release settings
#if FB_BUILD != FB_FINAL_RELEASE
	#if (FB_COMPILER == FB_MSC)
		/* optimization flags changed after including header, may be due to #pragma optimize() */
		#pragma warning( disable: 4426 )
		#pragma optimize( "gts", on )
		#pragma auto_inline(on)
		#pragma inline_depth(254)
	#endif
#endif
