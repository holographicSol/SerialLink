/*

Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

*/

#include <limits.h>

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03 // end of text character

// PINS -------------------------------------------------------------------------------------------------------------
// uncomment following 4 lines for ESP32 CYD
const int8_t ctsPin = -1;  // remap hardware serial TXD
const int8_t rtsPin = -1;  // remap hardware serial RXD
const byte txd = 27;       // CYD TXD
const byte rxd = 22;       // CYD RXD

// SERIAL LINK STRUCT -----------------------------------------------------------------------------------------------
struct SerialLinkStruct {
  int i_nbytes;
  long i_sync;
  char char_i_sync[56];
  char * token = strtok(BUFFER, ",");
  bool syn = false;
  bool data = false;
  char BUFFER[1024];           // read incoming bytes into this buffer
  char TMP[1024];               // buffer refined using ETX
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

// REMOVE ETX -------------------------------------------------------------------------------------------------------
void removeETX() {
  memset(SerialLink.TMP, 0, sizeof(SerialLink.TMP));
  SerialLink.nbytes = sizeof(SerialLink.BUFFER);
  if (SerialLink.nbytes != 0) {
    for(SerialLink.i_nbytes = 0; SerialLink.i_nbytes < SerialLink.nbytes; SerialLink.i_nbytes++) {
      if (SerialLink.BUFFER[SerialLink.i_nbytes] == ETX)
        break;
      else {SerialLink.TMP[SerialLink.i_nbytes] = SerialLink.BUFFER[SerialLink.i_nbytes];}
    }
  }
  memset(SerialLink.BUFFER, 0, sizeof(SerialLink.BUFFER));
  strcpy(SerialLink.BUFFER, SerialLink.TMP);
}

// READ RXD1 --------------------------------------------------------------------------------------------------------
void readRXD1() {
  if (Serial1.available() > 0) {
    memset(SerialLink.BUFFER, 0, sizeof(SerialLink.BUFFER));
    SerialLink.nbytes = (Serial1.readBytesUntil(ETX, SerialLink.BUFFER, sizeof(SerialLink.BUFFER)));
    if (SerialLink.nbytes > 1) {
      
      removeETX();
      Serial.print("[RXD] "); Serial.println(SerialLink.BUFFER);
      SerialLink.TOKEN_i = 0;

      SerialLink.token = strtok(SerialLink.BUFFER, ",");
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

// SEND SYN ---------------------------------------------------------------------------------------------------------
void sendSyn() {
  SerialLink.i_sync++;
  if (SerialLink.i_sync>LONG_MAX) {SerialLink.i_sync=0;}
  itoa(SerialLink.i_sync, SerialLink.char_i_sync, 10);
  memset(SerialLink.BUFFER, 0, 1024);
  strcat(SerialLink.BUFFER, "$SYN,");
  strcat(SerialLink.BUFFER, SerialLink.char_i_sync);
  writeTXD1();
}

// RECEIVE SYN ------------------------------------------------------------------------------------------------------
void receiveSyn() {
  while (1) {readRXD1(); if (SerialLink.syn == true) {SerialLink.syn = false; break;}}
}

// SEND RECEIVE SYN -------------------------------------------------------------------------------------------------
void synCom() {
  Serial.println("-------------------------------------------");
  sendSyn();
  receiveSyn();
}

// RECEIVE DATA -----------------------------------------------------------------------------------------------------
void receiveData() {
  while (1) {readRXD1(); if (SerialLink.data == true) {SerialLink.data = false; break;}}
}

// LOOP -------------------------------------------------------------------------------------------------------------
void loop() {

  synCom();

  // simulate long function time
  // delay(1000);
  // write data to other controller
  memset(SerialLink.BUFFER, 0, sizeof(SerialLink.BUFFER)); strcat(SerialLink.BUFFER, "$DATA,4,5,6"); writeTXD1();

  synCom();

  receiveData();

  synCom();

  // simulate long function time
  // delay(1000);
  // write data to other controller
  memset(SerialLink.BUFFER, 0, sizeof(SerialLink.BUFFER)); strcat(SerialLink.BUFFER, "$DATA,D,E,F"); writeTXD1();

  synCom();

  receiveData();

  // delay(1000);
}
