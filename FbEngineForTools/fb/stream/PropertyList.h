#pragma once

#include "fb/lang/Cstrlen.h"
#include "fb/lang/IsEnum.h"
#include "fb/lang/IsClass.h"
#include "fb/lang/EnableIf.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/time/Time.h"
#include "fb/lang/UnderlyingType.h"
#include "fb/math/DeclareTypes.h"
#include "fb/container/LinearHashMap.h"
#include "fb/container/LinearHashSet.h"
#include "fb/stream/InputStream2Decl.h"
#include "fb/stream/InputStream2Macro.h"
#include "fb/stream/OutputStream2Decl.h"
#include "fb/stream/OutputStream2Macro.h"
#include "fb/stream/PropertyListMacros.h"
#include "fb/stream/StreamMacro.h"
#include "fb/stream/VariableSizeU32.h"
#include "fb/stream/VariableSizeVector.h"
#include "fb/stream/BitInputStream.h"
#include "fb/stream/BitOutputStream.h"

FB_PACKAGE0()

// Used for safe serialization of values in streams
//
// Overhead is typically a 5 byte header for the list and a 3 byte header for each property
//
// Usage:
/*

struct MyStruct
{
	float f;
	float u;
	Something foo;
	
	static const ClassId getStaticClassId() { return FB_FOURCC('M','y','s',t'); }

	template<class T>
	bool stream(T &strm)
	{
		fb_beginPropertyList(T, strm, props);
		props.add(f, "f");
		props.add(u, "u");

		// refactoring example
		int old;
		if (props.refactor(old, "oldName"))
		{
			f = (float)old;
		}

		// stream through manager example
		props.add(this, foo, "foo");

		// stream whatever-you-want example
		fb_beginProperty(T, props, "CustomData", CustomDataClassId);
		{
			fb_stream(strm, something1);
			fb_stream(strm, something2);
		}
		fb_endProperty();
		
		// refactor whatever-you-want example
		fb_beginRefactoredProperty(T, props, "CustomData", OldCustomDataClassId);
		{
			fb_stream(strm, somethingOld);
			somethingNew = somethingOld;
		}
		fb_endProperty();

		fb_endPropertyList();
		return true;
	}
	
	// stream through manager example
	template<class T>
	bool stream(T &strm, Something &foo)
	{
		fb_beginPropertyList(T, strm, props);
		props.add(foo.value, "value");
		fb_endPropertyList();
		return true;
	}
};

*/

namespace propertylist
{
	static uint32_t getTypeIndex(const float &f) { return 0; }
	static uint32_t getTypeIndex(const double &f) { return 1; }
	static uint32_t getTypeIndex(const uint8_t &i) { return 2; }
	static uint32_t getTypeIndex(const uint16_t &i) { return 3; }
	static uint32_t getTypeIndex(const uint32_t &i) { return 4; }
	static uint32_t getTypeIndex(const uint64_t &i) { return 5; }
	static uint32_t getTypeIndex(const int8_t &i) { return 6; }
	static uint32_t getTypeIndex(const int16_t &i) { return 7; }
	static uint32_t getTypeIndex(const int32_t &i) { return 8; }
	static uint32_t getTypeIndex(const int64_t &i) { return 9; }
	static uint32_t getTypeIndex(const math::VC2 &i) { return 10; }
	static uint32_t getTypeIndex(const math::VC3 &i) { return 11; }
	static uint32_t getTypeIndex(const math::VC4 &i) { return 12; }
	static uint32_t getTypeIndex(const math::QUAT &i) { return 13; }
	static uint32_t getTypeIndex(const math::COL &i) { return 16; }
	static uint32_t getTypeIndex(const bool &f) { return 17; }
	static uint32_t getTypeIndex(const math::Bounds &i) { return 18; }
	static uint32_t getTypeIndex(const HeapString &i) { return 19; }
	static uint32_t getTypeIndex(const DynamicString &i) { return 20; }
	static uint32_t getTypeIndex(const StaticString &i) { return 21; }
	static uint32_t getTypeIndex(const math::VC2I &i) { return 22; }
	static uint32_t getTypeIndex(const math::VC3I &i) { return 23; }
	static uint32_t getTypeIndex(const Time &i) { return 24; }
	template<typename T>
	static typename lang::EnableIf<lang::IsEnum<T>::value, uint32_t>::type getTypeIndex(const T &i)
	{
		return getTypeIndex((typename lang::UnderlyingType<T>::type&)i);
	}
	template<class T>
	static typename lang::EnableIf<lang::IsClass<T>::value, uint32_t>::type getTypeIndex(const T &i)
	{
		return (uint32_t)T::getStaticClassId();
	}

