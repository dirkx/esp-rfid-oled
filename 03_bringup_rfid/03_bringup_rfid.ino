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
MFRC522_I2C mfrc522(0x28, MFRC_NRSTPD);   

// #include <MFRC522.h>
// TwoWire i2cBus = TwoWire(0);
// MFRC522_I2C dev = MFRC522_I2C(MFRC_NRSTPD, 0x28, Wire);
// MFRC522 mfrc522 = MFRC522(dev);

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);

  // Required to flush/empty any cruft/different baud-rate during programming.
  //
  while (!Serial) {
    Serial.println();
    delay(10);
  };

  Serial.println("\n\n" __FILE__ "\n" __DATE__ " " __TIME__);

  pinMode(LED_INDICATOR, OUTPUT);
  Wire.begin();

  // Software reset if pin not wired up.
  //
  if (MFRC_NRSTPD == -1) 
    mfrc522.PCD_Reset();

  mfrc522.PCD_Init();    // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
}

// the loop function runs over and over again forever
void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return; // no card in sight.
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Bad read (was card removed too quickly?)");
    return;
  };

  if ( mfrc522.uid.size == 0) {
    Serial.println("Bad card read (size = 0)");
    return;
  }

  Serial.println("Good scan: ");

  for (int i = 0; i < mfrc522.uid.size; i++) {
    Serial.printf("%s%d", i ? "-" : "", mfrc522.uid.uidByte[i]);
  };
  Serial.println();

  // disengage with the card.
  //
  mfrc522.PICC_HaltA();
}
