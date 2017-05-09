#pragma once
template <class T>
class Property
{
private:
	T value;
public:
	Property<T>(void) {};
	~Property<T>(void) {};
	T get()
	{
		return value;
	};
	void set(T v)
	{
		value = v;
	};
};

