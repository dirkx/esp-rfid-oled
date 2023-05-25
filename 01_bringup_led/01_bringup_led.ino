// ESP32-WROOM-32
// - programming speed reduced to 460800 baud
//
// Board revision 1.01 - 2023-04-18 (red)
//
#define LED_INDICATOR (5)
const uint8_t OUT0 = 16;
const uint8_t OUT1 = 17;
const uint8_t TOUCH0 = 4;
const uint8_t TOUCH1 = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_INDICATOR, OUTPUT);
  pinMode(OUT0, OUTPUT);
  pinMode(OUT1, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_INDICATOR, HIGH);   
  delay(100);                     
  digitalWrite(LED_INDICATOR, LOW);   
  delay(100);                     
  digitalWrite(OUT0, HIGH);  
  delay(100);                     
  digitalWrite(OUT0, LOW);   
  delay(100);                  
  digitalWrite(OUT1, HIGH); 
  delay(100);                     
  digitalWrite(OUT1, LOW);   
  delay(100);                  
}
