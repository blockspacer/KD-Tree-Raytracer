#include "Precompiled.h"
#include "DataGuard.h"

#if !defined(FB_DATAGUARD_DEBUGGING) || FB_DATAGUARD_DEBUGGING != FB_TRUE

FB_PACKAGE0()
void testDataGuard() { }
FB_END_PACKAGE0()

#else

#include "fb/lang/Atomics.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/StoreCallstack.h"
#include "fb/memory/HeapAddressCheck.h"
#include "fb/profiling/MutexProfiler.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/DynamicString.h"
#include "fb/string/util/CreateTemporaryHeapString.h"

FB_PACKAGE0()

void DataGuardBase::setDebugName(const char *nameParam, const Mutex *mutexPtr)
{
	name = nameParam;
	profiling::MutexProfiler::setMutexName(mutexPtr, nameParam);
}

void DataGuardBase::checkHeap()
{
	fb_assertf(!memory::isAddressInStack(this),  "DataGuard being allocated in stack. This is very likely a mistake. Did you forget 'static' keyword. 0x%p, '%s'", this, name);
}

namespace
{
	struct Thing
	{
		float x = 1.0f;
		float y = 2.0f;
		char buffer[100] = { 0 };
		DynamicString str;
		Thing() {}
		Thing(const Thing &t)
			: x(t.x)
			, y(t.y)
			, str(t.str)
		{
		}
	};

	static ScopedRef<Thing> getSingleThing()
	{
		static DataGuard<Thing> thing;
		return thing.get();
	}
	const ScopedRef<Thing> getConstSingleThing()
	{
		return getSingleThing();
	}

	static void foo(const DataGuard<Thing> &thing)
	{
		Thing t = thing.copy();
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		FB_PRINTF("foo\n");
	}
	static void foo(const DataGuardSharedMutex<Thing> &thing)
	{
		Thing t = thing.copy();
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		FB_PRINTF("foo\n");
	}
	static void foo0(ScopedRef<Thing> thing)
	{
		Thing t = thing.copy();
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		FB_PRINTF("foo0\n");
	}
	static void foo1(ScopedRefReadOnly<Thing> thing)
	{
		Thing t = thing.copy();
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		FB_PRINTF("foo1\n");
	}
	static void foo2(const Thing &thing)
	{
		Thing t = thing;
		FB_UNUSED_NAMED_VAR(float, x) = thing.x;
		FB_PRINTF("foo2\n");
	}
	static void foo3(const Thing *thing)
	{
		Thing t = *thing;
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		FB_PRINTF("foo3\n");
	}
	static void foo4(Thing *thing)
	{
		Thing t = *thing;
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		thing->x = 15;
		FB_PRINTF("foo4\n");
	}
	static void foo5(Thing &thing)
	{
		Thing t = thing;
		FB_UNUSED_NAMED_VAR(float, x) = thing.x;
		thing.x = 17;
		FB_PRINTF("foo5\n");
	}
	static void foo6(ScopedRef<Thing> &thing)
	{
		Thing t = thing.copy();
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		thing->x = 1;
		FB_PRINTF("foo6\n");
	}
	static void foo7(const ScopedRefReadOnly<Thing> &thing)
	{
		Thing t = thing.copy();
		FB_UNUSED_NAMED_VAR(float, x) = thing->x;
		FB_PRINTF("foo7\n");
	}

	static void test1()
	{
		DataGuard<Thing> thing;
		thing->x = 2.0f;

		foo(thing); // void foo(const DataGuard<Thing> &thing)
		foo0(thing); // void foo0(ScopedRef<Thing> thing)
		foo1(thing); // void foo1(ScopedRefReadOnly<Thing> thing)
		foo2(thing.copy()); // void foo2(const Thing &thing)
		foo3(&thing.get().getRawRef()); // void foo3(const Thing *thing)
		foo4(&thing.get().getRawRef()); // void foo4(Thing *thing)
		foo5(thing.get().getRawRef()); // void foo5(Thing &thing)

		for (int i = 0; i < 9; ++i)
		{
			// Entering and leaving mutex 3 per iteration
			thing->y *= thing->x;
			thing->x += i * 3.1f;
		}

		Thing aCopy = thing.copy();
		ScopedRef<Thing> thingRef = thing.get();
		ScopedRefReadOnly<Thing> thingRef2 = thing.get();
		foo0(thingRef);
		foo1(thingRef2);
		foo2(thing.copy());
		foo6(thingRef);
		foo7(thingRef);
		foo7(thingRef2);
		thingRef->x = 3.0f;
		thingRef->y = 4.0f;
	}

