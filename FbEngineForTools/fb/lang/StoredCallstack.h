#pragma once

FB_DECLARE0(HeapString)

FB_PACKAGE0()

//
// Example usage:
//    StoredCallstack callstack;
//    callstack.store();
//    callstack.outputOnSingleRow = true;
//    callstack.appendToString(str);
// or
//    FB_LOG_DEBUG(FB_MSG("Callstack: ", &callstack));
//

class StoredCallstack
{
public:
	StoredCallstack() {}

	StoredCallstack(bool storeNow);

	StoredCallstack(StoredCallstack &&other);
	StoredCallstack(const StoredCallstack &) = delete;

	~StoredCallstack();

	bool store();

	void appendToString(HeapString &result) const;

	bool outputOnSingleRow = false;
	bool dontAppendErrors = false;
	bool acceptNormalCallstacks = true;
	bool acceptZoneCallstacks = true;

private:
	struct Data;
	Data *data = nullptr;
};

HeapString &debugAppendToString(HeapString &result, const StoredCallstack *t);

FB_END_PACKAGE0()
