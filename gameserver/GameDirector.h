#pragma once

#include "LEDManager.h"
#include "Arduino.h"

struct GameContext {
	LEDManager *ledManager;
	ArduinoManager *arduinoManager;
};

class GameDirector
{
	boolean			running;
	GameContext		*rootContext;

public:
	GameDirector(LEDManager *ledManager, ArduinoManager *arduinoManager);
	~GameDirector();

	void run();

};
