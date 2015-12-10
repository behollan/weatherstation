/*

This file defines basic functions for Weather Meter readings a description of each function is listed below.


------------------------------------------------------------------------


For more information about these weather sensors, see the data sheet at: https://www.sparkfun.com/datasheets/Sensors/Weather/Weather%20Sensor%20Assembly..pdf

Anemometer uses a switch which closes once per second for a wind speed of 1.429 MPH
Rain gage closes a switch every 0.011" of rain
Wind Vane uses a voltage divider with a voltage divider of 10kohms

Written by Ben Holland for Digital Electronics, PHGN317 at the Colorado School of Mines
Nov. 2015

HI!! :)	
*/

//Pin definitions
#define ANEMOMETER_PIN 3
#define ANEMOMETER_INT 6
#define VANE_PWR 4
#define VANE_PIN A0
#define RAIN_GAUGE_PIN 5
#define RAIN_GAUGE_INT 2

#define WIND_FACTOR 1.492 //Conversion Factor
#define RAIN_FACTOR 0.011

float PollTime,Time_now,Time_prev; //Used for determining poll time

volatile int anem_count = 0;	//Wind Speed stuff here
volatile unsigned long anem_last = 0;
volatile unsigned long anem_min = 0xffffffff;
volatile unsigned long rain_count = 0;
volatile unsigned long rain_last = 0;

void rainGageClick() //ISR Called for rain gauge click
{
	long thisTime = micros() - rain_last; //Determine time elapsed between clicks
	rain_last = micros(); //Reset the timer
	if (thisTime>500) //Allow for debouncing
	{
		rain_count++; //Increment the counter if not debouncing
	}
}
void anemometerClick() //ISR Called for windspeed
{
	long thisTime = micros() - anem_last; //Determine Time elapsed
	anem_last = micros(); //Reset timer
	if (thisTime>500) //Increment if not debounce
	{
		anem_count++;
		if (thisTime<anem_min) //Is this the fastest one this poll?
		{
			anem_min = thisTime; //Make this value the new fastest 
		}
	}
}
void setupWeatherInts() //Called during setup, just sets appropriate interrupts and pins
{
	pinMode(ANEMOMETER_PIN, INPUT);
	digitalWrite(ANEMOMETER_PIN, HIGH);  // Turn on the internal Pull Up Resistor
	pinMode(RAIN_GAUGE_PIN, INPUT);
	digitalWrite(RAIN_GAUGE_PIN, HIGH);  // Turn on the internal Pull Up Resistor
	pinMode(VANE_PWR, OUTPUT);
	digitalWrite(VANE_PWR, LOW);
	attachInterrupt(INT1, anemometerClick, FALLING); //Trigger annemometer click on falling edge signal
	attachInterrupt(INT0, rainGageClick, FALLING);  //Trigger annemometer click on falling edge signal
	interrupts(); //Allow interrupts
	Time_prev = micros(); //Establish a start time
}



double getUnitWind()
{
	unsigned long reading = anem_count; //Get the total counter
	anem_count = 0; //Reset for next poll
	Time_now = micros(); //Get current system time
	PollTime = (Time_now - Time_prev)/(1000000.0); //Determine time since last poll
	Time_prev = micros(); //Reset timer to now
	return (WIND_FACTOR*reading) / (PollTime); //Return average wind speed for the poll
}

double getGust()
{

	unsigned long reading = anem_min; //Get minimum time between clicks
	anem_min = 0xffffffff; //Reset the minimum
	double time = reading / 1000000.0; //How fast between clicks

	return (1 / (reading / 1000000.0))*WIND_FACTOR; //Give fastest wind speed
}

const int vaneValues[] PROGMEM = { 66,84,92,127,184,244,287,406,461,600,631,702,786,827,889,946 }; //Analog read values 0-1024 mapped to 0-5V
const int vaneDirections[] PROGMEM = { 1125,675,900,1575,1350,2025,1800,225,450,2475,2250,3375,0,2925,3150,2700 }; //Integer degree measurements

double getWindVane()
{
	analogReference(DEFAULT);		//Use default reference values
	digitalWrite(VANE_PWR, HIGH);	//Turn on the voltage divider
	delay(100);						//Let everything settle
	for (int n = 0; n<10; n++)		//Throw away the first 10 ADC readings per reccommendations from people on the internet
	{
		analogRead(VANE_PIN);
	}

	unsigned int reading = analogRead(VANE_PIN);	//What is the analog value
	digitalWrite(VANE_PWR, LOW);					//Turn off the voltage divider
	unsigned int lastDiff = 2048;					//Used for below

	for (int n = 0; n<16; n++)						//Search through the memory for the closest value that matches the ADC reading
	{
		int diff = reading - pgm_read_word(&vaneValues[n]);	//Subtract the ADC value from the first possible value
		diff = abs(diff);
		if (diff == 0) //You nailed it! This is the value
			return pgm_read_word(&vaneDirections[n]) / 10.0;

		if (diff>lastDiff) //Too high go back
		{
			return pgm_read_word(&vaneDirections[n - 1]) / 10.0;
		}

		lastDiff = diff;	//Return the new differece for next approximation
	}

	return pgm_read_word(&vaneDirections[15]) / 10.0;

}


double getUnitRain() //Print the total rain fall
{

	unsigned long reading = rain_count; //How many click?
	double unit_rain = reading*RAIN_FACTOR; //Convert to appropriate units
	return unit_rain; //It's raining men!
}