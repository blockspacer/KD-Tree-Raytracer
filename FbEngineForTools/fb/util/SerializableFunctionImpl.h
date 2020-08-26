
template< TEMPLATE_DECLARE >
class CLASSNAME : public fb::util::SerializableFunctionBase
{
public:
	typedef void (Self::*Func)(ARG_DECLARE);

	CLASSNAME(Func func)
		: func(func)
	{
	}

	virtual int getNumParams() const
	{
		return NUM_PARAMS;
	}

	virtual bool call(void *selfPtr, void *strmPtr)
	{
		InputStreamType &strmIn = *(InputStreamType *)strmPtr;
		Self &self = *(Self *)selfPtr;

		ARG_READ;

		if (strmIn.hasStreamReadFailed())
			return false;

		(self.*func)(ARG_PASS);
		return true;
	}

	virtual void replicate(void *inputStream, void *outputStream)
	{
		/* This is to silence unused parameters spam */
		#if NUM_PARAMS != 0
			InputStreamType &strmIn = *(InputStreamType *)inputStream;
			OutputStreamType &strmOut = *(OutputStreamType *)outputStream;
			ARG_READ;
			ARG_WRITE;
		#endif
	}

private:
	Func func;
};

template< TEMPLATE_DECLARE >
static fb::util::SerializableFunctionBase *createSerializableFunction(void (Self::*func)(ARG_DECLARE))
{
	return new CLASSNAME < TEMPLATE_PASS > ( func );
}
