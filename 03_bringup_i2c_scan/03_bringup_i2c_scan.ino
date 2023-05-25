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
  // Wire.begin(I2C_SDA, I2C_SCL);
  Wire.begin();
}

void scan_i2c()
{
  Serial.println ();
  Serial.println ("I2C scanning:");
  byte count = 0;

  for (byte i = 8; i < 128; i++)
  {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  // Receive 0 = success (ACK response)
    {
      Serial.print (" - Device at address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
    }
  }
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_INDICATOR, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_INDICATOR, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  Serial.println("tock");

  static unsigned long lst = 10000;
  if (millis() - lst > 5000) {
    lst = millis();
    scan_i2c();
  };
}
