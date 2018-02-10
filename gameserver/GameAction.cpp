#include "stdafx.h"

#include "GameAction.h"

GameActionManager::GameActionManager(InputPinStates pins)
{
	numberOfPins = pins.pins.size();
	this->reset();
}

GameActionManager::~GameActionManager()
{
}

void GameActionManager::reset()
{
	pinEvents2Action.resize(this->numberOfPins);
	for (unsigned index = 0; index < this->numberOfPins; index++)
	{
		pinEvents2Action[index] = std::vector<GameAction>(GE_META__SIZE);
	}
}

void GameActionManager::mapPinEvent2Action(unsigned pin, GameEventType etype, GameActionType type, unsigned targetId, unsigned char  level)
{
	mapPinEvent2Action(pin, etype, type, targetId, level, level, level);
}

void GameActionManager::mapPinEvent2Action(unsigned pin, GameEventType etype, GameActionType type, unsigned targetId, unsigned char  rlevel, unsigned char  glevel, unsigned char  blevel)
{
	GameAction ga;
	ga.type = type;
	ga.rlevel = rlevel;
	ga.rlevel = glevel;
	ga.rlevel = blevel;
	ga.targetId = targetId;
	pinEvents2Action[pin][etype] = ga;
}

GameAction GameActionManager::getAction4PinEvent(unsigned pin, GameEventType etype)
{
	return pinEvents2Action[pin][etype];
}