	static void test2()
	{
		Mutex mutex;
		DataGuardSharedMutex<Thing> thing(mutex);
		DataGuardSharedMutex<Thing> thing2(mutex);
		thing->x = 2.0f;
		thing2->x = 2.0f;

		{
			Thing &fail = thing.get().getRawRef(); // FAIL! Leaves mutex after this line, not at the end of scope
			fail.x = 1337; // FAIL! Unsafe data-access!
			FB_PRINTF("FAIL!\n");
		}

		{
			ScopedRef<Thing> refGuard = thing.get(); // OK! Leaves mutex at the end of scope
			refGuard->x = 1337; // OK! This is safe, fast and cheap data-access

			Thing &ref = refGuard.getRawRef(); // Calling getRawRef should be avoided when possible, but ...
			ref.x = 1338; // ... this is safe as refGuard is still within scope
			FB_PRINTF("Success!\n");
		}

		foo(thing); // copy - void foo(const DataGuardSharedMutex<Thing> &thing)
		foo0(thing); // void foo0(ScopedRef<Thing> thing)
		foo1(thing); // void foo1(ScopedRefReadOnly<Thing> thing)
		foo2(thing.copy()); // copy - void foo2(const Thing &thing)
		foo3(&thing.get().getRawRef()); // void foo3(const Thing *thing)
		foo4(&thing.get().getRawRef()); // void foo4(Thing *thing)
		foo5(thing.get().getRawRef()); // void foo5(Thing &thing)

		for (int i = 0; i < 9; ++i)
		{
			// Entering and leaving mutex 6 per iteration
			thing->y *= thing->x;
			thing->x += i * 3.1f;
			thing2->y *= thing2->x;
			thing2->x += i * 3.1f;
		}

		Thing aCopy = thing.copy();
		Thing bCopy = thing2.copy();
		ScopedRef<Thing> thingRef = thing.get();
		ScopedRef<Thing> thing2Ref = thing2.get();
		ScopedRefReadOnly<Thing> thingRef2 = thing.get();
		ScopedRefReadOnly<Thing> thing2Ref2 = thing2.get();

		foo2(thingRef.copy());
		foo2(thingRef2.copy());
		foo6(thingRef);
		foo6(thing2Ref);
		foo7(thingRef);
		foo7(thingRef2);
		foo7(thing2Ref);
		foo7(thing2Ref2);

		thingRef->x = 3.0f;
		thing2Ref->x = 3.0f;
		thingRef->y = 4.0f;
		thing2Ref->y = 4.0f;
	}

	static void test3()
	{
		getSingleThing()->x += 1;
		FB_UNUSED_NAMED_VAR(float, x) = getConstSingleThing()->x;

		foo0(getSingleThing());

		Thing copy = getSingleThing().copy();
		// Thing &ref = getSingleThing(); // Compiler error, cannot have ref to a by-value result
		FB_UNUSED_NAMED_VAR(const Thing &, contsRef) = getSingleThing().copy(); // Is actually a copy with extended life-time

		Thing temp;
		temp.x = 13;
		temp.y = 14;
		temp.str = "hah";
		DataGuard<Thing> thing3(temp);
		temp.str = "uhu";
		FB_PRINTF("thing3: %s, %f, %f\n", thing3->str.getPointer(), thing3->x, thing3->y);
		FB_PRINTF("temp  : %s, %f, %f\n", temp.str.getPointer(), temp.x, temp.y);
	}

}

void testDataGuard()
{
	test1();
	test2();
	test3();
}

FB_END_PACKAGE0()

#endif
