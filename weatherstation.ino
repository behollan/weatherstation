/*
Weather Station project developed for PHGN317 at Colorado School of Mines

******	 Authors	*******

Ben Holland - behollan@mines.edu
Joe Gibson  - jogibson@mines.edu

******	Description	*******

Reads and reports data from several different on board sensors using I2C and analog read.

** Hookup Guide **
TSL2561 - Luminosity Sensor
	SDA to Analog Pin 4 Arduino
	SCL to Analog Pin 5 Arduino
	3.3V ONLY
	GND

HIH-6130 - Humidity and Temp Sensor
	SDA to Analog Pin 4 Arduino
	SCL to Analog Pin 5 Arduino
	3.3V ONLY
	GND

MPL3115A2 - Pressure and Temp Sensor
	SDA to Analog Pin 4 Arduino
	SCL to Analog Pin 5 Arduino
	3.3V ONLY
	GND

Weather Sensor Assembly p/n 80422
	To Be Determined

******	Change Log	*******
11/30/15

Added comments for remaining functions
Renamed some ambiguous variables to accommodate further sensor development
***************************
*/

#include <Wire.h>		//I2C library
#include "SparkFunMPL3115A2.h" //Sparkfun MPL Library found here: https://github.com/sparkfun/MPL3115A2_Breakout
#include "HIH_6130.h"	//Custom HIH-6130 Library, see dependencies
#include "WeatherMeter.h"

#define TRUE 1			//Allow us to use TRUE to indicate a HIGH (1)
#define FALSE 0			//Allow us to use FALSE to indicate a LOW (0)
volatile float start, finished;
volatile float elapsed=0, time=0, speedm=0;
volatile int bucketcount=0;
int test;

#define ANEMOMETER_PIN 3
#define ANEMOMETER_INT 6
#define VANE_PWR 4
#define VANE_PIN A0
#define RAIN_GAUGE_PIN 5
#define RAIN_GAUGE_INT 2

#define WIND_FACTOR 2.4
#define TEST_PAUSE 60000

volatile int anem_count = 0;
volatile unsigned long anem_last = 0;
volatile unsigned long anem_min = 0xffffffff;

#define RAIN_FACTOR 0.011

volatile unsigned long rain_count = 0;
volatile unsigned long rain_last = 0;

MPL3115A2 mpl;			//Create an instance of the object

void setup(void)
{
   Serial.begin(115200); //Start serial output 9600 Baud
   Wire.begin();       //Start i2c Bus
   setupWeatherInts();  //Setup our weather instruments
																 //Humidity and Temperature
   Serial.println("========================");  // just to be sure things are working, print author information
   Serial.println("< Weather Station Data >");
   Serial.println("<     -Ben Holland-	  >");
   Serial.println("< -behollan@mines.edu- >");
   Serial.println("<     -Joe Gibson-	  >");
   Serial.println("< -jogibson@mines.edu- >");
   Serial.println("========================");

//Barometer
   mpl.begin();				// Get sensor on-line

// Configure the MPL sensor
  mpl.setModeBarometer();	// Measure pressure in Pascals from 20 to 110 kPa
  mpl.setOversampleRate(7); // Set Oversample to the recommended 128
  mpl.enableEventFlags();	// Enable all three pressure and temp event flags 
  start = millis();
}//End Setup
    
void loop(void)
{

//Humidity and Temperature HIH
	byte _status;
	unsigned int H_dat, T_dat;
	float RH, temperatureHIH, tempavg;

	_status = fetch_humidity_temperature(&H_dat, &T_dat); //Fetch temp and humidity data, as well as chip status
      
		switch(_status) //Use the status code from the HIH-6130 to print the current state of the chip
			{
				case 0:  Serial.println("Normal.");
					break;
				case 1:  Serial.println("Stale Data.");
					break;
				case 2:  Serial.println("In command mode.");
					break;
				default: Serial.println("Diagnostic."); 
					break; 
			}//End Status Switch       

	RH = (float) H_dat * 6.10e-3;						//Per data sheet instructions for conversion to percent RH
	temperatureHIH = (float) T_dat * 1.007e-2 - 40.0;	//convert temperature data per data sheet instruction to degrees C


//Barometer MPL

	float pressure = mpl.readPressure();		//Read MPL Pressure Sensor Data and Store it
	float temperatureMPL = mpl.readTemp();		//Read MPL temp sensor data and store it
 

//General Print out instructions
	tempavg=(temperatureMPL+temperatureHIH)/2;	//Average MPL and HIH temp data
      
	  
	Serial.print("Pressure(kPa):");
	Serial.println(pressure / 1000, 2);			//Print Pressure data in kPa

	Serial.print("Relative Humidity(%): ");
	print_float(RH, 1);							//Print Relative Humidity Percentage
	Serial.print("\n");							//Line break to accommodate print_float
	  
	Serial.print("Temperature(C): ");
	Serial.println(tempavg,2);					//Print Temp Data
      
	Serial.print("Rainfall in the Last 24 Hours (in): ");
	Serial.println(getUnitRain());

	Serial.print("Wind speed (MPH): ");
	Serial.println(getUnitWind());
	Serial.println(micros());


	Serial.print("Wind gust (MPH): ");
	Serial.println(getGust());

	Serial.print("Wind direction: ");
	Serial.println(getWindVane());
	if (test >= 1) {
		Serial.println("I ran the interrupt.");
		test = 0;
	}
	for (int i = 0; i < 20; i++)
	{
		Serial.println();
		i++;
	}

	delay(1000);								//Will need to rework this delay when we incorporate the physical weather sensors.
												//	Needs some sort of iterator that allows us to measure the data from the sensor
}//End Loop

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
}



double getUnitWind()
{
	unsigned long reading = anem_count;
	Serial.println(anem_count);
	anem_count = 0;
	return (WIND_FACTOR*reading) / (TEST_PAUSE / 1000);
}

double getGust()
{

	unsigned long reading = anem_min;
	anem_min = 0xffffffff;
	double time = reading / 1000000.0;

	return (1 / (reading / 1000000.0))*WIND_FACTOR;
}
void anemometerClick()
{
	anem_count = 0;
	long thisTime = micros() - anem_last;
	anem_last = micros();
	test = 1;
	if (thisTime>500)
	{
		anem_count++;
		if (thisTime<anem_min)
		{
			anem_min = thisTime;
		}

	}
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
	rain_count = 0;
	double unit_rain = reading*RAIN_FACTOR;

	return unit_rain;
}

void rainGageClick()
{
	long thisTime = micros() - rain_last;
	rain_last = micros();
	if (thisTime>500)
	{
		rain_count++;
	}
}
