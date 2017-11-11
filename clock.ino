//#include <LiquidCrystal.h>
#include <SevSeg.h>
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68
#define PIN_TEMPRATURE 0
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte LT[8] = 
{
 B00111,
 B01111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111
};
byte UB[8] =
{
 B11111,
 B11111,
 B11111,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000
};
byte RT[8] =
{
 B11100,
 B11110,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111
};
byte LL[8] =
{
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B01111,
 B00111
};
byte LB[8] =
{
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B11111,
 B11111,
 B11111
};
byte LR[8] =
{
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11110,
 B11100
};
byte MB[8] =
{
 B11111,
 B11111,
 B11111,
 B00000,
 B00000,
 B00000,
 B11111,
 B11111
};
byte block[8] =
{
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111
};

byte x = 32;

byte digits[10][6] = {
  {0, 1, 2, 3, 4, 5},
  {1, 2, x, 4, 7, 4},
  {6, 6, 2, 3, 4, 4},
  {6, 6, 2, 4, 4, 5},
  {3, 4, 7, x, x, 7},
  {3, 6, 6, 4, 4, 5},
  {0, 6, 6, 3, 4, 5},
  {1, 1, 2, x, x, 7},
  {0, 6, 2, 3, 4, 5},
  {0, 6, 2, x, x, 7},
  };

// loop counter
int count = 0;

SevSeg sevseg; //Instantiate a seven segment controller object
 
void setup()
{
 // assignes each segment a write number
/*
 lcd.createChar(0,LT);
 lcd.createChar(1,UB);
 lcd.createChar(2,RT);
 lcd.createChar(3,LL);
 lcd.createChar(4,LB);
 lcd.createChar(5,LR);
 lcd.createChar(6,MB);
 lcd.createChar(7,block);
*/
 
 // sets the LCD's rows and colums:
// lcd.begin(16, 2);
       
  Wire.begin();
  
  // setDS3231time(10,54,22,5,9,11,17);



  byte numDigits = 4;
  byte digitPins[] = {2, 3, 4, 5};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(20);  
}

void displayDigit(int col, byte x) {
/*
  lcd.setCursor(col,0);
  lcd.write(digits[x][0]);
  lcd.write(digits[x][1]);
  lcd.write(digits[x][2]);
  lcd.setCursor(col,1);
  lcd.write(digits[x][3]);
  lcd.write(digits[x][4]);
  lcd.write(digits[x][5]);
*/
}

void displayTime() {

  byte second, m, h, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &m, &h, &dayOfWeek, &dayOfMonth, &month, &year);

  if (h == 0) {
    h = 12;
  }
  else if (h > 12) {
    h -= 12;
  }

  byte h1 = h / 10;
  byte h2 = h % 10;
  byte m1 = m / 10;
  byte m2 = m % 10;
  if (h1 > 0) {
    displayDigit(0, h1);
  }
  displayDigit(3, h2);
  displayDigit(7, m1);
  displayDigit(10, m2);

//  sevseg.setNumber(/*h1 * 1000 + h2 * 100 + m1 * 10 +*/  m2, 0);
  sevseg.setNumber(123);

  int millies = millis() / 500;
  if (second % 2 == 0)
  {
/*
    lcd.setCursor(6, 0);
    lcd.write('.');
    lcd.setCursor(6, 1);
    lcd.write('.');
*/
  }
  else
  {
/*
    lcd.setCursor(6, 0);
    lcd.write(' ');
    lcd.setCursor(6, 1);
    lcd.write(' ');
*/
  }
}

void loop()
{
  displayTime();
//  displayTemprature();
//  delay(1000); // every second

  sevseg.refreshDisplay(); // Must run repeatedly
}

void displayTemprature()
{
  int reading = analogRead(PIN_TEMPRATURE);  
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  float tempC = (voltage - 0.5) * 100 ;
  int t = (int) tempC;
  byte t1 = t / 10;
  byte t2 = t % 10;
  
/*
  lcd.setCursor(14, 1);
  lcd.print(t1);
  lcd.setCursor(15, 1);
  lcd.print(t2);
*/
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
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

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
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

