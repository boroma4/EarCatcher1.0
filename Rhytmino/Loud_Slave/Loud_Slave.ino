#include <SD.h>

#include <SPI.h>
#include <pcmConfig.h>
#include <pcmRF.h>
#include <TMRpcm.h>
#include <Wire.h>
#define SD_ChipSelectPin 4
#define MAX_SONGS 4


bool trackstart = false;
bool isPaused = false;
TMRpcm music;
char *songs[]  = { "rhytm.wav", "bells.wav", "rhytm3.wav", "rhytm4.wav", "rhytm5.wav"};
byte volumeCount = 3;
byte songCount = 0;
byte command = 0;
const int Reset PROGMEM = 8;

void setup()
{  
  Serial.begin(9600);
  music.speakerPin = 9;
  Wire.begin(3);
  Wire.onReceive(receiveEvent);
  if (!SD.begin(SD_ChipSelectPin))
  {
    Serial.println(F("SD fail"));
    return;
  }
  else
  {
    Serial.println(F("SD sucess"));
  }
  music.quality(1);
  music.loop(1);

}



void receiveEvent(int bytes)
{
  command  = Wire.read();
  Serial.print("I recieved ");// read one character from the I2C
  Serial.println(command);
  if(command == 6)
  {
    music.stopPlayback();
    volumeCount = 3;
    songCount = 0;
    trackstart = false;
    delay(100);
    if(isPaused)
    {
      music.pause();
    }
    return;
  }
  if (!trackstart && !isPaused)
  {
    trackstart = true;
    Serial.print("Time to start... ");
    music.play(songs[songCount]);
  }
  if (command == 7 or command == 8)
  {
    if (command == 7)
    {
      if (volumeCount < 4)
      {
        volumeCount++;
      }
    }
    if (command == 8)
    {
      if (volumeCount > 1)
      {
        volumeCount--;
      }
    }
    return;
  }
  
  if (command == 9)
  {
    if(trackstart)
    {
    music.pause();
    }
    if(!isPaused)
    {
      isPaused = true;
    }
    else
    {
      isPaused = false;
    }
    return;
  }
  
  if (command == 4 or command == 5)
  {
    music.disable();
    delay(500);
    if (command == 4)
    {
      songUp();
    }
    if (command == 5)
    {
      songDown();
    }
    return;
  }
  if (trackstart)
  {
    if (command == 0)
    {
      mute();
      Serial.println(F("Muted..."));
    }
    if (command == 2 or command == 1 )
    {
      music.setVolume(volumeCount);
      Serial.println(F("Unpaused..."));
    }
  }
}

void loop()
{
  Serial.println(command);
  delay(100);
}

void mute ()
{
  music.setVolume(1);

}

void songUp()
{
  if (songCount < MAX_SONGS)
  {
    songCount++;
    trackstart = false;
  }
  else
  {
    songCount = 0;
    trackstart = false;
  }
}
void songDown()
{
  if (songCount > 0) {
    songCount--;
    trackstart = false;
  }
  else
  {
    songCount = MAX_SONGS;
    trackstart = false;
  }
}
