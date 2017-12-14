#pragma once

#include <windows.h>
#include <stdio.h>
using namespace std;

#include "GameCommon.h"

#define SETUP_LAG_TIME_MS	2000

class Serial
{
private:
	HANDLE handler;
	bool connected;
	COMSTAT status;
	DWORD errors;

public:
	Serial(unsigned port, unsigned baudrate, unsigned bytesize, unsigned stopbits, unsigned parity);
	~Serial();

	bool isConnected();
	int readByte(byte *byte, unsigned int timeoutms);
	int readSerial(byte *buffer, unsigned int toRead, unsigned int timeoutms);
	bool writeSerial(byte *buffer, unsigned int buf_size);

};
