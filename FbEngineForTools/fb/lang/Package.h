#pragma once

// fb namespaces (packages)

#define FB_PACKAGE0() \
	namespace fb {

#define FB_PACKAGE1(p_namespace1) \
	namespace fb { namespace p_namespace1 {

#define FB_PACKAGE2(p_namespace1,p_namespace2) \
	namespace fb { namespace p_namespace1 { namespace p_namespace2 {

#define FB_PACKAGE3(p_namespace1,p_namespace2,p_namespace3) \
	namespace fb { namespace p_namespace1 { namespace p_namespace2 { namespace p_namespace3 {

#define FB_PACKAGE4(p_namespace1,p_namespace2,p_namespace3,p_namespace4) \
	namespace fb { namespace p_namespace1 { namespace p_namespace2 { namespace p_namespace3 { namespace p_namespace4 {

#define FB_PACKAGE5(p_namespace1,p_namespace2,p_namespace3,p_namespace4,p_namespace5) \
	namespace fb { namespace p_namespace1 { namespace p_namespace2 { namespace p_namespace3 { namespace p_namespace4 { namespace p_namespace5 {

#define FB_END_PACKAGE0() \
	}

#define FB_END_PACKAGE1() \
	} }

#define FB_END_PACKAGE2() \
	} } }

#define FB_END_PACKAGE3() \
	} } } }

#define FB_END_PACKAGE4() \
	} } } } }

#define FB_END_PACKAGE5() \
	} } } } } }
