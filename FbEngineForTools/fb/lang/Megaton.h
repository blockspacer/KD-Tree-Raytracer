#pragma once

#include "fb/lang/Const.h"
#include "fb/lang/MegatonClassDeclare.h"
#include "fb/lang/MegatonDebugger.h"
#include "fb/lang/Types.h"

// Enables validity checks for setters, and light checks for getters
// Negligible performance implications
#ifndef FB_MEGATON_DEBUG_ENABLED
	#if FB_BUILD != FB_FINAL_RELEASE
		#define FB_MEGATON_DEBUG_ENABLED FB_TRUE
	#else
		#define FB_MEGATON_DEBUG_ENABLED FB_FALSE
	#endif
#endif

// Enables usage statistics and more rigorous validity checks for getters
// Still pretty negligible performance implications
#ifndef FB_MEGATON_VERBOSE_DEBUG_ENABLED
	#if FB_BUILD == FB_DEBUG
		#define FB_MEGATON_VERBOSE_DEBUG_ENABLED FB_FALSE
	#else
		#define FB_MEGATON_VERBOSE_DEBUG_ENABLED FB_FALSE
	#endif
#endif

#if FB_MEGATON_DEBUG_ENABLED == FB_TRUE || FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE

#define FB_MEGATON_ASSERT(p_cond) fb_assert(p_cond)
#define FB_MEGATON_ASSERTF(p_cond, p_fmt, ...) fb_assertf(p_cond, p_fmt, __VA_ARGS__)

#else

#define FB_MEGATON_ASSERT(p_cond) do {} while (false)
#define FB_MEGATON_ASSERTF(p_cond, p_fmt, ...) do {} while (false)

#endif

FB_PACKAGE0()

extern SizeType megatonTypeIndexCounter;
extern bool g_megatonDefaultInEditor;

#if FB_MEGATON_DEBUG_ENABLED == FB_TRUE || FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
#define FB_CHECK_FOR_UNDEFINED() \
		FB_MEGATON_ASSERTF(i != getUndefinedIndex(), "%s has never been set in any megaton, the megatonIndex " \
			"is uninitialized. Trying to find it in %s.", T::getDebugMegatonName(), getDebugMegatonType())

#define FB_CHECK_RESULT(p_cond) \
		FB_MEGATON_ASSERTF((p_cond), "%s (megatonIndex: %d) is not set in %s.%s", \
			T::getDebugMegatonName(), T::megatonIndex, getDebugMegatonType(), (isMegaton(getOwnerMegatonType(i)) ? \
			debugConcat(" It IS set in ", getOwnerMegatonType(i), ". Did you mean to use that instead?") \
			: " In fact it is not set in any megaton currently."))

#define FB_CHECK_MEGATON_OWNER() \
		if (ptr == nullptr) \
			checkMegatonOwner(i, T::getDebugMegatonName(), getDebugMegatonType())
#else
#define FB_CHECK_FOR_UNDEFINED() do {} while(false)
#define FB_CHECK_RESULT(p_cond) do {} while(false)
#define FB_CHECK_MEGATON_OWNER() do {} while(false)
#endif

class Megaton
{
public:
	Megaton();
	~Megaton();
	
	FB_CONST_POD(SizeType, UndefinedIndex, 0xFFFFFFFF);
	
	template<typename T> inline
	const T &get() const
	{
		const SizeType i = getIndex<T>();
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		FB_CHECK_FOR_UNDEFINED();
#endif
		T *result = static_cast<T*>(getWithFallbackImpl(i));
		FB_CHECK_RESULT(result != nullptr);
		return *result;
	}

	template<typename T> inline
	T &get()
	{
		const SizeType i = getIndex<T>();
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		FB_CHECK_FOR_UNDEFINED();
#endif
		T *result = static_cast<T*>(getWithFallbackImpl(i));
		FB_CHECK_RESULT(result != nullptr);
		return *result;
	}

	template<typename T> FB_FORCEINLINE
	const T *getOptional() const
	{
		const SizeType i = getIndex<T>();
		T *ptr = (i < pointers.getSize()) ? static_cast<T*>(getWithFallbackImpl(i)) : nullptr;
		/* Commented out due to hansoft://hansoft;Frozenbyte;abcb8319/Task/716719?ID=596 . If there's a point to this 
		 * test, or a way to fix it, feel free. Otherwise the whole macro should be removed */
		//FB_CHECK_MEGATON_OWNER();
		return ptr;
	}

	template<typename T> FB_FORCEINLINE
	T *getOptional()
	{
		const SizeType i = getIndex<T>();
		T *ptr = (i < pointers.getSize()) ? static_cast<T*>(getWithFallbackImpl(i)) : nullptr;
		/* Commented out due to hansoft://hansoft;Frozenbyte;abcb8319/Task/716719?ID=596 . If there's a point to this
		 * test, or a way to fix it, feel free. Otherwise the whole macro should be removed */
		//FB_CHECK_MEGATON_OWNER();
		return ptr;
	}

