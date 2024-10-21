/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

Change Serial1/2/etc to any other isolated Serial as required.
Methods: I found that method 0 works perfectly for ATMEGA2560 on my specific dev board however method 0 did not yeild
on ESP32 where a simple read until ETX was all that was required. Both methods remain in this template/reference sketch. 

ESP32 TX -> io25 -> Serial2
ESP32 RX -> io26 -> Serial2

*/

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03 // end of text character

// SERIAL LINK STRUCT -----------------------------------------------------------------------------------------------
struct SerialLinkStruct {
  char debugData[1024];
  unsigned long nbytes;
  unsigned long i_nbytes;
  char BUFFER[1024];           // read incoming bytes into this buffer
  char DATA[1024];             // buffer refined using ETX
  unsigned long T0_RXD_2 = 0;  // hard throttle current time
  unsigned long T1_RXD_2 = 0;  // hard throttle previous time
  unsigned long TT_RXD_2 = 0;  // hard throttle interval
  unsigned long T0_TXD_2 = 0;   // hard throttle current time
  unsigned long T1_TXD_2 = 0;   // hard throttle previous time
  unsigned long TT_TXD_2 = 0;  // hard throttle interval
  unsigned long TOKEN_i;
  char * token = strtok(BUFFER, ",");
};
SerialLinkStruct SerialLink;

// DEBUG DATA -------------------------------------------------------------------------------------------------------
void DebugData(){
  strcat(SerialLink.debugData, "");
  // ...strcats
}

// DEBUG SERIAL -----------------------------------------------------------------------------------------------------
void DebugSerial(){
  Serial.print("[DEBUG] "); Serial.println(SerialLink.debugData);
}

// SETUP ------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200); while(!Serial);
  Serial1.begin(115200); while(!Serial1);
  Serial2.begin(115200); while(!Serial1);
  Serial.flush();
  Serial1.flush();
  Serial2.flush();
}

// PASSTHROUGH ANY --------------------------------------------------------------------------------------------------
void PTATAG() {
  while( SerialLink.token != NULL ) {
    Serial.print("[RXD TOKEN "); Serial.print(SerialLink.TOKEN_i); Serial.print("] "); Serial.println(SerialLink.token);
    SerialLink.token = strtok(NULL, ",");
    SerialLink.TOKEN_i++;
  }
}

// RXD1 THROTTLE CHECKS ---------------------------------------------------------------------------------------------
bool RXD1ThrottleChecks() {
  SerialLink.T0_RXD_2 = millis();
  if (SerialLink.T0_RXD_2 >= SerialLink.T1_RXD_2+SerialLink.TT_RXD_2) {
    SerialLink.T1_RXD_2 = SerialLink.T0_RXD_2;
    return true;
  }
}

// TXD1 THROTTLE CHECKS ---------------------------------------------------------------------------------------------
bool TXD2ThrottleChecks() {
SerialLink.T0_TXD_2 = millis();
if (SerialLink.T0_TXD_2 >= SerialLink.T1_TXD_2+SerialLink.TT_TXD_2) {
  SerialLink.T1_TXD_2 = SerialLink.T0_TXD_2;
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

// COMPARE TOKENS ---------------------------------------------------------------------------------------------------
void CompareTokens(char * tk) {
  if (strcmp(SerialLink.token, "$BAR") == 0) {PTATAG();}
  // ...more tags
}

// READ RXD1 METHOD 0 -----------------------------------------------------------------------------------------------
void readRXD1_Method0() {
  if (RXD1ThrottleChecks() == true) {
    if (Serial1.available() > 0) {
      if (readBytesUntilETX() == true) {
        Serial.println("-------------------------------------------");
        Serial.print("[RXD]         "); Serial.println(SerialLink.DATA);
        SerialLink.TOKEN_i = 0;
        SerialLink.token = strtok(SerialLink.DATA, ",");
        CompareTokens(SerialLink.token);
      }
    }
  }
}

// READ RXD: METHOD 1 -----------------------------------------------------------------------------------------------
void readRXD1_Method1() {
  if (RXD1ThrottleChecks() == true) {
    if (Serial1.available() > 0) {
      memset(SerialLink.DATA, 0, 1024);
      SerialLink.nbytes = (Serial1.readBytesUntil(ETX, SerialLink.DATA, sizeof(SerialLink.DATA)));
      Serial.println("-------------------------------------------");
      Serial.print("[RXD]         "); Serial.println(SerialLink.DATA);
      SerialLink.TOKEN_i = 0;
      SerialLink.token = strtok(SerialLink.DATA, ",");
      CompareTokens(SerialLink.token);
    }
  }
}

// WRITE TXD2 -------------------------------------------------------------------------------------------------------
void WriteTXD2() {
  if (TXD2ThrottleChecks() == true) {
    if (Serial2.availableForWrite()) {
      Serial.print("[TXD] "); Serial.println(SerialLink.BUFFER);
      Serial2.write(SerialLink.BUFFER);
      Serial2.write(ETX);
    }
  }
}

// TXD2 DATA --------------------------------------------------------------------------------------------------------
void TXD2Data() {
  // send a peripherals state to the display peripheral
  memset(SerialLink.BUFFER, 0, 1024);
  strcat(SerialLink.BUFFER, "$FOO,1,20,300,4000,50000,600000");
  WriteTXD2();
  // can send more to other and or same peripherals...
}

// LOOP -------------------------------------------------------------------------------------------------------------
void loop() {
  readRXD1_Method1();
  TXD2Data();
  delay(1);
}
