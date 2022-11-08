Description:

The sketch reads a value from an attached 10K potentiometer and prints those values in the Serial Monitor.<br>

<p align="center">
  <img src="https://github.com/sensorius/Calefactio/blob/main/Blobs/Calefactio_POTI_ADC_34_00.jpg?raw=true" alt="Poti ADC"/>
</p>

ESP's ADC is not perfectly linear therefore you might get identical values for 3.3V and 3.2V or 0.0V and 0.1V.
 

<p align="center">
  <img src="https://i0.wp.com/w4krl.com/wp-content/uploads/ESP-ADC-Figure-1-1.png?w=910&ssl=1?raw=true" alt="ADC Linearity"/>
</p>

Source: https://w4krl.com/esp32-analog-to-digital-conversion-accuracy/



Hardware:

* Freenove ESP32-WROVER-Dev

 

Libraries used:

none
