#pragma once

FB_PACKAGE0()

// just for stronger type handling
class OffsetInBits
{
public:
	explicit OffsetInBits(SizeType value)
		: value(value)
	{
	}
	
	explicit OffsetInBits(int value)
		: value((SizeType)value)
	{
	}
	
	template<class T>
	explicit OffsetInBits(const T &value)
		: value(value.get())
	{
	}

	SizeType get() const { return value; }
	
	OffsetInBits operator+(const OffsetInBits &other) const { return OffsetInBits(value + other.value); }
	OffsetInBits operator-(const OffsetInBits &other) const { return OffsetInBits(value - other.value); }
	
	OffsetInBits operator+(SizeType other) const { return OffsetInBits(value + other); }
	OffsetInBits operator-(SizeType other) const { return OffsetInBits(value - other); }
	
	OffsetInBits operator/(SizeType other) const { return OffsetInBits(value / other); }
	OffsetInBits operator*(SizeType other) const { return OffsetInBits(value * other); }
	
	bool operator==(SizeType other) const { return value == other; }
	bool operator!=(SizeType other) const { return value != other; }
	bool operator<(SizeType other) const { return value < other; }
	bool operator<=(SizeType other) const { return value <= other; }
	bool operator>(SizeType other) const { return value > other; }
	bool operator>=(SizeType other) const { return value >= other; }
	bool operator==(const OffsetInBits &other) const { return value == other.value; }
	bool operator!=(const OffsetInBits &other) const { return value != other.value; }
	bool operator<(const OffsetInBits &other) const { return value < other.value; }
	bool operator<=(const OffsetInBits &other) const { return value <= other.value; }
	bool operator>(const OffsetInBits &other) const { return value > other.value; }
	bool operator>=(const OffsetInBits &other) const { return value >= other.value; }
	friend bool operator<(uint32_t value, const OffsetInBits &other) { return value < other.value; }
	friend bool operator>(uint32_t value, const OffsetInBits &other) { return value > other.value; }
	friend bool operator<=(uint32_t value, const OffsetInBits &other) { return value <= other.value; }
	friend bool operator>=(uint32_t value, const OffsetInBits &other) { return value >= other.value; }
	friend bool operator==(uint32_t value, const OffsetInBits &other) { return value == other.value; }
	friend bool operator!=(uint32_t value, const OffsetInBits &other) { return value != other.value; }
	
	void operator+=(OffsetInBits other) { value += other.value; }
	void operator-=(OffsetInBits other) { value -= other.value; }

private:
	SizeType value = 0;
};

// just for stronger type handling
class SizeInBits
{
public:
	explicit SizeInBits(SizeType value)
		: value(value)
	{
	}
	
	explicit SizeInBits(int value)
		: value((SizeType)value)
	{
	}
	
	template<class T>
	explicit SizeInBits(const T &value)
		: value(value.get())
	{
	}

	SizeType get() const { return value; }
	
	SizeInBits operator+(const SizeInBits &other) const { return SizeInBits(value + other.value); }
	SizeInBits operator-(const SizeInBits &other) const { return SizeInBits(value - other.value); }
	
	SizeInBits operator+(SizeType other) const { return SizeInBits(value + other); }
	SizeInBits operator-(SizeType other) const { return SizeInBits(value - other); }

	SizeInBits operator/(SizeType other) const { return SizeInBits(value / other); }
	SizeInBits operator*(SizeType other) const { return SizeInBits(value * other); }
	
	bool operator==(SizeType other) const { return value == other; }
	bool operator!=(SizeType other) const { return value != other; }
	bool operator<(SizeType other) const { return value < other; }
	bool operator<=(SizeType other) const { return value <= other; }
	bool operator>(SizeType other) const { return value > other; }
	bool operator>=(SizeType other) const { return value >= other; }
	bool operator==(const SizeInBits &other) const { return value == other.value; }
	bool operator!=(const SizeInBits &other) const { return value != other.value; }
	bool operator<(const SizeInBits &other) const { return value < other.value; }
	bool operator<=(const SizeInBits &other) const { return value <= other.value; }
	bool operator>(const SizeInBits &other) const { return value > other.value; }
	bool operator>=(const SizeInBits &other) const { return value >= other.value; }
	friend bool operator<(uint32_t value, const SizeInBits &other) { return value < other.value; }
	friend bool operator>(uint32_t value, const SizeInBits &other) { return value > other.value; }
	friend bool operator<=(uint32_t value, const SizeInBits &other) { return value <= other.value; }
	friend bool operator>=(uint32_t value, const SizeInBits &other) { return value >= other.value; }
	friend bool operator==(uint32_t value, const SizeInBits &other) { return value == other.value; }
	friend bool operator!=(uint32_t value, const SizeInBits &other) { return value != other.value; }
	
	void operator+=(SizeInBits other) { value += other.value; }
	void operator-=(SizeInBits other) { value -= other.value; }

private:
	SizeType value = 0;
};

FB_END_PACKAGE0()
