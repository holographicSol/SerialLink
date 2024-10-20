/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

Change Serial1/2/etc to any other isolated Serial as required.
Methods: I found that method 0 works perfectly for ATMEGA2560 on my specific dev board however method 0 did not yeild
on ESP32 where a simple read until ETX was all that was required. Both methods remain in this template/reference sketch. 

ATMEGA2560 RX -> Serial1
ATMEGA2560 TX -> Serial1
*/

// ------------------------------------------------------------------------------------------------------------------
#define ETX 0x03  // end of text character

// SERIAL LINK STRUCT -----------------------------------------------------------------------------------------------
struct SerialLinkStruct {
  char debugData[1024];
  unsigned long nbytes;
  unsigned long i_nbytes;
  char BUFFER[1024];            // read incoming bytes into this buffer
  char DATA[1024];              // buffer refined using ETX
  unsigned long T0_RXD_1 = 0;   // hard throttle current time
  unsigned long T1_RXD_1 = 0;   // hard throttle previous time
  unsigned long TT_RXD_1 = 0;   // hard throttle interval
  unsigned long T0_TXD_1 = 0;   // hard throttle current time
  unsigned long T1_TXD_1 = 0;   // hard throttle previous time
  unsigned long TT_TXD_1 = 0;  // hard throttle interval
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
void setup(void) {
  Serial.begin(115200); while(!Serial);
  Serial1.begin(115200); while(!Serial1);
  Serial1.setTimeout(5);
  Serial.flush();
  Serial1.flush();
}

// PASSTHROUGH ANY --------------------------------------------------------------------------------------------------
void PTAny() {
  while( SerialLink.token != NULL ) {
    Serial.print("[RXD TOKEN "); Serial.print(SerialLink.TOKEN_i); Serial.print("] "); Serial.println(SerialLink.token);
    SerialLink.token = strtok(NULL, ",");
    SerialLink.TOKEN_i++;
  }
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

// COMPARE TOKENS ---------------------------------------------------------------------------------------------------
void CompareTokens(char * tk) {
  if (strcmp(SerialLink.token, "$FOO") == 0) {PTAny();}
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

// READ RXD1 METHOD 1 -----------------------------------------------------------------------------------------------
void readRXD1_Method1() {
  if (Serial1.available() > 0) {
    memset(SerialLink.BUFFER, 0, 1024);
    SerialLink.nbytes = (Serial1.readBytesUntil(ETX, SerialLink.BUFFER, sizeof(SerialLink.BUFFER)));
    Serial.println(SerialLink.BUFFER);
  }
}

// WRITE TXD1 -------------------------------------------------------------------------------------------------------
void WriteTXD1() {
  if (TXD1ThrottleChecks() == true) {
    if (Serial1.availableForWrite()) {
      Serial.print("[TXD] "); Serial.println(SerialLink.BUFFER);
      Serial1.write(SerialLink.BUFFER);
      Serial1.write(ETX);
    }
  }
}

// TXD2 DATA --------------------------------------------------------------------------------------------------------
void TXD1Data() {
  // send a peripherals state to the display peripheral
  memset(SerialLink.BUFFER, 0, 1024);
  strcat(SerialLink.BUFFER, "$BAR,1,20,300,4000,50000,600000");
  WriteTXD1();
  // can send more to other and or same peripherals...
}

// LOOP -------------------------------------------------------------------------------------------------------------
void loop() {
  readRXD1_Method0();
  TXD1Data();
}
