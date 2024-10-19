/*
Serial Link - Stable inter-microcontroller serial communication. Written by Benjamin Jack Cullen

Change Serial1/2/etc to any other isolated Serial as required.
Methods: I found that method 0 works perfectly for ATMEGA2560 on my specific dev board however method 0 did not yeild
on ESP32 where a simple read until ETX was all that was required. Both methods remain in this template/reference sketch. 

ATMEGA2560 RX -> Serial1
ATMEGA2560 TX -> Serial1

Note: The combination of the ILI9341 3.4" TFT Touchscreen Shield (Uno Shield) and ATMEGA2560 may not be great performing
at taks like drawing graphics but can however be useful in many ways like say as an illuminated control panel that can
also display some graphics and text at a reasonable performance, providing the text and graphics being written to the
display are in small, efficient quantities, which actually is great because it means we can make use of those parts.

*/

// ------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <SPI.h>
#include "TouchScreen_Setup.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SdFat.h>
#define ETX 0x03  // end of text character

// DISPLAY ----------------------------------------------------------------------------------------------------------
#define PORTRAIT 0
#define LANDSCAPE 1
#define USE_XPT2046 0
#define USE_LOCAL_KBV 1
#define TOUCH_ORIENTATION LANDSCAPE
#if defined(USB_PID) && USB_PID == 0x804E  // Arduino M0 Native
#define Serial SerialUSB
#endif
#define SWAP(x, y) { int t = x; x = y; y = t; }

// SDCARD -----------------------------------------------------------------------------------------------------------
#define USE_SDFAT
#if SPI_DRIVER_SELECT != 2
#endif
SoftSpiDriver<12, 11, 13> softSpi;  //Bit-Bang on the Shield pins SDFat.h v2
SdFat SD;
#define SD_CS SdSpiConfig(10, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
File root;
bool sdinit = false;  // sdcard initialized
char char_sdinit[4];

// DISPLAY ----------------------------------------------------------------------------------------------------------
bool tpz = false; // touchscreen pressed
unsigned long x0;
unsigned long x1;
unsigned long y0;
unsigned long y1;
unsigned long w;
unsigned long h;
unsigned long di;
uint16_t color0;
uint16_t color1;
char printData[1024];
unsigned long display_strlen[1][20] = {
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  }
};
float t_display_0;
float t_display_1;
float t_display_delta;
char debugData[1024];
float fps;
char char_fps[20];

// SERIAL LINK STRUCT -----------------------------------------------------------------------------------------------
struct SerialLinkStruct {
  unsigned long nbytes;
  char BUFFER[1024];            // read incoming bytes into this buffer
  char DATA[1024];              // buffer refined using ETX
  unsigned long T0_RXD_1 = 0;   // hard throttle current time
  unsigned long T1_RXD_1 = 0;   // hard throttle previous time
  unsigned long TT_RXD_1 = 0;   // hard throttle interval
  unsigned long T0_TXD_1 = 0;   // hard throttle current time
  unsigned long T1_TXD_1 = 0;   // hard throttle previous time
  unsigned long TT_TXD_1 = 10;  // hard throttle interval
  unsigned long TOKEN_i;
  char tpx[56];
  char tpy[56];
  char tpz[56];
};
SerialLinkStruct SerialLink;

// INITIALIZE DISPLAY -----------------------------------------------------------------------------------------------
void InitializeDisplay(){
  tft.reset();
  uint16_t ID;
  ID = tft.readID();
  Serial.println(ID, HEX);
  if (ID == 0x0D3D3) ID = 0x9481;
  TFT_BEGIN();
  tft.setRotation(TOUCH_ORIENTATION);
  disp_width = tft.width();
  disp_height = tft.height();
  tft.fillScreen(BLACK);
}

// INITIALIZE SDCARD ------------------------------------------------------------------------------------------------
void InitializeSDCard(){
  sdinit = SD.begin(SD_CS);
  if (!sdinit) {Serial.println("sdcard initialization failed.");}
  else {Serial.println("sdcard initialized.");}
  itoa(sdinit, char_sdinit, 10);
}

