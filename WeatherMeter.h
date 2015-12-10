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


#define ANEMOMETER_PIN 3
#define ANEMOMETER_INT 6
#define VANE_PWR 4
#define VANE_PIN A0
#define RAIN_GAUGE_PIN 5
#define RAIN_GAUGE_INT 2

#define WIND_FACTOR 1.492
float PollTime,Time_now,Time_prev;

volatile int anem_count = 0;
volatile unsigned long anem_last = 0;
volatile unsigned long anem_min = 0xffffffff;

#define RAIN_FACTOR 0.011

volatile unsigned long rain_count = 0;
volatile unsigned long rain_last = 0;

void rainGageClick()
{
	long thisTime = micros() - rain_last;
	rain_last = micros();
	if (thisTime>500)
	{
		rain_count++;
	}
}
void anemometerClick()
{
	long thisTime = micros() - anem_last;
	anem_last = micros();
	if (thisTime>500)
	{
		anem_count++;
		if (thisTime<anem_min)
		{
			anem_min = thisTime;
		}

	}
}
void setupWeatherInts()
{
	pinMode(ANEMOMETER_PIN, INPUT);
	digitalWrite(ANEMOMETER_PIN, HIGH);  // Turn on the internal Pull Up Resistor
	pinMode(RAIN_GAUGE_PIN, INPUT);
	digitalWrite(RAIN_GAUGE_PIN, HIGH);  // Turn on the internal Pull Up Resistor
	pinMode(VANE_PWR, OUTPUT);
	digitalWrite(VANE_PWR, LOW);
	attachInterrupt(INT1, anemometerClick, FALLING);
	attachInterrupt(INT0, rainGageClick, FALLING);
	interrupts();
	Time_prev = micros();
}



double getUnitWind()
{
	unsigned long reading = anem_count;
	anem_count = 0;
	Time_now = micros();
	PollTime = (Time_now - Time_prev)/(1000000.0);
	Time_prev = micros();
	return (WIND_FACTOR*reading) / (PollTime);
}

double getGust()
{

	unsigned long reading = anem_min;
	anem_min = 0xffffffff;
	double time = reading / 1000000.0;

	return (1 / (reading / 1000000.0))*WIND_FACTOR;
}

const int vaneValues[] PROGMEM = { 66,84,92,127,184,244,287,406,461,600,631,702,786,827,889,946 };
const int vaneDirections[] PROGMEM = { 1125,675,900,1575,1350,2025,1800,225,450,2475,2250,3375,0,2925,3150,2700 };

double getWindVane()
{
	analogReference(DEFAULT);
	digitalWrite(VANE_PWR, HIGH);
	delay(100);
	for (int n = 0; n<10; n++)
	{
		analogRead(VANE_PIN);
	}

	unsigned int reading = analogRead(VANE_PIN);
	digitalWrite(VANE_PWR, LOW);
	unsigned int lastDiff = 2048;

	for (int n = 0; n<16; n++)
	{
		int diff = reading - pgm_read_word(&vaneValues[n]);
		diff = abs(diff);
		if (diff == 0)
			return pgm_read_word(&vaneDirections[n]) / 10.0;

		if (diff>lastDiff)
		{
			return pgm_read_word(&vaneDirections[n - 1]) / 10.0;
		}

		lastDiff = diff;
	}

	return pgm_read_word(&vaneDirections[15]) / 10.0;

}


double getUnitRain()
{

	unsigned long reading = rain_count;
	double unit_rain = reading*RAIN_FACTOR;

	return unit_rain;
}

