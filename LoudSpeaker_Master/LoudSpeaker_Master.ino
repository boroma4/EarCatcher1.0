#include <EEPROM.h>

#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#include <LiquidCrystal_PCF8574.h>
#include <EarCatcher.h>
#include <Wire.h>


#define MAX_SONGS 5
#define VOLTAGE_LVL 0.15
#define COOLDOWN 5

byte command1 = 0;
byte command2 = 0;
byte command3 = 0;
byte command4 = 0;
byte trackNum = 1;
byte mute = 0;

int trackVol = 0;
int lastVol = 4;
int blowingCount = 0;
int highscoreCount = 0;
int highscore = 0;
int cooldown1 = COOLDOWN;
int cooldown2 = COOLDOWN;
int cooldown3 = COOLDOWN;
int cooldown4 = COOLDOWN;


float voltage1 = 0;
float voltage2 = 0;
float voltage3 = 0;
float voltage4 = 0;


bool signalRec = false;
bool bLcdSetup = false;
bool cheatMode = false;
bool begincom = false;
bool wasSwitched = false;
bool isMuted1 = false;
bool isPaused = false;
bool display_cycleOne = false;
bool display_cycleTwo = false;
bool wasBlown1 = false;
bool wasBlown2 = false;
bool wasBlown3 = false;
bool wasBlown4 = false;


const int RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;


LiquidCrystal_PCF8574 lcd(0x3F); // defining display through I2C
LiquidCrystal_PCF8574 lcd2(0x27); // defining display through I2C
ReadValues volts ; // object of ReadValues class EarCatcher lib

void setup()
{
  //  Start the I2C Bus as Master
  Wire.begin();
  Serial.begin(9600); //Serial Com for debugging
  lcd.init();                      // Инициализация дисплея
  lcd.setBacklight(255);
  lcd.begin(16, 2);
  lcd2.init();                      // Инициализация дисплея
  lcd2.setBacklight(255);
  lcd2.begin(16, 2);
  irrecv.enableIRIn();
  irrecv.blink13(true);

  EEPROM.get (0, highscore);
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("HIGHSCORE:");
  lcd2.setCursor(4, 1); // Установка курсора в начало второй строки
  lcd2.print(highscore);
  if (highscore != 1) {
    lcd2.print(" points");
  }
  else {
    lcd2.print(" point");
  }
  // Подключение подсветки
}

void(* resetFunc) (void) = 0;

