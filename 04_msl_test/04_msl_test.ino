// ESP32-WROOM-32
// - programming speed reduced to 460800 baud
//
//
//
const char BOARD[] = "Board revision 1.01 - 2023-04-18 (red) Rework #4";
const char terminalName[] = "RED-101-3";

const uint8_t LED_INDICATOR = 5;

const uint8_t OUT0 = 16;
const uint8_t OUT1 = 17;

const uint8_t TOUCH0_THRESHOLD = 50;
const uint8_t TOUCH0 = T0;

const uint8_t TOUCH1_THRESHOLD = 60;
const uint8_t TOUCH1 = T3;

const uint8_t I2C_SDA = SDA; // 21 - default
const uint8_t I2C_SCL = SCL; // 22 - default
const uint8_t OLED_MOSI = 12;
const uint8_t OLED_CLK = 14;

const uint8_t OLED_RST = 23;
const uint8_t OLED_DC_RS = 18;
const uint8_t OLED_CS = 0; // Was 2
const uint8_t MFRC_NRSTPD = -1; // not connected.
const uint8_t MFRC_IRQ = 25;

#include <Wire.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#ifndef ST77XX_DARKGREEN
#define ST77XX_DARKGREEN (0x03E0)
#endif

#include "MFRC522_I2C.h"
MFRC522_I2C mfrc522(0x28, MFRC_NRSTPD);   // Create MFRC522 instance.

enum { BOOT, WIFI, READY, WAITING, SCANNED } state;
static unsigned long last_state;
#define setState(x) { state = x; last_state = millis(); }

// #include <MFRC522.h>

// TwoWire i2cBus = TwoWire(0);
// MFRC522_I2C dev = MFRC522_I2C(MFRC_NRSTPD, 0x28, Wire);
// MFRC522 mfrc522 = MFRC522(dev);

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Fonts/FreeSansBold18pt7b.h>

Adafruit_ST7789 tft = Adafruit_ST7789(OLED_CS, OLED_DC_RS, OLED_MOSI, OLED_CLK, OLED_RST);
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIHGT = 135;

static volatile bool cardSeen = false;
static void cardScanned() {
  cardSeen = true;
}

static volatile unsigned touch0;
void gotTouch0() {
  touch0++;
}

static volatile unsigned touch1;
void gotTouch1() {
  touch1++;
}
/*
   The function sending to the MFRC522 the needed commands to activate the reception
*/
void activateRec(MFRC522_I2C mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
   The function to clear the pending interrupt bits after interrupt serving routine
*/
void clearInt(MFRC522_I2C mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}

void updateDisplay_progressBar(float p)
{ unsigned short l = tft.width() - 48 - 4;
  unsigned short w = l * p;
  static unsigned short lastw = w + 1;
  if (w == lastw) return;
  tft.fillRect(20 + 2, tft.height() - 40 + 2, w, 20 - 4, ST77XX_DARKGREEN);
  lastw = w;
};

// the setup function runs once when you press reset or power the board
void setup() {
  setState(BOOT);
  Serial.begin(115200);

  // Required to flush/empty any cruft/different baud-rate during programming.
  //
  while (!Serial) {
    Serial.println();
    delay(50);
  };

  Serial.println("\n\n" __FILE__ "\n" __DATE__ " " __TIME__);
  Serial.println(BOARD);

  pinMode(LED_INDICATOR, OUTPUT);
  pinMode(OUT0, OUTPUT);
  pinMode(OUT1, OUTPUT);

  tft.init(SCREEN_HEIHGT, SCREEN_WIDTH); // Swapped as we're rotating the screen 90 degrees.
  tft.setRotation(3);

  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.print("Started: " __DATE__ " " __TIME__ "\n");
  tft.print(BOARD);

  tft.setFont(&FreeSansBold18pt7b);

  ArduinoOTA.setHostname(terminalName);

#ifdef OTA_HASH
  ArduinoOTA.setPasswordHash(OTA_HASH);
#else
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif
#endif

  ArduinoOTA
  .onStart([]() {
    tft.fillScreen(ST77XX_WHITE);
    tft.setTextColor(ST77XX_BLUE);
    printCentered("OTA Update");
    tft.drawRect(20, tft.height() - 40, tft.width() - 40, 20, ST77XX_BLACK);
    updateDisplay_progressBar(0);
  })
  .onEnd([]() {
    tft.fillScreen(ST77XX_WHITE);
    tft.setTextColor(ST77XX_DARKGREEN);
    printCentered("Update OK");
    delay(1000);
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    updateDisplay_progressBar(1.0 * progress / total);
  })
  .onError([](ota_error_t error) {
    const char * str;
    if (error == OTA_AUTH_ERROR) str = "Auth Fail";
    else if (error == OTA_BEGIN_ERROR) str = "Begin Fail";
    else if (error == OTA_CONNECT_ERROR) str = "Conct Fail";
    else if (error == OTA_RECEIVE_ERROR) str = "Rec Fail";
    else if (error == OTA_END_ERROR) str = "End Fail";
    else str = "Ukn Err";
    tft.fillScreen(ST77XX_WHITE);
    tft.setTextColor(ST77XX_RED);
    printCentered((char *)str);
    delay(2500);
  });

  touchAttachInterrupt(TOUCH0, gotTouch0, TOUCH0_THRESHOLD);
  touchAttachInterrupt(TOUCH1, gotTouch1, TOUCH1_THRESHOLD);

  if (!Wire.begin())
    Serial.println("Could not start wire");

  // Software reset if pin not wired up.
  //
  if (MFRC_NRSTPD == -1)
    mfrc522.PCD_Reset();

  mfrc522.PCD_Init();    // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details

  pinMode(MFRC_IRQ, INPUT_PULLUP);
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg,  0xA0 /* Set1 | RxIrq (table 30, page 40) -- read completed */);

  activateRec(mfrc522);
  attachInterrupt(digitalPinToInterrupt(MFRC_IRQ), cardScanned, FALLING);
  delay(50);
  clearInt(mfrc522);
  mfrc522.PICC_HaltA();

  Serial.println("Loop started");
};

