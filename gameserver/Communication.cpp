#include "stdafx.h"
#include <boost/format.hpp>

#include "Communication.h"

#define PORT_LABEL_BUFFER_SIZE (4 + 5 + 1)  // name + max unsigned + null

Serial::Serial(unsigned port, unsigned baudrate, unsigned bytesize, unsigned stopbits, unsigned parity)
{
	connected = false;
	
	char portLabel[PORT_LABEL_BUFFER_SIZE];
	sprintf_s(portLabel, "\\\\.\\COM%d", port);

	handler = CreateFileA(static_cast<LPCSTR>(portLabel),
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (handler == INVALID_HANDLE_VALUE) {
		throw ComException((boost::format("Could not attach %s due to error code=%d.   See https://msdn.microsoft.com/en-us/library/windows/desktop/ms681382(v=vs.85).aspx") % std::string(portLabel) % GetLastError()).str());
	}

	DCB dcbSerialParameters = { 0 };

	if (!GetCommState(handler, &dcbSerialParameters)) 
	{
		throw ComException((boost::format("Could not get state for port %s") % std::string(portLabel) % GetLastError()).str());
	}
	else 
	{
		dcbSerialParameters.BaudRate = baudrate;
		dcbSerialParameters.ByteSize = bytesize;
		dcbSerialParameters.StopBits = stopbits;
		dcbSerialParameters.Parity = parity;
		dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

		if (!SetCommState(handler, &dcbSerialParameters))
		{
			throw ComException((boost::format("Could not comm state for port %s") % std::string(portLabel) % GetLastError()).str());
		}
		else 
		{
			connected = true;
			PurgeComm(handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
			Sleep(SETUP_LAG_TIME_MS);
		}
	}

}

Serial::~Serial()
{
	if (connected) 
	{
		connected = false;
		CloseHandle(handler);
	}
}

void setTimeout(HANDLE handler, unsigned int timeoutms)
{
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = timeoutms;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(handler, &timeouts)) throw ComException("Could not set comm timeout");
}

#define DRAIN_BUFFER_SIZE	256

void Serial::drain()
{
	// Purge it first
	PurgeComm(handler, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

	// Force it to immeadiately return, data or no data.
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(handler, &timeouts)) throw ComException("Could not set comm timeout");

	byte	drainDump[DRAIN_BUFFER_SIZE];
	COMSTAT	status;
	DWORD	errors;
	DWORD	bytesRead = 0;
	DWORD   bytesToRead = 0;
	ClearCommError(handler, &errors, &status);
	while (status.cbInQue > 0)
	{
		bytesToRead = status.cbInQue;
		if (bytesToRead > DRAIN_BUFFER_SIZE) bytesToRead = DRAIN_BUFFER_SIZE;
		ReadFile(handler, drainDump, bytesToRead, &bytesRead, NULL);
		Sleep(5);
		ClearCommError(handler, &errors, &status);
	};
}

int Serial::readByte(byte *byteBuffer, unsigned int timeoutms)
{
	COMSTAT	status;
	DWORD	errors;
	ClearCommError(handler, &errors, &status);

	setTimeout(handler, timeoutms);
	DWORD bytesRead = 0;
	ReadFile(handler, byteBuffer, 1, &bytesRead, NULL);
	return bytesRead;
}

int Serial::readSerial(byte *buffer, unsigned int toRead, unsigned int timeoutms)
{
	DWORD bytesRead;
	COMSTAT	status;
	DWORD	errors;

	ClearCommError(handler, &errors, &status);
	setTimeout(handler, timeoutms);

	if (ReadFile(handler, buffer, toRead, &bytesRead, NULL)) return bytesRead;

	return 0;
}

bool Serial::writeSerial(byte *buffer, unsigned int buf_size)
{
	DWORD bytesSend;
	COMSTAT	status;
	DWORD	errors;
	ClearCommError(handler, &errors, &status);

	if (!WriteFile(handler, (void*)buffer, buf_size, &bytesSend, 0)) {
		ClearCommError(handler, &errors, &status);
		return false;
	}
	else return true;
}

bool Serial::isConnected()
{
	return connected;
}