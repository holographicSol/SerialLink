/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

Change Serial1/2/etc to any other isolated Serial as required.
Methods: I found that method 0 works perfectly for ATMEGA2560 on my specific dev board however method 0 did not yeild
on ESP32 where a simple read until ETX was all that was required. Both methods remain in this template/reference sketch. 

ESP32 TX -> io25
ESP32 RX -> io26

*/

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03 // end of text character
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                 SERIAL LINK STRUCT
struct SerialLinkStruct {
  unsigned long nbytes;
  char BUFFER[1024];           // read incoming bytes into this buffer
  char DATA[1024];             // buffer refined using ETX
  unsigned long T0_RXD_2 = 0;  // hard throttle current time
  unsigned long T1_RXD_2 = 0;  // hard throttle previous time
  unsigned long TT_RXD_2 = 0;  // hard throttle interval
  unsigned long T0_TXD_2 = 0;   // hard throttle current time
  unsigned long T1_TXD_2 = 0;   // hard throttle previous time
  unsigned long TT_TXD_2 = 20;  // hard throttle interval
};
SerialLinkStruct SerialLink;
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                              SETUP 
void setup() {
  Serial.begin(115200); while(!Serial);
  Serial1.begin(115200); while(!Serial1);
  Serial2.begin(115200); while(!Serial1);
  Serial.flush();
  Serial1.flush();
  Serial2.flush();
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                 READ RXD: METHOD 0
bool readRXD1_Method00() {
  if (Serial2.available() > 0) {
    memset(SerialLink.BUFFER, 0, 1024);
    memset(SerialLink.DATA, 0, 1024);
    int rlen = Serial2.readBytes(SerialLink.BUFFER, 1024);
    Serial.println(rlen);
    if (rlen != 0) {
      for(int i = 0; i < rlen; i++) {
        if (SerialLink.BUFFER[i] == ETX)
          break;
        else {
          SerialLink.DATA[i] = SerialLink.BUFFER[i];
        }
      }
      return true;
    }
  }
}
void readRXD1_Method0() {
  SerialLink.T0_RXD_2 = millis();
  if (SerialLink.T0_RXD_2 >= SerialLink.T1_RXD_2+SerialLink.TT_RXD_2) {
    SerialLink.T1_RXD_2 = SerialLink.T0_RXD_2;
    if (readRXD1_Method00() == true) {
      Serial.println("-------------------------------------------");
      Serial.print("[RXD]       "); Serial.println(SerialLink.DATA);
      char * token = strtok(SerialLink.DATA, ",");
      while( token != NULL ) {
        Serial.print("[RXD TOKEN] "); Serial.println(token);
        token = strtok(NULL, ",");
      }
    }
  }
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                 READ RXD: METHOD 1
void readRXD1_Method1() {
  if (Serial1.available() > 0) {
    memset(SerialLink.DATA, 0, 1024);
    SerialLink.nbytes = (Serial1.readBytesUntil(ETX, SerialLink.DATA, sizeof(SerialLink.DATA)));
    Serial.println("-------------------------------------------");
    Serial.print("[RXD]       "); Serial.println(SerialLink.DATA);
    char * token = strtok(SerialLink.DATA, ",");
    while( token != NULL ) {
      Serial.print("[RXD TOKEN] "); Serial.println(token);
      token = strtok(NULL, ",");
    }
  }
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                               TXD 
void WriteTXD2() {
    SerialLink.T0_TXD_2 = millis();
  if (SerialLink.T0_TXD_2 >= SerialLink.T1_TXD_2+SerialLink.TT_TXD_2) {
    SerialLink.T1_TXD_2 = SerialLink.T0_TXD_2;
    memset(SerialLink.BUFFER, 0, 1024);
    strcat(SerialLink.BUFFER, "$FOO,1,20,300,4000,OneWirePanelController,12345");
    if (Serial2.availableForWrite()) {
      Serial.print(""); Serial.println(SerialLink.BUFFER);
      Serial2.write(SerialLink.BUFFER);
      Serial2.write(ETX);
    }
  }
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                          MAIN LOOP
void loop() {
  readRXD1_Method1();
  WriteTXD2();
}
