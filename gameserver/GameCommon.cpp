#include "stdafx.h"
#include "GameCommon.h"
#include <iostream>
#include <iomanip>
using namespace std;


void print_bytes(ostream *os, byte	*buffer)
{
	cerr << std::setfill('0');
	for (size_t i = 0; i < 8; ++i) {
		*os << std::hex << std::setw(2) << (int)buffer[i] << " ";
	}
};

void print_bytes_stdout(byte	*buffer)
{
	print_bytes(&cout, buffer);
}

void print_bytes_stderr(byte	*buffer)
{
	print_bytes(&cerr, buffer);
}

bool debugging()
{
	return true;
}

bool tracing()
{
	return true;
}

/****************************************************************************
* CONFIGURATION SYSTEM
*/


Configuration::Configuration()
{
	InputSource inputSource;
	inputSource.type = IT_ARDUINO;
	inputSource.port = 6;		// hardcode for now
	inputSource.id = 1;
	inputSources.emplace_back(inputSource);
};

Configuration::~Configuration() 
{
}