// DEBUG DATA -------------------------------------------------------------------------------------------------------
void DebugData(){
  memset(debugData, 0, sizeof(debugData));
  strcat(debugData, "tp.x:"); strcat(debugData, SerialLink.tpx); strcat(debugData, " ");
  strcat(debugData, "tp.y:"); strcat(debugData, SerialLink.tpy); strcat(debugData, " ");
  strcat(debugData, "tp.z:"); strcat(debugData, SerialLink.tpz); strcat(debugData, " ");
  strcat(debugData, "sd:");   strcat(debugData, char_sdinit); strcat(debugData, " ");
  strcat(debugData, "fps:");   strcat(debugData, char_fps); strcat(debugData, " "); // can be commented
}

// DEBUG SERIAL -----------------------------------------------------------------------------------------------------
void DebugSerial(){
  Serial.print("[DEBUG] ");
  Serial.print(""); Serial.print(debugData); Serial.print(" ");
}

// DEBUG DISPLAY ----------------------------------------------------------------------------------------------------
void DebugDisplay(){
  // clear debug data
  tft.setCursor(10, 10);
  tft.setTextColor(BLACK, BLACK);
  for (int i=0; i<display_strlen[0][0]; i++) {tft.print(" ");}
  display_strlen[0][0] = strlen(debugData);
  // // print debug data
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE, BLACK);
  tft.print(debugData);
  // // debug button
  tft.drawRect(tft.width()-50, 10, 40, 40, WHITE);
  tft.setCursor(tft.width()-42, 22);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(tft.width()-42, 22);
  tft.setTextColor(GREEN, BLACK);
  tft.print("TEST");
}

// TPZ FUNCTION -----------------------------------------------------------------------------------------------------
void TPZFunction() {
  Serial.println("[FUNCTION] RunClickFunction");
  if ((tp.x > 820) && (tp.x < 880) && (tp.y > 100) && (tp.y < 170)) {
    Serial.println("[RunClickFunction] LED0");
    memset(SerialLink.BUFFER, 0, 1024);
    strcat(SerialLink.BUFFER, "$CLICK,LEDZERO");
    WriteTXD1();
    }
}

// GET DISPLAY INFORMATION ------------------------------------------------------------------------------------------
void GetPanelXYZ(){
  tp = ts.getPoint();   // tp.x, tp.y, tp.z are ADC values
  pinMode(XM, OUTPUT);  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(YP, OUTPUT);  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {tpz = true; TPZFunction();} else {tpz = false;} // set tpz (display is touched bool)
  itoa(tp.x, SerialLink.tpx, 10);
  itoa(tp.y, SerialLink.tpy, 10);
  itoa(tp.z, SerialLink.tpz, 10);
  itoa((int)fps, char_fps, 10);  // can be commented as is only used for now during development to measure performance
}

// SETUP ------------------------------------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200); while(!Serial);
  Serial1.begin(115200); while(!Serial1);
  Serial1.setTimeout(5);
  Serial.flush();
  Serial1.flush();
  InitializeDisplay();
  InitializeSDCard();
  
}

uint16_t ConvertColor(char c) {
  if (strcmp(c, "BLACK") == 0)       {return BLACK;}
  if (strcmp(c, "NAVY") == 0)        {return NAVY;}
  if (strcmp(c, "DARKGREEN") == 0)   {return DARKGREEN;}
  if (strcmp(c, "DARKCYAN") == 0)    {return DARKCYAN;}
  if (strcmp(c, "MAROON") == 0)      {return MAROON;}
  if (strcmp(c, "PURPLE") == 0)      {return PURPLE;}
  if (strcmp(c, "OLIVE") == 0)       {return OLIVE;}
  if (strcmp(c, "LIGHTGREY") == 0)   {return LIGHTGREY;}
  if (strcmp(c, "DARKGREY") == 0)    {return DARKGREY;}
  if (strcmp(c, "BLUE") == 0)        {return BLUE;}
  if (strcmp(c, "GREEN") == 0)       {return GREEN;}
  if (strcmp(c, "CYAN") == 0)        {return CYAN;}
  if (strcmp(c, "RED") == 0)         {return RED;}
  if (strcmp(c, "MAGENTA") == 0)     {return MAGENTA;}
  if (strcmp(c, "YELLOW") == 0)      {return YELLOW;}
  if (strcmp(c, "WHITE") == 0)       {return WHITE;}
  if (strcmp(c, "ORANGE") == 0)      {return ORANGE;}
  if (strcmp(c, "GREENYELLOW") == 0) {return GREENYELLOW;}
  else {color0=WHITE; color1=BLACK;}
}

