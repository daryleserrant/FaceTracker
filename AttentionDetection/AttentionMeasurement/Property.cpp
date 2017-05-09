#include "StdAfx.h"
#include "Property.h"

template <class T>
Property<T>::Property(void)
{
}

template <class T>
Property<T>::~Property(void)
{
}

template <class T>
T Property<T>::get()
{
	return value;
}

template <class T>
void Property<T>::set(T v)
{
	value = v;
}