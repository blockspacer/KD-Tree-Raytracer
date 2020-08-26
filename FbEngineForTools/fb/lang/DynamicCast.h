#pragma once

#include "fb/lang/IsConvertible.h"
#include "fb/lang/IsSame.h"

FB_PACKAGE0()

/* Casts base class to derived class or returns NULL. Can be called with a NULL pointer */
template<typename Derived, typename Base>
Derived *dynamicCast(Base *obj)
{
	static_assert(sizeof(Base) && sizeof(Derived), "Class is not defined.");
	static_assert((lang::IsConvertible<const Base*, const Derived*>::value == true) || (lang::IsConvertible<const Derived*, const Base*>::value == true), "You are trying to cast between incompatible classes.");
	static_assert(lang::IsSame<const Base *, const Derived *>::value == false, "Don't use dynamicCast if not actually converting anything.");
	static_assert(lang::IsConvertible<const Base *, const Derived *>::value == false, "Don't use dynamicCast to convert object to its base class.");
	return Derived::template dynamicCastImpl<Derived>(obj);
}

/* Casts base class to derived class or returns NULL. Can be called with a NULL pointer */
template<typename Derived, typename Base>
const Derived *dynamicCast(const Base *obj)
{
	static_assert(sizeof(Base) && sizeof(Derived), "Class is not defined.");
	static_assert((lang::IsConvertible<const Base*, const Derived*>::value == true) || (lang::IsConvertible<const Derived*, const Base*>::value == true), "You are trying to cast between incompatible classes.");
	static_assert(lang::IsSame<const Base *, const Derived *>::value == false, "Don't use dynamicCast if not actually converting anything.");
	static_assert(lang::IsConvertible<const Base *, const Derived *>::value == false, "Don't use dynamicCast to convert object to its base class.");
	return Derived::template dynamicCastImpl<Derived>(obj);
}

/* Casts base class to derived class or returns NULL*/
template<typename Derived, typename Base>
Derived *dynamicCast(Base &obj)
{
	static_assert(sizeof(Base) && sizeof(Derived), "Class is not defined.");
	static_assert((lang::IsConvertible<const Base*, const Derived*>::value == true) || (lang::IsConvertible<const Derived*, const Base*>::value == true), "You are trying to cast between incompatible classes.");
	static_assert(lang::IsSame<const Base *, const Derived *>::value == false, "Don't use dynamicCast if not actually converting anything.");
	static_assert(lang::IsConvertible<const Base *, const Derived *>::value == false, "Don't use dynamicCast to convert object to its base class.");
	return Derived::template dynamicCastImpl<Derived>(obj);
}

/* Casts base class to derived class or returns NULL */
template<typename Derived, typename Base>
const Derived *dynamicCast(const Base &obj)
{
	static_assert(sizeof(Base) && sizeof(Derived), "Class is not defined.");
	static_assert((lang::IsConvertible<const Base*, const Derived*>::value == true) || (lang::IsConvertible<const Derived*, const Base*>::value == true), "You are trying to cast between incompatible classes.");
	static_assert(lang::IsSame<const Base *, const Derived *>::value == false, "Don't use dynamicCast if not actually converting anything.");
	static_assert(lang::IsConvertible<const Base *, const Derived *>::value == false, "Don't use dynamicCast to convert object to its base class.");
	return Derived::template dynamicCastImpl<Derived>(obj);
}

FB_END_PACKAGE0()