// READ RXD: METHOD 0 -----------------------------------------------------------------------------------------------
bool readRXD1_Method00() {
  if (Serial1.available() > 0) {
    memset(SerialLink.BUFFER, 0, 1024);
    memset(SerialLink.DATA, 0, 1024);
    int rlen = Serial1.readBytes(SerialLink.BUFFER, 1024);
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
  SerialLink.T0_RXD_1 = millis();
  if (SerialLink.T0_RXD_1 >= SerialLink.T1_RXD_1+SerialLink.TT_RXD_1) {
    SerialLink.T1_RXD_1 = SerialLink.T0_RXD_1;
    if (readRXD1_Method00() == true) {
      Serial.println("-------------------------------------------");
      Serial.print("[RXD]         "); Serial.println(SerialLink.DATA);
      SerialLink.TOKEN_i = 0;
      char * token = strtok(SerialLink.DATA, ",");

      // print: simple and a great place to start wiring up the passthrough
      if (strcmp(token, "$PRINT") == 0) {
        while( token != NULL ) {
          Serial.print("[RXD TOKEN "); Serial.print(SerialLink.TOKEN_i); Serial.print("] "); Serial.println(token);
          if (SerialLink.TOKEN_i == 1) {x0 = atol(token);}
          if (SerialLink.TOKEN_i == 2) {y0 = atol(token);}
          if (SerialLink.TOKEN_i == 3) {color0 = ConvertColor(token);}
          if (SerialLink.TOKEN_i == 4) {color1 = ConvertColor(token);}
          if (SerialLink.TOKEN_i == 5) {di = atol(token);}
          if (SerialLink.TOKEN_i == 6) {memset(printData, 0, sizeof(printData)); strcat(printData, token);}
          token = strtok(NULL, ",");
          SerialLink.TOKEN_i++;
        }
        tft.setCursor(x0, y0);
        tft.setTextColor(BLACK, BLACK);
        for (int i=0; i<display_strlen[0][di]; i++) {tft.print(" ");}
        display_strlen[0][di] = strlen(printData);
        tft.setCursor(x0, y0);
        tft.setTextColor(WHITE, BLACK);
        tft.print(printData);
        tft.println(" ");
      }

      // Plugin more functions when print completed (this SerialLink is for IL19486, there is more functionality to passthrough)
    }
  }
}

// READ RXD: METHOD 1 -----------------------------------------------------------------------------------------------
void readRXD1_Method1() {
  if (Serial1.available() > 0) {
    memset(SerialLink.BUFFER, 0, 1024);
    SerialLink.nbytes = (Serial1.readBytesUntil(ETX, SerialLink.BUFFER, sizeof(SerialLink.BUFFER)));
    Serial.println(SerialLink.BUFFER);
  }
}

// WRITE TXD1 -------------------------------------------------------------------------------------------------------
void WriteTXD1() {
  SerialLink.T0_TXD_1 = millis();
  if (SerialLink.T0_TXD_1 >= SerialLink.T1_TXD_1+SerialLink.TT_TXD_1) {
    SerialLink.T1_TXD_1 = SerialLink.T0_TXD_1;
    if (Serial1.availableForWrite()) {
      // Serial.print("[TXD] "); Serial.println(SerialLink.BUFFER);
      Serial1.write(SerialLink.BUFFER);
      Serial1.write(ETX);
    }
  }
}

double calculate_fps(float microseconds) {
    return 1000000.0 / microseconds;
}

// LOOP -------------------------------------------------------------------------------------------------------------
void loop() {
  readRXD1_Method0();
  GetPanelXYZ();
  DebugData();
  // DebugSerial(); // uncomment to see debug information over serial
  t_display_0 = micros();
  DebugDisplay();
  t_display_1 = micros();
  t_display_delta = t_display_1 - t_display_0;
  fps = calculate_fps(t_display_delta);
  Serial.print("[FPS]: "); Serial.println(fps); // (time delta is currently only aimed at writing to the display)
}
