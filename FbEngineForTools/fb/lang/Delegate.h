#pragma once

#include "fb/lang/EnableIf.h"
#include "fb/lang/IntTypes.h"
#include "fb/lang/IsClass.h"
#include "fb/lang/IsTriviallyCopyable.h"
#include "fb/lang/PlacementNew.h"

FB_PACKAGE0()

// This blob stores everything necessary to store and call delegate of any type.
// It's separated from the actual delegate template type to enable storing call data to non-template classes which reduces template bloat.
// Trying to create delegate which doesn't fit into it will generate a static assert.
// If that happens, optimise what you are trying to store or increase the size.
struct DelegateCallData
{
	// First element in storage is Delegate<Foo>::ImpFunction which adds void* pointer to given function syntax.
	// Rest of the data is call-specific information which gets passed to ImpFunction in order to call wanted function/method.
#if FB_COMPILER == FB_MSC
	static const int SizeInBytes = 48;
#elif FB_COMPILER == FB_CLANG
	/* T::*methodPointer is 16 bytes for Clang, while only 8 for MSC */
	static const int SizeInBytes = 56;
#elif FB_COMPILER == FB_GNUC
	/* No idea how GCC compares to MSC and Clang. If you run into this, update size */
	static const int SizeInBytes = 48;
#else
#error Unsupported compiler
#endif
	char storage[SizeInBytes];

	DelegateCallData();

	void clear();
	bool operator == (const DelegateCallData &other) const;

	operator bool() const
	{
		for (int i = 0; i < SizeInBytes; ++i)
		{
			if (storage[i] != 0)
				return true;
		}
		return false;
	}
};


// Dummy types to trigger specific overloads
struct DelegatePrefixType {};
struct DelegateCustomType {};

// Simple delegate implementation which supports methods and functions with arbitrary number of parameters.
// Never allocates memory, calling parameters are stored to local byte buffer.
// User is responsible for keeping instance data alive, and not calling operator() after that's not the case.
//
// As everything gets compressed into char[] blob, make sure all by-value data is trivially copyable.
// You'll get static_assert if violating this. Pointers are fine, obviously.
//
// Specifically - if using functors, beware that by-value version can't modify itself when called or results are undefined.
// For such cases, use pointer to functor. To help preventing this, by-value version calls operator() const.
// This can't be properly enforced, so you've been warned.

// Only enable functor overloads if parameter is a class.
#define FB_DELEGATE_ENABLE_FUNCTOR \
	typename = typename lang::EnableIf<lang::IsClass<T>::value>::type

template <typename T>
class Delegate;
template<typename ReturnType, typename ...ParamTypes>
class Delegate<ReturnType(ParamTypes...)>: public DelegateCallData
{
	typedef ReturnType(*ImpFunction)(const void *callingData, ParamTypes...);
	static const uint32_t ImpFunctionPointerSize = sizeof(ImpFunction);

	// -------------------------------------------------
	//  Implementation structs for various calling types
	// -------------------------------------------------

	// Information to call specified mutable method for a class instance
	template<typename T>
	struct ClassMethodData
	{
		ReturnType(T::*methodPointer)(ParamTypes...);
		T *instancePointer;
	};
	// With prefixed user parameters
	template<typename T, typename UserParamType0>
	struct ClassMethodDataPre1
	{
		ReturnType(T::*methodPointer)(UserParamType0, ParamTypes...);
		T *instancePointer;
		UserParamType0 userParam0;
	};
	template<typename T, typename UserParamType0, typename UserParamType1>
	struct ClassMethodDataPre2
	{
		ReturnType(T::*methodPointer)(UserParamType0, UserParamType1, ParamTypes...);
		T *instancePointer;
		UserParamType0 userParam0;
		UserParamType1 userParam1;
	};
	template<typename T, typename UserParamType0, typename UserParamType1, typename UserParamType2>
	struct ClassMethodDataPre3
	{
		ReturnType(T::*methodPointer)(UserParamType0, UserParamType1, UserParamType2, ParamTypes...);
		T *instancePointer;
		UserParamType0 userParam0;
		UserParamType1 userParam1;
		UserParamType2 userParam2;
	};
	// With user specified parameters only
	template<typename T>
	struct CustomClassMethodData0
	{
		ReturnType(T::*methodPointer)();
		T *instancePointer;
	};
	template<typename T, typename UserParamType0>
	struct CustomClassMethodData1
	{
		ReturnType(T::*methodPointer)(UserParamType0);
		T *instancePointer;
		UserParamType0 userParam0;
	};
	template<typename T, typename UserParamType0, typename UserParamType1>
	struct CustomClassMethodData2
	{
		ReturnType(T::*methodPointer)(UserParamType0, UserParamType1);
		T *instancePointer;
		UserParamType0 userParam0;
		UserParamType1 userParam1;
	};
	template<typename T, typename UserParamType0, typename UserParamType1, typename UserParamType2>
	struct CustomClassMethodData3
	{
		ReturnType(T::*methodPointer)(UserParamType0, UserParamType1, UserParamType2);
		T *instancePointer;
		UserParamType0 userParam0;
		UserParamType1 userParam1;
		UserParamType2 userParam2;
	};