	template<class T>
	static uint32_t getTypeIndex(const PodVector<T> &i) { return 100 + getTypeIndex(*i.getPointer()); }
	template<class T, int I>
	static uint32_t getTypeIndex(const CachePodVector<T, I> &i) { return 100 + getTypeIndex(*i.getPointer()); }
	template<class T>
	static uint32_t getTypeIndex(const Vector<T> &i) { return 100 + getTypeIndex(*i.getPointer()); }
	template<class T, int I>
	static uint32_t getTypeIndex(const CacheVector<T, I> &i) { return 100 + getTypeIndex(*i.getPointer()); }
	template<class T>
	static uint32_t getTypeIndex(const LinearHashSet<T> &i) { T *key = nullptr; return 200 + getTypeIndex(*key); }
	template<class Key, class Value>
	static uint32_t getTypeIndex(const LinearHashMap<Key,Value> &i) { Key *key = nullptr; Value *value = nullptr; return 300 + (getTypeIndex(*key) ^ getTypeIndex(*value)); }
	template<class T>
	static uint32_t getTypeIndex(const VariableSizeVector<T> &i) { return 400 + getTypeIndex(*i.getPointer()); }
	
	static HeapString concatTemplateTypeString(const StringRef &type1, const StringRef &type2)
	{
		HeapString str;
		str.reserve(type1.getLength() + type2.getLength() + 2);
		str << type1 << "<" << type2 << ">";
		return str;
	}

	static HeapString concatTemplateTypeString(const StringRef &type1, const StringRef &type2, const StringRef &type3)
	{
		HeapString str;
		str.reserve(type1.getLength() + type2.getLength() + type3.getLength() + 4);
		str << type1 << "<" << type2 << ", " << type3 << ">";
		return str;
	}

	typedef uint16_t PropertyHash;
	enum Constants
	{
		HeaderMagicBit = (1U<<31)
	};
		
	static PropertyHash getPropertyHashForTypeIndex(const char *name, uint32_t typeIndex)
	{
		return (PropertyHash)getHashValue(name, cstrlen(name), typeIndex);
	}
	
	template<class T>
	static PropertyHash getPropertyHash(const T &t, const char *name)
	{
		return getPropertyHashForTypeIndex(name, getTypeIndex(t));
	}
}


template<class StreamType> class PropertyList;

template<class S>
class OutputPropertyList
{
public:
	typedef S StreamType;

	StreamType &getStream() const { return *strmPointer; }

	bool stream(StreamType &strm)
	{
		strmPointer = &strm;
		startOffset = strm.getPosition();

		// make room for header
		uint32_t propListHeader = (uint32_t)propertylist::HeaderMagicBit;
		fb_stream_write(strm, propListHeader);
		return true;
	}

	~OutputPropertyList()
	{
		// patch header
		StreamType &strm = *strmPointer;
		uint32_t propListHeader = strm.getSize() - (startOffset + sizeof(uint32_t));
		propListHeader |= propertylist::HeaderMagicBit;
		strm.patchValue(startOffset, propListHeader);

		// write properties
		VariableSizeU32 numProperties = properties.getSize();
		fb_stream_write(strm, numProperties);
		for (SizeType i = 0; i < properties.getSize(); i++)
		{
			Property &prop = properties[i];
			fb_stream_write(strm, prop.hash);
			VariableSizeU32 compressedSize(prop.size);
			fb_stream_write(strm, compressedSize);
		}
		fb_assert(!propertyOpen && "beginProperty/endProperty mismatch");
	}
	
