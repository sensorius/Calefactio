#define LED_BUILTIN  2
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.print("Setup... ");
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize serial interface
  Serial.begin(115200);
  delay(1000);
  Serial.println("complete."); 
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("LED ON");          // Debug output
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  Serial.println("LED OFF");         // Debug output
  delay(1000);                       // wait for a second
}
