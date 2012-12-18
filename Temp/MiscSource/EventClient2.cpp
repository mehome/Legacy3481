// EventClient.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include "Event.h"

using namespace std;

class HasEvent
{
public:
	void FireEvent(const char* msg, int i, float f){iChanged.Fire(msg, i, f);}
	Event3<const char*, int, float> iChanged;
};

class HearsEvent
{
public:
	HearsEvent(HasEvent& hasEvent, int id) : _hasEvent(hasEvent), _id(id)
	{
		_hasEvent.iChanged.Subscribe(ehl, *this, &HearsEvent::EventCallback);
	}

	void RemoveHandler()
	{
		_hasEvent.iChanged.Remove(*this, &HearsEvent::EventCallback);
	}

	HasEvent& _hasEvent;
	int _id;

private:
	IEvent::HandlerList ehl;

protected:
	virtual void EventCallback(const char* msg, int i, float f)
	{
		cout << "HearsEvent(" << _id << ")::EventCallback(" << msg << ", " << i << ", " << f << ")" << endl;
	}
};

class HearsEvent2 : public HearsEvent
{
public:
	HearsEvent2(HasEvent& hasEvent, int id) : HearsEvent(hasEvent, id) {}

protected:
	virtual void EventCallback(const char* msg, int i, float f)
	{
		cout << "HearsEvent2(" << _id << ")::EventCallback(" << msg << ", " << i << ", " << f << ")" << endl;
		HearsEvent::EventCallback(msg, i, f);
		RemoveHandler();
	}
};


int main(int argc, const char* argv[])
{
	cout << "Event Handling Prototype Client" << endl;
	cout << "===============================" << endl << endl;

	HasEvent has;
	HearsEvent hears(has, 1);
	HearsEvent2 hears4(has, 4);

	{
		HearsEvent hears2(has, 2);

		cout << "main sending  inner" << endl;
		has.FireEvent("inner", -12, 3.1416f);
	}

	cout << "main removing" << endl;
	hears.RemoveHandler();

	HearsEvent hears3(has, 3);

	cout << "main sending" << endl;
	has.FireEvent("outer", 15, -1.111);

	cout << "main complete" << endl;
	return 0;
}

