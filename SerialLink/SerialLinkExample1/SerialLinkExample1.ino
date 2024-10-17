/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen
*/

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03  // end of text character
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                 SERIAL LINK STRUCT
struct SerialLinkStruct {
  char BUFFER[1024];           // read incoming bytes into this buffer
  char DATA[1024];             // buffer refined using ETX
  unsigned long T0_RXD_1 = 0;  // hard throttle current time
  unsigned long T1_RXD_1 = 0;  // hard throttle previous time
  unsigned long TT_RXD_1 = 0;  // hard throttle interval
};
SerialLinkStruct SerialLink;
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                              SETUP 
void setup(void) {
  Serial.begin(115200); while(!Serial);
  Serial1.begin(115200); while(!Serial1);
  Serial1.setTimeout(5);
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                           READ RXD
bool readRXD_1() {
  if (Serial1.available() > 0) {
    memset(SerialLink.BUFFER, 0, 1024);
    memset(SerialLink.DATA, 0, 1024);
    int rlen = Serial1.readBytes(SerialLink.BUFFER, 1024);
    if (rlen != 0) {
      for(int i = 0; i < rlen; i++) {
        if (SerialLink.BUFFER[i] == 0x03)
          break;
        else {
          SerialLink.DATA[i] = SerialLink.BUFFER[i];
        }
      }
      return true;
    }
  }
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                        PROCESS RXD
void processRXD_1() {
  SerialLink.T0_RXD_1 = millis();
  if (SerialLink.T0_RXD_1 >= SerialLink.T1_RXD_1+SerialLink.TT_RXD_1) {
    SerialLink.T1_RXD_1 = SerialLink.T0_RXD_1;
    if (readRXD_1() == true) {
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
//                                                                                                         MAIN LOOP
void loop() {
  processRXD_1();
}
