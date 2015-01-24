/* Libraries */
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
/* Constants */
#define CE_PIN   9
#define CSN_PIN 10
  
/* Radio */
RF24 radio(CE_PIN, CSN_PIN);
/* Transmit Pipe */
const uint64_t pipe = 0xE8E8F0F0E1LL;
/* Variables */
int msg[1];
int lastmsg = 1;
String theMessage = "";
void setup(void){
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}
void loop(void){
  if (radio.available()){
    bool done = false; 
      done = radio.read(msg, 1);
      char theChar = msg[0];
      if (msg[0] != 2){
        theMessage.concat(theChar);
        }
      else {
       Serial.println(theMessage);
       theMessage= "";
      }
   }
}