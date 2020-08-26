#pragma once

FB_PACKAGE1(util)

class SerializableFunctionBase
{
public:
	virtual ~SerializableFunctionBase() { }
	virtual int getNumParams() const = 0;
	virtual bool call(void *self, void *stream) = 0;
	virtual void replicate(void *inputStream, void *outputStream) = 0;
};

template<class T>
class ParameterContainer
{
public:
	T &ref() { return t; }
	T t;
};

template<class T>
class ParameterContainer<const T &>
{
public:
	T &ref() { return t; }
	T t;
};

template<class OutputStreamType>
static void serializeParams(OutputStreamType &strm)
{
}

template<class OutputStreamType, class Param1>
static void serializeParams(OutputStreamType &strm, const Param1 &param1)
{
	strm.write(param1);
}

template<class OutputStreamType, class Param1, class Param2>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2)
{
	strm.write(param1);
	strm.write(param2);
}

template<class OutputStreamType, class Param1, class Param2, class Param3>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
}

template<class OutputStreamType, class Param1, class Param2, class Param3, class Param4>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3, const Param4 &param4)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
	strm.write(param4);
}

template<class OutputStreamType, class Param1, class Param2, class Param3, class Param4, class Param5>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3, const Param4 &param4, const Param5 &param5)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
	strm.write(param4);
	strm.write(param5);
}

template<class OutputStreamType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3, const Param4 &param4, const Param5 &param5, const Param6 &param6)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
	strm.write(param4);
	strm.write(param5);
	strm.write(param6);
}

template<class OutputStreamType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3, const Param4 &param4, const Param5 &param5, const Param6 &param6, const Param7 &param7)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
	strm.write(param4);
	strm.write(param5);
	strm.write(param6);
	strm.write(param7);
}

template<class OutputStreamType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3, const Param4 &param4, const Param5 &param5, const Param6 &param6, const Param7 &param7, const Param8 &param8)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
	strm.write(param4);
	strm.write(param5);
	strm.write(param6);
	strm.write(param7);
	strm.write(param8);
}

template<class OutputStreamType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9>
static void serializeParams(OutputStreamType &strm, const Param1 &param1, const Param2 &param2, const Param3 &param3, const Param4 &param4, const Param5 &param5, const Param6 &param6, const Param7 &param7, const Param8 &param8, const Param9 &param9)
{
	strm.write(param1);
	strm.write(param2);
	strm.write(param3);
	strm.write(param4);
	strm.write(param5);
	strm.write(param6);
	strm.write(param7);
	strm.write(param8);
	strm.write(param9);
}


#define ARG_READ_IMPL(n) \
	ParameterContainer<Param##n> param##n; strmIn.read(param##n.ref());