	// Information to call specified const method for a class instance
	template<typename T>
	struct ClassConstMethodData
	{
		ReturnType(T::*methodPointer)(ParamTypes...) const;
		const T *instancePointer;
	};

	// Information to call specified function
	struct FunctionData
	{
		ReturnType(*functionPointer)(ParamTypes...);
	};
	// With prefixed user data
	template<typename UserParamType0>
	struct FunctionDataPre1
	{
		ReturnType(*functionPointer)(UserParamType0, ParamTypes...);
		UserParamType0 userParam0;
	};
	template<typename UserParamType0, typename UserParamType1>
	struct FunctionDataPre2
	{
		ReturnType(*functionPointer)(UserParamType0, UserParamType1, ParamTypes...);
		UserParamType0 userParam0;
		UserParamType1 userParam1;
	};
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2>
	struct FunctionDataPre3
	{
		ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2, ParamTypes...);
		UserParamType0 userParam0;
		UserParamType1 userParam1;
		UserParamType2 userParam2;
	};
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2, typename UserParamType3>
	struct FunctionDataPre4
	{
		ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2, UserParamType3, ParamTypes...);
		UserParamType0 userParam0;
		UserParamType1 userParam1;
		UserParamType2 userParam2;
		UserParamType3 userParam3;
	};
	// With user specified parameters only
	struct FunctionDataCustom0
	{
		ReturnType(*functionPointer)();
	};
	template<typename UserParamType0>
	struct FunctionDataCustom1
	{
		ReturnType(*functionPointer)(UserParamType0);
		UserParamType0 userParam0;
	};
	template<typename UserParamType0, typename UserParamType1>
	struct FunctionDataCustom2
	{
		ReturnType(*functionPointer)(UserParamType0, UserParamType1);
		UserParamType0 userParam0;
		UserParamType1 userParam1;
	};
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2>
	struct FunctionDataCustom3
	{
		ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2);
		UserParamType0 userParam0;
		UserParamType1 userParam1;
		UserParamType2 userParam2;
	};
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2, typename UserParamType3>
	struct FunctionDataCustom4
	{
		ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2, UserParamType3);
		UserParamType0 userParam0;
		UserParamType1 userParam1;
		UserParamType2 userParam2;
		UserParamType3 userParam3;
	};

	// Information to call given functor
	template<typename T>
	struct FunctorData
	{
		explicit FunctorData(const T &f) : functor(f) { }
		T functor;
	};
	// With prefixed user data
	template<typename T, typename UserParamType0>
	struct FunctorDataPre1
	{
		FunctorDataPre1(const T &f, const UserParamType0 &u0) : functor(f), userParam0(u0) {}
		T functor;
		UserParamType0 userParam0;
	};
	template<typename T, typename UserParamType0, typename UserParamType1>
	struct FunctorDataPre2
	{
		FunctorDataPre2(const T &f, const UserParamType0 &u0, const UserParamType0 &u1) : functor(f), userParam0(u0), userParam1(u1) {}
		T functor;
		UserParamType0 userParam0;
		UserParamType1 userParam1;
	};
	// With user specified parameters only
	template<typename T>
	struct FunctorDataCustom0
	{
		explicit FunctorDataCustom0(const T &f) : functor(f) {}
		T functor;
	};
	template<typename T, typename UserParamType0>
	struct FunctorDataCustom1
	{
		FunctorDataCustom1(const T &f, const UserParamType0 &u0) : functor(f), userParam0(u0) {}
		T functor;
		UserParamType0 userParam0;
	};
	template<typename T, typename UserParamType0, typename UserParamType1>
	struct FunctorDataCustom2
	{
		FunctorDataCustom2(const T &f, const UserParamType0 &u0, const UserParamType1 &u1) : functor(f), userParam0(u0), userParam1(u1) {}
		T functor;
		UserParamType0 userParam0;
		UserParamType1 userParam1;
	};


	// -----------------------------------------------------------------------------------------
	//  Implementation function pointers (casting implementation struct to call the right thing)
	// -----------------------------------------------------------------------------------------

	// Wrapper to unpack single data pointer to instance/method pointer combination
	template<typename T>
	static ReturnType classMethodCallerImp(const void *data, ParamTypes... params)
	{
		const ClassMethodData<T> *d = static_cast<const ClassMethodData<T> *>  (data);
		return (d->instancePointer->*d->methodPointer)(params...);
	}

	// Wrapper to unpack single data pointer to instance/method pointer combination with prefixed user data
	template<typename T, typename UserParamType0>
	static ReturnType classMethodCallerPreImp(const void *data, ParamTypes... params)
	{
		const ClassMethodDataPre1<T, UserParamType0> *d = static_cast<const ClassMethodDataPre1<T, UserParamType0> *>  (data);
		return (d->instancePointer->*d->methodPointer)(d->userParam0, params...);
	}
	template<typename T, typename UserParamType0, typename UserParamType1>
	static ReturnType classMethodCallerPreImp(const void *data, ParamTypes... params)
	{
		const ClassMethodDataPre2<T, UserParamType0, UserParamType1> *d = static_cast<const ClassMethodDataPre2<T, UserParamType0, UserParamType1> *>  (data);
		return (d->instancePointer->*d->methodPointer)(d->userParam0, d->userParam1, params...);
	}
	template<typename T, typename UserParamType0, typename UserParamType1, typename UserParamType2>
	static ReturnType classMethodCallerPreImp(const void *data, ParamTypes... params)
	{
		const ClassMethodDataPre3<T, UserParamType0, UserParamType1, UserParamType2> *d = static_cast<const ClassMethodDataPre3<T, UserParamType0, UserParamType1, UserParamType2> *>  (data);
		return (d->instancePointer->*d->methodPointer)(d->userParam0, d->userParam1, d->userParam2, params...);
	}

	// Wrapper to unpack single data pointer to instance/method pointer combination using only user parameters
	template<typename T>
	static ReturnType customClassMethodCallerImp(const void *data, ParamTypes... params)
	{
		const CustomClassMethodData0<T> *d = static_cast<const CustomClassMethodData0<T> *>  (data);
		return (d->instancePointer->*d->methodPointer)();
	}
	template<typename T, typename UserParamType0>
	static ReturnType customClassMethodCallerImp(const void *data, ParamTypes... params)
	{
		const CustomClassMethodData1<T, UserParamType0> *d = static_cast<const CustomClassMethodData1<T, UserParamType0> *>  (data);
		return (d->instancePointer->*d->methodPointer)(d->userParam0);
	}
	template<typename T, typename UserParamType0, typename UserParamType1>
	static ReturnType customClassMethodCallerImp(const void *data, ParamTypes... params)
	{
		const CustomClassMethodData2<T, UserParamType0, UserParamType1> *d = static_cast<const CustomClassMethodData2<T, UserParamType0, UserParamType1> *>  (data);
		return (d->instancePointer->*d->methodPointer)(d->userParam0, d->userParam1);
	}
	template<typename T, typename UserParamType0, typename UserParamType1, typename UserParamType2>
	static ReturnType customClassMethodCallerImp(const void *data, ParamTypes... params)
	{
		const CustomClassMethodData3<T, UserParamType0, UserParamType1, UserParamType2> *d = static_cast<const CustomClassMethodData3<T, UserParamType0, UserParamType1, UserParamType2> *>  (data);
		return (d->instancePointer->*d->methodPointer)(d->userParam0, d->userParam1, d->userParam2);
	}

	// Wrapper to unpack single data pointer to const instance/method pointer combination
	template<typename T>
	static ReturnType classConstMethodCallerImp(const void *data, ParamTypes... params)
	{
		const ClassConstMethodData<T> *d = static_cast<const ClassConstMethodData<T> *>  (data);
		return (d->instancePointer->*d->methodPointer)(params...);
	}

	// Wrapper to unpack single data pointer to function call
	static ReturnType functionCallerImp(const void *data, ParamTypes... params)
	{
		const FunctionData *d = static_cast<const FunctionData *>  (data);
		return (*d->functionPointer)(params...);
	}

	// Wrapper to unpack single data pointer to function call with prefixed user data
	template<typename UserParamType0>
	static ReturnType functionCallerPreImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataPre1<UserParamType0> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, params...);
	}
	template<typename UserParamType0, typename UserParamType1>
	static ReturnType functionCallerPreImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataPre2<UserParamType0, UserParamType1> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, d->userParam1, params...);
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2>
	static ReturnType functionCallerPreImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataPre3<UserParamType0, UserParamType1, UserParamType2> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, d->userParam1, d->userParam2, params...);
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2, typename UserParamType3>
	static ReturnType functionCallerPreImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataPre4<UserParamType0, UserParamType1, UserParamType2, UserParamType3> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, d->userParam1, d->userParam2, d->userParam3, params...);
	}

	// Wrapper to unpack single data pointer to function call using only user parameters
	static ReturnType functionCallerCustomImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataCustom0 FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)();
	}
	template<typename UserParamType0>
	static ReturnType functionCallerCustomImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataCustom1<UserParamType0> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0);
	}
	template<typename UserParamType0, typename UserParamType1>
	static ReturnType functionCallerCustomImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataCustom2<UserParamType0, UserParamType1> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, d->userParam1);
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2>
	static ReturnType functionCallerCustomImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataCustom3<UserParamType0, UserParamType1, UserParamType2> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, d->userParam1, d->userParam2);
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2, typename UserParamType3>
	static ReturnType functionCallerCustomImp(const void *data, ParamTypes... params)
	{
		typedef FunctionDataCustom4<UserParamType0, UserParamType1, UserParamType2, UserParamType3> FuncData;
		const FuncData *d = static_cast<const FuncData *>  (data);
		return (*d->functionPointer)(d->userParam0, d->userParam1, d->userParam2, d->userParam3);
	}

	// Wrapper to unpack single data pointer to functor call
	template<typename T>
	static ReturnType functorCallerImp(const void *dataConst, ParamTypes... params)
	{
		// We hold it by-value so cast away default constness
		void *data = const_cast<void*> (dataConst);
		typedef FunctorData<T> FuncData;
		FuncData *d = static_cast<FuncData *> (data);
		return d->functor(params...);
	}

	// Wrapper to unpack single data pointer to functor call with prefixed user data
	template<typename T, typename UserParamType0>
	static ReturnType functorCallerPreImp(const void *dataConst, ParamTypes... params)
	{
		// We hold it by-value so cast away default constness
		void *data = const_cast<void*> (dataConst);
		typedef FunctorDataPre1<T, UserParamType0> FuncData;
		FuncData *d = static_cast<FuncData *> (data);
		return d->functor(d->userParam0, params...);
	}
	template<typename T, typename UserParamType0, typename UserParamType1>
	static ReturnType functorCallerPreImp(const void *dataConst, ParamTypes... params)
	{
		// We hold it by-value so cast away default constness
		void *data = const_cast<void*> (dataConst);
		typedef FunctorDataPre2<T, UserParamType0, UserParamType1> FuncData;
		FuncData *d = static_cast<FuncData *> (data);
		return d->functor(d->userParam0, d->userParam1, params...);
	}

	// Wrapper to unpack single data pointer to functor call using only user parameters
	template<typename T>
	static ReturnType functorCallerCustomImp(const void *dataConst, ParamTypes... params)
	{
		// We hold it by-value so cast away default constness
		void *data = const_cast<void*> (dataConst);
		typedef FunctorDataCustom0<T> FuncData;
		FuncData *d = static_cast<FuncData *> (data);
		return d->functor();
	}
	template<typename T, typename UserParamType0>
	static ReturnType functorCallerCustomImp(const void *dataConst, ParamTypes... params)
	{
		// We hold it by-value so cast away default constness
		void *data = const_cast<void*> (dataConst);
		typedef FunctorDataCustom1<T, UserParamType0> FuncData;
		FuncData *d = static_cast<FuncData *> (data);
		return d->functor(d->userParam0);
	}
	template<typename T, typename UserParamType0, typename UserParamType1>
	static ReturnType functorCallerCustomImp(const void *dataConst, ParamTypes... params)
	{
		// We hold it by-value so cast away default constness
		void *data = const_cast<void*> (dataConst);
		typedef FunctorDataCustom2<T, UserParamType0, UserParamType1> FuncData;
		FuncData *d = static_cast<FuncData *> (data);
		return d->functor(d->userParam0, d->userParam1);
	}

