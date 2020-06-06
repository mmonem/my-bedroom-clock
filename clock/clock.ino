/*
    My bedroom Clock
    ATMega328p based seven segment wall-clock

    By Mohamed Abdul-Monem & Shams El-Din Mohamed
    2017-2020
*/

// Libraries used - might differ when using CMake
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "IRLremote.h"
#include "Wire.h"

// Pinout depebding on My-bed-room-clock PCB
#define PIN_LATCH  8
#define PIN_CLOCK 12
#define PIN_DATA  11
#define PIN_TRANSISTOR  3
#define PIN_IR 2
#define PIN_PHOTOCELL A1

// Generic casette remote control codes
#define BUTTON_MINUS 0x15
#define BUTTON_PLUS 0x9
#define BUTTON_SET 0x19
#define BUTTON_RESET 0x45
#define BUTTON_0 0x16
#define BUTTON_1 0xc
#define BUTTON_2 0x18
#define BUTTON_3 0x5E
#define BUTTON_4 0x8
#define BUTTON_5 0x1C
#define BUTTON_6 0x5A
#define BUTTON_7 0x42
#define BUTTON_8 0x52
#define BUTTON_9 0x4A

// Definetions
#define BRIGHTNESS_STEP 1
#define BRIGHTNESS_SAMPESLS 100
#define BRIGHTNESS_SAMP_TIME 10

#define MODE_NORMAL 1
#define MODE_SET_HOUR 100
#define MODE_SET_MINUTE 101

// IIC Addresses
#define DS3231_I2C_ADDRESS 0x68

// Variables
byte h1, h2, m1, m2;
int count = 0; // loop counter
byte mode;
bool blink = 0;
byte brightness = 126;

unsigned long police_time;
int police_address;
int police_change = 0;

int sensor_value;
byte sensor_mapping[1024];
bool stop_sensor = false;
bool save = false;

CNec IRLremote;
SoftwareSerial ESPserial(PD5, PD6); // RX | TX

void setup()
{
  mode = MODE_NORMAL;
  
  Serial.begin(9600);
  ESPserial.begin(9600);
  Wire.begin();
  
  //setDS3231time(0,41,21,1,20,6,19);

  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_TRANSISTOR, OUTPUT);
  if (!IRLremote.begin(PIN_IR)) {
    Serial.println(F("You did not choose a valid pin."));
  }
  
  //resetPoints();  
  loadPoints();
  
  setBrightness(brightness);
  
}

void prepareArray() {
  for(int i = 0;i<1024;i++){
    sensor_mapping[i] = 0;
  }  
}

void calc(int addr_1, int addr_2) {
  
  int address_1, address_2, number, diffrence, repeat;
  float change;
  
  address_1 = addr_1;
  address_2 = addr_2;
  number = address_2 - address_1;
  diffrence = sensor_mapping[address_2] - sensor_mapping[address_1];
  change = (float)diffrence / (float)number;
  repeat = 0;
  
  for (int i = address_1 + 1 ; i < address_2; i++) {
    sensor_mapping[i] = sensor_mapping[address_1] + round(change * repeat);
    repeat++;
  }
  
  repeat = 0;
}


int pointsTrue[20];
int pointsTrueCounter = 0;

int trueAddress[10];
byte counter = 0;
void calculateCurve(){
  Serial.println("Calculating....");
  for(int i = 0;i<1024;i++){
   if(sensor_mapping[i] > 0){
    trueAddress[counter] = i;
    counter++;  
   }  
  }
  for(int i =0;i<counter;i++){
    calc(trueAddress[i], trueAddress[i+1]);  
  }
  counter = 0 ;
  Serial.println("Fineshed !");
}

void dumpMapping() {
  for (int i = 0; i <= 1023; i++) {
    Serial.println(sensor_mapping[i]);
  }
}

void setBrightness(byte value)
{
  analogWrite(PIN_TRANSISTOR, value);
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
  return (x == 2) || (x == 3) ||   (x == 4) || (x == 5) || (x == 6) || (x == 8) || (x == 9);
}

