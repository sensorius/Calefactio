
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Webserver
#include "WebServer.h"

// DS18B20 Libs
#include <OneWire.h>
#include <DallasTemperature.h>


//LCD Display
//#define SDA 21    // Default SDA GPIO
//#define SCL 22    // Default SCL GPIO

#define LED_BUILTIN  2  // Blue LED on ESP32 board

#define MAX_SENSOR_COUNT 3   // Maximum number of DS18B20 sensors in this sktech
#define SENSOR_PIN 4         // GIOP4 connected to DS18B20's DQ pin

// Set LCD's number of columns and rows
const int iLcdColumns = 16;
const int iLcdRows = 2;

// UTC <-> MEZ (Winterzeit) = 3600 seconds (1 hour)
// UTC <-> MEZ (Sommerzeit) = 7200 seconds (2 hour)
const long lUtcOffsetInSeconds = 3600;

// WLAN credentials
const char* pSSID = "YOUR_SSID";
const char* pPassword = "YOUR_PASSWORD";

// API openweathermap.org request data strings
String strRequestWeatherDataFromServer = "https://api.openweathermap.org/data/2.5/"\
                                  "weather?q=L%C3%B8kken,DK&units=metric&"\
                                  "appid=YOUR_APPID";

String strRequestDataForecastFromServer = "https://api.openweathermap.org/data/2.5/"\
                                  "forecast?q=L%C3%B8kken,DK&units=metric&"\
                                  "appid=YOUR_APPID";
                          
// Delays to update current weather data, forecast data and sensor readings 
unsigned long ulTimerDelay = 60000*5;            // 5 minutes
unsigned long ulTimerDelayForecast = 60000*60;   // 1 hour
unsigned long ulTimerDelaySensors = 15000;       // 15 seconds

unsigned long ulLastTime = ulTimerDelay;
unsigned long ulLastTimeForecast = ulTimerDelayForecast;
unsigned long ulLastTimeSensors = ulTimerDelaySensors;

#define MAX_FORECAST_COUNT 16 

struct webDataType {
String strTemperature = "?";
String strWindspeed = "?";
String strArrayForecastWindspeed[MAX_FORECAST_COUNT] = {};
String strArrayForecastDateTime[MAX_FORECAST_COUNT] = {};
};

struct webDataType myWebData;

#define POTENTIOMETER_PIN 34  
int iDigitalValue = 0;        // Variable to store value coming from potentiometerstore DS18B20 sensor readings
int iSensorsFound = 0;        // Number of sensors found on OneWire Bus
String strSensorTemp[MAX_SENSOR_COUNT] = {"n/a ", "n/a ", "n/a "}; // Array to store DS18B20 sensor readings

// LCD Display
LiquidCrystal_I2C lcd(0x27,iLcdColumns,iLcdRows); 

// WLAN access
WiFiUDP ntpUDP;

// Network Time Protocol
NTPClient timeClient(ntpUDP, "pool.ntp.org", lUtcOffsetInSeconds);

// DS18B20 sensor (made by Dallas)
OneWire oneWire(SENSOR_PIN);  
DallasTemperature DS18B20(&oneWire);

// Our webserve on port 80
WebServer server(80);