public:
	Delegate()
	{
	}

	// ----------------------------------------------------
	// Method and class instance pointer variants (mutable)
	// ----------------------------------------------------

	// Method and class instance pointer
	template<typename T>
	Delegate(ReturnType(T::*methodPointer)(ParamTypes...), T *instancePointer)
	{
		new (storage) ImpFunction(&classMethodCallerImp<T>);

		typedef ClassMethodData<T> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
	}

	// Method and class instance pointer. Adds user data before ParamTypes
	template<typename T, typename UserParamType0>
	Delegate(DelegatePrefixType, ReturnType(T::*methodPointer)(UserParamType0, ParamTypes...), T *instancePointer, UserParamType0 userParam0)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&classMethodCallerPreImp<T, UserParamType0>);

		typedef ClassMethodDataPre1<T, UserParamType0> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
		d->userParam0 = userParam0;
	}
	template<typename T, typename UserParamType0, typename UserParamType1>
	Delegate(DelegatePrefixType, ReturnType(T::*methodPointer)(UserParamType0, UserParamType1, ParamTypes...), T *instancePointer, UserParamType0 userParam0, UserParamType1 userParam1)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&classMethodCallerPreImp<T, UserParamType0, UserParamType1>);

		typedef ClassMethodDataPre2<T, UserParamType0, UserParamType1> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
	}
	template<typename T, typename UserParamType0, typename UserParamType1, typename UserParamType2>
	Delegate(DelegatePrefixType, ReturnType(T::*methodPointer)(UserParamType0, UserParamType1, UserParamType2, ParamTypes...), T *instancePointer, UserParamType0 userParam0, UserParamType1 userParam1, UserParamType2 userParam2)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType2>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&classMethodCallerPreImp<T, UserParamType0, UserParamType1, UserParamType2>);

		typedef ClassMethodDataPre3<T, UserParamType0, UserParamType1, UserParamType2> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
		d->userParam2 = userParam2;
	}

	// Method and class instance pointer. Ignores ParamTypes... and only uses user parameters
	template<typename T>
	Delegate(DelegateCustomType, ReturnType(T::*methodPointer)(), T *instancePointer)
	{
		new (storage) ImpFunction(&customClassMethodCallerImp<T>);

		typedef CustomClassMethodData0<T> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
	}
	template<typename T, typename UserParamType0>
	Delegate(DelegateCustomType, ReturnType(T::*methodPointer)(UserParamType0), T *instancePointer, UserParamType0 userParam0)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&customClassMethodCallerImp<T, UserParamType0>);

		typedef CustomClassMethodData1<T, UserParamType0> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
		d->userParam0 = userParam0;
	}
	template<typename T, typename UserParamType0, typename UserParamType1>
	Delegate(DelegateCustomType, ReturnType(T::*methodPointer)(UserParamType0, UserParamType1), T *instancePointer, UserParamType0 userParam0, UserParamType1 userParam1)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&customClassMethodCallerImp<T, UserParamType0, UserParamType1>);

		typedef CustomClassMethodData2<T, UserParamType0, UserParamType1> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
	}
	template<typename T, typename UserParamType0, typename UserParamType1, typename UserParamType2>
	Delegate(DelegateCustomType, ReturnType(T::*methodPointer)(UserParamType0, UserParamType1, UserParamType2), T *instancePointer, UserParamType0 userParam0, UserParamType1 userParam1, UserParamType2 userParam2)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType2>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&customClassMethodCallerImp<T, UserParamType0, UserParamType1, UserParamType2>);
		typedef CustomClassMethodData3<T, UserParamType0, UserParamType1, UserParamType2> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
		d->userParam2 = userParam2;
	}

	// ----------------------------------------------------
	// Method and class instance pointer variants (const)
	// ----------------------------------------------------

	// Method and class instance pointer
	template<typename T>
	Delegate(ReturnType(T::*methodPointer)(ParamTypes...) const, const T *instancePointer)
	{
		new (storage) ImpFunction(&classConstMethodCallerImp<T>);

		typedef ClassConstMethodData<T> MethodData;
		static_assert(ImpFunctionPointerSize + sizeof(MethodData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		MethodData *d = new (storage + ImpFunctionPointerSize) MethodData();
		d->instancePointer = instancePointer;
		d->methodPointer = methodPointer;
	}

	// -------------------------------
	// Function/static method variants
	// -------------------------------

	// Function/static method
	explicit Delegate(ReturnType(*functionPointer)(ParamTypes...))
	{
		static_assert(ImpFunctionPointerSize + sizeof(FunctionData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage) ImpFunction(&functionCallerImp);

		FunctionData *d = new (storage + ImpFunctionPointerSize) FunctionData();
		d->functionPointer = functionPointer;
	}

	// Function/static method. Adds user data before ParamTypes
	template<typename UserParamType0>
	Delegate(DelegatePrefixType, ReturnType(*functionPointer)(UserParamType0, ParamTypes...), UserParamType0 userParam0)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerPreImp<UserParamType0>);

		typedef FunctionDataPre1<UserParamType0> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
	}
	template<typename UserParamType0, typename UserParamType1>
	Delegate(DelegatePrefixType, ReturnType(*functionPointer)(UserParamType0, UserParamType1, ParamTypes...), UserParamType0 userParam0, UserParamType1 userParam1)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerPreImp<UserParamType0, UserParamType1>);

		typedef FunctionDataPre2<UserParamType0, UserParamType1> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2>
	Delegate(DelegatePrefixType, ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2, ParamTypes...), UserParamType0 userParam0, UserParamType1 userParam1, UserParamType2 userParam2)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType2>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerPreImp<UserParamType0, UserParamType1, UserParamType2>);

		typedef FunctionDataPre3<UserParamType0, UserParamType1, UserParamType2> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
		d->userParam2 = userParam2;
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2, typename UserParamType3>
	Delegate(DelegatePrefixType, ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2, UserParamType3, ParamTypes...), UserParamType0 userParam0, UserParamType1 userParam1, UserParamType2 userParam2, UserParamType3 userParam3)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType2>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType3>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerPreImp<UserParamType0, UserParamType1, UserParamType2, UserParamType3>);

		typedef FunctionDataPre4<UserParamType0, UserParamType1, UserParamType2, UserParamType3> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
		d->userParam2 = userParam2;
		d->userParam3 = userParam3;
	}

	// Function/static method. Ignores ParamTypes... and only uses user parameters
	Delegate(DelegateCustomType, ReturnType(*functionPointer)())
	{
		new (storage) ImpFunction(&functionCallerCustomImp);

		typedef FunctionDataCustom0 FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
	}
	template<typename UserParamType0>
	Delegate(DelegateCustomType, ReturnType(*functionPointer)(UserParamType0), UserParamType0 userParam0)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerCustomImp<UserParamType0>);

		typedef FunctionDataCustom1<UserParamType0> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
	}
	template<typename UserParamType0, typename UserParamType1>
	Delegate(DelegateCustomType, ReturnType(*functionPointer)(UserParamType0, UserParamType1), UserParamType0 userParam0, UserParamType1 userParam1)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerCustomImp<UserParamType0, UserParamType1>);

		typedef FunctionDataCustom2<UserParamType0, UserParamType1> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2>
	Delegate(DelegateCustomType, ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2), UserParamType0 userParam0, UserParamType1 userParam1, UserParamType2 userParam2)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType2>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerCustomImp<UserParamType0, UserParamType1, UserParamType2>);

		typedef FunctionDataCustom3<UserParamType0, UserParamType1, UserParamType2> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
		d->userParam2 = userParam2;
	}
	template<typename UserParamType0, typename UserParamType1, typename UserParamType2, typename UserParamType3>
	Delegate(DelegateCustomType, ReturnType(*functionPointer)(UserParamType0, UserParamType1, UserParamType2, UserParamType3), UserParamType0 userParam0, UserParamType1 userParam1, UserParamType2 userParam2, UserParamType3 userParam3)
	{
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType2>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType3>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functionCallerCustomImp<UserParamType0, UserParamType1, UserParamType2, UserParamType3>);

		typedef FunctionDataCustom4<UserParamType0, UserParamType1, UserParamType2, UserParamType3> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		FuncData *d = new (storage + ImpFunctionPointerSize) FuncData();
		d->functionPointer = functionPointer;
		d->userParam0 = userParam0;
		d->userParam1 = userParam1;
		d->userParam2 = userParam2;
		d->userParam3 = userParam3;
	}

	// ---------------------------
	// Functor (by-value) variants
	// ---------------------------

	// Functor (by value)
	template<typename T, FB_DELEGATE_ENABLE_FUNCTOR>
	explicit Delegate(const T &functor)
	{
		static_assert(lang::IsTriviallyCopyable<T>::value, "Given functor must be trivially copyable.");
		new (storage) ImpFunction(&functorCallerImp<T>);

		typedef FunctorData<T> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage + ImpFunctionPointerSize) FuncData(functor);
	}

	// Functor (by value). Adds user data before ParamTypes
	template<typename T, typename UserParamType0, FB_DELEGATE_ENABLE_FUNCTOR>
	explicit Delegate(DelegatePrefixType, const T &functor, UserParamType0 userParam0)
	{
		static_assert(lang::IsTriviallyCopyable<T>::value, "Given functor must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functorCallerPreImp<T, UserParamType0>);

		typedef FunctorDataPre1<T, UserParamType0> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage + ImpFunctionPointerSize) FuncData(functor, userParam0);
	}
	template<typename T, typename UserParamType0, typename UserParamType1, FB_DELEGATE_ENABLE_FUNCTOR>
	explicit Delegate(DelegatePrefixType, const T &functor, UserParamType0 userParam0, UserParamType1 userParam1)
	{
		static_assert(lang::IsTriviallyCopyable<T>::value, "Given functor must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functorCallerPreImp<T, UserParamType0, UserParamType1>);

		typedef FunctorDataPre2<T, UserParamType0, UserParamType1> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage + ImpFunctionPointerSize) FuncData(functor, userParam0, userParam1);
	}

	// Functor (by value). Ignores ParamTypes... and only uses user parameters
	template<typename T, FB_DELEGATE_ENABLE_FUNCTOR>
	Delegate(DelegateCustomType, const T &functor)
	{
		static_assert(lang::IsTriviallyCopyable<T>::value, "Given functor must be trivially copyable.");
		new (storage) ImpFunction(&functorCallerCustomImp<T>);

		typedef FunctorDataCustom0<T> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage + ImpFunctionPointerSize) FuncData(functor);
	}
	template<typename T, typename UserParamType0, FB_DELEGATE_ENABLE_FUNCTOR>
	Delegate(DelegateCustomType, const T &functor, UserParamType0 userParam0)
	{
		static_assert(lang::IsTriviallyCopyable<T>::value, "Given functor must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functorCallerCustomImp<T, UserParamType0>);

		typedef FunctorDataCustom1<T, UserParamType0> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage + ImpFunctionPointerSize) FuncData(functor, userParam0);
	}
	template<typename T, typename UserParamType0, typename UserParamType1, FB_DELEGATE_ENABLE_FUNCTOR>
	Delegate(DelegateCustomType, const T &functor, UserParamType0 userParam0, UserParamType1 userParam1)
	{
		static_assert(lang::IsTriviallyCopyable<T>::value, "Given functor must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType0>::value, "Stored user parameter must be trivially copyable.");
		static_assert(lang::IsTriviallyCopyable<UserParamType1>::value, "Stored user parameter must be trivially copyable.");
		new (storage) ImpFunction(&functorCallerCustomImp<T, UserParamType0, UserParamType1>);

		typedef FunctorDataCustom2<T, UserParamType0, UserParamType1> FuncData;
		static_assert(ImpFunctionPointerSize + sizeof(FuncData) <= DelegateCallData::SizeInBytes, "Not enough local storage to hold given delegate.");
		new (storage + ImpFunctionPointerSize) FuncData(functor, userParam0, userParam1);
	}


	ReturnType operator()(ParamTypes... params) const
	{
		return callImplementation(getCallData(), params...);
	}

	static ReturnType callImplementation(const DelegateCallData &callData, ParamTypes... params)
	{
		ImpFunction *impFunction = (ImpFunction*)callData.storage;
		return (*impFunction)(static_cast<const void*>(callData.storage + ImpFunctionPointerSize), params...);
	}

	const DelegateCallData &getCallData() const { return *this; }
};