	bool beginProperty(const char *name, ClassId classId)
	{
		fb_assert(!propertyOpen && "beginProperty/endProperty mismatch");
		propertyOpen = true;
		preWriteProperty((uint32_t)classId, name);
		return true;
	}

	bool beginRefactoredProperty(const char *name, ClassId classId)
	{
		return false;
	}

	bool endProperty()
	{
		fb_assert(propertyOpen && "beginProperty/endProperty mismatch");
		postWriteProperty();
		propertyOpen = false;
		return true;
	}
	
	template<class T>
	bool add(T &t, const char *name)
	{
		StreamType &strm = *strmPointer;
		preWriteProperty(propertylist::getTypeIndex(t), name);
		fb_stream_write(strm, t);
		postWriteProperty();
		return true;
	}
	
	template<class T, class Manager>
	bool add(Manager *manager, T &t, const char *name)
	{
		StreamType &strm = *strmPointer;
		preWriteProperty(propertylist::getTypeIndex(t), name);
		streamValueWithManager(strm, t, *manager);
		postWriteProperty();
		return true;
	}
	
	template<class T>
	bool refactor(T &t, const char *name)
	{
		return false;
	}
	
	template<class T, class Manager>
	bool refactor(Manager *manager, T &t, const char *name)
	{
		return false;
	}

	bool isReading() const { return false; }
	bool isWriting() const { return true; }
	// Rewriting means property list was read and immediately written again without finalizing in between
	bool isRewriting() const { return rewriting; }

	struct Property
	{
#if FB_BUILD != FB_FINAL_RELEASE
		const char *name;
#endif
		propertylist::PropertyHash hash;
		uint32_t size;
		uint32_t offset;
	};
	const PodVector<Property> &getProperties() const { return properties; }

	void setRewriting(bool enabled) { rewriting = enabled; }

protected:

	void preWriteProperty(uint32_t typeIndex, const char *name)
	{
		StreamType &strm = *strmPointer;
		const propertylist::PropertyHash hash = propertylist::getPropertyHashForTypeIndex(name, typeIndex);
		checkProperty(hash, typeIndex, name);
		Property &prop = properties.pushBack();
#if FB_BUILD != FB_FINAL_RELEASE
		prop.name = name;
#endif
		prop.hash = hash;
		prop.offset = strm.getPosition();
	}

	void postWriteProperty()
	{
		StreamType &strm = *strmPointer;
		properties.getBack().size = strm.getSize() - properties.getBack().offset;
	}

	void checkProperty(propertylist::PropertyHash hash, uint32_t typeIndex, const char *name) const
	{
#if FB_BUILD != FB_FINAL_RELEASE
		for (SizeType i = 0, c = properties.getSize(); i < c; i++)
		{
			if (properties[i].hash == hash)
			{
				fb_assertf(0, "Property '%s' has already been added or there is a hash collision with property '%s', in which case you should be more creative with the name", name, properties[i].name);
				return;
			}
		}
#endif
	}

	StreamType *strmPointer = nullptr;
	uint32_t startOffset = 0;
	CachePodVector<Property, 64> properties;
	bool propertyOpen = false;
	bool rewriting = false;
};

template<class S>
class InputPropertyList
{
public:
	typedef S StreamType;

	StreamType &getStream() const { return *strmPointer; }

