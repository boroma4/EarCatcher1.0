#include <Adafruit_NeoPixel.h>

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
#define PIN 6
#define LEDNUM 6
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);

#define COOLDOWN 5
#define SCORE_DIVIDER 2
#define SCORE_LIMIT 201

byte command[] = {0, 0, 0, 0};
byte trackNum = 1;
byte pixel[] = {0, 0, 0, 0};

int address = 0;
int trackVol = 0;
int workingTurbines = -1;
int lastVol = 4;
int blowingCount = 0;
int highscoreCount = 0;
int lastScore = 0;
int highscore = 0;
int cooldown[] = {COOLDOWN, COOLDOWN, COOLDOWN, COOLDOWN};

float voltage[] = {0, 0, 0, 0};
bool wasBlown[] = {false, false, false, false};

bool signalRec = false;
bool bLcdSetup = false;
bool cheatMode = false;
bool wasSwitched = false;
bool isPaused = false;
bool updateLast = false;


const int RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;


LiquidCrystal_PCF8574 lcd(0x3F); // defining display through I2C
LiquidCrystal_PCF8574 lcd2(0x27); // defining display through I2C
ReadValues volts ; // object of ReadValues class EarCatcher lib

void setup()
{
  strip_setup();
  //  Start the I2C Bus as Master
  Wire.begin();
  Serial.begin(9600); //Serial Com for debugging
  lcd.init();                      // display initialisation
  lcd.setBacklight(255);
  lcd.begin(16, 2);
  lcd2.init();
  lcd2.setBacklight(255);
  lcd2.begin(16, 2);
  irrecv.enableIRIn();
  irrecv.blink13(true);

  EEPROM.get(address, highscore);
  address += sizeof(int);
  EEPROM.get(address, highscoreCount);
  address = 0;
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("HIGH:");
  lcd2.print(highscore);
  if (highscore != 1) {
    lcd2.print(" points");
  }
  else {
    lcd2.print(" point");
  }
  lcd2.setCursor(0, 1);
  lcd2.print("LAST: ");
  lcd2.print(lastScore);
  if (highscore != 1) {
    lcd2.print(" points");
  }
  else {
    lcd2.print(" point");
  }
}

void(* resetFunc) (void) = 0;

