#include "EarCatcher.h"

float EarCatcherGeneral::readVolt(int pin) 
{
	int sum = 0;  
	unsigned char sample_count = 0;
    float voltage = 0;	

  while (sample_count < NUM_SAMPLES)
  {
    sum += analogRead(pin);
    sample_count++;
    delay(10);
    voltage = ((float)sum / (float)NUM_SAMPLES * 5.015) / 1024.0;
  }
  sample_count = 0;
  sum = 0;
  return voltage;
}

void EarCatcherGeneral::sendCommandsToSlaves(byte command[])
{
	int address = 1, i = 0;
	for(address; address < 4; address++)
	{
	Wire.beginTransmission(address); // transmit to device #1
    Wire.write(command[i]);
	Wire.endTransmission(false);
	i++;
	}
	Wire.beginTransmission(address); // transmit to device #1
    Wire.write(command[i]);
	Wire.endTransmission();
}

 void EarCatcherGeneral::stripPixelChange(Adafruit_NeoPixel strip, byte pixel, bool UP )
 {
	 uint32_t yellow = strip.Color(255, 255, 0);
     uint32_t blank = strip.Color(226, 226, 226);
	 if(!UP && pixel > 0)
	 {
          strip.setPixelColor(pixel, blank);
          strip.show();
          pixel --;
	 }
	 if(UP && pixel < strip.numPixels())
	 {
		 strip.setPixelColor(pixel,yellow);
         strip.show();
         pixel ++; 
	 }	 	 
 }

void EarCatcherGeneral::stripSetup(Adafruit_NeoPixel strip[],int brightnesss)
{
	for (int i = 0; i < 4; i++)
	{
	strip[i].begin();
	strip[i].show();
	strip[i].setBrightness(brightnesss);
	}
}

 int EarCatcherGeneral::changeSong (int track, bool UP)
 {
	if(UP)
	{
		track < MAX_TRACK ? track ++ : track = 1; 
    }
	else
	{
		track > 1 ? track -- : track = MAX_TRACK;
	} 
	return track;
 }

 void EarCatcherGeneral::scoreDisplayUpdate(LiquidCrystal_PCF8574 lcd,int high, int last)
 {
  lcd.setCursor(0, 0);
  lcd.print("HIGH:");
  lcd.print(high);
  high != 1 ? lcd.print(" points") : lcd.print(" point");
  lcd.setCursor(0, 1);
  lcd.print("LAST: ");
  lcd.print(last);
  last != 1 ? lcd.print(" points") : lcd.print(" point");
 }
 
void EarCatcherGeneral::mainDisplayUpdateHelper(LiquidCrystal_PCF8574 lcd,int track, int volume,bool isPaused)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Song 0");
  lcd.print(track);
  !isPaused ? lcd.print(" playing") : lcd.print(" paused");
  lcd.setCursor(0, 1);
  lcd.print("Volume: 0");
  lcd.print(volume);
}

void EarCatcherGeneral::setupDisplay(LiquidCrystal_PCF8574 lcd)
{
  lcd.init();                      
  lcd.setBacklight(255);
  lcd.begin(16, 2);	
}

int EarCatcherGeneral::workingTurbines(float voltage[], float max_voltage)
{
  int turbines = 0;
  for (int i; i < 4; i++)
  {
    if (voltage[i] > max_voltage);
    {
      turbines++;
    }
  }
  return turbines;
}