#define ARG_WRITE_IMPL(n) \
	strmOut.write(param##n.ref());

////////////////////////////////////////////////

#define NUM_PARAMS 0
#define CLASSNAME SerializableFunction0
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self
#define ARG_DECLARE 
#define ARG_PASS 
#define ARG_READ
#define ARG_WRITE

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 1
#define CLASSNAME SerializableFunction1
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1
#define ARG_DECLARE Param1 param1
#define ARG_PASS param1.ref()
#define ARG_READ ARG_READ_IMPL(1)
#define ARG_WRITE ARG_WRITE_IMPL(1)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 2
#define CLASSNAME SerializableFunction2
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2
#define ARG_DECLARE Param1 param1, Param2 param2
#define ARG_PASS param1.ref(), param2.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2)
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 3
#define CLASSNAME SerializableFunction3
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3
#define ARG_PASS param1.ref(), param2.ref(), param3.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3)
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 4
#define CLASSNAME SerializableFunction4
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3, class Param4
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3, Param4
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3, Param4 param4
#define ARG_PASS param1.ref(), param2.ref(), param3.ref(), param4.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3); ARG_READ_IMPL(4)
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3); ARG_WRITE_IMPL(4)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 5
#define CLASSNAME SerializableFunction5
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3, class Param4, class Param5
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3, Param4, Param5
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3, Param4 param4, Param5 param5
#define ARG_PASS param1.ref(), param2.ref(), param3.ref(), param4.ref(), param5.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3); ARG_READ_IMPL(4); ARG_READ_IMPL(5)
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3); ARG_WRITE_IMPL(4); ARG_WRITE_IMPL(5)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 6
#define CLASSNAME SerializableFunction6
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3, Param4, Param5, Param6
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3, Param4 param4, Param5 param5, Param6 param6
#define ARG_PASS param1.ref(), param2.ref(), param3.ref(), param4.ref(), param5.ref(), param6.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3); ARG_READ_IMPL(4); ARG_READ_IMPL(5); ARG_READ_IMPL(6)
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3); ARG_WRITE_IMPL(4); ARG_WRITE_IMPL(5); ARG_WRITE_IMPL(6)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 7
#define CLASSNAME SerializableFunction7
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3, Param4, Param5, Param6, Param7
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3, Param4 param4, Param5 param5, Param6 param6, Param7 param7
#define ARG_PASS param1.ref(), param2.ref(), param3.ref(), param4.ref(), param5.ref(), param6.ref(), param7.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3); ARG_READ_IMPL(4); ARG_READ_IMPL(5); ARG_READ_IMPL(6); ARG_READ_IMPL(7)
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3); ARG_WRITE_IMPL(4); ARG_WRITE_IMPL(5); ARG_WRITE_IMPL(6); ARG_WRITE_IMPL(7)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 8
#define CLASSNAME SerializableFunction8
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3, Param4 param4, Param5 param5, Param6 param6, Param7 param7, Param8 param8
#define ARG_PASS param1.ref(), param2.ref(), param3.ref(), param4.ref(), param5.ref(), param6.ref(), param7.ref(), param8.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3); ARG_READ_IMPL(4); ARG_READ_IMPL(5); ARG_READ_IMPL(6); ARG_READ_IMPL(7); ARG_READ_IMPL(8);
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3); ARG_WRITE_IMPL(4); ARG_WRITE_IMPL(5); ARG_WRITE_IMPL(6); ARG_WRITE_IMPL(7); ARG_WRITE_IMPL(8)

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#define NUM_PARAMS 9
#define CLASSNAME SerializableFunction9
#define TEMPLATE_DECLARE class InputStreamType, class OutputStreamType, class Self, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9
#define TEMPLATE_PASS InputStreamType, OutputStreamType, Self, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9
#define ARG_DECLARE Param1 param1, Param2 param2, Param3 param3, Param4 param4, Param5 param5, Param6 param6, Param7 param7, Param8 param8, Param9 param9
#define ARG_PASS param1.ref(), param2.ref(), param3.ref(), param4.ref(), param5.ref(), param6.ref(), param7.ref(), param8.ref(), param9.ref()
#define ARG_READ ARG_READ_IMPL(1); ARG_READ_IMPL(2); ARG_READ_IMPL(3); ARG_READ_IMPL(4); ARG_READ_IMPL(5); ARG_READ_IMPL(6); ARG_READ_IMPL(7); ARG_READ_IMPL(8); ARG_READ_IMPL(9);
#define ARG_WRITE ARG_WRITE_IMPL(1); ARG_WRITE_IMPL(2); ARG_WRITE_IMPL(3); ARG_WRITE_IMPL(4); ARG_WRITE_IMPL(5); ARG_WRITE_IMPL(6); ARG_WRITE_IMPL(7); ARG_WRITE_IMPL(8); ARG_WRITE_IMPL(9);

#include "SerializableFunctionImpl.h"
#undef NUM_PARAMS
#undef CLASSNAME
#undef TEMPLATE_DECLARE
#undef TEMPLATE_PASS
#undef ARG_DECLARE
#undef ARG_PASS
#undef ARG_READ
#undef ARG_WRITE

////////////////////////////////////////////////

#undef ARG_READ_IMPL
#undef ARG_WRITE_IMPL


FB_END_PACKAGE1()
