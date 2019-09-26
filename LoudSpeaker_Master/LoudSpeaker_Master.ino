#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <LiquidCrystal_PCF8574.h>
#include <EarCatcher.h>
#include <Wire.h>

#define VOLTAGE_LVL 0.15
#define LEDNUM 6
#define BRIGHTNESS 6
#define COOLDOWN 5
#define SCORE_DIVIDER 2
#define SCORE_LIMIT 201
#define RECV_PIN 5
#define UP true
#define DOWN false
const int PIN[] = {6, 8, 10, 12};

Adafruit_NeoPixel strip[] =
{ Adafruit_NeoPixel(LEDNUM, PIN[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LEDNUM, PIN[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LEDNUM, PIN[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LEDNUM, PIN[3], NEO_GRB + NEO_KHZ800)
};

byte trackNum = 1;
byte pixel[] = {0, 0, 0, 0};
byte command[] = {0, 0, 0, 0};

int address = 0;
int trackVol = 0;
int workingTurbines = -1;
int lastVol = 4;
int blowingCount = 0;
int highscoreCount = 0;
int lastScore = 0;
int highscore = 0;
int cooldown[] = {COOLDOWN, COOLDOWN, COOLDOWN, COOLDOWN};
int voltPin[] = {A4, A15, A6, A9};
float voltage[] = {0, 0, 0, 0};

bool wasBlown[] = {false, false, false, false};
bool bLcdSetup = false;
bool cheatMode = false;
bool isPaused = false;
bool updateLast = false;

IRrecv irrecv(RECV_PIN);
decode_results results;
LiquidCrystal_PCF8574 lcd(0x3F); // defining display through I2C
LiquidCrystal_PCF8574 lcd2(0x27); // defining display through I2C
EarCatcherGeneral General ; // object of General class EarCatcher lib

void setup()
{
  General.stripSetup(strip, BRIGHTNESS);
  //  Start the I2C Bus as Master
  Wire.begin();
  Serial.begin(9600); //Serial Com for debugging
  General.setupDisplay(lcd);
  General.setupDisplay(lcd2);
  irrecv.enableIRIn();
  irrecv.blink13(true);

  EEPROM.get(address, highscore);
  address += sizeof(int);
  EEPROM.get(address, highscoreCount);
  address = 0;
  lcd2.clear();
  General.scoreDisplayUpdate(lcd2, highscore, lastScore);
}

void(* resetFunc) (void) = 0;

void loop()
{
  General.sendCommandsToSlaves(command);
  delay(100);

  if (command[0] == 6) // resetting master arduino
  {
    resetFunc();
  }

  readVoltage(); // reads voltage from all turbines

  if (!bLcdSetup) // Update main display info
  {
    main_display_update();
  }
  if (cheatMode) // Cheats
  {
    for (int i = 0; i < 3; i++)
    {
      voltage[i] = 999;
    }
  }

  /*
     HIGHSCORE CHECK
  */
  if (!isPaused and !cheatMode and (voltage[0] > VOLTAGE_LVL or voltage[1] > VOLTAGE_LVL or voltage[2] > VOLTAGE_LVL or voltage[3] > VOLTAGE_LVL ))
  {
    workingTurbines = General.workingTurbines(voltage, VOLTAGE_LVL);
    switch (workingTurbines)
    {
      case 1:
        blowingCount++;
        lcd.setCursor(12, 1);
        lcd.print(" x1");
        break;

      case 2:
        blowingCount += 2;
        lcd.setCursor(12, 1);
        lcd.print(" x2");
        break;

      case 3:
        blowingCount += 3;
        lcd.setCursor(12, 1);
        lcd.print(" x3");
        break;

      case 4:
        blowingCount += 4;
        lcd.setCursor(12, 1);
        lcd.print(" x4");
        break;

      default: break;
    }

    // constant update of Last score
    if (blowingCount >= SCORE_DIVIDER && blowingCount < SCORE_LIMIT)
    {
      if (updateLast)
      {
        lastScore = 0;
        updateLast = false;
      }
      lastScore = blowingCount / SCORE_DIVIDER;
    }
    if (blowingCount < SCORE_DIVIDER)
    {
      lastScore = 0;
    }
    if (blowingCount > highscoreCount && blowingCount < SCORE_LIMIT)
    {
      highscoreCount = blowingCount;
      if (highscoreCount >= SCORE_DIVIDER)
      {
        highscore = highscoreCount / SCORE_DIVIDER;
        updateHighscoreEEPROM();
      }
      else
      {
        highscore = 0;
      }
    }
    General.scoreDisplayUpdate(lcd2, highscore, lastScore);
  }
  else
  {
    blowingCount = 0;
    updateLast = true;
    General.scoreDisplayUpdate(lcd2, highscore, lastScore);
  }

  // REMOTW COMMANDS FOR ALL SLAVES

  if (irrecv.decode(&results)) {

    switch (results.value)
    {
      case 0xFF02FD:

        Serial.println ("next track");
        setSlaveValues(4);
        bLcdSetup = false;
        trackNum =  General.changeSong(trackNum, UP);
        irrecv.resume();
        return;

      case 0xFF22DD:

        Serial.println ("previous track");
        setSlaveValues(5);
        bLcdSetup = false;
        trackNum =  General.changeSong(trackNum, DOWN);
        irrecv.resume();
        return;

      // reset highscore
      case 0xFF5AA5 :

        highscore = 0;
        highscoreCount = 0;
        updateHighscoreEEPROM();

        lcd2.clear();
        General.scoreDisplayUpdate(lcd2, highscore, lastScore);
        irrecv.resume();
        return;

      case 0xFF42BD:

        Serial.println ("Cheat mode activated!");
        cheatMode = true;
        irrecv.resume();
        return;

      case  0xFF4AB5:

        Serial.println ("Cheat mode disabled!");
        cheatMode = false;
        irrecv.resume();
        return;

      case 0xFFA857:

        Serial.println ("Volume up!");
        setSlaveValues(7);
        if (lastVol < 5)
        {
          lastVol++;
        }
        bLcdSetup = false;
        irrecv.resume();
        return;

      case 0xFFE01F :

        Serial.println ("Volume down!");
        setSlaveValues(8);
        if (lastVol > 1)
        {
          lastVol--;
        }
        bLcdSetup = false;
        irrecv.resume();
        return;

      // PAUSE
      case 0xFFC23D :
        setSlaveValues(10);
        bLcdSetup = false;
        isPaused = (!isPaused) ? true : false;
        irrecv.resume();
        return;

      case 0xFF52AD : // reset function

        setSlaveValues(6);
        irrecv.resume();
        return;

      default :
        irrecv.resume();
    }
  }
  // SEPARATE COMMANDS FOR SLAVES

  /*
     FAN N1 ( pin A4)
  */
  PlayOrMuteTheTrack(0);
  /*
      FAN N2 ( pin A15)
  */
  PlayOrMuteTheTrack(1);
  /*
      FAN N3 ( pin A6)
  */
  PlayOrMuteTheTrack(2);
  /*
        FAN N4 ( pin A9)
  */
  PlayOrMuteTheTrack (3);
}

// SUPPORT FUNCTIONS

void main_display_update()
{
  trackVol = trackVol + lastVol;
  General.mainDisplayUpdateHelper(lcd, trackNum, trackVol, isPaused);
  bLcdSetup = true;
  trackVol = 0;
}

void readVoltage()
{
  for (int i = 0; i < 4; i++)
  {
    voltage[i] =  General.readVolt(voltPin[i]); // returns voltage
  }
}
void setSlaveValues(byte value)
{
  for (int i = 0; i < 3; i++)
  {
    command[i] = value;
  }
}

void PlayOrMuteTheTrack (int track)
{
  if (voltage[track] < VOLTAGE_LVL) // if u need to mute track n1
  {
    if (wasBlown[track])  // if used to turn
    {
      if (cooldown[track] > 0) // if cooldown time wasnt reached
      {
        cooldown[track] -= 1;
        command[track] = 2;
        Serial.println("Turbine cooling down...");
        General.stripPixelChange(strip[track], pixel[track], DOWN);
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine cooled down...");
        wasBlown[track] = false; // escape these conditions
      }
    }
    else
    {
      command[track] = 0; // act as usually
    }
  }
  if (voltage[track] > VOLTAGE_LVL ) // if u need to unmute track 1
  {
    wasBlown[track] = true;// was moved
    cooldown[track] = COOLDOWN;
    command[track] = 2;
    General.stripPixelChange(strip[track], pixel[track], UP);
  }
}

void updateHighscoreEEPROM()
{
  EEPROM.put(address, highscore);
  address += sizeof(int);
  EEPROM.put(address, highscoreCount);
  address = 0;
}
