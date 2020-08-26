#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/Delegate.h"
#include "fb/lang/EnableIf.h"
#include "fb/lang/GlobalFixedAllocateFunctions.h"

FB_PACKAGE1(lang)

class SignalBase;
struct SignalBindImpl
{
	SignalBase *signalBase;
	int32_t refcount;

	SignalBindImpl()
	:	signalBase(NULL)
	,	refcount(0)
	{
	}

	FB_ADD_CLASS_MEMORY_OVERLOADS(SignalBindImpl);
};

FB_END_PACKAGE1()

FB_PACKAGE0()

class SignalBind
{
	lang::SignalBindImpl *impl;

public:
	SignalBind();
	explicit SignalBind(lang::SignalBindImpl *impl);
	~SignalBind();

	// Shouldn't really be copyable, but have to support some legacy crap
	// Remove these (and refcount) after done with refactoring
	SignalBind(const SignalBind &other);
	void operator= (const SignalBind &other);

	// Movable
	SignalBind(SignalBind &&other);
	void operator =(SignalBind &&other);
	void swap(SignalBind &other);

	lang::SignalBindImpl *getImpl() const { return impl;  }
	bool isConnected() const;
	void disconnect();
};

FB_END_PACKAGE0()

FB_PACKAGE1(lang)

class SignalBase
{
	friend class fb::SignalBind;
protected:
	struct DelegateInfo
	{
		DelegateCallData delegateData;
		SignalBindImpl *bindImpl;

		DelegateInfo(const DelegateCallData data_, SignalBindImpl *bindImpl_)
		:	delegateData(data_)
		,	bindImpl(bindImpl_)
		{
		}
	};

	CachePodVector<DelegateInfo, 1> delegates;
	CachePodVector<uint32_t*, 1> iterationStack;

#if FB_BUILD != FB_FINAL_RELEASE
	void validate();
	SizeType numConnects = 0;
	SizeType numDisconnects = 0;
#endif

	// Not copyable
	SignalBase(const SignalBase &) = delete;
	void operator= (const SignalBase &) = delete;

public:
	SignalBase();
	~SignalBase();

	bool isEmpty() const;
	SizeType getDelegateCount() const;
	void clear();

	SignalBind connect(const DelegateCallData &d);
protected:
	void disconnect(SignalBind &bind);
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template <typename T>
class Signal;
template<typename ReturnType, typename ...ParamTypes>
class Signal<ReturnType(ParamTypes...)>: public lang::SignalBase
{
public:
	typedef Delegate<ReturnType(ParamTypes...)> DelegateType;

	SignalBind connect(const DelegateType &d) { return SignalBase::connect(d.getCallData()); }
	
	void operator() (ParamTypes... params)
	{
#if FB_BUILD != FB_FINAL_RELEASE
		validate();
#endif
		uint32_t delegateAmount = (uint32_t) delegates.getSize();

		if (delegateAmount == 1)
		{
			DelegateType::callImplementation(delegates[0].delegateData, params...);
		}
		else if (delegateAmount)
		{
			uint32_t i = 0;
			iterationStack.pushBack(&i);

			for (; i < delegates.getSize(); ++i)
			{
				DelegateType::callImplementation(delegates[i].delegateData, params...);

				// If call above caused our destruction, exit without touching anything
				if (i == 0x7ffffff)
					return;
			}

			iterationStack.popBack();
#if FB_BUILD != FB_FINAL_RELEASE
			if (iterationStack.isEmpty())
			{
				numConnects = 0;
				numDisconnects = 0;
			}
#endif
		}
	}
};

// Overloads which create delegate into given signal. 
// Extremely useful as signal provides exact type leaving more room for various user parameter helpers without massive template mess.

// Note that custom variants are only enabled for non-empty signal parameters, as otherwise signature would match with the prefix version and that results to compiler error.
// For functors, prefix and custom variants have to defined manually be calling either bindDelegatePrefix or bindDelegateCustom.
#define FB_SIGNAL_ENABLE_CUSTOM \
	typename = typename lang::EnableIf<(sizeof...(SignalParamTypes)) != 0>::type

// Mutable method/instance, using signal parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename T>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(SignalParamTypes...), T *instance)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(methodPointer, instance));
}

// Mutable method/instance, using signal parameters and prefixing user parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(UserParam0, SignalParamTypes...), T *instance, UserParam0 userParam0)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), methodPointer, instance, userParam0));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, typename UserParam1>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(UserParam0, UserParam1, SignalParamTypes...), T *instance, UserParam0 userParam0, UserParam1 userParam1)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), methodPointer, instance, userParam0, userParam1));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, typename UserParam1, typename UserParam2>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(UserParam0, UserParam1, UserParam2, SignalParamTypes...), T *instance, UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), methodPointer, instance, userParam0, userParam1, userParam2));
}

