#pragma once

#include <list>
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include "GameCommon.h"
#include "Communication.h"


// Hardcode port for now
#define IM_CONFIG_SERIAL_BAUDRATE		CBR_115200
#define IM_CONFIG_SERIAL_BITS2FRAME		8
#define IM_CONFIG_SERIAL_STOPBITS		ONESTOPBIT
#define IM_CONFIG_SERIAL_PARITY			NOPARITY


/****************************************************************************
 * INPUT MANAGER
 */

struct InputPinStates
{
	unsigned		id;
	vector<bool>	pins;

	InputPinStates()
	{
		id = -1;
	}
	InputPinStates(unsigned id, int size);
};

void tracing__stateChange(string message, InputPinStates newState);

class InputPinSourceInterface
{

public:
	InputPinStates				lastPinStates;
	std::queue<InputPinStates>	pinStateQueue;

	InputPinSourceInterface() {}
	virtual ~InputPinSourceInterface() {}
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual unsigned numberOfPins() = 0;

};

class InputManager {

	std::vector<InputPinSourceInterface *>		sources;
	InputPinStates								lastPinStates;   // Last aggregate pin state

public:
	InputManager(Configuration *config);
	~InputManager();

	void start();
	void stop();
	InputPinStates	checkPins();
};

/****************************************************************************
* ARDUINO SUPPORT
*/
#define ARD_FRAME_START					0x66
#define ARD_NUM_DIGITAL_INPUT_PINS		54

enum ArduinoUplinkCommandType
{
	ARDCOMMAND_NOP = 0,
	ARDCOMMAND_KEY_STATES = 1,
	ARDCOMMAND_MAGIC = 211
};

class ArduinoInput : public InputPinSourceInterface
{
	unsigned		id;
	bool			alive;
	Serial*			serial2Arduino;
	boost::thread*	arduinoThread;

	void runThread();

public:
	ArduinoInput(unsigned id, unsigned port);
	~ArduinoInput();
	void start();
	void stop();
	unsigned numberOfPins();
};


