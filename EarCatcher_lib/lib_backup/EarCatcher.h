#ifndef EarCatcher_h
#define EarCatcher_h

#include "Arduino.h"
#define NUM_SAMPLES 10

  class EarCatcher
{
	public:
	 ReadValues();
	 float readVolt(int pin);
	 
	 
	private:
	int sum = 0;  
	unsigned char sample_count = 0;
    float voltage1 = 0;	
	float voltage2 = 0;
	
 };
 
 class Remote
{
	
 };

#endif