void refreshDisplay()
{
  /* +-----------------+-----------------+-----------------+
     |      Byte 3     |      Byte 2     |     Byte 1      |
     | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
     +-----------------+-----------------+-----------------+
     | : c b g f e d c | b a g f e d c b | a g f e d c b a |
     | : 4 4 3 3 3 3 3 | 3 3 2 2 2 2 2 2 | 2 1 1 1 1 1 1 1 |
     +-----------------+-----------------+-----------------+
  */
  byte b1 =
    (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (a(m2) << 0))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (b(m2) << 1))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (c(m2) << 2))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (d(m2) << 3))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (e(m2) << 4))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (f(m2) << 5))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (g(m2) << 6))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (a(m1) << 7));

  byte b2 =
    (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (b(m1) << 0))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (c(m1) << 1))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (d(m1) << 2))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (e(m1) << 3))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (f(m1) << 4))
    | (mode == MODE_SET_MINUTE && blink == 0 ? 0 : (g(m1) << 5))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : (a(h2) << 6))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : (b(h2) << 7));

  byte b3 =
    (mode == MODE_SET_HOUR && blink == 0 ? 0 : (c(h2) << 0))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : (d(h2) << 1))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : (e(h2) << 2))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : (f(h2) << 3))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : (g(h2) << 4))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : ((h1 ? 1  << 5 : 0)))
    | (mode == MODE_SET_HOUR && blink == 0 ? 0 : ((h1 ? 1  << 6 : 0)))
    | (0     << 7);

  digitalWrite(PIN_LATCH, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, ~b3);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, ~b2);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, ~b1);
  digitalWrite(PIN_LATCH, HIGH);
}


void loop()
{
  blink = (millis() % 1000) / 500;

  //autoAdjust();
  if (!stop_sensor) {
    getSensorValue();
    updateTransistor();
  } else {
    sensorPolice();
  }

  recieveInfrared();

  updateTimeNTP();
  readTime();
  refreshDisplay(); // Must run repeatedly
  //dumpMapping();
  delay(10);
}

void updateTransistor() {
  adjustTransistor(sensor_mapping[sensor_value]);
}

void getSensorValue() {
   int buffer[32];
   int total = 0;
    for(int i =0;i<32;i+=1){
      buffer[i] = analogRead(PIN_PHOTOCELL);   
      delay(2);
    }
  for(int i =0;i<32;i+=1){
      total += buffer[i];
    }
  sensor_value = total/32;
}

void adjustTransistor(int value) {
  byte real_value = sensor_mapping[sensor_value];
  setBrightness(real_value);
}

void sensorPolice() {
  if (police_time + 5000 < millis()) {
    stop_sensor = false; 
  }
  
  if(save)
  {
    
    sensor_mapping[police_address] += police_change;
    
    
    setPoint(police_address, sensor_mapping[police_address]);
    loadPoints();

    
    stop_sensor = false;
    save = false;
    police_change = 0;
      
    digitalWrite(PIN_LATCH, LOW);
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, B11100100);
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, B10000100);
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, B01000001);
    digitalWrite(PIN_LATCH, HIGH);

    delay(2000);
    
  }
}

void userAdjustBrightness(unsigned long time,int change) {
  getSensorValue();
  byte real_value = sensor_mapping[sensor_value];
  
  stop_sensor = true;

  police_time = time;
  police_address = sensor_value;
  
  if((real_value + (police_change + change)) > -1 && (real_value + (police_change + change)) < 256){
    police_change += change;  
  }

  Serial.println(police_change);
  setBrightness(real_value + police_change);
}

//// INFRARED API ////////////////////////////////////////////////////////////////////

void recieveInfrared() {
  if (IRLremote.available())
  {

    /*
       Important Note !
       Tested generic remote controls send the real button code ONCE at address 0xFF00
       if user keep pressing it sends 0x0 at address 0xFFFF

    */

    // Get the new data from the remote
    auto data = IRLremote.read();

    if (data.address == 0xFF00 /* To prevent recieving 0, look at note above */) {
      switch (data.command) {
        case 0x43:
          userAdjustBrightness(millis(), 10);
          Serial.println("INCREASE");
          break;
        case BUTTON_PLUS:
          userAdjustBrightness(millis(), 1);
          Serial.println("INCREASE");
          break;
        
        case 0x40:
          userAdjustBrightness(millis(), -10);
          Serial.println("DECREASE");
          break;
        case BUTTON_MINUS:
          userAdjustBrightness(millis(), -1);
          Serial.println("DECREASE");
          break;  
        case BUTTON_SET:
          save = true;
          break;
        case BUTTON_RESET:
          resetPoints();
          prepareArray();
          loadPoints();
          break;
        case 0x47:
          Serial.println("==================<>==================");
          describePoints();
          Serial.println("==================<>==================");
          dumpMapping();
          break;
      }
//      Serial.print(F("Address: 0x"));
//      Serial.println(data.address, HEX);
//      Serial.print(F("Command: 0x"));
//      Serial.println(data.command, HEX);
//      Serial.println();

    }
  }
}

//// INFRARED API Finishes ////////////////////////////////////////////////////////////////////

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
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

//void buttonMinus()
//{
//  if (mode == MODE_NORMAL) {
//    Serial.print("Brightness Level Down: ");
//    adjustBrightness(-BRIGHTNESS_STEP);
//    Serial.println(brightness);
//  }
//}
//
//void buttonPlus()
//{
//  if (mode == MODE_NORMAL) {
//    Serial.print("Brightness Level Up: ");
//    adjustBrightness(BRIGHTNESS_STEP);
//    Serial.println(brightness);
//  }
//}

