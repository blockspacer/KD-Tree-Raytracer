#include "Precompiled.h"
#include "ContainerTest.h"

#if FB_BUILD != FB_FINAL_RELEASE
	#define FB_ENABLE_TESTS FB_TRUE
#else
#define FB_ENABLE_TESTS FB_FALSE
#endif

#if FB_ENABLE_TESTS == FB_TRUE
	#include "fb/container/PodVector.h"
	#include "fb/container/Vector.h"
#endif

FB_PACKAGE1(container)

#if FB_ENABLE_TESTS == FB_TRUE

	struct TestType
	{
		int data;
		FB_NOINLINE explicit TestType(int userData) : data(userData) { ++instanceCounter; }
		FB_NOINLINE TestType() : data(0) { ++instanceCounter; }
		FB_NOINLINE TestType(const TestType &other) : data(other.data) { ++instanceCounter; ++copyConstructCounter; }
		FB_NOINLINE TestType(TestType &&other) : data(0) { lang::swap(data, other.data);  ++instanceCounter; ++moveConstructCounter; }
		FB_NOINLINE ~TestType() { --instanceCounter; }

		FB_NOINLINE void operator =(const TestType &other) { data = other.data; ++copyCounter; }
		FB_NOINLINE void operator =(TestType &&other) { lang::swap(data, other.data); ++moveCounter; }

		static int instanceCounter;
		static int moveConstructCounter;
		static int copyConstructCounter;
		static int moveCounter;
		static int copyCounter;
		static void validateCounter()
		{
			fb_assert(instanceCounter == 0);
		}
		static void resetCounter()
		{
			instanceCounter = 0;
			moveConstructCounter = 0;
			copyConstructCounter = 0;
			moveCounter = 0;
			copyCounter = 0;
		}
};
	int TestType::instanceCounter = 0;
	int TestType::moveConstructCounter = 0;
	int TestType::copyConstructCounter = 0;
	int TestType::moveCounter = 0;
	int TestType::copyCounter = 0;

	template<typename T>
	void testVector()
	{
		// Move
		{
			T vec;

			vec.pushBack(TestType(42));
			fb_assert(TestType::instanceCounter == 1 && TestType::moveConstructCounter == 1);
			fb_assert(vec[0].data == 42);
			vec[0] = (TestType&&) TestType(24);
			fb_assert(TestType::instanceCounter == 1 && TestType::moveCounter == 1);
			fb_assert(vec[0].data == 24);
		}
		TestType::validateCounter();
		TestType::resetCounter();

		// Copy
		{
			T vec;

			TestType t1(42);
			vec.pushBack(t1);
			fb_assert(TestType::instanceCounter == 2 && TestType::copyConstructCounter == 1);
			fb_assert(vec[0].data == 42);

			TestType t2(24);
			vec[0] = t2;
			fb_assert(TestType::instanceCounter == 3 && TestType::copyCounter == 1);
			fb_assert(vec[0].data == 24);
		}
		TestType::validateCounter();
		TestType::resetCounter();

		// Erase index
		{
			T vec;

			int startState[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			int startStateSize = sizeof(startState) / sizeof(int);

			for (int i = 0; i < startStateSize; ++i)
				vec.pushBack(TestType(startState[i]));
			fb_assert(TestType::instanceCounter == startStateSize && TestType::copyConstructCounter == 0);

			vec.eraseIndex(0);
			fb_assert(TestType::instanceCounter == (startStateSize - 1) && TestType::copyConstructCounter == 0);
			for (int i = 0; i < startStateSize - 1; ++i)
			{
				fb_assert(vec[(SizeType) i].data == startState[i + 1]);
			}

			vec.eraseIndex(vec.getSize() - 1);
			fb_assert(TestType::instanceCounter == (startStateSize - 2) && TestType::copyConstructCounter == 0);
			for (int i = 0; i < startStateSize - 2; ++i)
			{
				fb_assert(vec[(SizeType) i].data == startState[i + 1]);
			}
		}
		TestType::validateCounter();
		TestType::resetCounter();

		// Erase range
		{
			T vec;

			int startState[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			int startStateSize = sizeof(startState) / sizeof(int);

			for (int i = 0; i < startStateSize; ++i)
				vec.pushBack(TestType(startState[i]));
			fb_assert(TestType::instanceCounter == startStateSize && TestType::copyConstructCounter == 0);

			vec.eraseIndex(0, 2);
			fb_assert(TestType::instanceCounter == (startStateSize - 2) && TestType::copyConstructCounter == 0);
			for (int i = 0; i < startStateSize - 2; ++i)
			{
				fb_assert(vec[(SizeType)i].data == startState[i + 2]);
			}

			vec.eraseIndex(vec.getSize() - 2, 2);
			fb_assert(TestType::instanceCounter == (startStateSize - 4) && TestType::copyConstructCounter == 0);
			for (int i = 0; i < startStateSize - 4; ++i)
			{
				fb_assert(vec[(SizeType)i].data == startState[i + 2]);
			}
		}
		TestType::validateCounter();
		TestType::resetCounter();

	}

	void testPodVectors()
	{
	}

	void testVectors()
	{
		// testVector() uses more than 2 but less than 16 size'd vector
		testVector<Vector<TestType>>();
		testVector<CacheVector<TestType, 2>>();
		testVector<CacheVector<TestType, 16>>();
		testVector<StaticVector<TestType, 16>>();
	}

	void testSets()
	{
	}

	void testAssosiative()
	{
	}

	void runTestImp()
	{
		testPodVectors();
		testVectors();
		testSets();
		testAssosiative();
	}

#endif

void runTest()
{
	#if FB_ENABLE_TESTS == FB_TRUE
		runTestImp();
	#endif
}

FB_END_PACKAGE1()
