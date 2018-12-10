//#include <LiquidCrystal.h>
#include "Wire.h"

#define DS3231_I2C_ADDRESS 0x68

#define PIN_LATCH  8
#define PIN_CLOCK 12
#define PIN_DATA  11
#define PIN_TRANSISTOR  3


byte h1, h2, m1, m2;

// loop counter
int count = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  // setDS3231time(10,54,22,5,9,11,17);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_TRANSISTOR, OUTPUT);  
}

void readTime() {

  byte second, m, h, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &m, &h, &dayOfWeek, &dayOfMonth, &month, &year);

  if (h == 0) {
    h = 12;
  }
  else if (h > 12) {
    h -= 12;
  }

  h1 = h / 10;
  h2 = h % 10;
  m1 = m / 10;
  m2 = m % 10;

/*
  h1 = 1;
  h2 = 8;
  m1 = 8;
  m2 = 8;
*/
}

int a(byte x) {
  return (x == 0) || (x == 2) || (x == 3) || (x == 5) || (x == 6) || (x == 7) || (x == 8) || (x == 9);
}

int b(byte x) {
  return (x == 0) || (x == 1) || (x == 2) || (x == 3) || (x == 4) || (x == 7) || (x == 8) || (x == 9);
}

int c(byte x) {
  return (x == 0) || (x == 1) || (x == 3) || (x == 4) || (x == 5) || (x == 6) || (x == 7) || (x == 8) || (x == 9);
}

int d(byte x) {
  return (x == 0) || (x == 2) || (x == 3) || (x == 5) || (x == 6) || (x == 8) || (x == 9);
}

int e(byte x) {
  return (x == 0) || (x == 2) || (x == 6) || (x == 8);
}

int f(byte x) {
  return (x == 0) || (x == 4) || (x == 5) || (x == 6) || (x == 8) || (x == 9);
}

int g(byte x) {
  return (x == 2) || (x == 3) || (x == 4) || (x == 5) || (x == 6) || (x == 8) || (x == 9);
}

void refreshDisplay()
{
  /* +-----------------+-----------------+-----------------+
   * |      Byte 3     |      Byte 2     |     Byte 1      |
   * | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
   * +-----------------+-----------------+-----------------+
   * | : c b g f e d c | b a g f e d c b | a g f e d c b a |
   * | : 4 4 3 3 3 3 3 | 3 3 2 2 2 2 2 2 | 2 1 1 1 1 1 1 1 |
   * +-----------------+-----------------+-----------------+
   */
  byte b1 = 
      a(m2) << 0
    | b(m2) << 1
    | c(m2) << 2
    | d(m2) << 3
    | e(m2) << 4
    | f(m2) << 5
    | g(m2) << 6
    | a(m1) << 7;
    
  byte b2 = 
      b(m1) << 0
    | c(m1) << 1
    | d(m1) << 2
    | e(m1) << 3
    | f(m1) << 4
    | g(m1) << 5
    | a(h2) << 6
    | b(h2) << 7;
    
  byte b3 = 
      c(h2) << 0
    | d(h2) << 1
    | e(h2) << 2
    | f(h2) << 3
    | g(h2) << 4
    | (h1 ? 1  << 5 : 0)
    | (h1 ? 1  << 6 : 0)
    | 0     << 7;

    digitalWrite(PIN_LATCH, LOW);
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, ~b3);  
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, ~b2);  
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, ~b1);  
    digitalWrite(PIN_LATCH, HIGH);
}

void loop()
{
  analogWrite(PIN_TRANSISTOR, 255);
  //digitalWrite(PIN_TRANSISTOR, HIGH);
  readTime();
  refreshDisplay(); // Must run repeatedly
  delay(100);
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
