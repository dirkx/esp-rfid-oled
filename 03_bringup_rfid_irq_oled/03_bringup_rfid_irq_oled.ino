// ESP32-WROOM-32
// - programming speed reduced to 460800 baud
//
//
//
const char BOARD[] = "Board revision 1.01 - 2023-04-18 (red) Rework #3";

const uint8_t LED_INDICATOR = 5;

const uint8_t OUT0 = 16;
const uint8_t OUT1 = 17;

const uint8_t TOUCH0_THRESHOLD = 65;
const uint8_t TOUCH0 = T0;

const uint8_t TOUCH1_THRESHOLD = 72;
const uint8_t TOUCH1 = T3;

const uint8_t I2C_SDA = SDA; // 21 - default
const uint8_t I2C_SCL = SCL; // 22 - default
const uint8_t OLED_MOSI = 12;
const uint8_t OLED_CLK = 14;
const uint8_t OLED_RST = 23;
const uint8_t OLED_DC_RS = 18;
const uint8_t OLED_CS = 2;
const uint8_t MFRC_NRSTPD = -1; // not connected.
const uint8_t MFRC_IRQ = 25;

#include <Wire.h>

#include "MFRC522_I2C.h"
MFRC522_I2C mfrc522(0x28, MFRC_NRSTPD);   // Create MFRC522 instance.

// #include <MFRC522.h>

// TwoWire i2cBus = TwoWire(0);
// MFRC522_I2C dev = MFRC522_I2C(MFRC_NRSTPD, 0x28, Wire);
// MFRC522 mfrc522 = MFRC522(dev);

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

Adafruit_ST7789 tft = Adafruit_ST7789(OLED_CS, OLED_DC_RS, OLED_MOSI, OLED_CLK, OLED_RST);
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIHGT = 135;

static volatile bool cardSeen = false;
static void cardScanned() {
  cardSeen = true;
}

static volatile unsigned touch0, touch1;
void gotTouch0() {
  touch0++;
}

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

// the setup function runs once when you press reset or power the board
void setup() {
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

  touchAttachInterrupt(TOUCH0, gotTouch0, TOUCH0_THRESHOLD);
  touchAttachInterrupt(TOUCH1, gotTouch1, TOUCH1_THRESHOLD);

  Wire.begin();

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

  Serial.println("loop started");
}


// the loop function runs over and over again forever
void loop() {
  static unsigned long lst_scan = millis();
  if (cardSeen) {
    if ( mfrc522.PICC_ReadCardSerial()) {
      lst_scan = millis();
      digitalWrite(LED_INDICATOR, HIGH);

      char buff[sizeof(mfrc522.uid.size) * 4] = { '\0' };
      for (int i = 0; i < mfrc522.uid.size; i++) {
        char app[6];
        snprintf(app, sizeof(app), "%s%02x", i ? "-" : "",  mfrc522.uid.uidByte[i]);
        strncat(buff, app, sizeof(buff) - 1);
      }

      Serial.print("Good scan: ");
      Serial.println(buff);

      static unsigned char scans = 0;
      scans++;
      if (scans > 8) {
        tft.fillScreen(ST77XX_WHITE);
        tft.setCursor(0, 10);
        scans = 0;
      };
      tft.print("\n     scan: ");
      tft.print(buff);

      clearInt(mfrc522);
      mfrc522.PICC_HaltA();
    } else {
      Serial.println("Bad read (was card removed too quickly?)");
    };
    cardSeen = false;
  }

  if (millis() - lst_scan > 350) {
    lst_scan = millis();
    digitalWrite(LED_INDICATOR, LOW);
  };

  if (1) {
    static unsigned long int lst = millis();
    if (millis() - lst > 10`0) {
      lst = millis();
      activateRec(mfrc522);
    }
  }

  if (touch0) {
    static unsigned long int lst = millis();
    if (millis() - lst > 250) {
      Serial.println("Left touch button touched");
      digitalWrite(OUT0, !digitalRead(OUT0));
    };
    lst = millis();
    touch0 = 0;
  }

  if (touch1) {
    static unsigned long int lst = millis();
    if (millis() - lst > 250) {
      Serial.println("Right touch button touched");
      digitalWrite(OUT1, !digitalRead(OUT1));
    };
    lst = millis();
    touch1 = 0;
  }
}
