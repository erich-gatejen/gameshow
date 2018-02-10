#include "stdafx.h"
#include "GameDirector.h" 
#include <iostream>
using namespace std;

enum GameDirectorKeyCommands
{
	GDKC_HELP = 'h',
	GDKC_SHOW_IKEYS = 'k',
	GDKC_LOAD_PROGRAM = 'l',
	GDKC_RESET = 'r',
	GDKC_QUIT = 'q'
};

void showHelp()
{
	cout << "GAMESHOW!" << std::endl;
	cout << "key  : action" << std::endl;
	cout << "h    : show help" << std::endl;
	cout << "l    : load program" << std::endl;
	cout << "r    : reset" << std::endl;
	cout << "k    : show input keys" << std::endl;
	cout << "q    : quit" << std::endl;
	cout << std::endl;
}

void showIkeys(InputPinStates	ikeys)
{
	cout << "Input Key States" << std::endl;
	cout << "ID: " << ikeys.id << std::endl;
	cout << "pin #  : on state" << std::endl;
	for (int pin = 0; pin < ikeys.pins.size(); pin++)
	{
		cout << pin << "  : " << std::boolalpha << ikeys.pins[pin] << std::endl;
	}
	cout << std::endl;
}


GameDirector::GameDirector(LEDManager *ledManager, InputManager *inputManager, EventManager *eventManager)
{
	rootContext = new GameContext();
	rootContext->ledManager = ledManager;
	rootContext->inputManager = inputManager;
	rootContext->eventManager = eventManager;
	rootContext->actionManager = new GameActionManager(rootContext->inputManager->checkPins());

	rootContext->eventDispatcher = new GameDirectorEventDispatcher(rootContext);

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE)
	{
		throw "Could not get handle to console stdin.";
	}
}

GameDirector::~GameDirector()
{
	delete rootContext;
}

boolean GameDirector::checkCommand()
{
	INPUT_RECORD irInBuf[256];
	DWORD numberEvents;
	char character;

	GetNumberOfConsoleInputEvents(hStdin, &numberEvents);

	if (numberEvents > 0)
	{
		if (!ReadConsoleInputA(hStdin, irInBuf, 256, &numberEvents))
		{
			throw "Console read error";
		}

		for (DWORD index = 0; index < numberEvents; index++)
		{
			if ((irInBuf[index].EventType == KEY_EVENT) && (irInBuf[index].Event.KeyEvent.bKeyDown))
			{
				character = irInBuf[index].Event.KeyEvent.uChar.AsciiChar;

				// Ignore the rest.  Let them die.
				break;
			}
		}

		switch (character)
		{
		case GDKC_HELP:
			showHelp();
			break;

		case GDKC_SHOW_IKEYS:
			showIkeys(rootContext->inputManager->checkPins());
			break;

		case GDKC_LOAD_PROGRAM:
			loadProgram();
			break;
			
		case GDKC_RESET:
			reset();
			break;

		case GDKC_QUIT:
			return false;
		}

	}

	return true;
}

void GameDirector::run()
{
	try
	{
		running = true;
		while (running)
		{
			running = checkCommand();
			Sleep(200);
		}
	}
	catch (const char* msg) {
		cerr << "Spurious exception.  Quiting." << std::endl;
		cerr << msg << endl;
	}
	catch (const std::exception& e)
	{
		cerr << "Subsystem failed.  Quiting." << std::endl;
		cerr << e.what() << std::endl;
	}

}

void tempLoadHelper(GameContext *rootContext, unsigned pin, GameEventType etype, GameActionType type, unsigned target, unsigned char r, unsigned char g, unsigned char b)
{
	rootContext->eventManager->subscribe(pin, etype, rootContext->eventDispatcher);
	rootContext->actionManager->mapPinEvent2Action(pin, etype, type, target, r, g, b);
}

void GameDirector::loadProgram()
{
	// Hardcode
	tempLoadHelper(rootContext, 32, GE_PIN_TRANSITION_ON, GA_LED_SET_LEVELS, 0, 255, 0, 255);
	tempLoadHelper(rootContext, 32, GE_PIN_TRANSITION_OFF, GA_LED_SET_LEVELS, 0, 0, 0, 0);
	tempLoadHelper(rootContext, 33, GE_PIN_TRANSITION_ON, GA_LED_SET_LEVELS, 1, 0, 255, 0);
	tempLoadHelper(rootContext, 33, GE_PIN_TRANSITION_OFF, GA_LED_SET_LEVELS, 1, 0, 0, 0);
}

void GameDirector::reset()
{
	// Zap event subscriptions for changing LEDs.
	rootContext->eventManager->reset();
	rootContext->actionManager->reset();
	rootContext->ledManager->reset();
}

/****************************************************************************
* INPUT MANAGER
*/

GameDirectorEventDispatcher::GameDirectorEventDispatcher(GameContext *context)
{
	this->context = context;
}

GameDirectorEventDispatcher::~GameDirectorEventDispatcher()
{
}

void GameDirectorEventDispatcher::handlePinEvent(int pin, GameEventType etype, bool state)
{
	GameAction ga = context->actionManager->getAction4PinEvent(pin, etype);
	switch (ga.type)
	{
	case GA_NOP:
		break;

	case GA_LED_ON:
		context->ledManager->setRBG(ga.targetId, MAX_LED_LEVEL, MAX_LED_LEVEL, MAX_LED_LEVEL);
		break;

	case GA_LED_SET_LEVELS:
		context->ledManager->setRBG(ga.targetId, ga.rlevel, ga.glevel, ga.blevel);
		break;

	case GA_LED_OFF:
		context->ledManager->setRBG(ga.targetId, MIN_LED_LEVEL, MIN_LED_LEVEL, MIN_LED_LEVEL);
		break;
	}
}
