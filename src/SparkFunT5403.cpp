/******************************************************************************
SparkFunT5403.cpp
Library for T5403 barometric pressure sensor.
Casey Kuhns @ SparkFun Electronics
Original Creation Date: February 12, 2014
https://github.com/sparkfun/T5403_Barometric_Breakout

The T5403 by Epcos is a low cost I2C barometric pressure sensor.  This sensor 
can be used in weather stations and for altitude estimations.  

Resources:
This library uses the Arduino Wire.h to complete I2C transactions.

Development environment specifics:
	IDE: Arduino 1.0.5
	Hardware Platform: Arduino Pro 3.3V/8MHz
	T5403 Breakout Version: 1.0
	
**Updated to Arduino 1.6.4 5/2015**
	
NOTE:  SPI is currently unsupported in the hardware.  If a release comes with
hardware support this file will be updated.  All reference to SPI is currently
a place holder for future development.

This code is beerware. If you see me (or any other SparkFun employee) at the
local pub, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include "SparkFunT5403.h"
#include <Wire.h> // Wire library is used for I2C
//#include <SPI.h> // SPI library is used for SPI.

T5403::T5403(interface_mode interface)
// Base library type
{
	_interface = interface; //set interface used for communication
}

void T5403::begin(void)
// Initialize library for subsequent pressure measurements
{  
	// Set up the communication port
	communicationBegin();

	getData(T5403_C1, (int16_t*)&c1);  //Retrieve C1 from device
	getData(T5403_C2, (int16_t*)&c2);  //Retrieve C2 from device
	getData(T5403_C3, (int16_t*)&c3);  //Retrieve C3 from device
	getData(T5403_C4, (int16_t*)&c4);  //Retrieve C4 from device
	getData(T5403_C5, &c5);  //Retrieve C5 from device
	getData(T5403_C6, &c6);  //Retrieve C6 from device
	getData(T5403_C7, &c7);  //Retrieve C7 from device
	getData(T5403_C8, &c8);  //Retrieve C8 from device
}
	
int16_t T5403::getTemperature(temperature_units units)
// Return a temperature reading.
{
	// Create variables for conversion and raw data. 
	int16_t temperature_raw; 
	int32_t temperature_actual; 
	
	// Start temperature measurement
	sendCommand(T5403_COMMAND_REG, COMMAND_GET_TEMP); 
	// Wait 5ms for conversion to complete
	sensorWait(5); 
	// Receive raw temp value from device.
	getData(T5403_DATA_REG, &temperature_raw);		
	// Perform calculation specified in data sheet
	temperature_actual = (((((int32_t) c1 * temperature_raw) >> 8) 
						 + ((int32_t) c2 << 6)) * 100) >> 16;


	// If Fahrenheit is selected return the temperature converted to F
	if(units == FAHRENHEIT){
		temperature_actual = ((temperature_actual * 9) / 5) + 3200;
		return temperature_actual;
		}
		
	// If Celsius is selected return the temperature converted to C	
	else if(units == CELSIUS){
		return (int16_t) temperature_actual;
	}
}

int32_t T5403::getPressure(uint8_t commanded_precision)
// Return a pressure reading.
{
	// Create variables for conversion and raw data. 
	int16_t temperature_raw; 
	uint16_t pressure_raw;

	// Start temperature measurement
	sendCommand(T5403_COMMAND_REG, COMMAND_GET_TEMP); 
	// Wait 5ms for conversion to complete
	sensorWait(5); 
	// Receive raw temp value from device.
	getData(T5403_DATA_REG, &temperature_raw);		
	
	// Load measurement noise level into command along with start command bit and start pressure measurement
	sendCommand(T5403_COMMAND_REG, (commanded_precision << 3)|(0x01)); 
	
	//Select delay time based on precision level selected.
	switch(commanded_precision){
		case MODE_LOW:
		{	
			sensorWait(5); //  Wait 5ms for conversion to complete
			break;
		}
		case MODE_STANDARD:
		{
			sensorWait(11); //  Wait 11 ms for conversion to complete
			break;
		}
		case MODE_HIGH:{
			sensorWait(19); //  Wait 19 ms for conversion to complete
			break;
		}
		case MODE_ULTRA:{
			sensorWait(67); //  Wait 67 ms for conversion to complete
			break;
		}
	};
	
	//  Receive raw pressure value from device.
	getData(T5403_DATA_REG, (int16_t*)&pressure_raw);	
	
	// Create variables to hold calculated pressure and working variables for 
	// calculations.
	int32_t pressure_actual, s, o; 

	// Calculations come from application note. 
	s = (((((int32_t) c5 * temperature_raw)  >> 15) * temperature_raw) >> 19)
			+ c3 + (((int32_t) c4 * temperature_raw) >> 17); 
			
	o = (((((int32_t) c8 * temperature_raw) >> 15) * temperature_raw) >> 4) 
			+ (((int32_t) c7 * temperature_raw) >> 3) + ((int32_t)c6 * 0x4000);
			
	pressure_actual = (s * pressure_raw + o) >> 14;

	return pressure_actual;
}

void T5403::sensorWait(uint8_t time)
// Delay function.  This can be modified to work outside of Arduino based MCU's
{
	delay(time);
};

void T5403::communicationBegin()
// Initialize the communication protocol used.  SPI is currently unsupported in 
// the hardware.  If a release comes with hardware support this file will be 
// updated.  All reference to SPI is currently a place holder for future 
// development.
// This can be modified to work outside of Arduino based MCU's
{
	// If SPI is selected use SPI begin
	if( _interface == MODE_SPI){  
	//	SPI.begin();
	}
	// If i2c is selected for communication use i2c begin
	else{
		Wire.begin();
	}

}

int8_t T5403::getData(uint8_t location, int16_t* output)
// Receive data from the device.  SPI is currently unsupported in the hardware.  
// If a release comes with hardware support this function will be updated.  
// All reference to SPI is currently a place holder for future development.
// This can be modified to work outside of Arduino based MCU's  
{
	uint8_t byteLow, byteHigh;
	int16_t _output;
		
	if( _interface == MODE_SPI){  
	/*	byteLow = SPI.transfer(0x00);
		byteHigh = SPI.transter(0x00);
	*/
	}
	else {  // If i2c is selected for communication use i2c commands
		Wire.beginTransmission(T5403_I2C_ADDR); 
		Wire.write(location);
		Wire.endTransmission();    // Transmit data
		Wire.requestFrom(T5403_I2C_ADDR,2);

		while(Wire.available()){
			byteLow = Wire.read(); // receive low byte 
			byteHigh = Wire.read(); // receive high byte
		}
	}
	
	_output = (byteHigh << 8)|(byteLow);
	*output = _output;
	
}

int8_t T5403::sendCommand(uint8_t location, uint8_t command)
// Send command to the device.  SPI is currently unsupported in the hardware.  
// If a release comes with hardware support this function will be updated.  
// All reference to SPI is currently a place holder for future development.
// This can be modified to work outside of Arduino based MCU's
{
	// If SPI is selected for communication use SPI commands
	if(_interface == MODE_SPI){
	//	SPI.transfer(location);
	//	SPI.transter(command);
	}
	// If i2c is selected for communication use i2c commands
	else{
		Wire.beginTransmission(T5403_I2C_ADDR); 
		Wire.write(location);
		Wire.write(command);
		Wire.endTransmission();
	}
}
