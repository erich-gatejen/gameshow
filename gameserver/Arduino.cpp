#include "stdafx.h"
#include <iostream>

#include "Arduino.h"

#define FRAME_BYTE_TIMEOUT_MS		500
#define PACKET_TIMEOUT_MS			3000


ArduinoKeyStates::ArduinoKeyStates(unsigned id) 
{
	this->id = id;
}

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

ArduinoKeyStates	*unpackArduinoKeyStates(unsigned id, byte buffer[8])
{
	ArduinoKeyStates *states = new ArduinoKeyStates(id);

	int byteIndex = 0;
	int bitIndex = 0;
	for (int pinIndex = 0; pinIndex < ARD_NUM_DIGITAL_INPUT_PINS; pinIndex++)
	{
		if ((buffer[byteIndex] & bitMasks[bitIndex]) > 0)
		{
			states->pins[pinIndex] = true;
		}
		else
		{
			states->pins[pinIndex] = false;
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

ArduinoManager::ArduinoManager(unsigned id, int port)
{
	this->id = id;
	serial2Arduino = new Serial(port, ARD_CONFIG_SERIAL_BAUDRATE, ARD_CONFIG_SERIAL_BITS2FRAME, ARD_CONFIG_SERIAL_STOPBITS, ARD_CONFIG_SERIAL_PARITY);
	serial2Arduino->drain();

	lastKeyStates = new ArduinoKeyStates(id);
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

	if (!keysQueue.empty())
	{
		delete lastKeyStates;
		lastKeyStates = keysQueue.front();
		keysQueue.pop();
	}

	return lastKeyStates;
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
							ArduinoKeyStates *keyStates = unpackArduinoKeyStates(this->id, buffer);
							//keysQueue.push(Reference((void *)keyStates));
							keysQueue.push(keyStates);

							if (debugging)
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
		cerr << msg << endl;

	}
	catch (const std::exception& e)
	{
		cerr << "Fatal error to Arduino device connection." << std::endl;
		cerr << e.what() << std::endl;
	}

	alive = false;
}