void buttonSet()
{
  if (mode == MODE_NORMAL) {
    mode = MODE_SET_MINUTE;
  }
  else if (mode == MODE_SET_MINUTE) {
    mode = MODE_SET_HOUR;
  }
  else if (mode == MODE_SET_HOUR) {
    mode = MODE_NORMAL;
  }

  Serial.print("Setting mode: ");
  Serial.println(mode);
}

void updateTimeNTP() {
  if (ESPserial.available()) {
    String readString;
    while (ESPserial.available()) {
      delay(1);
      if (ESPserial.available() > 0) {
        char c = ESPserial.read();
        readString += c;
      }
    }

    if (validTimeString(readString)) {
      setTime(readString.substring(0, 10));
    }
  }
}

bool validTimeString(String timeStringWithChecksum) {
  int len = timeStringWithChecksum.length();
  if (len < 11 || len > 13)
    return false;

  String timeString = timeStringWithChecksum.substring(0, 10);
  String checkSumStr = timeStringWithChecksum.substring(10);

  //Serial.print(timeStringWithChecksum);
  //Serial.print(" --> ");
  //Serial.print(timeString);
  //Serial.print(" : ");
  //Serial.print(checkSumStr);
  //Serial.print(" ==>");

  String computedChecksum = checksum(timeString);
  computedChecksum += "\r\n";

  //Serial.print(computedChecksum);

  if (computedChecksum != checkSumStr) {
    //Serial.println("Noooooo");
    return false;
  }
  else {
    // Serial.println("True");
    return true;
  }
}

String checksum(String s) {
  byte c = 0;
  for (int i = 0; i < s.length(); i++) {
    c ^= s[i];
  }
  return String(c);
}


void setTime(String timeString) {
  unsigned long time = timeString.toInt();
  //Serial.println(time);
  // print the hour, minute and second:
  int hr = (time  % 86400L) / 3600;
  int min = (time % 3600) / 60;
  int sec = (time % 60);

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(sec)); // set seconds
  Wire.write(decToBcd(min)); // set minutes
  Wire.write(decToBcd(hr)); // set hours
  Wire.endTransmission();
}

///// EEPROM API ///////////////////////////////////////////////////////////////////////////////////////

void describePoints(){
  for(int i =1;i<=EEPROM.read(0);i+=1){
    describePoint(i);      
  }
}

void loadPoints(){
  prepareArray();
  
  byte n = EEPROM.read(0);
  for(int i =1; i <= n; i+=1) {
     sensor_mapping[fetchPointSensor(i)] = fetchPointValue(i);   
  }
  
  calculateCurve();
}

void resetPoints(){
  EEPROM.write(3, 0); 
  EEPROM.write(4, 0); 
  EEPROM.write(5, 1); 

  EEPROM.write(6, B11111111); 
  EEPROM.write(7, B00000011); 
  EEPROM.write(8, 255); 

  EEPROM.write(0, 2);
}

void describePoint(unsigned int point){
   Serial.print("Sensor Value: ");
   Serial.print(fetchPointSensor(point));
   Serial.print(" Transistor Value: ");
   Serial.println(fetchPointValue(point));
}

unsigned int fetchPointSensor(unsigned int point){
  return ByteCombine(EEPROM.read(point*3+1),EEPROM.read(point*3));
}

unsigned int fetchPointValue(unsigned int point){
  return EEPROM.read(point*3+2);
}

byte getPointValue(unsigned int address){
  byte r;
  for(int i =1;i<=EEPROM.read(0);i+=1){
    if(fetchPointSensor(i) == address){
      r = fetchPointValue(i);
    }
  }

  return r;
}

void setPoint(unsigned int point, byte value){
  unsigned int romValues = EEPROM.read(0);
  romValues+=1;

  EEPROM.write(romValues*3, IntSplitLowByte(point));
  EEPROM.write(romValues*3+1, IntSplitHighByte(point));

  EEPROM.write(romValues*3+2, value);
  
  EEPROM.write(0, romValues);
}

byte IntSplitHighByte(unsigned int x){
  return highByte(x);  
}

byte IntSplitLowByte(unsigned int x){
  return lowByte(x);  
}


unsigned int ByteCombine(byte x_high,byte x_low) {
  unsigned int x;
  for( int t = 7; t >= 0; t--)
  {
    bitWrite(x, t,  bitRead(x_low, t));
  }
  for( int t = 7; t >= 0; t--)
  {
    bitWrite(x, t + 8,  bitRead(x_high, t));
  }
  return x;
}

///// EEPROM API FINISHES ///////////////////////////////////////////////////////////////////////////////////////
