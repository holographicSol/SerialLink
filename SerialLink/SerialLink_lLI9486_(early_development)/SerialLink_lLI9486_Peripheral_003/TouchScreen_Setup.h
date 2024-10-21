// DISPLAY -------------------------------------------------------------------------------------------------------
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;
// MCUFRIEND UNO shield shares pins with the TFT.
#if defined(ESP32)
// int XP = 27, YP = 4, XM = 15, YM = 14;  //most common configuration
#else
// int XP = 7, YP = A2, XM = A1, YM = 6;  //next common configuration
#endif
#if USE_LOCAL_KBV
#include "TouchScreen_kbv.h"         //my hacked version
#define TouchScreen TouchScreen_kbv
#define TSPoint     TSPoint_kbv
#else
#include <TouchScreen.h>         //Adafruit Library
#endif

// ---- pasted from the callibrator
int XP=8,XM=A2,YP=A3,YM=9; //320x480 ID=0x9486
int TS_LEFT=144,TS_RT=882,TS_TOP=946,TS_BOT=89;
#define MINPRESSURE 200
#define MAXPRESSURE 1000
// TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
// TSPoint tp;
int16_t BOXSIZE;
int16_t PENRADIUS = 1;
// -----

TouchScreen ts(XP, YP, XM, YM, 300);   //re-initialised after diagnose
TSPoint tp;                            //global point
uint16_t readID(void) {
    uint16_t ID = tft.readID();
    if (ID == 0xD3D3) ID = 0x9486;
    return ID;
}

#define TFT_BEGIN()  tft.begin(ID)

// Assign human-readable names to some common 16-bit color values:
// New color definitions.  thanks to Bodmer
#define BLACK       0x0000      //   0,   0,   0
#define NAVY        0x000F      //   0,   0, 128
#define DARKGREEN   0x03E0      //   0, 128,   0
#define DARKCYAN    0x03EF      //   0, 128, 128
#define MAROON      0x7800      // 128,   0,   0
#define PURPLE      0x780F      // 128,   0, 128
#define OLIVE       0x7BE0      // 128, 128,   0
#define LIGHTGREY   0xC618      // 192, 192, 192
#define DARKGREY    0x7BEF      // 128, 128, 128
#define BLUE        0x001F      //   0,   0, 255
#define GREEN       0x07E0      //   0, 255,   0
#define CYAN        0x07FF      //   0, 255, 255
#define RED         0xF800      // 255,   0,   0
#define MAGENTA     0xF81F      // 255,   0, 255
#define YELLOW      0xFFE0      // 255, 255,   0
#define WHITE       0xFFFF      // 255, 255, 255
#define ORANGE      0xFDA0      // 255, 180,   0
#define GREENYELLOW 0xB7E0      // 180, 255,   0
// #define PINK        0xFC9F      // x

// #define RESTORE_SPI_GPIO() SPCR = 0
// int SD_CS = 10;

uint16_t DISPLAYCOL = GREEN;
uint32_t cx, cy, cz;
uint32_t rx[8], ry[8];
int32_t clx, crx, cty, cby;
float px, py;
int disp_width, disp_height, text_y_center, swapxy;
uint32_t calx, caly, cals;
char *Aval(int pin)
{
    static char buf[2][10], cnt;
    cnt = !cnt;
#if defined(ESP32)
    sprintf(buf[cnt], "%d", pin);
#else
    sprintf(buf[cnt], "A%d", pin - A0);
#endif
    return buf[cnt];
}
void showpins(int A, int D, int value, const char *msg)
{
    char buf[40];
    sprintf(buf, "%s (%s, D%d) = %d", msg, Aval(A), D, value);
    Serial.println(buf);
}
#if USE_XPT2046 == 0
bool diagnose_pins()
{
    uint8_t i, j, Apins[2], Dpins[2], found = 0;
    uint16_t value, Values[2];
    Serial.println(F("Making all control and bus pins INPUT_PULLUP"));
    Serial.println(F("Typical 30k Analog pullup with corresponding pin"));
    Serial.println(F("would read low when digital is written LOW"));
    Serial.println(F("e.g. reads ~25 for 300R X direction"));
    Serial.println(F("e.g. reads ~30 for 500R Y direction"));
    Serial.println(F(""));
    for (i = A0; i < A5; i++) pinMode(i, INPUT_PULLUP);
    for (i = 2; i < 10; i++) pinMode(i, INPUT_PULLUP);
    for (i = A0; i < A4; i++) {
        pinMode(i, INPUT_PULLUP);
        for (j = 5; j < 10; j++) {
            pinMode(j, OUTPUT);
            digitalWrite(j, LOW);
            value = analogRead(i);  // ignore first reading
            value = analogRead(i);
            if (value < 100 && value > 0) {
                showpins(i, j, value, "Testing :");
                if (found < 2) {
                    Apins[found] = i;
                    Dpins[found] = j;
                    Values[found] = value;
                }
                found++;
            }
            pinMode(j, INPUT_PULLUP);
        }
        pinMode(i, INPUT_PULLUP);
    }
    if (found == 2) {
        int idx = Values[0] < Values[1];
        XM = Apins[!idx]; XP = Dpins[!idx]; YP = Apins[idx]; YM = Dpins[idx];
        ts = TouchScreen(XP, YP, XM, YM, 300);  // re-initialise with pins
        return true;                            // success
    }
    if (found == 0) Serial.println(F("MISSING TOUCHSCREEN"));
    return false;
}
#endif
// END DISPLAY ----------------------------------------------------------------------------------------------------