	template<typename T> inline
	const T &getQuickly() const
	{
		const SizeType i = getIndex<T>();
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		FB_CHECK_FOR_UNDEFINED();
		if (i < FB_MEGATON_MAX_POINTER_COUNT)
			++(lang::MegatonDebugger::megatonQuickUsage[i]);
#endif
		FB_CHECK_RESULT(i < pointers.getSize() && pointers[i] != nullptr);
		return *static_cast<T*>(pointers[i]);
	}

	template<typename T> inline
	T &getQuickly()
	{
		const SizeType i = getIndex<T>();
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		FB_CHECK_FOR_UNDEFINED();
		if (i < FB_MEGATON_MAX_POINTER_COUNT)
			++(lang::MegatonDebugger::megatonQuickUsage[i]);
#endif
		FB_CHECK_RESULT(i < pointers.getSize() && pointers[i] != nullptr);
		return *static_cast<T*>(pointers[i]);
	}

	template<typename T> FB_FORCEINLINE
	const T *getQuicklyOptional() const
	{
		const SizeType i = getIndex<T>();
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		if (i < FB_MEGATON_MAX_POINTER_COUNT)
			++(lang::MegatonDebugger::megatonQuickUsage[i]);
#endif
		return (i < pointers.getSize()) ? static_cast<T*>(pointers[i]) : nullptr;
	}

	template<typename T> FB_FORCEINLINE
	T *getQuicklyOptional()
	{
		const SizeType i = getIndex<T>();
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		if (i < FB_MEGATON_MAX_POINTER_COUNT)
			++(lang::MegatonDebugger::megatonQuickUsage[i]);
#endif
		return (i < pointers.getSize()) ? static_cast<T*>(pointers[i]) : nullptr;
	}

	template<typename T>
	void set(T *ptr)
	{
		setImpl(T::megatonIndex, ptr, T::getDebugMegatonName(), getDebugMegatonType());
	}

	FB_NOINLINE void setImpl(SizeType& index, void *ptr, const char *debugMegatonName, const char *debugMegatonType);
	
	template<typename T>
	void trySet(T *ptr)
	{
		T *existing = getOptional<T>();
		if (existing)
			return;

		set(ptr);
	}

	template<typename T>
	void unset(T *pointer)
	{
		FB_MEGATON_ASSERTF(pointer != nullptr, "%s is being unset in %s using a nullptr pointer.", T::getDebugMegatonName(), getDebugMegatonType());
		FB_MEGATON_ASSERTF(T::megatonIndex != getUndefinedIndex(), "%s is being unset in %s before it has been set even once. megatonIndex is unitialized.",
			T::getDebugMegatonName(), getDebugMegatonType());
		FB_MEGATON_ASSERTF(pointer == getQuicklyOptional<T>(), "%s is being unset in %s using a different value.%s",
			T::getDebugMegatonName(), getDebugMegatonType(), (isMegaton(getOwnerMegatonType(T::megatonIndex, pointer)) ?
				debugConcat(" The pointer in question is actually in ", getOwnerMegatonType(T::megatonIndex), ". Did you mean to use that?") :
				" This pointer isn't in any megaton."));
		unset<T>();
	}
	
	template<typename T>
	void tryUnset(T *pointer)
	{
		T *existing = getOptional<T>();
		if (existing && existing == pointer)
		{
			unset(pointer);
		}
	}

	template<typename T>
	void unset()
	{
		const SizeType i = getIndex<T>();

		if (i < pointers.getSize())
		{
			FB_MEGATON_ASSERTF(pointers[i] != nullptr, "%s is being unset in %s while it's not set.", T::getDebugMegatonName(), getDebugMegatonType());
			pointers[i] = nullptr;
		}
	}

	void inherit(Megaton *other)
	{
		fallbackMegaton = other;
	}
	const Megaton *getParentMegaton() const
	{
		FB_MEGATON_ASSERT(fallbackMegaton);
		return fallbackMegaton;
	}

	Megaton *getParentMegaton()
	{
		FB_MEGATON_ASSERT(fallbackMegaton);
		return fallbackMegaton;
	}

private:
	friend class lang::MegatonDebugger;
	FB_NOINLINE static const char *debugConcat(const char *a, const char *b, const char *c);
	FB_NOINLINE static void checkMegatonOwner(SizeType i, const char *name, const char *megatonName);

	void *getWithFallbackImpl(SizeType index) const;
	const char *getDebugMegatonType() const;

	template<typename T> FB_FORCEINLINE
	static SizeType getIndex()
	{
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		// This assert detects when megaton index is larger than it should be but also not invalid
		// if it fires the megatonIndex is somehow truly borked
		FB_MEGATON_ASSERTF(T::megatonIndex == getUndefinedIndex() || T::megatonIndex < megatonTypeIndexCounter, 
			"%s's megaton index is invalid, it is larger than megatonTypeIndexCounter. Index: %u, counter: %u.", 
			T::getDebugMegatonName(), T::megatonIndex, megatonTypeIndexCounter);
#endif
		// If megatonIndex is missing from a class, add FB_MEGATON_CLASS_DECL and FB_MEGATON_CLASS_IMPL macros to the class header and cpp
		return T::megatonIndex;
	}

