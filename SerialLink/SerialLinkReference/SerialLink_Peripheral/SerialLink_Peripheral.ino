/*

SerialLink - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

Wiring (between central, peripheral device):

  ATMEGA2560 RX -> Serial1
  ATMEGA2560 TX -> Serial1

*/

#include <limits.h>

// ------------------------------------------------------------------------------------------------------------------

#define ETX 0x03  // end of text character

// SERIAL LINK STRUCT -----------------------------------------------------------------------------------------------
long i_sync;
struct SerialLinkStruct {
  int i_nbytes;
  long i_sync;
  char char_i_sync[56];
  char * token = strtok(BUFFER, ",");
  bool ack = false;
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
void setup(void) {
  Serial.begin(115200); while(!Serial);
  Serial1.begin(115200); while(!Serial1);
  Serial1.setTimeout(5);
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

// RXD1 READ BYTES UNTIL ETX ----------------------------------------------------------------------------------------
bool readBytesUntilETX() {
  memset(SerialLink.BUFFER, 0, 1024);
  memset(SerialLink.DATA, 0, 1024);
  SerialLink.nbytes = Serial1.readBytes(SerialLink.BUFFER, 1024);
  if (SerialLink.nbytes != 0) {
    for(SerialLink.i_nbytes = 0; SerialLink.i_nbytes < SerialLink.nbytes; SerialLink.i_nbytes++) {
      if (SerialLink.BUFFER[SerialLink.i_nbytes] == ETX)
        return true;
      else {SerialLink.DATA[SerialLink.i_nbytes] = SerialLink.BUFFER[SerialLink.i_nbytes];}
    }
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

      else if (strcmp(SerialLink.token, "$SYN") == 0) {SerialLink.ack = true;}
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
  while (1) {readRXD1(); if (SerialLink.ack == true) {SerialLink.ack = false; break;}}
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
  memset(SerialLink.BUFFER, 0, 1024); strcat(SerialLink.BUFFER, "$DATA,1,2,3"); writeTXD1();

  synCom();

  receiveData();

  synCom();

  // simulate long function time
  delay(1000);
  // write data to other controller
  memset(SerialLink.BUFFER, 0, 1024); strcat(SerialLink.BUFFER, "$DATA,A,B,C"); writeTXD1();

  synCom();

  receiveData();

  // delay(1000);
}
