#pragma once

#include <list>
#include <queue>
#include <boost/thread.hpp>

#include "GameCommon.h"
#include "Communication.h"


#define ARD_NUM_DIGITAL_INPUT_PINS		54

// Hardcode port for now
#define ARD_CONFIG_SERIAL_BAUDRATE		CBR_115200
#define ARD_CONFIG_SERIAL_BITS2FRAME	8
#define ARD_CONFIG_SERIAL_STOPBITS		ONESTOPBIT
#define ARD_CONFIG_SERIAL_PARITY		NOPARITY

#define ARD_FRAME_START					0x66

enum ArduinoUplinkCommandType
{
	ARDCOMMAND_NOP = 0,
	ARDCOMMAND_KEY_STATES = 1,
	ARDCOMMAND_MAGIC = 211
};

struct ArduinoKeyStates
{
	unsigned	id;
	bool		pins[ARD_NUM_DIGITAL_INPUT_PINS] { false };

	ArduinoKeyStates(unsigned id);
};

class ArduinoManager {
	
	//std::queue<Reference> keysQueue;
	std::queue<ArduinoKeyStates *> keysQueue;

	unsigned id;

	boost::thread* serialThread;
	bool	alive;
	Serial*	serial2Arduino;
	void run();

	ArduinoKeyStates	*lastKeyStates;

public:
	ArduinoManager(unsigned id, int port);
	~ArduinoManager();

	ArduinoKeyStates*	checkKeys();

	void start();
	void stop();

};
