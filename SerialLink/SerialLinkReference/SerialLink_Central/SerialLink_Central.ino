/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

Change Serial1/2/etc to any other isolated Serial as required.
Methods: I found that method 0 works perfectly for ATMEGA2560 on my specific dev board however method 0 did not yeild
on ESP32 where a simple read until ETX was all that was required. Both methods remain in this template/reference sketch. 

ESP32 TX -> io25
ESP32 RX -> io26

*/

#include <limits.h>

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03 // end of text character
#define LED0 33

// uncomment following 4 lines for CYD
const int8_t ctsPin = -1;  // remap hardware serial TXD
const int8_t rtsPin = -1;  // remap hardware serial RXD
const byte txd = 27;  // CYD TXD (remapped TXD pin 27) --> to ATMEGA2560 RXD1 (pin 19)
const byte rxd = 22;   // GPS TXD to --> CYD RXD (remapped RXD pin 22)

// SERIAL LINK STRUCT -----------------------------------------------------------------------------------------------

struct SerialLinkStruct {
  int i_nbytes;
  long i_sync;
  char char_i_sync[56];
  char * token = strtok(BUFFER, ",");
  bool syn = false;
  bool data = false;
  char BUFFER[1024];           // read incoming bytes into this buffer
  char DATA[1024];             // buffer refined using ETX
  unsigned long nbytes;
  unsigned long T0_RXD_1 = 0;   // hard throttle current time
  unsigned long T1_RXD_1 = 0;   // hard throttle previous time
  unsigned long TT_RXD_1 = 0;   // hard throttle interval
  unsigned long T0_TXD_1 = 0;   // hard throttle current time
  unsigned long T1_TXD_1 = 0;   // hard throttle previous time
  unsigned long TT_TXD_1 = 10;  // hard throttle interval
  unsigned long TOKEN_i;
};
SerialLinkStruct SerialLink;

// SETUP ------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200); while(!Serial);
  Serial1.setPins(rxd, txd, ctsPin, rtsPin);
  Serial1.begin(115200); while(!Serial1);
  Serial.flush();
  Serial1.flush();
}

// RXD1 THROTTLE CHECKS ---------------------------------------------------------------------------------------------
bool RXD1ThrottleChecks() {
  SerialLink.T0_RXD_1 = millis();
  if (SerialLink.T0_RXD_1 >= SerialLink.T1_RXD_1+SerialLink.TT_RXD_1) {
    SerialLink.T1_RXD_1 = SerialLink.T0_RXD_1;
    return true;
  }
}

// TXD1 THROTTLE CHECKS ---------------------------------------------------------------------------------------------
bool TXD1ThrottleChecks() {
  SerialLink.T0_TXD_1 = millis();
  if (SerialLink.T0_TXD_1 >= SerialLink.T1_TXD_1+SerialLink.TT_TXD_1) {
    SerialLink.T1_TXD_1 = SerialLink.T0_TXD_1;
    return true;
  }
}

// READ RXD -----------------------------------------------------------------------------------------------
void readRXD1() {
  if (Serial1.available() > 0) {
    memset(SerialLink.DATA, 0, 1024);
    SerialLink.nbytes = (Serial1.readBytesUntil(ETX, SerialLink.DATA, sizeof(SerialLink.DATA)));
    
    if (SerialLink.nbytes > 1) {
      Serial.print("[RXD] "); Serial.println(SerialLink.DATA);
      SerialLink.TOKEN_i = 0;

      SerialLink.token = strtok(SerialLink.DATA, ",");

      if (strcmp(SerialLink.token, "$DATA") == 0) {SerialLink.data = true;}

      else if (strcmp(SerialLink.token, "$SYN") == 0) {SerialLink.syn = true;}
    }
  }
}

// WRITE TXD1 -------------------------------------------------------------------------------------------------------
void writeTXD1() {
  if (TXD1ThrottleChecks() == true) {
    if (Serial1.availableForWrite()) {
      Serial.print("[TXD] "); Serial.println(SerialLink.BUFFER);
      Serial1.write(SerialLink.BUFFER);
      Serial1.write(ETX);
    }
  }
}

void sendSyn() {
  SerialLink.i_sync++;
  if (SerialLink.i_sync>LONG_MAX) {SerialLink.i_sync=0;}
  itoa(SerialLink.i_sync, SerialLink.char_i_sync, 10);
  memset(SerialLink.BUFFER, 0, 1024);
  strcat(SerialLink.BUFFER, "$SYN,");
  strcat(SerialLink.BUFFER, SerialLink.char_i_sync);
  writeTXD1();
}

void receiveSyn() {
  while (1) {readRXD1(); if (SerialLink.syn == true) {SerialLink.syn = false; break;}}
}

void synCom() {
  Serial.println("-------------------------------------------");
  sendSyn();
  receiveSyn();
}

void receiveData() {
  while (1) {readRXD1(); if (SerialLink.data == true) {SerialLink.data = false; break;}}
}

// LOOP -------------------------------------------------------------------------------------------------------------
void loop() {

  synCom();

  // simulate long function time
  delay(1000);
  // write data to other controller
  memset(SerialLink.BUFFER, 0, 1024); strcat(SerialLink.BUFFER, "$DATA,4,5,6"); writeTXD1();

  synCom();

  receiveData();

  synCom();

  // simulate long function time
  delay(1000);
  // write data to other controller
  memset(SerialLink.BUFFER, 0, 1024); strcat(SerialLink.BUFFER, "$DATA,D,E,F"); writeTXD1();

  synCom();

  receiveData();

  // delay(1000);
}