void setup() {
  Serial.begin(115200); 

  // Initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin(SDA, SCL);           // Attach LCD's I2C pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open backlight
  lcd.setCursor(0,0);             // Move cursor to column 0, row 0
  lcd.print("Connecting to:");    
  lcd.setCursor(0,1);             // Move cursor to column 0, row 1
  lcd.print(String(pSSID));        

  // Connect to WLAN with given credentials
  WiFi.begin(pSSID, pPassword);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi, my IP Address is: ");
  Serial.println(WiFi.localIP());
  
  lcd.clear();
  lcd.setCursor(0,0);             // Move cursor to column 0, row 0
  lcd.print("Connected!");  
  lcd.setCursor(0,1);             // Move cursor to column 0, row 1
  lcd.print(WiFi.localIP().toString());  

  delay(5000);
  
  lcd.clear();
  lcd.setCursor(0,0);             // Move cursor to column 0, row 0  
  lcd.print(WiFi.localIP().toString()); 
  lcd.setCursor(0,1);             // Move cursor to column 0, row 1            
  lcd.print("RSSI: "+String(WiFi.RSSI())+"dBm");  // WiFi signal strength
  
  Serial.print("Wifi RSSI=");
  Serial.println(WiFi.RSSI());

  delay(5000);

  // Start NTP client to receive current date/time
  timeClient.begin();

  // Initialize DS18B20 sensor(s)
  DS18B20.begin(); 
  iSensorsFound = DS18B20.getDeviceCount();  // Get number of sensors attached
  
  lcd.clear();
  lcd.setCursor(0,0);  // Move cursor to column 0, row 0
  lcd.print("Sensors found:"+String(iSensorsFound));


  server.on("/", wshandle_OnConnect);
  //server.on("/post", wshandle_OnPost);
  server.onNotFound(wshandle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  delay(5000);
  lcd.clear();
}




void wshandle_OnConnect() {
  Serial.println("WebServerHandle: OnConnect");

  String strCurrentWeather = "<h1>"+timeClient.getFormattedTime()+"<\h1>"\
  "<h5>(Automatic refresh every 10 seonds)<\h5>"\
  "<h4><br>Temperature: "+myWebData.strTemperature+"&degC,"\
  " Wind speed: "+myWebData.strWindspeed+"m/s</h4>";

  String strForecastData = "<center><h4><table><tr><th align='left'>Datetime</th><th>    Wind speed</th></tr>";
  for( int iCount=0; iCount<MAX_FORECAST_COUNT; iCount++ ) {
    strForecastData += "<tr><td align='right'>"+myWebData.strArrayForecastDateTime[iCount]+\
                       "</td><td align='right'>"+myWebData.strArrayForecastWindspeed[iCount]+"m/s</td></tr>";
  }
  strForecastData += "</table></h4></center>";

 String strSensorData = "<h4>DS18B20 sensor readings"\
                        "<br>S1: "+strSensorTemp[0]+"&degC,"\
                        " S2: "+strSensorTemp[1]+"&degC,"\
                        " S3: "+strSensorTemp[2]+"&degC</h4>";

  String strHTML = "<!DOCTYPE html> <html>"\
  "<head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>"\
  "<meta http-equiv='refresh' content='10'>"\
  "<title>Calefactio</title>"\
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"\
  "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}"\
  "p {font-size: 14px;color: #888;margin-bottom: 10px;}</style>"\
  "\n</head><body>"\
  "<h1>Cerebri calefacientis</h1>";

  strHTML += strCurrentWeather;
  strHTML += strSensorData;
  strHTML += "<h4>2-day wind speed forecast<\h4>";
  strHTML += strForecastData;

  strHTML += "</body></html>";
  
  server.send(200, "text/html", strHTML);
}

void wshandle_NotFound(){
  server.send(404, "text/plain", "Not found");
}



// Function to get weather data from openweathermap.org
webDataType getDataFromServer(String strRequest) {

  // Turn builtin LED on to indicate web request
  digitalWrite(LED_BUILTIN, HIGH);   
  
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      // Get data from server
      http.begin(strRequest.c_str());
     
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();

        Serial.println(payload);

        // Get numeric data we are interested in -> <SNIP> main":{"temp":7.3,"fee <SNIP>

        int iValuePosStart = 0, iValuePosEnd = 0;  // Variable to store position of substrings

        // Check if current weather is requested
        if( strRequest.indexOf("weather?") >= 0 ) {
          Serial.println("Extracting weather data");
          
          int iValuePosStart = payload.indexOf("temp")+6;
          int iValuePosEnd = payload.indexOf(",",iValuePosStart );
          myWebData.strTemperature = payload.substring(iValuePosStart,iValuePosEnd);

          iValuePosStart = payload.indexOf("speed")+7;
          iValuePosEnd = payload.indexOf(",",iValuePosStart );
          myWebData.strWindspeed = payload.substring(iValuePosStart,iValuePosEnd);

        // Check if weather forecast is requested 
        } else if ( strRequest.indexOf("forecast?") >= 0  ) {
          Serial.println("Extracting forecast data");

          // Get wind speed forecast -> "speed"
          iValuePosStart=0, iValuePosEnd=0;
          for( int iCounter=0; iCounter<MAX_FORECAST_COUNT; iCounter++ ) {
            iValuePosStart = payload.indexOf("speed",iValuePosStart)+7;
            iValuePosEnd = payload.indexOf(",",iValuePosStart);
            myWebData.strArrayForecastWindspeed[iCounter] = payload.substring(iValuePosStart,iValuePosEnd);
            Serial.println("Data:"+myWebData.strArrayForecastWindspeed[iCounter]);
          }

          // Get corresponding date/time of forecast -> "dt_txt"
          iValuePosStart=0, iValuePosEnd=0;
          for( int iCounter = 0; iCounter <MAX_FORECAST_COUNT; iCounter++ ) {
            iValuePosStart = payload.indexOf("dt_txt",iValuePosStart)+9;
            iValuePosEnd = payload.indexOf(",",iValuePosStart )-5;
            myWebData.strArrayForecastDateTime[iCounter] = payload.substring(iValuePosStart,iValuePosEnd);
            Serial.println("Data:"+myWebData.strArrayForecastDateTime[iCounter]);
          }
          
        // Ooops, an error in the request string defintion?  
        } else {
          Serial.println("Error, weather nor forecast found - Check HTML Request:");
          Serial.println(strRequest);
        } 
       
      } else {
        Serial.print("Response code: ");
        Serial.println(httpResponseCode);
      }
      
      // Free resources
      http.end();

    // Not connected to WLAN  
    } else { 
      Serial.println("WiFi Disconnected");
    }
    
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED off 
    return myWebData;
}