void loop()
{
  Wire.beginTransmission(1); // transmit to device #1
  Wire.write(command[0]);
  Wire.endTransmission(false);

  Wire.beginTransmission(2); // transmit to device #2
  Wire.write(command[1]);
  Wire.endTransmission(false);

  Wire.beginTransmission(3); // transmit to device #3
  Wire.write(command[2]);
  Wire.endTransmission(false);

  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(command[3]);
  Wire.endTransmission();
  /*
     SEND COMMAND TO OTHER ARDUINOS HERE

  */
  delay(100);

  if (command[0] == 6) // resetting master arduino
  {
    resetFunc();
  }

  voltage[0] =  volts.readVolt(A4); // returns voltage
  voltage[1] = volts.readVolt(A15);
  voltage[2] =  volts.readVolt(A6); // returns voltage
  voltage[3] = volts.readVolt(A9);

  if (!bLcdSetup) // Update display info
  {
    display_update(); // ONLY SLAVE ONE UPDATES DISPLAY WITH VOLUME AND TRACK
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
  // adding them in powerCheck
  if (!isPaused and !cheatMode and (powerCheck(voltage[0], VOLTAGE_LVL) or powerCheck(voltage[1], VOLTAGE_LVL) or powerCheck(voltage[2], VOLTAGE_LVL) or powerCheck(voltage[3], VOLTAGE_LVL) ))
  {
    workingTurbines = wTurbines();
    switch (workingTurbines)
    {
      case 1:
        blowingCount++;
        break;

      case 2:
        blowingCount += 2;
        break;

      case 3:
        blowingCount += 3;
        break;

      case 4:
        blowingCount += 4;
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
        EEPROM.put(address, highscore);
        address += sizeof(int);
        EEPROM.put(address, highscoreCount);
        address = 0;
      }
      else
      {
        highscore = 0;
      }
    }
    lcd2.setCursor(0, 0);
    lcd2.print("HIGH:");             // Установка курсора в начало второй строки
    lcd2.print(highscore);
    if (highscore != 1) {
      lcd2.print(" points");
    }
    else {
      lcd2.print(" point");
    }
    switch (workingTurbines)
    {
      case 1:
        lcd.setCursor(12, 1);
        lcd.print(" x1");
        break;

      case 2:
        lcd.setCursor(12, 1);
        lcd.print(" x2");
        break;

      case 3:
        lcd.setCursor(12, 1);
        lcd.print(" x3");
        break;

      case 4:
        lcd.setCursor(12, 1);
        lcd.print(" x4");
        break;

      default: break;
    }
    lcd2.setCursor(0, 1);
    lcd2.print("LAST: ");
    lcd2.print(lastScore);
    if (lastScore != 1) {
      lcd2.print(" points");
    }
    else
    {
      lcd2.print(" point");
    }
  }

  else
  {
    strip1.setBrightness(0);
    strip1.show();

    blowingCount = 0;
    updateLast = true;
    lcd2.setCursor(0, 0);
    lcd2.print("HIGH:");
    lcd2.print(highscore);
    if (highscore != 1) {
      lcd2.print(" points");
    }
    else {
      lcd2.print(" point");
    }
    lcd.setCursor(12, 1);
    lcd.print(" x0");
    lcd2.setCursor(0, 1);
    lcd2.print("LAST: ");
    lcd2.print(lastScore);
    if (lastScore != 1) {
      lcd2.print(" points");
    }
    else
    {
      lcd2.print(" point");
    }
  }

  // COMMON COMMANDS FOR ALL SLAVES

  if (irrecv.decode(&results)) {

    switch (results.value)
    {
      case 0xFF02FD:

        Serial.println ("next track");
        for (int i = 0; i < 3; i++)
        {
          command[i] = 4;
        }
        bLcdSetup = false;
        songUp();
        irrecv.resume();
        return;

      case 0xFF22DD:

        Serial.println ("previous track");
        for (int i = 0; i < 3; i++)
        {
          command[i] = 5;
        }
        bLcdSetup = false;
        songDown();
        irrecv.resume();
        return;

      // reset highscore
      case 0xFF5AA5 :

        highscore = 0;
        highscoreCount = 0;
        EEPROM.put(address, highscore);
        address += sizeof(int);
        EEPROM.put(address, highscoreCount);
        address = 0;

        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("HIGH:");
        lcd2.print(highscore);
        lcd2.print(" points");
        lcd2.setCursor(0, 1);
        lcd2.print("LAST: ");
        lcd2.print(lastScore);
        if (lastScore != 1) {
          lcd2.print(" points");
        }
        else {
          lcd2.print(" point");
        }
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
        for (int i = 0; i < 3; i++)
        {
          command[i] = 7;
        }

        if (lastVol < 5)
        {
          lastVol++;
        }
        bLcdSetup = false;
        irrecv.resume();
        return;

      case 0xFFE01F :

        Serial.println ("Volume down!");
        for (int i = 0; i < 3; i++)
        {
          command[i] = 8;
        }
        if (lastVol > 1)
        {
          lastVol--;
        }
        bLcdSetup = false;
        irrecv.resume();

        return;

      // PAUSE
      case 0xFFC23D :
        for (int i = 0; i < 3; i++)
        {
          command[i] = 9;
        }
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

        for (int i = 0; i < 3; i++)
        {
          command[i] = 6;
        }
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
  uint32_t yellow = strip1.Color(255, 255, 0);
  uint32_t blank = strip1.Color(226, 226, 226);

  if (!(powerCheck(voltage[0], VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown[0])  // if used to turn
    {
      if (cooldown[0] > 0) // if cooldown time wasnt reached
      {
        cooldown[0] -= 1;
        command[0] = 2;
        Serial.println("Turbine 1 cooling down...");
        if (pixel[0] > 0)
        {
          strip1.setPixelColor(pixel[0], blank);
          strip1.show();
          pixel[0] --;
        }
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 1 cooled down...");
        wasBlown[0] = false; // escape these conditions
      }
    }
    else
    {
      command[0] = 0; // act as usually
    }

  }
  if (powerCheck(voltage[0], VOLTAGE_LVL) ) // if u need to unmute track 1
  {

    wasBlown[0] = true;// was moved
    cooldown[0] = COOLDOWN;
    command[0] = 2;
    if (pixel[0] < strip1.numPixels())
    {
      strip1.setPixelColor(pixel[0], yellow);
      strip1.show();
      pixel[0] ++;
    }
  }

  /*
      FAN N2 ( pin A15)
  */
  if (!(powerCheck(voltage[1], VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown[1])  // if used to turn
    {
      if (cooldown[1] > 0) // if cooldown time wasnt reached
      {
        cooldown[1] -= 1;
        command[1] = 2;
        Serial.println("Turbine 2 cooling down...");
        if (pixel[1] > 0)
        {
          strip2.setPixelColor(pixel[1], blank);
          strip2.show();
          pixel[1] --;
        }
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 2 cooled down...");
        wasBlown[1] = false; // escape these conditions
      }
    }
    else
    {
      command[1] = 0; // act as usually
    }
  }
  if (powerCheck(voltage[1], VOLTAGE_LVL) ) // if u need to unmute track 1
  {
    wasBlown[1] = true;
    cooldown[1] = COOLDOWN;
    command[1] = 2;
    if (pixel[1] < strip1.numPixels())
    {
      strip2.setPixelColor(pixel[1], yellow);
      strip2.show();
      pixel[1] ++;
    }
  }

  /*
      FAN N3 ( pin A6)
  */
  if (!(powerCheck(voltage[2], VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown[2])  // if used to turn
    {
      if (cooldown[2] > 0) // if cooldown time wasnt reached
      {
        cooldown[2] -= 1;
        command[2] = 2;
        Serial.println("Turbine 3 cooling down...");
        if (pixel[2] > 0)
        {
          strip3.setPixelColor(pixel[2], blank);
          strip3.show();
          pixel[2] --;
        }
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 3 cooled down...");
        wasBlown[2] = false; // escape these conditions
      }
    }
    else
    {
      command[2] = 0; // act as usually
    }
  }
  if (powerCheck(voltage[2], VOLTAGE_LVL) ) // if u need to unmute track 1
  {
    wasBlown[2] = true;
    cooldown[2] = COOLDOWN;
    command[2] = 2;
    if (pixel[2] < strip1.numPixels())
    {
      strip3.setPixelColor(pixel[2], yellow);
      strip3.show();
      pixel[2] ++;
    }
  }
  /*
        FAN N4 ( pin A9)
  */
  if (!(powerCheck(voltage[3], VOLTAGE_LVL))) // if u need to mute track n1
  {
    if (wasBlown[3])  // if used to turn
    {
      if (cooldown[3] > 0) // if cooldown time wasnt reached
      {
        cooldown[3] -= 1;
        command[3] = 2;
        Serial.println("Turbine 4 cooling down...");
        if (pixel[3] > 0)
        {
          strip4.setPixelColor(pixel[3], blank);
          strip4.show();
          pixel[3] --;
        }
      }
      else // if cooldown time was reached
      {
        Serial.println("Turbine 4 cooled down...");
        wasBlown[3] = false; // escape these conditions
      }
    }
    else
    {
      command[3] = 0; // act as usually
    }
  }
  if (powerCheck(voltage[3], VOLTAGE_LVL) ) // if u need to unmute track 1
  {
    wasBlown[3] = true;
    cooldown[3] = COOLDOWN;
    command[3] = 2;
    if (pixel[3] < strip1.numPixels())
    {
      strip4.setPixelColor(pixel[3], yellow);
      strip4.show();
      pixel[3] ++;
    }
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

int wTurbines ()
{
  int turbines = 0;
  for (int i; i < 3; i++)
  {
    if (powerCheck(voltage[i], VOLTAGE_LVL));
    {
      turbines++;
    }
  }
  return turbines;
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

void strip_setup ()
{
  strip1.begin();
  strip1.show(); // Initialize all pixels to 'off'
  strip1.setBrightness(6);
  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'
  strip2.setBrightness(6);
  strip3.begin();
  strip3.show(); // Initialize all pixels to 'off'
  strip3.setBrightness(6);
  strip4.begin();
  strip4.show(); // Initialize all pixels to 'off'
  strip4.setBrightness(6);
}
