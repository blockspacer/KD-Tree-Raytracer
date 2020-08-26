#pragma once

FB_PACKAGE0()

template <typename FirstT, typename SecondT>
struct Pair
{
	typedef FirstT FirstType;
	typedef SecondT SecondType;

	FirstT first;
	SecondT second;

	Pair()
		: first()
		, second()
	{
		// nop
	}

	Pair(const FirstT& p1, const SecondT& p2)
		: first(p1)
		, second(p2)
	{
		// nop
	}

	bool operator == (const FirstT &other) const
	{
		return (this->first == other);
	}

	bool operator== (const Pair<FirstT, SecondT> &other) const
	{
		return (this->first == other.first && this->second == other.second);
	}

	bool operator!= (const Pair<FirstT, SecondT> &other) const
	{
		return !(this->first == other.first && this->second == other.second);
	}

	bool operator< (const Pair<FirstT, SecondT> &other) const
	{
		return (this->first < other.first || (!(other.first < this->first) && this->second < other.second));
	}
};

template <typename FirstT, typename SecondT>
static Pair<FirstT, SecondT> makePair(const FirstT &p1, const SecondT &p2)
{
	return Pair<FirstT, SecondT>(p1, p2);
}

FB_END_PACKAGE0()
