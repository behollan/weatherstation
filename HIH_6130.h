// HIH_6130.h

#ifndef _HIH_6130_h
#define _HIH_6130_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class HIH_6130Class
{
 protected:


 public:
	void init();
	byte fetch_humidity_temperature;
};

extern HIH_6130Class HIH_6130;

#endif

