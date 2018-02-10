#pragma once

#include <map>
#include "InputManager.h"
#include "GameEvents.h"

enum GameActionType
{
	GA_NOP,
	GA_LED_ON,
	GA_LED_SET_LEVELS,
	GA_LED_OFF
};

struct GameAction
{
	GameActionType type		= GA_NOP;
	unsigned char  rlevel;
	unsigned char  glevel;
	unsigned char  blevel;
	unsigned	   targetId;
};

class GameActionManager
{

	unsigned numberOfPins;
	std::vector<std::vector<GameAction>> pinEvents2Action;

public:

	GameActionManager(InputPinStates pins);
	~GameActionManager();
	void reset();

	void mapPinEvent2Action(unsigned pin, GameEventType etype, GameActionType type, unsigned targetId, unsigned char  level);
	void mapPinEvent2Action(unsigned pin, GameEventType etype, GameActionType type, unsigned targetId, unsigned char  rlevel, unsigned char  glevel, unsigned char  blevel);
	GameAction getAction4PinEvent(unsigned pin, GameEventType etype);

};
