#pragma once

#include <list>
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>

#include "Communication.h"

using namespace boost::lockfree;

#define ARD_NUM_DIGITAL_INPUT_PINS		54

// Hardcode port for now
#define ARD_CONFIG_SERIAL_BAUDRATE		CBR_115200
#define ARD_CONFIG_SERIAL_BITS2FRAME	8
#define ARD_CONFIG_SERIAL_STOPBITS		ONESTOPBIT
#define ARD_CONFIG_SERIAL_PARITY		NOPARITY

enum ArduinoUplinkCommandType
{
	ARDCOMMAND_NOP = 0,
	ARDCOMMAND_KEY_STATES,
	ARDCOMMAND_MAGIC = 59134563
};

struct ArduinoKeyStates
{
	unsigned	id;
	bool		pins[ARD_NUM_DIGITAL_INPUT_PINS];
};

class ArduinoManager {
	
	queue<ArduinoKeyStates*> keysQueue;

	boost::thread* serialThread;
	bool	alive;
	Serial*	serial2Arduino;
	void run();

public:
	ArduinoManager(int port);
	~ArduinoManager();

	ArduinoKeyStates*	checkKeys();

	void start();
	void stop();

};
