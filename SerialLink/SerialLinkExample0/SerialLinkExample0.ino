/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen
*/

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03 // end of text character
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                 SERIAL LINK STRUCT
struct SerialLinkStruct {
  char BUFFER[1024];            // read incoming bytes into this buffer
  char DATA[1024];              // buffer refined using ETX
  unsigned long T0_TXD_2 = 0;   // hard throttle current time
  unsigned long T1_TXD_2 = 0;   // hard throttle previous time
  unsigned long TT_TXD_2 = 10;  // hard throttle interval
};
SerialLinkStruct SerialLink;
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                              SETUP 
void setup() {
  Serial.begin(115200); while(!Serial);
  Serial2.begin(115200); while(!Serial2);
}
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                          MAIN LOOP
void loop() {
  SerialLink.T0_TXD_2 = millis();
  if (SerialLink.T0_TXD_2 >= SerialLink.T1_TXD_2+SerialLink.TT_TXD_2) {
    SerialLink.T1_TXD_2 = SerialLink.T0_TXD_2;

    // $tagged, comma delimited data sentence
    memset(SerialLink.BUFFER, 0, 1024);
    strcat(SerialLink.BUFFER, "$FOOBAR,1,20,300,4000,OneWirePanelController,12345");

    if (Serial2.availableForWrite()) {
      Serial.print(""); Serial.println(SerialLink.BUFFER);
      Serial2.write(SerialLink.BUFFER);
      Serial2.write(ETX);
    }
  }
}