void printCentered(const char * string) {
  int16_t  x1, y1;
  uint16_t w, h;
  tft.getTextBounds(string, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((tft.width() - w) / 2, (tft.height()) / 2 + h / 2);
  tft.print(string);
  Serial.print("TFT: ");
  Serial.println(string);
}

// the loop function runs over and over again forever
void loop() {
  static unsigned long lst_scan = millis();
  if (state > WIFI)
    ArduinoOTA.handle();

  if (state == BOOT) {
    tft.fillScreen(ST77XX_WHITE);
    tft.setTextColor(ST77XX_BLUE);
    printCentered("connecting");

    WiFi.setHostname(terminalName);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NETWORK, WIFI_PASSWD);

    setState(WIFI);
  }

  if (state == WIFI) {
    if (WiFi.isConnected()) {
      state = READY;
      tft.fillScreen(ST77XX_WHITE);
      tft.setTextColor(ST77XX_DARKGREEN);
      printCentered(WiFi.localIP().toString().c_str());
      ArduinoOTA.begin();
    };
  };

  if (state == READY) {
    tft.fillScreen(ST77XX_WHITE);
    tft.setTextColor(ST77XX_BLUE);
    printCentered("waiting");
    setState(WAITING);
  };

  if (state == WAITING && cardSeen) {
    if ( mfrc522.PICC_ReadCardSerial()) {
      lst_scan = millis();
      digitalWrite(LED_INDICATOR, HIGH);

      tft.fillScreen(ST77XX_WHITE);
      tft.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIHGT / 2, (SCREEN_HEIHGT / 2) * 0.8, ST77XX_DARKGREEN);
      tft.setTextColor(ST77XX_WHITE);
      tft.setFont(&FreeSansBold18pt7b);
      printCentered("OK");

      setState(SCANNED);

      char buff[sizeof(mfrc522.uid.size) * 4] = { '\0' };
      for (int i = 0; i < mfrc522.uid.size; i++) {
        char app[6];
        snprintf(app, sizeof(app), "%s%02x", i ? "-" : "",  mfrc522.uid.uidByte[i]);
        strncat(buff, app, sizeof(buff) - 1);
      }

      Serial.print("Good scan: ");
      Serial.println(buff);

      clearInt(mfrc522);
      mfrc522.PICC_HaltA();
    } else {
      Serial.println("Bad read (was card removed too quickly?)");
    };
    cardSeen = false;
  }

  if (state == SCANNED && millis() - last_state > 1500)
    state = READY;

  if (millis() - lst_scan > 350) {
    lst_scan = millis();
    digitalWrite(LED_INDICATOR, LOW);
  };

  if (1) {
    static unsigned long int lst = millis();
    if (millis() - lst > 100) {
      lst = millis();
      activateRec(mfrc522);
    }
  }

  if (touch0) {
    static unsigned long int lst = millis();
    if (millis() - lst > 250) {
      tft.fillScreen(ST77XX_WHITE);
      tft.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIHGT / 2, (SCREEN_HEIHGT / 2) * 0.8, ST77XX_RED);
      tft.setTextColor(ST77XX_BLACK);
      tft.setFont(&FreeSansBold18pt7b);
      printCentered("No");
      setState(SCANNED);
    };
    lst = millis();
    touch0 = 0;
  }

  if (touch1) {
    static unsigned long int lst = millis();
    if (millis() - lst > 250) {
      tft.fillScreen(ST77XX_WHITE);
      tft.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIHGT / 2, (SCREEN_HEIHGT / 2) * 0.8, ST77XX_BLUE);
      tft.setTextColor(ST77XX_WHITE);
      tft.setFont(&FreeSansBold18pt7b);
      printCentered("Yes");
      setState(SCANNED);
    };
    lst = millis();
    touch1 = 0;
  }
}
