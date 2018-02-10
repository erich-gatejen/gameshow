#include "stdafx.h"
#include "InputManager.h"
#include "GameEvents.h"
#include <iostream>
using namespace std;

EventManager::EventManager(InputManager *inputManager)
{
	this->inputManager = inputManager;
}

EventManager::~EventManager()
{
	stop();
}

void EventManager::start()
{
	lastPinStates = inputManager->checkPins();
	subscriptions.resize(lastPinStates.());

	alive = true;
	pollThread = new boost::thread(&EventManager::run, this);
}

void EventManager::stop()
{
	if (alive)
	{
		alive = false;
		pollThread->join();
	}

	if (pollThread != nullptr)
	{
		delete pollThread;
		pollThread = nullptr;
	}
}

void EventManager::reset()
{
	subscriptions.resize(0);
	subscriptions.resize(lastPinStates.pins.size());
}

void EventManager::subscribe(int pin, GameEventType etype, EventHandlerInterface *handler)
{
	if (pin >= lastPinStates.pins.size()) throw "BUG: pin out of range.";

	subscriptions[pin].emplace_back(EventSubscription(etype, handler));
}

void EventManager::run()
{
	try
	{
		// I wanted a purely event based system, but I have four days to get this ready so no time for that now.  
		// Poll at the debounce frequency.
		while (alive)
		{
			Sleep(20);

			InputPinStates	newPinStates = inputManager->checkPins();

			for (int index = 0; index < newPinStates.pins.size(); index++)
			{
				if (newPinStates.pins[index] != lastPinStates.pins[index])
				{
					// Transition
					for (auto const& subscription : subscriptions[index]) {

						switch (subscription.etype)
						{
						case GE_NOP:
							if (debugging())
							{
								cout << "Pin #" << index << " transtioned to " << newPinStates.pins[index] << std::endl;
							}
							break;

						case GE_PIN_TRANSITION:
							// Any transition
							subscription.handler->handlePinEvent(index, subscription.etype, newPinStates.pins[index]);
							break;

						case GE_PIN_TRANSITION_ON:
							if ((lastPinStates.pins[index] == false) && (newPinStates.pins[index] == true))
							{
								subscription.handler->handlePinEvent(index, subscription.etype, newPinStates.pins[index]);
							}
							break;

						case GE_PIN_TRANSITION_OFF:
							if ((lastPinStates.pins[index] == true) && (newPinStates.pins[index] == false))
							{
								subscription.handler->handlePinEvent(index, subscription.etype, newPinStates.pins[index]);
							}
							break;

						}

					} // end if for subscriptions
				}

			}  // end if while

			lastPinStates = newPinStates;

		} // end while alive

	}
	catch (const char* msg) {
		cerr << "Fatal error in event subsystem." << std::endl;
		cerr << msg << endl;

	}
	catch (const std::exception& e)
	{
		cerr << "Fatal error in event subsystem." << std::endl;
		cerr << e.what() << std::endl;
	}

	alive = false;
}
