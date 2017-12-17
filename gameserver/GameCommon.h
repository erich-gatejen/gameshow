#pragma once

#include <windows.h>
#include <stdexcept>
using namespace std;

enum ErrorCodes
{
	ERROR_CODE_NONE = 0,
	ERROR_CODE_SPURIOUS_EXCEPTION = 1,
	ERROR_CODE_SUBSYSTEM_FAILURE = 2,
	ERROR_CODE_SHUTDOWN_FAULT = 3
};

class DeviceException : public runtime_error {
public:	DeviceException(string reason = "Reason unknown") : runtime_error("Device failure: " + reason) {};
};

class ComException : public runtime_error {
public:	ComException(string reason = "Reason unknown") : runtime_error("Communication failure: " + reason) {};
};

class ComRecoverableException : public runtime_error {
public:	ComRecoverableException(string reason = "Reason unknown") : runtime_error("Communication failure: " + reason) {};
};

void print_bytes_stdout(byte	*buffer);
void print_bytes_stderr(byte	*buffer);
void print_bytes(ostream *os, byte	*buffer);

bool debugging();


