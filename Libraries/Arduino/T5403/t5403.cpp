/******************************************************************************
SFE_BMP180.cpp
Library for T5403 barometric pressure sensor.
Casey Kuhns, SparkFun Electronics
2/12/2014
https://github.com/sparkfun/T5403_Barometric_Breakout

The T5403 by Epcos is a low cost I2C barometric pressure sensor.  This sensor 
can be used in weather stations and for altitude estimations.  

Resources:
This library uses the Arduino Wire.h to complete I2C transactions.

Development environment specifics:
Arduino 1.0.5
Board - v10

NOTE:  SPI is currently unsupported in the hardware.  If a release comes with
hardware support this file will be updated.  All reference to SPI is currently
a place holder for future development.

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include "t5403.h"
#include "Wire.h"
//#include "SPI.h"

T5403::T5403(uint8_t interface)
// Base library type
{
	_interface = interface; //set interface used for communication
}

void T5403::begin(void)
// Initialize library for subsequent pressure measurements
{  

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
	
int16_t T5403::getTemperature(uint8_t units){
	
	int8_t status = 0; // Variable to store and report error codes
	
	int16_t temperature_raw; // create variable to contain raw temperature.
	int32_t temperature_actual; // create variable to store calculated actual temperature
	status =+ sendCommand(T5403_COMMAND_REG, COMMAND_GET_TEMP); //  Start temperature measurement
	sensorWait(4500); //  Wait 4.5ms for conversion to complete
	status =+ getData(T5403_DATA_REG, &temperature_raw);	//  Get raw temp value	
	// Perform calculation specified in data sheet
	temperature_actual = (((((int32_t) c1 * temperature_raw) >> 8) + ((int32_t) c2 << 6)) * 100) >> 16;
	
	if(units == FAHRENHEIT){
		temperature_actual = ((temperature_actual * 9) / 5) + 3200;
		return temperature_actual;
		}
	else{
		return (int16_t) temperature_actual;
	}
}

int32_t T5403::getPressure(uint8_t precision){
	
	int8_t status = 0; // Variable to store and report error codes
	
	int16_t temperature_raw; // create variable to contain raw temperature.
	uint16_t pressure_raw; // create variable to contain raw pressure.

	uint8_t command; // variable to contain command data.

	status =+ sendCommand(T5403_COMMAND_REG, COMMAND_GET_TEMP); //  Start temperature measurement
	sensorWait(4500); //  Wait 4.5ms for conversion to complete
	status =+ getData(T5403_DATA_REG, &temperature_raw);	//  Get raw temp value	
	
	command = (precision << 3)|(0x01); // Load measurement noise level into command along with start command bit.
	status =+ sendCommand(T5403_COMMAND_REG, command); //  Start pressure measurement
	
	switch(precision){ //Select delay time based on noise level selected.
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
	
	status =+ getData(T5403_DATA_REG, (int16_t*)&pressure_raw);	//  Get raw pressure value
	
	// Begin calculation of actual pressure using constants
	
	int32_t pressure_actual, s, o; //create variables to hold actual pressure and working variables for calculations.

	// Calculations come from application note. 
	s = (((((int32_t) c5 * temperature_raw)  >> 15) * temperature_raw) >> 19) + c3 + (((int32_t) c4 * temperature_raw) >> 17); 
	o = (((((int32_t) c8 * temperature_raw) >> 15) * temperature_raw) >> 4) + (((int32_t) c7 * temperature_raw) >> 3) + ((int32_t)c6 * 0x4000); 
	pressure_actual = (s * pressure_raw + o) >> 14;

	return pressure_actual;
	
	}

void T5403::sensorWait(uint8_t time){
	delay(time);
};

void T5403::communicationBegin()
// Initialize the communication protocol used.  SPI is currently unsupported in 
// the hardware.  If a release comes with hardware support this file will be 
// updated.  All reference to SPI is currently a place holder for future 
// development.
{
	if( _interface == SPI){  // If SPI is selected use SPI begin
	//	SPI.begin();
	}
	else{  // If i2c is selected for communication use i2c begin
		Wire.begin();
	}

}

int8_t T5403::getData(uint8_t location, int16_t* output)
// Communication transport function.  
{

	uint8_t byteLow, byteHigh;
	int16_t _output;
		
	if( _interface == SPI){  
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

int8_t T5403::sendCommand(uint8_t location, uint8_t command){
	if(_interface == SPI){  // If SPI is selected for communication use SPI commands
	/*	SPI.transfer(location);
		SPI.transter(command);
	*/
	}
	else{  // If i2c is selected for communication use i2c commands
		Wire.beginTransmission(T5403_I2C_ADDR); 
		Wire.write(location);
		Wire.write(command);
		Wire.endTransmission();    // stop transmitting
	}
}