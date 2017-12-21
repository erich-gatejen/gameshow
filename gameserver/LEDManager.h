#pragma once

#include "windows.h"
#include <list>

enum RGB {
	RED = 1,
	BLUE,
	GREEN
};

typedef	unsigned	rgbID;
typedef unsigned	ledID;
typedef	INT			deviceId;
typedef BYTE		INTENSITY;

#define LED_PER_RGB 		3
#define LEDS_PER_GROUP		8
#define LEDS_PER_DEVICE		64

#define MIN_LED_LEVEL		0
#define MAX_LED_LEVEL		255

class RGBLED
{
public:
	BYTE	red;
	BYTE	green;
	BYTE	blue;
};


class LEDManager
{

private:
	size_t		numberOfDevices;

	// Do not access these variables directly!  Use the following accessor.  
	unsigned	numDevices;
	int			numLeds;
	int			*ids;
	deviceId getListIdForDevice(ledID ledid);
	deviceId getIdForDevice(ledID ledid);
	deviceId getIdForLED(ledID ledid);
	ledID    getRedIdForRGB(ledID ledid);
	ledID    getGreenIdForRGB(ledID ledid);
	ledID    getBlueIdForRGB(ledID ledid);

public:

	LEDManager();
	~LEDManager();

	void setRBG(rgbID rgbid, RGBLED rgbled);
	void setRBG(rgbID rgbid, INTENSITY redIntensity, INTENSITY greenIntensity, INTENSITY blueIntensity);
	void setLED(ledID ledid, INTENSITY intensity);
	void reset();

};

