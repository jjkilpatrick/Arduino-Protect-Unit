/* Libraries */
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <Adafruit_BMP085.h>
#include <OneWire.h>

/* Constants */
#define CE_PIN   9
#define CSN_PIN 10
#define MOTION 7
#define LED 2
#define AIR A0
#define LIGHT A1
#define RAIN A2

/* Radio */
RF24 radio(CE_PIN, CSN_PIN);

/* Temp */
Adafruit_BMP085 bmp;
OneWire ds(8);

/* Transmit Pipe */
const uint64_t pipe = 0xE8E8F0F0E1LL;

/* Variables */
int msg[1];
int m, air, light, rain;
float t, p, a;
float fahrenheit, celsius;
const int rainMin = 70;
const int rainMax = 1024;

void setup(void){
  Serial.begin(9600);
  bmp.begin(); 
  radio.begin();
  radio.openWritingPipe(pipe);
  pinMode(MOTION, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(AIR, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(RAIN, INPUT);
}

void loop(void){
  calculateTemp();
  
  t = celsius;
  p = bmp.readPressure();
  a = bmp.readAltitude();
  m = digitalRead(MOTION);
  air = analogRead(AIR);
  light = analogRead(LIGHT);
  rain = analogRead(RAIN);
  
  // form a JSON-formatted string:
  String jsonString = "{\"temperature\":";
  jsonString += t;
  jsonString +=",\"pressure\":";
  jsonString += p;
  jsonString +=",\"altitude\":";
  jsonString += a;
  jsonString +=",\"motion\":";
  jsonString += m;
  jsonString +=",\"air\":";
  jsonString += air;
  jsonString +=",\"light\":";
  jsonString += light;
  jsonString +=",\"rain\":";
  jsonString += rain;
  jsonString +="}";
  
  Serial.println(jsonString);
  
  int jsonSize = jsonString.length();
  for (int i = 0; i < jsonSize; i++) {
    int charToSend[1];
    charToSend[0] = jsonString.charAt(i);
    radio.write(charToSend,1);
  }  
  //send the 'terminate string' value...  
  msg[0] = 2; 
  radio.write(msg,1);
  
  radio.powerDown(); 
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500); 
  radio.powerUp();
}

void calculateTemp(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
    
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
      return;
  }
   
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);
    
  delay(1000);
    
  present = ds.reset();
  ds.select(addr);   
  ds.write(0xBE);         // Read Scratchpad
  
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  
  // convert the data to actual temperature
  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
}