	bool stream(StreamType &strm)
	{
		strmPointer = &strm;

		if (strm.getBytesLeft() == 0)
		{
			// no data is the same as no properties
			properties.clear();
		}
		else
		{
			// read header
			uint32_t propListHeader = 0;
			fb_stream_read(strm, propListHeader);
			if (propListHeader & propertylist::HeaderMagicBit)
			{
				uint32_t firstPropertyOffset = strm.getPosition();

				// skip to property table
				uint32_t offset = propListHeader & (~propertylist::HeaderMagicBit);
				fb_stream_setPosition(strm, strm.getPosition() + offset);
				
				// read properties
				VariableSizeU32 numProperties = 0;
				fb_stream_read(strm, numProperties);
				properties.clear();
				uint32_t nextPropertyOffset = firstPropertyOffset;
				for (SizeType i = 0; i < numProperties.get(); i++)
				{
					Property &prop = properties.pushBack();
					fb_stream_read(strm, prop.hash);
					VariableSizeU32 compressedSize;
					fb_stream_read(strm, compressedSize);
					prop.size = compressedSize.get();
					prop.offset = nextPropertyOffset;
					nextPropertyOffset += prop.size;
				}
			}
			else
			{
				// LEGACY header

				// read all properties to table
				uint32_t numProperties = propListHeader;
				if (numProperties >= 100)
				{
					// sanity check
					fb_assert(0 && "Stream is corrupt");
					return false;
				}
				properties.clear();
				for (SizeType i = 0; i < numProperties; i++)
				{
					Property &prop = properties.pushBack();
					uint32_t legacyHash;
					fb_stream_read(strm, legacyHash);
					uint32_t legacySize;
					fb_stream_read(strm, legacySize);
					prop.hash = (propertylist::PropertyHash)legacyHash;
					prop.size = legacySize;
					prop.offset = strm.getPosition();
					fb_stream_setPosition(strm, strm.getPosition() + prop.size);
				}
			}
		}

		endPos = strm.getPosition();
		return true;
	}

	~InputPropertyList()
	{
		fb_assert(currentProperty == nullptr && "beginProperty/endProperty mismatch");
	}

	bool beginProperty(const char *name, ClassId classId)
	{
		fb_assert(currentProperty == nullptr && "beginProperty/endProperty mismatch");
		StreamType &strm = *strmPointer;
		propertylist::PropertyHash hash = propertylist::getPropertyHashForTypeIndex(name, (uint32_t)classId);
		currentProperty = findNextProperty(hash, name);
		if (currentProperty)
		{
			fb_stream_setPosition(strm, currentProperty->offset);
			return true;
		}
		return false;
	}

	bool beginRefactoredProperty(const char *name, ClassId classId)
	{
		return beginProperty(name, classId);
	}

	bool skipProperty()
	{
		fb_assert(currentProperty != nullptr && "skipProperty must be called between beginProperty and endProperty");
		StreamType &strm = *strmPointer;
		fb_stream_setPosition(strm, currentProperty->offset + currentProperty->size);
		return true;
	}

	bool endProperty()
	{
		fb_assert(currentProperty != nullptr && "beginProperty/endProperty mismatch");
		StreamType &strm = *strmPointer;
		const SizeType finalPos = strm.getPosition();
		const SizeType expectedFinalPos = currentProperty->offset + currentProperty->size;
		fb_stream_setPosition(strm, endPos);
		currentProperty = nullptr;
		// check that read the exact correct amount of data
		fb_stream_check(finalPos == expectedFinalPos);
		return true;
	}
	
	template<class T>
	bool add(T &t, const char *name)
	{
		StreamType &strm = *strmPointer;
		propertylist::PropertyHash hash = propertylist::getPropertyHash(t, name);
		const Property *prop = findNextProperty(hash, name);
		if (!readProperty(prop, t))
			return false;
		fb_stream_setPosition(strm, endPos);
		return true;
	}
	
	template<class T, class Manager>
	bool add(Manager *manager, T &t, const char *name)
	{
		StreamType &strm = *strmPointer;
		propertylist::PropertyHash hash = propertylist::getPropertyHash(t, name);
		const Property *prop = findNextProperty(hash, name);
		if (!readPropertyWithManager(prop, t, *manager))
			return false;
		fb_stream_setPosition(strm, endPos);
		return true;
	}
		
