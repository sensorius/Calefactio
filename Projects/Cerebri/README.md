Description:

This is the sketch of the project's goal to act as a man in the middle for Nibe hot water storage tank temperature sensors.
The project objective is to override temperature readings to implement a calculus which makes use of forecasting electricity produced by a wind turbine.

What has been implemented:
- Connecting to local WLAN.
- Fetching current weather data and forecast of wind speed from openweathermap.org.
- Getting temperature data of  a maximum of 3 attached DS18B20 sensors.
- Displaying data on a LCD using a potentiometer as a device to scroll between different screens.
- A HTML page delivered by a web server showing current weather, forecast data and sensor readings.

To do:
- Fetching price for produced electricity (https://www.energidataservice.dk/)
- Implemention of digital potentimeters to mimik heatpump's NTC sensors
- Logic implementation
- ...


<p align="center">
  <img src="https://github.com/sensorius/Calefactio/blob/main/Blobs/Calefactio_Cerebri-00.jpg?raw=true" alt="DMI Weather Station"/>
</p>

Libraries used:

* LiquidCrystal_I2C by Marco Schwartz Version 1.1.2
* OneWire by Paul Stoffregen Version 2.3.7
* NTPClient by Fabrice Weinberg Version 3.2.1
