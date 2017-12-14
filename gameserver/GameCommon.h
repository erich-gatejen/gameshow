#pragma once

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
	public:	DeviceException(string reason = "Reason unknown") : runtime_error("Device failure: " + reason) {}
};

class ComException : public runtime_error {
	public:	ComException(string reason = "Reason unknown") : runtime_error("Communication failure: " + reason) {}
};


