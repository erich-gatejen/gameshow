#include "stdafx.h"
#include <iostream>
using namespace std;

#include "Arduino.h"

#define COMMAND_BYTE_TIMEOUT_MS		750
#define COMMAND_PACKET_TIMEOUT_MS	100

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
int ArduinoUplinkPacketSizes[] = { 0, 8, 0 };  // WARNING!!!!  WARNING!!!  The largest number in this list must be defined as PACKET_MAX (or it is buffer overrun time).
#define PACKET_MAX 8

byte bitMasks[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };

ArduinoKeyStates	*unpackArduinoKeyStates(byte *buffer)
{
	ArduinoKeyStates *states = new ArduinoKeyStates();

	int pinIndex = 0;
	for (int byteIndex = 0; byteIndex < 8; byteIndex++)
	{
		for (int bitIndex = 0; bitIndex < 8; bitIndex++)
		{

			if ((buffer[byteIndex] & bitMasks[bitIndex]) > 0)
			{
				states->pins[pinIndex] = true;
			}
			else
			{
				states->pins[pinIndex] = false;
			}
		}
	}

	return states;
}

ArduinoManager::ArduinoManager(int port)
{
	serial2Arduino = new Serial(port, ARD_CONFIG_SERIAL_BAUDRATE, ARD_CONFIG_SERIAL_BITS2FRAME, ARD_CONFIG_SERIAL_STOPBITS, ARD_CONFIG_SERIAL_PARITY);

}

ArduinoManager::~ArduinoManager()
{
	stop();
}

ArduinoKeyStates*	ArduinoManager::checkKeys()
{
	if (!alive)
	{
		throw DeviceException("Arduino device connection is dead.");
	}

	ArduinoKeyStates* result = nullptr;
	if (!keysQueue.empty())
	{
		 keysQueue.pop(result);
	}
	return result;
}

void ArduinoManager::start()
{
	alive = true;
	serialThread = new boost::thread(&ArduinoManager::run, this);

}

void ArduinoManager::stop()
{
	if (serial2Arduino != nullptr)
	{
		delete serial2Arduino;
		serial2Arduino = nullptr;
	}

	if (alive)
	{
		alive = false;
		serialThread->join();
	}
}

void ArduinoManager::run() 
{
	byte	readByte;
	byte	buffer[PACKET_MAX];

	try
	{

		while (alive)
		{

			// Wait for something over the wire.  Occasionally check if we are being killed.
			if (serial2Arduino->readByte(&readByte, COMMAND_BYTE_TIMEOUT_MS) > 0)
			{
				if (ArduinoUplinkPacketSizes[ARDCOMMAND_KEY_STATES] > 0)
				{
					if (serial2Arduino->readSerial(buffer, ArduinoUplinkPacketSizes[ARDCOMMAND_KEY_STATES], COMMAND_PACKET_TIMEOUT_MS) < ArduinoUplinkPacketSizes[ARDCOMMAND_KEY_STATES])
					{
						throw "Packet not fully read before timeout. Connection is bad.";
					}
				}

				switch (readByte)
				{
				case ARDCOMMAND_KEY_STATES:
					ArduinoKeyStates  *keyStates = unpackArduinoKeyStates(buffer);
					keysQueue.push(keyStates);
					break;
				}

			}

		} // end while alive

	}
	catch (const char* msg) {
		cerr << msg << endl;
	}
	catch (const std::exception& e)
	{
		cerr << "Fatal error to Arduino device connection." << std::endl;
		cerr << e.what() << std::endl;
	}

}

