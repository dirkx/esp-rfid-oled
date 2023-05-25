// ESP32-WROOM-32
// - programming speed reduced to 460800 baud
//
const char BOARD[] = "Board revision 1.01 - 2023-04-18 (red) Rework #3";

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

const uint8_t LED_INDICATOR = 5;
const uint8_t OUT0 = 16;
const uint8_t OUT1 = 17;
const uint8_t TOUCH0 = 4;
const uint8_t TOUCH1 = 0;
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

// https://nl.aliexpress.com/item/1005003404417634.html
// Ips 1.14 Inch Lcd 1.14 Inch Tft Lcd Ips Display Lcd Kleuren Lcd Hd Display Module
// Similar to  LilyGo T-Display board (ST7789)
//
Adafruit_ST7789 tft = Adafruit_ST7789(OLED_CS, OLED_DC_RS, OLED_MOSI, OLED_CLK, OLED_RST);
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIHGT = 135;

void setup() {
  Serial.begin(115200);

  // Required to flush/empty any cruft/different baud-rate during programming.
  //
  while (!Serial) {
    delay(10);
    Serial.println();
  };

  Serial.println("\n\n" __FILE__ "\n" __DATE__ " " __TIME__);
  Serial.println(BOARD);

  pinMode(LED_INDICATOR, OUTPUT);
  tft.init(SCREEN_HEIHGT, SCREEN_WIDTH); // Swapped as we're rotating the screen 90 degrees.
  tft.setRotation(3);

}


// the loop function runs over and over again forever
void loop() {
  tft.fillScreen(ST77XX_WHITE);

  tft.setTextColor(ST77XX_BLACK);
  tft.setTextWrap(true);

  tft.setCursor(0, 0);
  tft.print("0,0 at top left");
  tft.setCursor(SCREEN_WIDTH - 48, SCREEN_HEIHGT - 10);
  tft.print("BotRight");

  tft.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIHGT / 2, 32, ST77XX_RED);


  digitalWrite(LED_INDICATOR, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second

  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIHGT / 2, 42, ST77XX_YELLOW);

  digitalWrite(LED_INDICATOR, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  Serial.println("tock");

}
