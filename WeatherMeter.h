/*

This file defines basic functions for Weather Meter readings a description of each function is listed below.


------------------------------------------------------------------------


For more information about these weather sensors, see the data sheet at: https://www.sparkfun.com/datasheets/Sensors/Weather/Weather%20Sensor%20Assembly..pdf

Anemometer uses a switch which closes once per second for a wind speed of 1.429 MPH
Rain gage closes a switch every 0.011" of rain
Wind Vane uses a voltage divider with a voltage divider of 10kohms

Written by Ben Holland for Digital Electronics, PHGN317 at the Colorado School of Mines
Nov. 2015

******	Change Log	*******
11/30/15
Initial Commit
***************************

*/

char windDir() {
	float voltdir=analogRead(0);
	char dir;
	if (voltdir >= 3.6 && voltdir < 3.95) {
		dir = 'N';
	}
	else if (voltdir >= 1.7 && voltdir < 2.1) {
		dir = 'NNE';
	}
	else if (voltdir >= 2.1 && voltdir < 2.35) {
		dir = 'NE';
	}
	else if (voltdir >= 0.37 && voltdir < 0.43) {
		dir = 'ENE';
	}
	else if (voltdir >= 0.43 && voltdir < 0.5) {
		dir = 'E';
	}
	else if (voltdir >= 0.3 && voltdir < 0.37) {
		dir = 'ESE';
	}
	else if (voltdir >= 0.8 && voltdir < 1) {
		dir = 'SE';
	}
	else if (voltdir >= 0.57 && voltdir < 0.7) {
		dir = 'SSE';
	}
	else if (voltdir >= 1.3 && voltdir < 1.5) {
		dir = 'S';
	}
	else if (voltdir >= 1.1 && voltdir < 1.3) {
		dir = 'SSW';
	}
	else if (voltdir >= 3 && voltdir < 3.2) {
		dir = 'SW';
	}
	else if (voltdir >= 2.87 && voltdir < 3) {
		dir = 'WSW';
	}
	else if (voltdir >= 4.5 && voltdir < 4.7) {
		dir = 'W';
	}
	else if (voltdir >= 3.9 && voltdir < 4.1) {
		dir = 'WNW';
	}
	else if (voltdir >= 4.7 && voltdir < 4.9) {
		dir = 'NW';
	}
	else if (voltdir >= 3.35 && voltdir < 3.5) {
		dir = 'NNW';
	}
	else
	{
		Serial.println("No Direction defined... either your code sucks or something isn't connected.");
	}
	return dir;
}

