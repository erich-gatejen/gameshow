#pragma once

#include "InputManager.h"

enum GameEventType
{
	GE_NOP,
	GE_PIN_TRANSITION,
	GE_PIN_TRANSITION_ON,
	GE_PIN_TRANSITION_OFF,
	GE_META__SIZE				// Must be the last element!  My lazy demands it.
};

class EventHandlerInterface
{

public:

	EventHandlerInterface() {}
	virtual ~EventHandlerInterface() {}
	virtual void handlePinEvent(int pin, GameEventType etype, bool state) = 0;

};

struct EventSubscription
{
	GameEventType etype;
	EventHandlerInterface *handler;

	EventSubscription(GameEventType etype, EventHandlerInterface *handler)
	{
		this->etype = etype;
		this->handler = handler;
	}
};

class EventManager
{
	list<EventSubscription>	*subscriptions;
	InputPinStates			*lastPinStates;

	InputManager	*inputManager;
	bool			alive;
	boost::thread*	pollThread;
	void run();

public:
	EventManager(InputManager *inputManager);
	~EventManager();
	
	void start();
	void stop();
	void reset();
	void subscribe(int pin, GameEventType etype, EventHandlerInterface *handler);
};


