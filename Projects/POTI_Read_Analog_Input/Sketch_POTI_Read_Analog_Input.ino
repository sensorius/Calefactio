// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 
const int iPotPin = 34;

// variable to store the value coming from the sensor
int iDigitalValue = 0;

// variable to store the calculated analog value 
float fAnalogVoltage = 0.0;

void setup() {
  
  Serial.begin(115200);
  delay(1000);
  
}

void loop() {
  
  // read the value from the analog channel
  iDigitalValue = analogRead(iPotPin);
  Serial.print("digital value = ");
  Serial.print(iDigitalValue);      
  
  //convert digital value to analog voltage
  fAnalogVoltage = (iDigitalValue * 3.3)/4095.0;
  Serial.print("  analog voltage = ");
  Serial.println(fAnalogVoltage);
  delay(1000);
  
}
