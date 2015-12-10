/*

This file defines basic functions for HIH-6130 sensor readings a description of each function is listed below.


------------------------------------------------------------------------
fetch_humidity_temperature(unsigned int *p_H_dat, unsigned int *p_T_dat)
		
		Fetch Humidity value from HIH-6130, send it back via variables 1 and 2 (p_H_dat) and (p_T_dat) (Using pass by reference)

------------------------------------------------------------------------
print_float(float f, int num_digits)

		Print a float, f, of num_digits long.

------------------------------------------------------------------------


Arduino and HIH-6130 BOB from SparkFun Electronics (https://www.sparkfun.com/products/11295)
based on code from Peter H Anderson, Baltimore, MD, Nov, '11 (http://www.phanderson.com/arduino/hih6130.html)
	* copyright, Peter H Anderson, Baltimore, MD, Nov, '11
	* You may use it, but please give credit.  


Edited and rewritten by Ben Holland for Digital Electronics, PHGN317 at the Colorado School of Mines
Nov. 2015
*/

/*
Return two bytes to the variable passed to the function (*p_H_dat and *p_T_dat). 
First int byte will be the Humidity data, the second variable will be compensated temperature value.
*/
byte fetch_humidity_temperature(unsigned int *p_H_dat, unsigned int *p_T_dat) //Fetch Humidity value from HIH-6130, send it back via variables 1 and 2 (p_H_dat) and (p_T_dat) (Using pass by reference)
{
	byte hih_addr, Hum_H, Hum_L, Temp_H, Temp_L, _status;
	unsigned int H_dat, T_dat;
	hih_addr = 0x27;  //HIH-6130 I2C address
	Wire.beginTransmission(hih_addr);  //Start transmission HIH-6130
	Wire.endTransmission(); //End transmission to other devices
	delay(100);

	Wire.requestFrom((int)hih_addr, (int)4);	//Request all 4 registers from the HIH-6130
	Hum_H = Wire.read();						//First register is Humidity (Most Significant 6 bits), first two bits are the current state of the device
	Hum_L = Wire.read();						//Second register is the remaining humidity bits (8 bits)
	Temp_H = Wire.read();						//Third is Temp data formatted again like humidity data
	Temp_L = Wire.read();						//Last of the Temp data
	Wire.endTransmission();

	_status = (Hum_H >> 6) & 0x03;				//Read the first 2 bits, report it as status (The >> are bit shifts to get the first two values, then bitwise add this to 0x03)
	Hum_H = Hum_H & 0x3f;//Use the remaining bits from Hum_H as the initial, bitwise add 63 to this value, this shifts it out to an byte wide
	H_dat = (((unsigned int)Hum_H) << 8) | Hum_L;		//Concatenate the values from the first H_dat register and the second H_dat register to get a 14 bit compensated humidity value
	T_dat = (((unsigned int)Temp_H) << 8) | Temp_L;		//No need to do any tricky shifting here, temp data comes out from the device properly
	T_dat = T_dat / 4;							//Divide by four as per data sheet instructions, two LSBs are ignored
	*p_H_dat = H_dat;							//Return the Humidity data to the first variable passed (Pass by reference)
	*p_T_dat = T_dat;							//Return the Temperature data to the second variable passed (Pass by reference)
	return(_status);							//Return the status of the HIH Sensor
}


/*
print_float(float f. int num_digits)
Print a float, f, of num_digits long. 
*/
void print_float(float f, int num_digits)
{
	int f_int;
	int pows_of_ten[4] = { 1, 10, 100, 1000 };
	int multiplier, whole, fract, d, n;

	multiplier = pows_of_ten[num_digits];
	if (f < 0.0)
	{
		f = -f;
		Serial.print("-");
	}
	whole = (int)f;
	fract = (int)(multiplier * (f - (float)whole));

	Serial.print(whole);
	Serial.print(".");

	for (n = num_digits - 1; n >= 0; n--) // print each digit with no leading zero suppression
	{
		d = fract / pows_of_ten[n];
		Serial.print(d);
		fract = fract % pows_of_ten[n];
	}
}