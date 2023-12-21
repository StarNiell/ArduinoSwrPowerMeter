# Description
This project help you to build a Power and Swr Meter (for HF) using Arduino Uno R3 (or compatible) and cheap board SWR Bridge v.1.4 available on Amazon or Ali Express for about 5$

## Component List
* Arduino Uno R3 (or Nano, all compatible device)
* Display LCD I2C 16 columns 2 rows
* SWR Bridge v1.4 (cheap board)
* 2 PL connectors
* 2 100K resistors (Between FWD -> GND and REV -> GND)
* Metal box

## Connections
* VCC and GND of the I2C Display to Arduino standard Pins
* SDA of the I2C Display to Arduino A4 analog pin
* SCL of the I2C Display to Arduino A5 analog pin
* FWD of SWR Bridge to Arduino A6 analog pin (with a 100K resistor to GND)
* REV of SWR Bridge to Arduino A7 analog pin (with a 100K resistor to GND)
* Both PL connectors on IN and OUT of SWR Bridge v1.4

**NOTE: The IN and OUT of the SWR Bridge v.1.4 are inverted!! This is a bug of the cheap board!!!**

## Current Volt <-> Watt Table
* 1 watt = 0.90 volt
* 2 watt = 1.16 volt
* 5 watt = 1.90 volt
* 6 watt = 2.00 volt
* 7 watt = 2.15 volt
* 10 watt = 2.40 volt
* 15 watt = 2.80 volt
* 20 watt = 3.35 volt
* 25 watt = 3.80 volt
* 30 watt = 4.23 volt
* 40 watt = 4.65 volt
  
This values are valid using a 100K resistor between FWD (and REV) and GND!

You can write your own table by measuring the volt values between FWD and GND with a voltmeter and read the corrisponfance power in watt on the external Power Meter.
The table is stored in `rwatt[]` array and `rvolt[]` array of the arduino code.

**NOTE: You can use a divider for increment the scale**
