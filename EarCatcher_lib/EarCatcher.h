#ifndef EarCatcher_h
#define EarCatcher_h

#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_NeoPixel.h"
#include "LiquidCrystal_PCF8574.h"
#define NUM_SAMPLES 10
#define MAX_TRACK 5

  class EarCatcherGeneral
{
	public:
	 float readVolt(int pin);
	 void sendCommandsToSlaves(byte command[]);
	 void stripPixelChange(Adafruit_NeoPixel strip, byte pixel, bool UP );
	 void stripSetup(Adafruit_NeoPixel strip[],int brightnesss);
	 int changeSong (int track, bool UP);
	 void scoreDisplayUpdate(LiquidCrystal_PCF8574 lcd,int high, int last);
	 void mainDisplayUpdateHelper(LiquidCrystal_PCF8574 lcd,int track, int volume,bool isPaused);
	 void setupDisplay(LiquidCrystal_PCF8574 lcd);
	 int workingTurbines(float voltage[], float max_voltage);
	
 };
 

#endif