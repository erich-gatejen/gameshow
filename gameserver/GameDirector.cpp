#include "stdafx.h"
#include "GameDirector.h" 
#include <iostream>
using namespace std;

void showHelp()
{
	cout << "GAMESHOW!" << std::endl;
	cout << "key  : action" << std::endl;
	cout << "h    : show help" << std::endl;
	cout << "k    : show input keys" << std::endl;
	cout << "q    : quit" << std::endl;
	cout << std::endl;
}

void showIkeys(ArduinoKeyStates	*ikeys)
{
	cout << "Input Key States" << std::endl;
	cout << "ID: " << ikeys->id << std::endl;
	cout << "pin #  : on state" << std::endl;
	for (int pin = 0; pin < ARD_NUM_DIGITAL_INPUT_PINS; pin++)
	{
		cout << ikeys->pins[pin] << "  : " << ikeys->pins[pin] << std::endl;
	}
	cout << std::endl;
}


GameDirector::GameDirector(LEDManager *ledManager, ArduinoManager *arduinoManager)
{
	rootContext = new GameContext();
	rootContext->ledManager = ledManager;
	rootContext->arduinoManager = arduinoManager;
}

GameDirector::~GameDirector()
{
	delete rootContext;
}

boolean checkCommand(GameContext *rootContext)
{
	int character = getchar();
	if (character != EOF)
	{
		// Drain stdin so we don't bounce around.
		while (getchar() != EOF);
		switch (character)
		{
		case GDKC_HELP:
			showHelp();
			break;

		case GDKC_SHOW_IKEYS:
			showIkeys(rootContext->arduinoManager->checkKeys());
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
			running = checkCommand(rootContext);
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

enum GameDirectorKeyCommands
{
	GDKC_HELP = 'h',
	GDKC_SHOW_IKEYS = 'k',
	GDKC_QUIT = 'q',
};