	template<class T>
	bool refactor(T &t, const char *name)
	{
		return add(t, name);
	}
	
	template<class T, class Manager>
	bool refactor(Manager *manager, T &t, const char *name)
	{
		return add(manager, t, name);
	}
	
	bool isReading() const { return true; }
	bool isWriting() const { return false; }
	bool isRewriting() const { return false; }

	struct Property
	{
		propertylist::PropertyHash hash;
		uint32_t size;
		uint32_t offset;
		const char *name = nullptr;
	};
	const PodVector<Property> &getProperties() const { return properties; }

protected:
	StreamType *strmPointer = nullptr;
	
	template<class T>
	bool readProperty(const Property *prop, T &t)
	{
		if (!prop)
			return false;
		StreamType &strm = *strmPointer;
		fb_stream_setPosition(strm, prop->offset);
		fb_stream_read(strm, t);
		// check that read the exact correct amount of data
		fb_stream_check(strm.getPosition() == prop->offset + prop->size);
		return true;
	}
		
	template<class T, class Manager>
	bool readPropertyWithManager(const Property *prop, T &t, Manager &manager)
	{
		if (!prop)
			return false;
		StreamType &strm = *strmPointer;
		fb_stream_setPosition(strm, prop->offset);
		fb_stream_check(streamValueWithManager(strm, t, manager));
		// check that read the exact correct amount of data
		fb_stream_check(strm.getPosition() == prop->offset + prop->size);
		return true;
	}
	
	const Property *findNextProperty(propertylist::PropertyHash hash, const char *name)
	{
		for (SizeType i = numReadProperties; i < properties.getSize(); i++)
		{
			if (properties[i].hash == hash)
			{
				properties[i].name = name;
				// optimization when property tables match exactly
				if (i == numReadProperties)
					numReadProperties++;
				return &properties[i];
			}
		}
		return nullptr;
	}

	PodVector<Property> properties;
	SizeType numReadProperties = 0;
	SizeType endPos = 0;
	const Property *currentProperty = nullptr;
};

template<class S>
class PropertyList< BitOutputStream<S> >
{
public:
	PropertyList()
	{
		// constructor
		new (implData)Impl();
	}

	OutputStream2<S> &getStream() const { return wrapperStream; }

	bool stream(BitOutputStream<S> &strm)
	{
		strmPointer = &strm;
		getImpl().stream(wrapperStream);
		return true;
	}

	~PropertyList()
	{
		// finalize impl
		getImpl().~Impl();

		// write data
		VariableSizeU32 dataSize = wrapperStream.getSize();
		fb_stream_write((*strmPointer), dataSize);
		fb_stream_write((*strmPointer), wrapperStream.getData(), wrapperStream.getSize());
	}

	bool beginProperty(const char *name, ClassId classId)
	{
		return getImpl().beginProperty(name, classId);
	}

	bool beginRefactoredProperty(const char *name, ClassId classId)
	{
		return getImpl().beginRefactoredProperty(name, classId);
	}

	bool endProperty()
	{
		return getImpl().endProperty();
	}

	template<class T>
	bool add(T &t, const char *name)
	{
		return getImpl().add(t, name);
	}

	template<class T, class Manager>
	bool add(Manager *manager, T &t, const char *name)
	{
		return getImpl().add(manager, t, name);
	}

	template<class T>
	bool refactor(T &t, const char *name)
	{
		return getImpl().refactor(t, name);
	}

	template<class T, class Manager>
	bool refactor(Manager *manager, T &t, const char *name)
	{
		return getImpl().refactor(manager, t, name);
	}

