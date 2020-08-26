#include "Precompiled.h"
#include "InputStream.h"

#include "fb/lang/ByteOrderSwap.h"
#include "fb/math/curve/MultiCurve.h"
#include "fb/math/curve/TCBCurve.h"

FB_PACKAGE1(stream)

void setStreamReadFailed(bool &streamReadFailed, bool value)
{
	streamReadFailed = value;
}

template<typename StreamType, typename KnotType>
static void readTCBCurveKnotImpl(StreamType &strm, KnotType &v)
{
	strm.read(v.point);
	strm.read(v.tEntering);
	strm.read(v.cEntering);
	strm.read(v.bEntering);
	strm.read(v.tLeaving);
	strm.read(v.cLeaving);
	strm.read(v.bLeaving);
	strm.read(v.time);
}

template<typename StreamType>
static void readTCBCurveImpl(StreamType &strm, math::TCBCurve &v)
{
	PodVector<typename math::TCBCurveKnot> knots;
	strm.read(knots);
	v.setKnotsWithoutSorting(lang::move(knots));
}

template<> void InputStream<lang::LittleEndian>::read(math::TCBCurveKnot &v) { readTCBCurveKnotImpl(*this, v); }
template<> void InputStream<lang::LittleEndian>::read(math::TCBCurve &v) { readTCBCurveImpl(*this, v); }
template<> void InputStream<lang::BigEndian>::read(math::TCBCurveKnot &v) { readTCBCurveKnotImpl(*this, v); }
template<> void InputStream<lang::BigEndian>::read(math::TCBCurve &v) { readTCBCurveImpl(*this, v); }

template<typename StreamType>
static void readMultiCurveKnotImpl(StreamType &strm, math::MultiCurveKnot &v)
{
	strm.read(v.point);
	strm.read(v.enterTangent);
	strm.read(v.leaveTangent);
	uint8_t enterType = 0;
	uint8_t leaveType = 0;
	strm.read(enterType);
	strm.read(leaveType);
	v.enterType = static_cast<math::MultiCurveTangentType>(enterType);
	v.leaveType = static_cast<math::MultiCurveTangentType>(leaveType);
}

template<typename StreamType>
static void readMultiCurveImpl(StreamType &strm, math::MultiCurve &v)
{
	PodVector<math::MultiCurveKnot> knots;
	strm.read(knots);
	v.setKnots(lang::move(knots));
}

template<> void InputStream<lang::LittleEndian>::read(math::MultiCurveKnot &v) { readMultiCurveKnotImpl(*this, v); }
template<> void InputStream<lang::LittleEndian>::read(math::MultiCurve &v) { readMultiCurveImpl(*this, v); }
template<> void InputStream<lang::BigEndian>::read(math::MultiCurveKnot &v) { readMultiCurveKnotImpl(*this, v); }
template<> void InputStream<lang::BigEndian>::read(math::MultiCurve &v) { readMultiCurveImpl(*this, v); }

FB_END_PACKAGE1()