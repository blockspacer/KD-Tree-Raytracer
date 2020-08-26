#include "Precompiled.h"

#if FB_BUILD != FB_FINAL_RELEASE

#include "FBSpam.h"
#include "fb/lang/thread/IThreadEntry.h"
#include "fb/lang/thread/Thread.h"
#include "fb/sys/windows/GetProcessId.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/container/PodVector.h"

FB_PACKAGE1(util)

class FBSpam : public IThreadEntry
{
public:
	FBSpam()
		: thread(0)
	{
		lang::atomicIncRelaxed(running);
		FB_STATIC_CONST_STRING(threadName, "FBSpam");
		thread = Thread::createNewThread(this, threadName);
	}

	~FBSpam()
	{
		lang::atomicDecRelaxed(running);
		Thread::joinThread(thread);
		for (SizeType i = 0; i < messages.getSize(); i++)
			delete[] messages[i];
	}

	void pushMessage(char *message)
	{
		mutex.enter();
		messages.pushBack(message);
		mutex.leave();
	}

private:
	void entry()
	{
		SizeType procId = sys::windows::GetProcessId::getProcessId();
		char filename[256];
		sprintf(filename, "log/%u.log", procId);
		bool append = false;
		while (lang::atomicLoadRelaxed(running))
		{
			Thread::sleep(100);
			
			PodVector<char *> localMessages;
			mutex.enter();
			localMessages.swap(messages);
			mutex.leave();

			if (!localMessages.isEmpty())
			{
				FILE *f = fopen(filename, append ? "at" : "wt");
				append = true;
				if (f)
				{
					for (SizeType i = 0; i < localMessages.getSize(); i++)
						fputs(localMessages[i], f);
					fclose(f);
				}
			
				for (SizeType i = 0; i < localMessages.getSize(); i++)
					delete[] localMessages[i];
			}
		}
	}

	Thread *thread;
	lang::AtomicInt32 running;
	Mutex mutex;
	PodVector<char *> messages;
};

void spamF(const char *fmt, ...)
{
	static FBSpam fbSpam;
	va_list args;
	va_start(args, fmt);

	size_t neededSpace = size_t(_vscprintf(fmt, args) + 1);

	char *tmp = new char[neededSpace];
	tmp[0] = 0;
	tmp[neededSpace - 1] = 0;

	vsprintf_s(tmp, neededSpace, fmt, args);
	va_end(args);
	
	fbSpam.pushMessage(tmp);
}

FB_END_PACKAGE1()

#endif