void loop()
{
  Wire.beginTransmission(1); // transmit to device #9
  Wire.write(command1);
  Wire.endTransmission(false);

  Wire.beginTransmission(2); // transmit to device #9
  Wire.write(command2);
  Wire.endTransmission(false);

  Wire.beginTransmission(3); // transmit to device #9
  Wire.write(command3);
  Wire.endTransmission(false);

  Wire.beginTransmission(4); // transmit to device #9
  Wire.write(command4);
  Wire.endTransmission();
  /*
     SEND COMMAND TO OTHER ARDUINOS HERE

  */
  delay(100);

  if (command1 == 6) // resetting master arduino
  {
    resetFunc();
  }

  voltage1 =  volts.readVolt(A4); // returns voltage
  voltage2 = volts.readVolt(A15);
  voltage3 =  volts.readVolt(A6); // returns voltage
  voltage4 = volts.readVolt(A9);

  if (!bLcdSetup) // Update display info
  {
    display_update(); // ONLY SLAVE ONE UPDATES DISPLAY WITH VOLUME AND TRACK
  }
  if (cheatMode) // Cheats
  {
    voltage1 = 999;
    voltage2 = 999;
    voltage3 = 999;
    voltage4 = 999;
  }

  /*
     HIGHSCORE CHECK
  */

  if (!isPaused and !cheatMode and (powerCheck(voltage1, VOLTAGE_LVL) or powerCheck(voltage2, VOLTAGE_LVL) or powerCheck(voltage3, VOLTAGE_LVL) or powerCheck(voltage4, VOLTAGE_LVL) ))
  {
    blowingCount++;
  }
  else
  {
    if (blowingCount > highscoreCount)
    {
      highscoreCount = blowingCount;
      if (highscoreCount > 2) {
        highscore = highscoreCount / 2;
      }
      lcd2.clear();
      lcd2.setCursor(0, 0);
      lcd2.print("HIGHSCORE:");             // Установка курсора в начало второй строки

      lcd2.setCursor(4, 1);
      lcd2.print(highscore);
      if (highscore != 1) {
        lcd2.print(" points");
      }
      else {
        lcd2.print(" point");
      }
      EEPROM.put (0, highscore);
    }
    blowingCount = 0;
  }

  // COMMON COMMANDS FOR ALL SLAVES

  if (irrecv.decode(&results)) {

    switch (results.value)
    {
      case 0xFF02FD:

        Serial.println ("next track");
        command1 = 4;
        command2 = 4;
        command3 = 4;
        command4 = 4;
        bLcdSetup = false;
        songUp();
        irrecv.resume();
        return;

      case 0xFF22DD:

        Serial.println ("previous track");
        command1 = 5;
        command2 = 5;
        command3 = 5;
        command4 = 5;
        bLcdSetup = false;
        songDown();
        irrecv.resume();
        return;

      // reset highscore
      case 0xFF5AA5 :

        EEPROM.put(0, 0);
        highscore = 0;
        highscoreCount = 0;
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("HIGHSCORE:");             // Установка курсора в начало второй строки
        lcd2.setCursor(4, 1);
        lcd2.print(highscore);
        lcd2.print(" points");
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
        command1 = 7;
        command2 = 7;
        command3 = 7;
        command4 = 7;

        if (lastVol < 5)
        {
          lastVol++;
        }
        bLcdSetup = false;
        irrecv.resume();
        return;

      case 0xFFE01F :

        Serial.println ("Volume down!");
        command1 = 8;
        command2 = 8;
        command3 = 8;
        command4 = 8;
        if (lastVol > 1)
        {
          lastVol--;
        }
        bLcdSetup = false;
        irrecv.resume();

        return;

      // PAUSE
      case 0xFFC23D :
        command1 = 9;
        command2 = 9;
        command3 = 9;
        command4 = 9;
        bLcdSetup = false;

        if (!isPaused)
        {
          isPaused = true;
        }
        else
        {
          isPaused = false;
        }
        irrecv.resume();

        return;


      case 0xFF52AD : // reset function

        command1 = 6;
        command2 = 6;
        command3 = 6;
        command4 = 6;
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

  if (!(powerCheck(voltage1, VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown1)  // if used to turn
    {   
      if (cooldown1 > 0) // if cooldown time wasnt reached
      {
        cooldown1 -= 1;
        command1 = 2;
        Serial.println("Turbine 1 cooling down...");
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 1 cooled down...");
        wasBlown1 = false; // escape these conditions
      }
    }
    else
    {
      command1 = 0; // act as usually
    }
    
  }
  if (powerCheck(voltage1, VOLTAGE_LVL) ) // if u need to unmute track 1
  {

    wasBlown1 = true;// was moved
    cooldown1 = COOLDOWN;
    command1 = 2;
  }

  /*
      FAN N2 ( pin A15)
  */
  if (!(powerCheck(voltage2, VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown2)  // if used to turn
    {    
      if (cooldown2 > 0) // if cooldown time wasnt reached
      {
        cooldown2 -= 1;
        command2 = 2;
        Serial.println("Turbine 2 cooling down...");
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 2 cooled down...");
        wasBlown2 = false; // escape these conditions
      }
    }
    else
    {
      command2 = 0; // act as usually
    }
  }
  if (powerCheck(voltage2, VOLTAGE_LVL) ) // if u need to unmute track 1
  {
    wasBlown2 = true;
    cooldown2 = COOLDOWN;
    command2 = 2;
  }

  /*
      FAN N3 ( pin A6)
  */
  if (!(powerCheck(voltage3, VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown3)  // if used to turn
    {    
      if (cooldown3 > 0) // if cooldown time wasnt reached
      {
        cooldown3 -= 1;
        command3 = 2;
        Serial.println("Turbine 3 cooling down...");
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 3 cooled down...");
        wasBlown3 = false; // escape these conditions
      }
    }
    else
    {
      command3 = 0; // act as usually
    }
  }
  if (powerCheck(voltage3, VOLTAGE_LVL) ) // if u need to unmute track 1
  {
    wasBlown3 = true;
    cooldown3 = COOLDOWN;
    command3 = 2;
  }
  /*
        FAN N4 ( pin A9)
  */
  if (!(powerCheck(voltage4, VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown4)  // if used to turn
    {    
      if (cooldown4 > 0) // if cooldown time wasnt reached
      {
        cooldown4 -= 1;
        command4 = 2;
        Serial.println("Turbine 4 cooling down...");
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 4 cooled down...");
        wasBlown4 = false; // escape these conditions
      }
    }
    else
    {
      command4 = 0; // act as usually
    }
  }
  if (powerCheck(voltage4, VOLTAGE_LVL) ) // if u need to unmute track 1
  {
    wasBlown4 = true;
    cooldown4 = COOLDOWN;
    command4 = 2;
  }

}

// SUPPORT FUNCTIONS
bool powerCheck (float vol, float level) // comparing read voltage to barrier one
{
  if ( vol < level)
  {
    signalRec = false;
  }
  else
  {
    signalRec = true;
  }
  return signalRec;
}

void songDown()
{
  if (trackNum > 1)
  {
    trackNum--;
  }
  else
  {
    trackNum = MAX_SONGS;
  }
}

void songUp()
{
  if (trackNum < MAX_SONGS)
  {
    trackNum++;
  }
  else
  {
    trackNum = 1;
  }
}
void display_update()
{
  trackVol = trackVol + lastVol;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Song 0");
  lcd.print(trackNum);
  if (!isPaused) {
    lcd.print(" playing");
  }
  else
  {
    lcd.print(" paused");
  }

  lcd.setCursor(0, 1);             // Установка курсора в начало второй строки
  lcd.print("Volume: 0");
  lcd.print(trackVol);

  bLcdSetup = true;
  trackVol = 0;
}
