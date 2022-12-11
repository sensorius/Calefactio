Description:

The sketch interfaces a X9C digital potentiometer

<p align="center">
  <img src="https://github.com/sensorius/Calefactio/blob/main/Blobs/Calefactio_X9C_00.jpg?raw=true" alt="X9C"/>
</p>


Cables 
* orange GPIO35 ADC1_CH7 -> CS
* yellow GPIO32 ADC1_CH4 -> INC
* green  GPIO33 ADC1_CH5 -> U/D

* grey   GPIO34 ADC1_CH6 -> RW


<p align="center">
  <img src="https://github.com/sensorius/Calefactio/blob/main/Blobs/Calefactio_X9C_01.jpg?raw=true" alt="X9C"/>
</p>

Hardware:

* Freenove ESP32-WROVER-Dev
* X9C104S 100KΩ 


Libraries used:

* DigiPotX9Cxxx Version 0.1.0


Thoughts:

Possible digital potentiometer substitue, higher resolution and 2 potis onboard: https://opencircuit.nl/product/fermion-dual-digital-potentiometer-100k
