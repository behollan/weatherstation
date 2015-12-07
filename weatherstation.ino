

/*
 MPL3115A2 Barometric Pressure Sensor Library Example Code
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 24th, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Uses the MPL3115A2 library to display the current altitude and temperature
 
 Hardware Connections (Breakoutboard to Arduino):
 -VCC = 3.3V
 -SDA = A4 (use inline 10k resistor if your board is 5V)
 -SCL = A5 (use inline 10k resistor if your board is 5V)
 -INT pins can be left unconnected for this demo
 
 During testing, GPS with 9 satellites reported 5393ft, sensor reported 5360ft (delta of 33ft). Very close!
 During testing, GPS with 8 satellites reported 1031ft, sensor reported 1021ft (delta of 10ft).
 
 Available functions:
 .begin() Gets sensor on the I2C bus.
 .readAltitude() Returns float with meters above sealevel. Ex: 1638.94
 .readAltitudeFt() Returns float with feet above sealevel. Ex: 5376.68
 .readPressure() Returns float with barometric pressure in Pa. Ex: 83351.25
 .readTemp() Returns float with current temperature in Celsius. Ex: 23.37
 .readTempF() Returns float with current temperature in Fahrenheit. Ex: 73.96
 .setModeBarometer() Puts the sensor into Pascal measurement mode.
 .setModeAltimeter() Puts the sensor into altimetery mode.
 .setModeStandy() Puts the sensor into Standby mode. Required when changing CTRL1 register.
 .setModeActive() Start taking measurements!
 .setOversampleRate(byte) Sets the # of samples from 1 to 128. See datasheet.
 .enableEventFlags() Sets the fundamental event flags. Required during setup.
 

 Testing HIH6130 Library from https://github.com/dhhagan/HIH6130 code will not compile with out it installed. Will probably remove for faster, more versatile hard coding.
*/

#include <HIH6130.h>
#include <Wire.h> //I2C library
#include "SparkFunMPL3115A2.h"
#include "HIH_6130.h"
#include "SparkFunTSL2561.h"

#define TRUE 1
#define FALSE 0

//Light!!
SFE_TSL2561 light; // For the Luminosity sensor

boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds


//End Luminosity sensor things

//Create an instance of the object
MPL3115A2 myPressure;

byte address = 0x27;
HIH6130 rht(address);
void setup(void)
{
   Serial.begin(9600); //Start serial output
   Wire.begin();       //Start i2c Bus

  //Humidity and Tempurature
   pinMode(4, OUTPUT);    // HIH3610 PIN 4
   digitalWrite(4, HIGH); // this turns on the HIH3610
   delay(1000);
   Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>");  // just to be sure things are working


   light.begin();
   gain = 0;
   unsigned char timelight = 2;
   Serial.println("Set timing...");
  // light.setTiming(gain, timelight, ms);

   Serial.println("Powerup...");
   light.setPowerUp();

}
    
void loop(void)
{
	Serial.print("1");
//Humidity and Temperature HIH
//   byte _status;
//   unsigned int H_dat, T_dat;
//   float RH, temperatureHIH, tempavg;
//
//   _status = fetch_humidity_temperature(&H_dat, &T_dat); //Fetch temp and humidity data, as well as chip status
//      
//       switch(_status) //Use the status code from the HIH-6130 to print the current state of the chip
//      {
//          case 0:  Serial.println("Normal.");
//                   break;
//          case 1:  Serial.println("Stale Data.");
//                   break;
//          case 2:  Serial.println("In command mode.");
//                   break;
//          default: Serial.println("Diagnostic."); 
//                   break; 
//      }       
//	  Serial.println(H_dat);
//	  Serial.println(T_dat);
//      RH = (float) H_dat * 6.10e-3;  //Per data sheet instructions for conversion to percent RH
//      temperatureHIH = (float) T_dat * 1.007e-2 - 40.0; //convert temperature data per data sheet instruction to degrees C
//
//
////Barometer MPL
////Barometer
//	  //myPressure.begin(); // Get sensor online
//
//			//			  // Configure the sensor
//	  //myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
//
//	  //myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
//
//	  //myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 
//
//  float pressure = myPressure.readPressure();
//  Serial.print("Pressure(kPa):");
//  Serial.println(pressure/1000, 2);
//
//  float temperatureMPL = myPressure.readTemp();
//  //Serial.print(" Temp(c):");
//  //Serial.print(temperature, 2);
//
//  //float temperature = myPressure.readTempF();
//  //Serial.print(" Temp(f):");
//  //Serial.print(temperature, 2);


  //Light Sensor

  

  unsigned int data0, data1;

  if (light.getData(data0, data1))
  {

	  Serial.print("data0: ");
	  Serial.print(data0);
	  Serial.print(" data1: ");
	  Serial.print(data1);
	  double lux;
	  boolean good;
	  good = light.getLux(gain, ms, data0, data1, lux);
	  Serial.print(" lux: ");
	  Serial.print(lux);
	  if (good) Serial.println(" (good)"); else Serial.println(" (BAD)");
  }
  else
  {
	  byte error = light.getError();
	  printError(error);
  }


  //END LIGHT


//General
  //    tempavg=(temperatureMPL+temperatureHIH)/2; //Average MPL and HIH temp data
      
	  Serial.print("Relative Humidity(%): ");
	//  print_float(RH, 1); //Print Relative Humidity Percentage
	  Serial.print("\n"); //Line break to accomodate print_float
	  Serial.print("Temperature(C): ");
	//  Serial.println(tempavg,2);
      Serial.println();


	  rht.readRHT();

	  // Access the variables and print to serial monitor
	  Serial.print(rht.humidity); Serial.print("\t");
	  Serial.println(rht.temperature);
	  delay(1000);


}


//LIGHT and GENERAL but I like The error report so I kept it

void printError(byte error)
// If there's an I2C error, this function will
// print out an explanation.
{
	Serial.print("I2C error: ");
	Serial.print(error, DEC);
	Serial.print(", ");

	switch (error)
	{
	case 0:
		Serial.println("success");
		break;
	case 1:
		Serial.println("data too long for transmit buffer");
		break;
	case 2:
		Serial.println("received NACK on address (disconnected?)");
		break;
	case 3:
		Serial.println("received NACK on data");
		break;
	case 4:
		Serial.println("other error");
		break;
	default:
		Serial.println("unknown error");
	}
}