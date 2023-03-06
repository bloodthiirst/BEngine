#pragma once

class Event
{

};

template<typename TClass>
class EventBase : public Event
{
public:
	static size_t GetID ()
	{
		static size_t id = IDProvider<Event>::template GetNewID<TClass> ();

		return id;
	}
};

template<typename TClass>
class IDCache
{
public:
	inline static size_t id;
};

template<typename TBase>
class IDProvider
{
public:
	inline static size_t counter = 0;


public:
	static size_t CurrentCounter ()
	{
		return counter;
	}

	template< typename TClass>
	static size_t GetNewID ()
	{
		IDCache<TClass>::id = counter++;
		return IDCache<TClass>::id;
	}
};

