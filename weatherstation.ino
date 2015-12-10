/*
Weather Station project developed for PHGN317 at Colorado School of Mines for deployment in Lake City, CO

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
	See Attached design document for debouncing circuit
	Rain Gauge Interrupt to Dig. Pin 2
	Annemometer Interrupt to Dig. Pin 3
	Wind Vane Power to Dig. Pin 4
	Wind Vane Sensor to Analog Pin 0
	5V power and Ground Appropriately
*/

#include <Wire.h>		//I2C library
#include "SparkFunMPL3115A2.h" //Sparkfun MPL Library found here: https://github.com/sparkfun/MPL3115A2_Breakout
#include "HIH_6130.h"	//Custom HIH-6130 Library, see dependencies
#include "WeatherMeter.h" //Custom Weather Meter Assemly Library
#include "SparkFunTSL2561.h" //Luminosity Sensor Library https://github.com/sparkfun/SparkFun_TSL2561_Arduino_Library/tree/V_1.1.0

#define TRUE 1			//Allow us to use TRUE to indicate a HIGH (1)
#define FALSE 0			//Allow us to use FALSE to indicate a LOW (0)

//Variable Definition for use later, Self explanatory
volatile float start, finished;  
volatile float elapsed=0, time=0, speedm=0;
volatile int bucketcount=0;


SFE_TSL2561 light;		//Begin looking at sun (Note this is bad if using a telescope)
MPL3115A2 mpl;			//Create an instance of the object

//LIGHT things
boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms = 1000;  // Integration ("shutter") time in milliseconds

void setup(void)
{
   Serial.begin(115200); //Start serial output 9600 Baud
   Wire.begin();       //Start i2c Bus
   setupWeatherInts();  //Setup our weather instruments
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

  light.begin(); //let there be light

  gain = 0; //more light things

  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration

  unsigned char time = 2;


  //Serial.println("Set timing...");
  light.setTiming(gain, 1000);
  //Serial.println("Powerup...");
  light.setPowerUp();

}//End Setup
    
void loop(void)
{

	//Humidity and Temperature HIH Status
	byte _status;
	unsigned int H_dat, T_dat;
	float RH, temperatureHIH, tempavg;

	_status = fetch_humidity_temperature(&H_dat, &T_dat); //Fetch temp and humidity data, as well as chip status

	switch (_status) //Use the status code from the HIH-6130 to print the current state of the chip
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

	RH = (float)H_dat * 6.10e-3;						//Per data sheet instructions for conversion to percent RH
	temperatureHIH = (float)T_dat * 1.007e-2 - 40.0;	//convert temperature data per data sheet instruction to degrees C


//Barometer MPL

	float pressure = mpl.readPressure();		//Read MPL Pressure Sensor Data and Store it
	float temperatureMPL = mpl.readTemp();		//Read MPL temp sensor data and store it
 //LUX DELUXE FO DUX
	unsigned int data0, data1;
	double lux;
	if (light.getData(data0, data1))
	{
		boolean good;  // True if neither sensor is saturated

		good = light.getLux(gain, ms, data0, data1, lux);
	}
	//General Print out instructions
	tempavg = (temperatureMPL + temperatureHIH) / 2;	//Average MPL and HIH temp data


	Serial.print("Pressure(kPa):");
	Serial.println(pressure / 1000, 2);			//Print Pressure data in kPa

	Serial.print("Relative Humidity(%): ");
	print_float(RH, 1);							//Print Relative Humidity Percentage
	Serial.print("\n");							//Line break to accommodate print_float

	Serial.print("Temperature(C): ");
	Serial.println(tempavg, 2);					//Print Temp Data
	Serial.println("===========================================================");
	Serial.print("Rainfall in the Last 24 Hours (in): ");
	Serial.println(getUnitRain());				//Rainfall for last 24 Hours
	Serial.println("===========================================================");
	Serial.print("Wind speed (MPH): ");
	Serial.println(getUnitWind());				//Windspeed average since last poll
	Serial.print("Wind gust (MPH): ");
	Serial.println(getGust());					//Windspeed Max Gust since last poll
	Serial.print("Wind direction: ");
	Serial.println(getWindVane());				//Wind Direction
	Serial.println("===========================================================");
	Serial.print("Luminosity: ");
	Serial.print(lux);							//Lux (W)

	for (int i = 0; i < 20; i++)
	{
		Serial.println();				//Clear the display, makes for easier reading
		i++;
	}

	delay(1000);
}//End the main loop!

