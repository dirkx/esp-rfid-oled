// ESP32-WROOM-32
// - programming speed reduced to 460800 baud
//
// Board revision 1.01 - 2023-04-18 (red)
//

const uint8_t LED_INDICATOR = 5;
const uint8_t OUT0 = 16;
const uint8_t OUT1 = 17;
const uint8_t TOUCH0 = 4;
const uint8_t TOUCH1 = 0;
const uint8_t I2C_SDA = SDA; // 21 - default
const uint8_t I2C_SCL = SCL; // 22 - default
const uint8_t OLED_RST = 23;
const uint8_t OLED_DC = 18;
const uint8_t MFRC_NRSTPD = -1; // not connected.
const uint8_t MFRC_IRQ = 25;

#include <Wire.h>

#include "MFRC522_I2C.h"
MFRC522_I2C mfrc522(0x28, MFRC_NRSTPD);   // Create MFRC522 instance.

// #include <MFRC522.h>

// TwoWire i2cBus = TwoWire(0);
// MFRC522_I2C dev = MFRC522_I2C(MFRC_NRSTPD, 0x28, Wire);
// MFRC522 mfrc522 = MFRC522(dev);

static bool cardSeen = false;
static void cardScanned() {
  cardSeen = true;
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

  pinMode(LED_INDICATOR, OUTPUT);
  pinMode(OUT0, OUTPUT);
  pinMode(OUT1, OUTPUT);
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

      Serial.print("Good scan: ");
      for (int i = 0; i < mfrc522.uid.size; i++)
        Serial.printf("%s%02x", i ? "-" : "",  mfrc522.uid.uidByte[i]);
      Serial.println();

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
    if (millis() - lst > 100) {
      lst = millis();
      activateRec(mfrc522);
    }
  }
  {
    static unsigned long int lst = millis();
    if (millis() - lst > 2500) {
      lst = millis();
      digitalWrite(OUT0, HIGH);
      delay(5);
      digitalWrite(OUT0, LOW);
    }
  }
}
