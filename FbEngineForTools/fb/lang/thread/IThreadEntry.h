#pragma once

FB_DECLARE0(Thread)

FB_PACKAGE0()

class IThreadEntry
{
	friend class Thread;
public:
	IThreadEntry()
		: thread(nullptr)
	{
	}

	virtual ~IThreadEntry() {}
	/* Override this */
	virtual void entry() = 0;
	/* Override this to return true, if you want Thread and IThreadEntry destroyed after entry() has returned */
	virtual bool shouldSelfDestruct() const
	{
		return false;
	}

private:
	/* This is the actual entry point that Thread calls */
	void runner();

	Thread *thread;
};

FB_END_PACKAGE0()
