#include "Precompiled.h"
#include "Signal.h"

#include "fb/profiling/ZoneProfiler.h"

FB_PACKAGE0()

// SignalBind

SignalBind::SignalBind()
:	impl(NULL)
{
}

SignalBind::SignalBind(lang::SignalBindImpl *impl_)
:	impl(impl_)
{
}

SignalBind::~SignalBind()
{
	if (impl)
	{
		if (impl->refcount == 1)
			disconnect();
		else
			--impl->refcount;
	}
}

SignalBind::SignalBind(SignalBind &&other)
:	impl(NULL)
{
	swap(other);
}

void SignalBind::operator =(SignalBind &&other)
{
	swap(other);
}

void SignalBind::swap(SignalBind &other)
{
	lang::swap(impl, other.impl);
}

SignalBind::SignalBind(const SignalBind &other)
:	impl(other.impl)
{
	if (impl)
		++impl->refcount;
}

void SignalBind::operator= (const SignalBind &other)
{
	if (other.impl)
		++other.impl->refcount;

	if (impl)
	{
		if (impl->refcount == 1)
			disconnect();
		else
			--impl->refcount;
	}

	impl = other.impl;
}


bool SignalBind::isConnected() const
{
	bool connected = impl != NULL && impl->signalBase != NULL;
	fb_assert(!connected || impl->refcount > 0);
	return connected;
}

void SignalBind::disconnect()
{
	if (impl)
	{
		FB_ZONE("SignalBind::disconnect");

		if (impl->signalBase)
		{
			impl->signalBase->disconnect(*this);
			/* SignalBase must NULL impl->signalBase */
			fb_assert(impl->signalBase == NULL);
		}

		if ((--impl->refcount) == 0)
			delete impl;

		impl = NULL;
	}
}

FB_END_PACKAGE0()

FB_PACKAGE1(lang)

// SignalBase

SignalBase::SignalBase()
{
}

SignalBase::~SignalBase()
{
	clear();

#if FB_BUILD != FB_FINAL_RELEASE
	// detect deleted signals
	numConnects = 1111111;
	numDisconnects = 1111111;
#endif
}

#if FB_BUILD != FB_FINAL_RELEASE
void SignalBase::validate()
{
	// signal was deleted
	fb_assert(numConnects < 1111111 && numDisconnects < 1111111);
}
#endif

bool SignalBase::isEmpty() const
{
	return delegates.isEmpty();
}

SizeType SignalBase::getDelegateCount() const
{
	return delegates.getSize();
}

void SignalBase::clear()
{
#if FB_BUILD != FB_FINAL_RELEASE
	validate();
#endif
	for (DelegateInfo &it : delegates)
	{
		SignalBindImpl *impl = it.bindImpl;
		impl->signalBase = NULL;
	}
	delegates.clear();

	// Signal deletion for active iterations
	for (uint32_t *i: iterationStack)
	{
		*i = 0x7ffffff;
	}
	iterationStack.clear();
}

#if FB_BUILD != FB_FINAL_RELEASE
static const SizeType infiniteLoopDetectionLimit = 1000000;
#endif

SignalBind SignalBase::connect(const DelegateCallData &d)
{
	FB_ZONE("SignalBase::connect");

#if FB_BUILD != FB_FINAL_RELEASE
	validate();
#endif
	fb_assert(d);

	SignalBindImpl *bindImpl = new SignalBindImpl();
	bindImpl->signalBase = this;
	bindImpl->refcount = 1;

	delegates.pushBack(DelegateInfo(d, bindImpl));
	
#if FB_BUILD != FB_FINAL_RELEASE
	if (!iterationStack.isEmpty())
	{
		numConnects++;
		// looks like you're repeatedly connecting and disconnecting something while the signal is being called!
		fb_assert(numConnects < infiniteLoopDetectionLimit || numDisconnects < infiniteLoopDetectionLimit);
	}
#endif

	return SignalBind(bindImpl);
}

void SignalBase::disconnect(SignalBind &bind)
{
#if FB_BUILD != FB_FINAL_RELEASE
	validate();
#endif
	SignalBindImpl *impl = bind.getImpl();
#if FB_BUILD != FB_FINAL_RELEASE
	bool foundBindImpl = false;
#endif
	for (uint32_t i = 0, iend = (uint32_t) delegates.getSize(); i != iend; ++i)
	{
		if (delegates[i].bindImpl == impl)
		{
			// If there are active iterations going on, make sure they don't miss a signal as one disappears.
			// Happens surprisingly often as delegate can disconnect itself when called by signal.
			for (uint32_t *j: iterationStack)
			{
				if (*j >= i)
					--(*j);
			}

			delegates.swapOutIndex(i);
			impl->signalBase = NULL;
#if FB_BUILD != FB_FINAL_RELEASE
			foundBindImpl = true;
#endif
			break;
		}
	}
#if FB_BUILD != FB_FINAL_RELEASE
	fb_assert(foundBindImpl && "BindImpl not found");
	if (!iterationStack.isEmpty())
	{
		numDisconnects++;
		// looks like you're repeatedly connecting and disconnecting something while the signal is being called!
		fb_assert(numConnects < infiniteLoopDetectionLimit || numDisconnects < infiniteLoopDetectionLimit);
	}
#endif
}

FB_END_PACKAGE1()