	Megaton *fallbackMegaton;

	struct
	{
		void *buffer[FB_MEGATON_MAX_POINTER_COUNT];
		SizeType getSize() const { return FB_MEGATON_MAX_POINTER_COUNT; }
		void *&operator[](SizeType index)
		{
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
			FB_MEGATON_ASSERTF(index < getSize(), "Index (%d) out of bouds", index);
#endif
			return buffer[index];
		}
		void *operator[](SizeType index) const
		{
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
			FB_MEGATON_ASSERTF(index < getSize(), "Index (%d) out of bouds", index);
#endif
			return buffer[index];
		}
	} pointers;

protected:
	static bool isMegaton(const char *name)
	{
		// Used only to see if name reads "NONE" or not
		return name[0] != 'N';
	}

	static const char *getOwnerMegatonType(SizeType index);
	static const char *getOwnerMegatonType(SizeType index, void *pointer);
			   
	const char *debugMegatonType; // Points to a string literal
	lang::MegatonDebugger *megatonDebugger;
};
// class Megaton end

#undef FB_CHECK_MEGATON_OWNER
#undef FB_CHECK_RESULT
#undef FB_CHECK_FOR_UNDEFINED

class StateMegaton : public Megaton 
{ 
public: 
	StateMegaton() 
		: Megaton() 
	{
	} 
};

// Enables storing these Megaton classes inside other Megaton classes
class GameStateMegaton : public StateMegaton 
{ 
	FB_MEGATON_CLASS_DECL()

public: 
	GameStateMegaton() 
		: StateMegaton()
	{ 
		debugMegatonType = GameStateMegaton::getDebugMegatonName();
	} 
};

class SceneMegaton : public Megaton 
{ 
	FB_MEGATON_CLASS_DECL() 

public: 
	SceneMegaton() 
		: Megaton() 
	{ 
		debugMegatonType = SceneMegaton::getDebugMegatonName();
	} 
};

#if FB_EDITOR_ENABLED == FB_TRUE
	class EditorStateMegaton : public StateMegaton 
	{ 
		FB_MEGATON_CLASS_DECL() 
	public: 
		EditorStateMegaton() 
			: StateMegaton() 
		{ 
			debugMegatonType = EditorStateMegaton::getDebugMegatonName(); 
		} 
	};
#endif 

typedef Megaton GlobalMegaton;

extern GlobalMegaton *g_globalMegaton;
extern GameStateMegaton *g_gameStateMegaton;

FB_FORCEINLINE GlobalMegaton &getGlobalMegaton()
{ 
	FB_MEGATON_ASSERT(g_globalMegaton); 
	return *g_globalMegaton; 
}

FB_FORCEINLINE GameStateMegaton &getGameStateMegaton()
{ 
	FB_MEGATON_ASSERT(g_gameStateMegaton); 
	return *g_gameStateMegaton; 
}

/* It's probably a sign of bad style to need these hasXXXMegaton() functions. It should be pretty easy to know that we 
 * are not in a state where whatever is needed is created, if not even megaton of its level is not yet there */
FB_FORCEINLINE bool hasGlobalMegaton()
{
	return g_globalMegaton != nullptr;
}

FB_FORCEINLINE bool hasGameStateMegaton()
{
	return g_gameStateMegaton != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//       Global getters for StateMegatons and SceneMegatons
//
//         With great power comes great responsibility.
//                                              - me, just now
//
// Please, only use these when you absolutely must. Let's not start
// over-abusing globals! Always prefer Blobject's getMegaton() over these.
//
// A valid use case would be for example a module from which state and scene
// are unreachable.
//
// Do NOT use these inside static functions. The static keyword loses all
// meaning if the function can access everything in the whole engine.
//
// - Riku R. 2016-08-05
//
///////////////////////////////////////////////////////////////////////////////


// Usage: fb::getStateMegaton(isInEditor())->get<...>()
inline static StateMegaton &getStateMegaton(bool inEditor)
{
#if FB_EDITOR_ENABLED == FB_TRUE
	if (inEditor)
	{
		return getGlobalMegaton().getQuickly<EditorStateMegaton>();
	}
#else
	fb_assert(inEditor == false);
#endif
	return getGameStateMegaton();
}

inline static StateMegaton &getStateMegaton()
{
#if FB_EDITOR_ENABLED == FB_TRUE
	return getStateMegaton(g_megatonDefaultInEditor);
#else
	return getGameStateMegaton();
#endif
}

// Usage: fb::getSceneMegaton(isInEditor())->get<...>()
inline static SceneMegaton &getSceneMegaton(bool inEditor)
{
	return getStateMegaton(inEditor).getQuickly<SceneMegaton>();
}

inline static SceneMegaton &getSceneMegaton()
{
	return getStateMegaton().getQuickly<SceneMegaton>();
}

FB_END_PACKAGE0()
