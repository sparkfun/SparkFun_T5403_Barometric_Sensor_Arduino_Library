/*

  T5403 barometric sensor test program
  
  This program demonstrates basic functionality of the sensor

  Copyright (c) 2013 SparkFun Electronics. http://sparkfun.com LGPL 3.0

 */

#include <Wire.h>
#include <t5403.h>

T5403 barometer(I2C);

float temperature_c, temperature_f;
int pressure_pa;

void setup() {
    Serial.begin(9600);
    barometer.init();
}

void loop() {
  
  // Gather values from sensor
  temperature_c = barometer.getTemperature(CELSIUS);
  temperature_f = barometer.getTemperature(FAHRENHEIT);
  pressure_pa  = barometer.getPressure(MODE_ULTRA);
  
  //Convert absolute pressure to equivalent pressure (mb) at sea level.
  // Given a pressure Pa (mb) taken at a specific altitude (meters),
  // This produces pressure readings that can be used for weather measurements.
  
  // Report values
  Serial.print("Temperature C = ");
  Serial.println(temperature_c / 100);  
  Serial.print("Temperature F = ");
  Serial.println(temperature_f / 100);  
  Serial.print("Pressure Pa = ");
  Serial.println(pressure_pa);  
  
  delay(1000);
}