// Function to read DS18B20 sensor data
int getDataFromSensors() {
  // Send command to request temperature readings
  DS18B20.requestTemperatures(); 

  // Get temperature readings of attached sensors
  for(int iCount=0; iCount<iSensorsFound; iCount++) { 
    float fTemp = DS18B20.getTempCByIndex(iCount);    // Read temperature in °C
    if( fTemp>0.0 && fTemp<99.0) {                    // Plausibility check
      strSensorTemp[iCount] = String(fTemp,1);        // 1 decimal point
      Serial.println("S"+String(iCount+1)+"-Temp.: "+strSensorTemp[iCount]+"°C");      
    }
  }
  return 0;
}

// Main loop
void loop() {

  // Position of Potentiometer
  static int iPosPotiOld = -1;

  if ((millis() - ulLastTimeSensors) > ulTimerDelaySensors) {
    getDataFromSensors();

    //for(int iCount=0; iCount<MAX_SENSOR_COUNT; iCount++) { 
    //  Serial.println("S"+String(iCount+1)+"-Temp.: "+strSensorTemp[iCount]+"°C");   
    //}
    
    ulLastTimeSensors = millis();
  }

  // Get current weather data
  if ((millis() - ulLastTime) > ulTimerDelay) {
    myWebData = getDataFromServer(strRequestWeatherDataFromServer);
    Serial.println("Current outdoor temperature: "+myWebData.strTemperature+"°C");
    Serial.println("Current wind speed "+myWebData.strWindspeed+"m/s");
    ulLastTime = millis();
  }

  // Get forecast data
  if ((millis() - ulLastTimeForecast) > ulTimerDelayForecast) {
    myWebData = getDataFromServer(strRequestDataForecastFromServer);
    //Serial.println("Current outdoor temperature: "+myWebData.strTemperature+"°C");
    //Serial.println("Current wind speed "+myWebData.strWindspeed+"m/s");
    ulLastTimeForecast = millis();
  }

  // Get current 
  timeClient.update();

  // Read value from potentiometer's analog channel (default: 12 bits (0 – 4095) resolution)
  iDigitalValue = analogRead(POTENTIOMETER_PIN);
  int iPosPoti = iDigitalValue / 227;      // 4095/(MAX_FORECAST_COUNT+2) -> 18 potentiometer positions
  //Serial.print("digital value = ");
  //Serial.println(iPos);      

  // Clear LCD only if potentiometer position has changed
  if( iPosPotiOld != iPosPoti ) {
    lcd.clear();
  }

  // Display data corresponding to potentimeter's position

  // Show current time, temperature and wind speed
  if( iPosPoti==0 ){
    lcd.setCursor(4,0);             // Move cursor to column 4, row 0
    lcd.print(timeClient.getFormattedTime()); 
    lcd.setCursor(0,1);             // Move cursor to column 0, row 1 
    lcd.print(myWebData.strTemperature+"\xDF""C");
    lcd.setCursor(iLcdColumns-myWebData.strWindspeed.length()-3,1);  // Align right          
    lcd.print(myWebData.strWindspeed+"m/s");

  // Show 16 sets of wind speed forecast data  
  } else if( iPosPoti>0 && iPosPoti<17 ) {
     if( iPosPotiOld != iPosPoti ) {
       lcd.setCursor(0,0);          // Move cursor to column 0, row 0   
       lcd.print(myWebData.strArrayForecastDateTime[iPosPoti-1]);
       lcd.setCursor(0,1);          // Move cursor to column 0, row 1
       lcd.print("Speed: "+myWebData.strArrayForecastWindspeed[iPosPoti-1]+"m/s");
     }

  // Show assigned local IP and signal strength 
  } else if ( iPosPoti==17 ) {
    lcd.setCursor(0,0);             // Move cursor to column 0, row 0  
    lcd.print(WiFi.localIP().toString()); 
    lcd.setCursor(0,1);             // Move cursor to column 0, row 1            
    lcd.print("RSSI: "+String(WiFi.RSSI())+"dBm");  // WiFi signal strength

  // Show DS18B20 sensor readings
  } else if ( iPosPoti==18 ) {
    lcd.setCursor(0,0);             
    lcd.print("S1   S2   S3  \xDF""C"); 
    lcd.setCursor(0,1);  lcd.print(strSensorTemp[0]); 
    lcd.setCursor(5,1);  lcd.print(strSensorTemp[1]); 
    lcd.setCursor(10,1); lcd.print(strSensorTemp[2]);

    // Enforce fast reading of DS18B20 temperature by setting timer above delay
    ulLastTimeSensors = ulTimerDelaySensors+1;            
  }

  // Store current position of potentiometer 
  iPosPotiOld = iPosPoti;
  
  server.handleClient();
  delay(100);
}
