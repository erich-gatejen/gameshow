#include "stdafx.h"
#include <iostream>

#include "InputManager.h"

#define FRAME_BYTE_TIMEOUT_MS		500
#define PACKET_TIMEOUT_MS			3000


InputPinStates::InputPinStates(unsigned id, int size)
{
	this->id = id;
	this->pins.resize(size);
}

/****************************************************************************
* INPUT MANAGER
*/

InputManager::InputManager(Configuration *config)
{
	if (config->inputSources.size() < 1) throw ConfigurationException("No input sources configures.  An InputManager is useless.");

	int expectedId = 1;

	// Input sources
	sources.resize(config->inputSources.size());
	for (auto const& sourceConfig : config->inputSources) {		
		if (expectedId != sourceConfig.id) throw ConfigurationException("Input source ids out of order.  They must start with 1 and increment.");

		switch (sourceConfig.type)
		{
		case IT_ARDUINO:
			sources[expectedId - 1] = (InputPinSourceInterface *) new ArduinoInput(sourceConfig.id, sourceConfig.port);
		}
		expectedId++;
	}

}

InputManager::~InputManager()
{
	stop();
}

InputPinStates	InputManager::checkPins()
{

	// Support a single source for now.
	if (sources.size() > 1)
	{
		// Not supported yet.
	} 
	else
	{
		// New state?  
		// TODO: backlog protection.   Eventually, we need to ditch stuff if we get behind.
		if (!sources[0]->pinStateQueue.empty())
		{
			lastPinStates = sources[0]->pinStateQueue.front();
			sources[0]->pinStateQueue.pop();

			if (tracing())
			{
				tracing__stateChange("new pin state", lastPinStates);
			}

		}
		else
		{
			// TODO needless copy and used for priming.  Find a better way.
			lastPinStates = sources[0]->lastPinStates;
		}

	}

	return lastPinStates;
}

void InputManager::start()
{
	for (auto source : sources) {
		source->start();
	}
	this->checkPins();		// Prime aggregate pin state
}

void InputManager::stop()
{
	for (auto source : sources) {
		source->stop();
	}
}


/****************************************************************************
* ARDUINO INPUT
*/

/**
Over-the-wire formats:

byte 1: ArduinoUplinkCommandType = KEY_STATES
byte 2: key states 0 - 7 (note that 0 and 1 will never be set because these pins are used by the serial controller).
byte 3: key states 8 - 15
byte 4: key states 16 - 31
byte 5: key states 32 - 39
byte 6: key states 40 - 47
byte 7: key states 48 - 55  - 54 and 55 will always be 0
byte 8: key states 56 - 63	- Not used and will always be 0

*/
int ArduinoUplinkPacketSizes[] = { 0, 8, 0, -1, -1, -1, -1, -1,  // WARNING!!!!  WARNING!!!  The largest number in this list must be defined as PACKET_MAX (or it is buffer overrun time).
-1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
};
#define PACKET_MAX 10   
#define ARD_BAD_UPLINK_COMMAND	-1

byte bitMasks[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };

InputPinStates	unpackArduinoKeyStates(unsigned id, byte buffer[8])
{
	InputPinStates states = InputPinStates(id, ARD_NUM_DIGITAL_INPUT_PINS);

	int byteIndex = 0;
	int bitIndex = 0;
	for (int pinIndex = 0; pinIndex < ARD_NUM_DIGITAL_INPUT_PINS; pinIndex++)
	{
		if ((buffer[byteIndex] & bitMasks[bitIndex]) > 0)
		{
			states.pins[pinIndex] = true;
		}
		else
		{
			states.pins[pinIndex] = false;
		}

		bitIndex++;
		if (bitIndex > 7)
		{
			bitIndex = 0;
			byteIndex++;
		}
	}

	return states;
}