// Helpers for nicer creation syntax.
// Funtion parameters are extracted from the pointer type.
// Eg, 
//	makeDelegate(&FooClass::barMethod, instancePointer);
//	makeDelegate(&fooFunction);

// Mutable method/instance
template<typename T, typename ReturnType, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(T::*methodPointer)(ParamTypes...), T *instance)
{
	return Delegate<ReturnType(ParamTypes...)>(methodPointer, instance);
}
template<typename T, typename ReturnType, typename UserParam0, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(T::*methodPointer)(UserParam0, ParamTypes...), T *instance, UserParam0 userParam0)
{
	return Delegate<ReturnType(ParamTypes...)>(DelegatePrefixType(), methodPointer, instance, userParam0);
}
template<typename T, typename ReturnType, typename UserParam0, typename UserParam1, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(T::*methodPointer)(UserParam0, UserParam1, ParamTypes...), T *instance, UserParam0 userParam0, UserParam1 userParam1)
{
	return Delegate<ReturnType(ParamTypes...)>(DelegatePrefixType(), methodPointer, instance, userParam0, userParam1);
}

// Mutable method/instance, using only custom user parameters
template<typename T, typename ReturnType, typename UserParam0>
Delegate<ReturnType()> makeDelegateCustom(ReturnType(T::*methodPointer)(UserParam0), T *instance, UserParam0 userParam0)
{
	return Delegate<ReturnType()>(DelegateCustomType(), methodPointer, instance, userParam0);
}
template<typename T, typename ReturnType, typename UserParam0, typename UserParam1>
Delegate<ReturnType()> makeDelegateCustom(ReturnType(T::*methodPointer)(UserParam0, UserParam1), T *instance, UserParam0 userParam0, UserParam1 userParam1)
{
	return Delegate<ReturnType()>(DelegateCustomType(), methodPointer, instance, userParam0, userParam1);
}

// Function pointer / static method
template<typename ReturnType, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(*functionPointer)(ParamTypes...))
{
	return Delegate<ReturnType(ParamTypes...)>(functionPointer);
}
template<typename ReturnType, typename UserParam0, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(*functionPointer)(UserParam0, ParamTypes...), UserParam0 userParam0)
{
	return Delegate<ReturnType(ParamTypes...)>(DelegatePrefixType(), functionPointer, userParam0);
}
template<typename ReturnType, typename UserParam0, typename UserParam1, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(*functionPointer)(UserParam0, UserParam1, ParamTypes...), UserParam0 userParam0, UserParam1 userParam1)
{
	return Delegate<ReturnType(ParamTypes...)>(DelegatePrefixType(), functionPointer, userParam0, userParam1);
}
template<typename ReturnType, typename UserParam0, typename UserParam1, typename UserParam2, typename ...ParamTypes>
Delegate<ReturnType(ParamTypes...)> makeDelegate(ReturnType(*functionPointer)(UserParam0, UserParam1, UserParam2, ParamTypes...), UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2)
{
	return Delegate<ReturnType(ParamTypes...)>(DelegatePrefixType(), functionPointer, userParam0, userParam1, userParam2);
}

FB_END_PACKAGE0()
