#pragma once

#include "LEDManager.h"
#include "InputManager.h"
#include "GameEvents.h"
#include "GameAction.h"

class GameDirectorEventDispatcher;

struct GameContext {
	LEDManager			*ledManager;
	InputManager		*inputManager;
	EventManager		*eventManager;

	// Owned by Game Director
	GameActionManager				*actionManager;
	GameDirectorEventDispatcher		*eventDispatcher;
};

class GameDirectorEventDispatcher : public EventHandlerInterface
{
	GameContext		*context;

public:
	GameDirectorEventDispatcher(GameContext *context);
	~GameDirectorEventDispatcher();

	void handlePinEvent(int pin, GameEventType etype, bool state);

};

class GameDirector
{
	boolean			running;
	GameContext		*rootContext;

	HANDLE hStdin;
	boolean checkCommand();
	void	loadProgram();
	void	reset();

public:
	GameDirector(LEDManager *ledManager, InputManager *inputManager, EventManager *eventManager);
	~GameDirector();

	void run();

};