	bool isReading() const { return false; }
	bool isWriting() const { return true; }
	bool isRewriting() const { return false; }

private:
	typedef BitOutputStream<S> StreamType;
	typedef PropertyList<OutputStream2<S>> Impl;
	OutputStream2<S> wrapperStream;
	uint8_t implData[sizeof(Impl)];
	StreamType *strmPointer = nullptr;

	Impl &getImpl() { return *(Impl *)implData; }
};


template<class S>
class PropertyList< BitInputStream<S> >
{
public:
	InputStream2<S> &getStream() const { return wrapperStream; }

	bool stream(BitInputStream<S> &strm)
	{
		VariableSizeU32 dataSize = 0;
		fb_stream_read(strm, dataSize);
		fb_stream_check(strm.getBytesLeft() >= dataSize.get());
		wrapperStreamData.uninitialisedResize(dataSize.get());
		fb_stream_read(strm, wrapperStreamData.getPointer(), wrapperStreamData.getSize());
		wrapperStream = InputStream2<>(wrapperStreamData.getPointer(), wrapperStreamData.getSize());
		return impl.stream(wrapperStream);
	}

	bool beginProperty(const char *name, ClassId classId)
	{
		return impl.beginProperty(name, classId);
	}

	bool beginRefactoredProperty(const char *name, ClassId classId)
	{
		return impl.beginRefactoredProperty(name, classId);
	}

	bool skipProperty()
	{
		return impl.skipProperty();
	}

	bool endProperty()
	{
		return impl.endProperty();
	}

	template<class T>
	bool add(T &t, const char *name)
	{
		return impl.add(t, name);
	}

	template<class T, class Manager>
	bool add(Manager *manager, T &t, const char *name)
	{
		return impl.add(manager, t, name);
	}

	template<class T>
	bool refactor(T &t, const char *name)
	{
		return impl.refactor(t, name);
	}

	template<class T, class Manager>
	bool refactor(Manager *manager, T &t, const char *name)
	{
		return impl.refactor(manager, t, name);
	}

	bool isReading() const { return true; }
	bool isWriting() const { return false; }
	bool isRewriting() const { return false; }

private:
	typedef BitInputStream<S> StreamType;
	typedef PropertyList<InputStream2<S>> Impl;
	PodVector<uint8_t> wrapperStreamData;
	InputStream2<S> wrapperStream;
	Impl impl;
};

template<class S>
class PropertyList< OutputStream2<S> > : public OutputPropertyList< OutputStream2<S> >
{
public:
};

template<class S>
class PropertyList< InputStream2<S> > : public InputPropertyList< InputStream2<S> >
{
public:
};

template<class T>
struct ScopedPropertyListProperty
{
	ScopedPropertyListProperty(PropertyList<T> &props)
		: props(props)
	{
	}
	~ScopedPropertyListProperty()
	{
		props.endProperty();
	}
	PropertyList<T> &props;
};

template<class T>
struct WritePropertyListValue
{
	T &value;
	const char *name;
};

template<class T>
static WritePropertyListValue<T> writePropertyValue(T &value, const char *name)
{
	return WritePropertyListValue<T>{ value, name };
}

template<class T>
static void unwindWritePropertyListValue(PropertyList<OutputStream2<>> &propsList, WritePropertyListValue<T> &prop)
{
	propsList.add(prop.value, prop.name);
}

template<class T, typename... Property>
static void unwindWritePropertyListValue(PropertyList<OutputStream2<>> &propsList, WritePropertyListValue<T> &prop, Property... p1)
{
	propsList.add(prop.value, prop.name);
	unwindWritePropertyListValue(propsList, p1...);
}

template<typename... Property>
static bool writePropertyList(PodVector<uint8_t> &props, Property... p1)
{
	OutputStream2<> strm;
	strm.swapDataVector(props);
	fb_beginPropertyList(OutputStream2<>, strm, propsList);
	unwindWritePropertyListValue(propsList, p1...);
	fb_endPropertyList();
	strm.swapDataVector(props);
	return true;
}

FB_END_PACKAGE0()