ArduinoInput::ArduinoInput(unsigned id, unsigned port)
{
	this->id = id;
	this->lastPinStates = InputPinStates(id, ARD_NUM_DIGITAL_INPUT_PINS);
	serial2Arduino = new Serial(port, IM_CONFIG_SERIAL_BAUDRATE, IM_CONFIG_SERIAL_BITS2FRAME, IM_CONFIG_SERIAL_STOPBITS, IM_CONFIG_SERIAL_PARITY);
	serial2Arduino->drain();
}

ArduinoInput::~ArduinoInput()
{
}

unsigned ArduinoInput::numberOfPins()
{
	return ARD_NUM_DIGITAL_INPUT_PINS;
}

void ArduinoInput::start()
{
	this->pinStateQueue = pinStateQueue;
	alive = true;
	arduinoThread = new boost::thread(&ArduinoInput::runThread, this);

}

void ArduinoInput::stop()
{
	if (alive)
	{
		alive = false;
		arduinoThread->join();
	}

	if (serial2Arduino != nullptr)
	{
		delete serial2Arduino;
		serial2Arduino = nullptr;
	}
}

void ArduinoInput::runThread()
{
	byte	readByte;
	byte	buffer[PACKET_MAX];

	try
	{

		while (alive)
		{
			try
			{
				// Wait for something over the wire.  Occasionally check if we are being killed.
				if (serial2Arduino->readByte(&readByte, FRAME_BYTE_TIMEOUT_MS) > 0)
				{

					// Frame.  This is to cut down on noisy packets, which persistant timing issues cause.
					if (readByte == ARD_FRAME_START)
					{

						if (serial2Arduino->readByte(&readByte, PACKET_TIMEOUT_MS) < 1) throw ComRecoverableException("Timeout waiting on command byte");
						if (readByte == ARD_BAD_UPLINK_COMMAND)
						{
							throw "Bad command from device.  Something is wrong.";
						}

						if (ArduinoUplinkPacketSizes[ARDCOMMAND_KEY_STATES] > 0)
						{
							DWORD readBytes = serial2Arduino->readSerial(buffer, ArduinoUplinkPacketSizes[ARDCOMMAND_KEY_STATES], PACKET_TIMEOUT_MS);

							if (readBytes < ArduinoUplinkPacketSizes[ARDCOMMAND_KEY_STATES])
							{
								cerr << "Bytes read: " << readBytes << " : bytes=";
								print_bytes_stderr(buffer);
								cerr << endl;
								throw ComRecoverableException("Packet ruined.  Dropping.");
							}
						}

						switch (readByte)
						{
						case ARDCOMMAND_KEY_STATES:
							InputPinStates keyStates = unpackArduinoKeyStates(this->id, buffer);
							pinStateQueue.push(keyStates);

							if (debugging())
							{
								cerr << "Key state change. encoded bytes (first 8)= ";
								print_bytes_stderr(buffer);
								cerr << endl;
							}

							break;
						}

					}

				}

			}
			catch (const ComRecoverableException& ce)
			{
				cerr << "Recoverable com error: " << ce.what() << std::endl;
				serial2Arduino->drain();
			}

		} // end while alive

	}
	catch (const char* msg) {
		cerr << "Fatal error to Arduino device connection." << std::endl;
		cerr << msg << endl;

	}
	catch (const std::exception& e)
	{
		cerr << "Fatal error to Arduino device connection." << std::endl;
		cerr << e.what() << std::endl;
	}

	alive = false;
}

/****************************************************************************
* TOOLS
*/

void tracing__stateChange(string message, InputPinStates newState)
{
	cout << "===== TRACE : " << message << " ========================================" << std::endl;
	cout << "ID: " << newState.id << std::endl;
	cout << "pin #  : on state" << std::endl;
	for (int pin = 0; pin < newState.pins.size(); pin++)
	{
		cout << pin << "  : " << std::boolalpha << newState.pins[pin] << std::endl;
	}
	cout << std::endl;
	cout << "===================================================================" << std::endl;
}
