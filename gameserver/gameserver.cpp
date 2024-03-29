// gameserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "PacDrive.h"
#include "GameCommon.h"
#include "LEDManager.h"
#include "InputManager.h"
#include "GameEvents.h"
#include "GameDirector.h"

#include <iostream>
using namespace std;

#define ARDUINO_PORT	6

int destruct(void *object)
{
	int result = 0;
	try
	{
		delete object;
	}
	catch (const char* msg) {
		cerr << "Spurious exception during shutdown" << std::endl;
		cerr << msg << endl;
		result = 1;
	}
	catch (const std::exception& e)
	{
		cerr << "Subsystem failure duing shitdown" << std::endl;
		cerr << e.what() << std::endl;
		result = 1;
	}
	return result;
}

int main(int argc, char* argv[])
{

	Configuration config;
	LEDManager *ledManager = nullptr;
	InputManager *inputManager = nullptr;
	EventManager *eventManager = nullptr;
	GameDirector *gameDirector = nullptr;

	int returnValue = 0;

	try
	{
		ledManager = new LEDManager();

		inputManager = new InputManager(&config);
		inputManager->start();

		eventManager = new EventManager(inputManager);
		eventManager->start();

		gameDirector = new GameDirector(ledManager, inputManager, eventManager);
		gameDirector->run();

	}
	catch (const char* msg) {
		cerr << msg << endl;
		returnValue = ERROR_CODE_SPURIOUS_EXCEPTION;
	}
	catch (const std::exception& e)
	{
		cerr << "Fatal error" << std::endl;
		cerr << e.what() << std::endl;
		returnValue = ERROR_CODE_SUBSYSTEM_FAILURE;
	}

	int shutdownResult = destruct(gameDirector);
	shutdownResult += destruct(inputManager);
	shutdownResult += destruct(ledManager);
	shutdownResult += destruct(eventManager);
	if (shutdownResult != 0) returnValue = ERROR_CODE_SHUTDOWN_FAULT;

	return returnValue;
}

