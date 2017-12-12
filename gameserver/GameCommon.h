#pragma once

#include <stdexcept>
using namespace std;

class DeviceException : public runtime_error {
public:	DeviceException(string reason = "Reason unknown") : runtime_error("Device failure: " + reason) {}
};


