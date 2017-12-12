// gameserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "PacDrive.h"
#include "GameCommon.h"
#include "LEDManager.h"

#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{	

	try
	{
		LEDManager *ledManager = new LEDManager();

		bool on = false;
		while (true)
		{
			if (!on)
			{
				ledManager->setLED(16, 255);
				on = true;
			}
			else {
				ledManager->setLED(16, 0);
				on = false;
			}

			Sleep(1000);
		}
	}
	catch (const char* msg) {
		cerr << msg << endl;
	}
	catch (const std::exception& e)
	{
		cerr << "Fatal error" << std::endl;
		cerr << e.what() << std::endl;
	}

	return 0;
}

