// DS3231 "library"

#ifndef DS3231_h
#define DS3231_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "Wire.h"

// CLOCK
#define DS3231_I2C_ADDRESS 0x68
#define DS3231_TEMPERATURE_ADDR     0x11

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);

  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

// temperature
void readDS3231temperature(byte *temp) {
  uint8_t temp_msb, temp_lsb;
  int8_t nint;

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_TEMPERATURE_ADDR);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
  temp_msb = Wire.read();
  temp_lsb = Wire.read() >> 6;

  if ((temp_msb & 0x80) != 0)
    nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
  else
    nint = temp_msb;

  *temp = (byte) (0.25 * temp_lsb + nint);
}

// Set system time
void updateTime() {
  byte sec, min, hr, wd, dy, mon, yr;
  
  readDS3231time(&sec, &min, &hr, &wd, &dy, &mon, &yr);
  setTime(hr, min, sec, dy, mon, 2000 + yr);
}

#endif
