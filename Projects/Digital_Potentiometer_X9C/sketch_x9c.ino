/*                                                                                                                                     
 INC - GPIO32 ADC1_CH4
 U/D - GPIO33 ADC1_CH5
 RH  - 3.3V
 VSS - GND
 RW  - GPIO34 ADC1_CH6 for analogRead
 RL  - GND
 CS  - GPIO35 ADC1_CH7
 VCC - 3.3V
 */

// Image
// orange GPIO35 ADC1_CH7 -> CS
// yellow GPIO32 ADC1_CH4 -> INC
// green  GPIO33 ADC1_CH5 -> U/D

// grey   GPIO34 ADC1_CH6 -> RW


#include <DigiPotX9Cxxx.h>


const int potPIN = 34;
DigiPot pot(32,33,35);   // INC, U/D, CS

void setup() {
  Serial.begin(9600);
}


void loop() {
  float voltage;
  
  for (int i=0; i<100; i++) {
    pot.increase(1);
    voltage = 3.3 * analogRead(potPIN) / 4095.0;
    //Serial.print(i);
    //Serial.print("->");
    Serial.println(voltage);
    delay(20);
  }
  
  for (int i=0; i<100; i++) {
    pot.decrease(1);
    voltage = 3.3 * analogRead(potPIN) / 4095.0 ;
    //Serial.print(i);
    //Serial.print("->");
    Serial.println(voltage);
    delay(20);
  }
}
 