// Mutable method/instance, ignores signal parameters and uses only user provided ones (if any)

template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename = typename lang::EnableIf<(sizeof...(SignalParamTypes)) != 0>::type>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(), T *instance)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), methodPointer, instance));
}


template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(UserParam0), T *instance, UserParam0 userParam0)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), methodPointer, instance, userParam0));
}

template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, typename UserParam1, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(UserParam0, UserParam1), T *instance, UserParam0 userParam0, UserParam1 userParam1)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), methodPointer, instance, userParam0, userParam1));
}

template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, typename UserParam1, typename UserParam2, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(UserParam0, UserParam1, UserParam2), T *instance, UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), methodPointer, instance, userParam0, userParam1, userParam2));
}

// Const method/instance, using signal parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename T>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(T::*methodPointer)(SignalParamTypes...) const, const T *instance)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(methodPointer, instance));
}

// Function/static method, using signal parameters
template<typename SignalReturnType, typename ...SignalParamTypes>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(SignalParamTypes...))
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(functionPointer));
}

// Function/static method, using signal parameters and prefixing user parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, SignalParamTypes...), UserParam0 userParam0)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), functionPointer, userParam0));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, typename UserParam1>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, UserParam1, SignalParamTypes...), UserParam0 userParam0, UserParam1 userParam1)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), functionPointer, userParam0, userParam1));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, typename UserParam1, typename UserParam2>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, UserParam1, UserParam2, SignalParamTypes...), UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), functionPointer, userParam0, userParam1, userParam2));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, typename UserParam1, typename UserParam2, typename UserParam3>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, UserParam1, UserParam2, UserParam3, SignalParamTypes...), UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2, UserParam3 userParam3)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), functionPointer, userParam0, userParam1, userParam2, userParam3));
}

// Function/static method, ignores signal parameters and uses only user parameters
template<typename SignalReturnType, typename ...SignalParamTypes, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)())
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functionPointer));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0), UserParam0 userParam0)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functionPointer, userParam0));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, typename UserParam1, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, UserParam1), UserParam0 userParam0, UserParam1 userParam1)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functionPointer, userParam0, userParam1));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, typename UserParam1, typename UserParam2, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, UserParam1, UserParam2), UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functionPointer, userParam0, userParam1, userParam2));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename UserParam0, typename UserParam1, typename UserParam2, typename UserParam3, FB_SIGNAL_ENABLE_CUSTOM>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, SignalReturnType(*functionPointer)(UserParam0, UserParam1, UserParam2, UserParam3), UserParam0 userParam0, UserParam1 userParam1, UserParam2 userParam2, UserParam3 userParam3)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functionPointer, userParam0, userParam1, userParam2, userParam3));
}

// Functor, using signal parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, FB_DELEGATE_ENABLE_FUNCTOR>
SignalBind bindDelegate(Signal<SignalReturnType(SignalParamTypes...)> &signal, const T &functor)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(functor));
}

// Functor, using signal parameters and prefixing user parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, FB_DELEGATE_ENABLE_FUNCTOR>
SignalBind bindDelegatePrefix(Signal<SignalReturnType(SignalParamTypes...)> &signal, const T &functor, UserParam0 userParam0)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), functor, userParam0));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, typename UserParam1, FB_DELEGATE_ENABLE_FUNCTOR>
SignalBind bindDelegatePrefix(Signal<SignalReturnType(SignalParamTypes...)> &signal, const T &functor, UserParam0 userParam0, UserParam1 userParam1)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegatePrefixType(), functor, userParam0, userParam1));
}

// Functor, ignores signal parameters and uses only user parameters
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, FB_DELEGATE_ENABLE_FUNCTOR>
SignalBind bindDelegateCustom(Signal<SignalReturnType(SignalParamTypes...)> &signal, const T &functor)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functor));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, FB_DELEGATE_ENABLE_FUNCTOR>
SignalBind bindDelegateCustom(Signal<SignalReturnType(SignalParamTypes...)> &signal, const T &functor, UserParam0 userParam0)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functor, userParam0));
}
template<typename SignalReturnType, typename ...SignalParamTypes, typename T, typename UserParam0, typename UserParam1, FB_DELEGATE_ENABLE_FUNCTOR>
SignalBind bindDelegateCustom(Signal<SignalReturnType(SignalParamTypes...)> &signal, const T &functor, UserParam0 userParam0, UserParam1 userParam1)
{
	typedef Delegate<SignalReturnType(SignalParamTypes...)> DelegateType;
	return signal.connect(DelegateType(DelegateCustomType(), functor, userParam0, userParam1));
}

FB_END_PACKAGE0()
