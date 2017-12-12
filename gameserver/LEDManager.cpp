#include "stdafx.h"
#include "PacDrive.h"
#include "GameCommon.h"
#include "LEDManager.h"

#include <list>


LEDManager::LEDManager()
{
	int numAllDevices = PacInitialize();

	// Find PacLED64 devices
	std::list<int> idList;
	for (int index = 0; index < numAllDevices; index++) {
		auto deviceType = PacGetDeviceType(index);
		if (deviceType == DEVICETYPE_PACLED64) {
			idList.emplace_back(index);
		}
	}

	numDevices = idList.size();
	if (numDevices < 1) 
	{
		throw DeviceException("No PacLED64 attached.");
	}

	ids = new int[idList.size()];
	// Blah, don't want to learn the finer details of checked iterators right now.  Brute it.
	for (int index = 0; index < numDevices; index++)
	{
		ids[index] = idList.front();
		idList.pop_front();
	}
	return;
}

LEDManager::~LEDManager()
{
	PacShutdown();
}

int LEDManager::getListIdForDevice(ledID ledid)
{
	int listId = (int) (ledid / LEDS_PER_DEVICE);
	if (listId > numDevices) {
		throw DeviceException("LED id outside range of attached devices.  Number of devices = " + numDevices);
	}
	return listId;
}

deviceId LEDManager::getIdForDevice(ledID ledid)
{
	 return (deviceId) ids[getListIdForDevice(ledid)];
}

deviceId LEDManager::getIdForLED(ledID ledid)
{
	int listId = getListIdForDevice(ledid);
	return (deviceId) ledid - (listId * LEDS_PER_DEVICE);
}

void LEDManager::setRBG(rgbID rgbid, RGBLED rgbled)
{
}

void LEDManager::setRBG(rgbID rgbid, INTENSITY redIntensity, INTENSITY greenIntensity, INTENSITY blueIntensity)
{
}

void LEDManager::setLED(ledID ledid, INTENSITY intensity)
{
	Pac64SetLEDIntensity(getIdForDevice(ledid), getIdForLED(ledid), intensity);
}

