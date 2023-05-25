// ESP32-WROOM-32
// - programming speed reduced to 460800 baud
//
// Board revision 1.01 - 2023-04-18 (red)
//
#define LED_INDICATOR (5)

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);

  // Required to flush/empty any cruft/different baud-rate during programming.
  //
  while(!Serial) { Serial.println(); delay(10); };

  Serial.println("\n\n" __FILE__ "\n" __DATE__ " " __TIME__);

  pinMode(LED_INDICATOR, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_INDICATOR, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_INDICATOR, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  Serial.println("tock");
}
