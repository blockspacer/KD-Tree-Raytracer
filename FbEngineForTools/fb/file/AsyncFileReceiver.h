#pragma once

FB_DECLARE(file, File)

FB_PACKAGE1(file)

/**
 * AsyncFileReceiver is an interface for FileManager to get back to you once async file reading operation completes
 */

struct AsyncOperationID
{
	AsyncOperationID() { }
	AsyncOperationID(uint32_t id) 
		: id(id)
	{
	}

	bool operator==(const AsyncOperationID &other) const { return id == other.id; }
	bool operator!=(const AsyncOperationID &other) const { return id != other.id; }
	bool isValid() const { return *this != AsyncOperationID(); }
	void clear() { id = AsyncOperationID().id; }

private:
	uint32_t id = 0xFFFFFFFF;
};

class AsyncFileReceiver
{
public:


	virtual ~AsyncFileReceiver() { }
	virtual void readFinished(const DynamicString &fileName, File &file, AsyncOperationID id) = 0;
};

FB_END_PACKAGE1()