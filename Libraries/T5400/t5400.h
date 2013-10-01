/*
  t5400.h - Library for T5400 barometric pressure sensor.
  Created by Casey Kuhns.
  Released into the public domain.
*/
#ifndef t5400_h
#define t5400_h

#include "Arduino.h"

#define I2C 0
#define SPI 1

#define T5400_I2C_ADDR	0x77

#define COMMAND_GET_TEMP	0x03
	
#define T5400_COMMAND_REG	0xF1
#define T5400_DATA_REG		0xF5

#define T5400_C1		0x8E
#define T5400_C2		0x90
#define T5400_C3		0x92
#define T5400_C4		0x94
#define T5400_C5		0x96
#define T5400_C6		0x98
#define T5400_C7		0x9A
#define T5400_C8		0x9C

#define FAHRENHEIT		1
#define CELSIUS 		0

#define MODE_LOW		0x00
#define MODE_STANDARD	0x01
#define MODE_HIGH		0x10
#define MODE_ULTRA		0x11

class T5400
{
	public:
		T5400(uint8_t interface);
		void init(void);
		int16_t getTemperature(uint8_t units);
		int32_t getPressure(uint8_t precision);

	private:

		uint8_t _interface; // Choose I2C or SPI

		// Constants filled in with T5400();
		uint16_t c1, c2, c3, c4;		
		int16_t c5, c6, c7, c8;
		
		void sensorWait(uint8_t time);
		
		void communicationBegin();
		int8_t getData(uint8_t location, int16_t* output);
		int8_t sendCommand(uint8_t location, uint8_t command);

};

#endif