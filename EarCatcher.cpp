
#include "EarCatcher.h"
#include <TMRpcm.h>

TMRpcm music1;


float EarCatcher::readVolt(int pin) 
{
	
	
  while (sample_count < NUM_SAMPLES)
  {
    sum += analogRead(pin);
    sample_count++;
    delay(10);
    voltage1 = ((float)sum / (float)NUM_SAMPLES * 5.015) / 1024.0;
  }
  sample_count = 0;
  sum = 0;
  return voltage1;
}





