#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//#define SDA 21    //Default SDA GPIO
//#define SCL 22    //Default SCL GPIO

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;


// UTC <-> MEZ (Winterzeit) = 3600 seconds (1 hour)
// UTC <-> MEZ (Sommerzeit) = 7200 seconds (2 hour)
const long utcOffsetInSeconds = 3600;

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
                              
String strRequestDataFromServer = "https://api.openweathermap.org/data/2.5/"\
                                  "weather?q=L%C3%B8kken,DK&units=metric&"\
                                  "appid=YOUR_APPID";

String strRequestDataForecastFromServer = "https://api.openweathermap.org/data/2.5/"\
                                  "forecast?q=L%C3%B8kken,DK&units=metric&"\
                                  "appid=YOUR_APPID";
                          

unsigned long timerDelay = 60000*5;            // 5 minutes
unsigned long timerDelayForecast = 60000*60;   // 1 hour
unsigned long lastTime = timerDelay;
unsigned long lastTimeForecast = timerDelayForecast;

#define MAX_FORECAST_COUNT 16 

struct webDataType {
String strTemperature = "?";
String strWindspeed = "?";
String strArrayForecastWindspeed[MAX_FORECAST_COUNT] = {};
String strArrayForecastDateTime[MAX_FORECAST_COUNT] = {};
};

struct webDataType myWebData;



const int iPotPin = 34;
int iDigitalValue = 0; // variable to store the value coming from the sensor



LiquidCrystal_I2C lcd(0x27,lcdColumns,lcdRows); 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(115200); 

  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("Connecting to:");        // The print content is displayed on the LCD
  lcd.setCursor(0,1);             // Move the cursor to row 0, column 0
  String str = ssid;
  lcd.print(str);        // The print content is displayed on the LCD

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi, my IP Address is: ");
  Serial.println(WiFi.localIP());
  
  lcd.clear();
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("Connected!");  
  lcd.setCursor(0,1);             // Move the cursor to row 1, column 0
  lcd.print(WiFi.localIP().toString());  

  timeClient.begin();
  
  delay(15000);
  lcd.clear();
}

webDataType getDataFromServer(String strRequest) {

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

        int iValuePosStart = 0;
        int iValuePosEnd = 0;

        if( strRequest.indexOf("weather?") >= 0 ) {
          Serial.println("Extracting weather data");
          
          int iValuePosStart = payload.indexOf("temp")+6;
          int iValuePosEnd = payload.indexOf(",",iValuePosStart );
          myWebData.strTemperature = payload.substring(iValuePosStart,iValuePosEnd);

          iValuePosStart = payload.indexOf("speed")+7;
          iValuePosEnd = payload.indexOf(",",iValuePosStart );
          myWebData.strWindspeed = payload.substring(iValuePosStart,iValuePosEnd);
        } else if ( strRequest.indexOf("forecast?") >= 0  ) {
          Serial.println("Extracting forecast data");

          iValuePosStart = 0;
          iValuePosEnd = 0;
          for( int iCounter = 0; iCounter <MAX_FORECAST_COUNT; iCounter++ ) {
            iValuePosStart = payload.indexOf("speed",iValuePosStart)+7;
            iValuePosEnd = payload.indexOf(",",iValuePosStart );
            myWebData.strArrayForecastWindspeed[iCounter] = payload.substring(iValuePosStart,iValuePosEnd);
            Serial.println("Data:"+myWebData.strArrayForecastWindspeed[iCounter]);
          }

          iValuePosStart = 0;
          iValuePosEnd = 0;
          for( int iCounter = 0; iCounter <MAX_FORECAST_COUNT; iCounter++ ) {
            iValuePosStart = payload.indexOf("dt_txt",iValuePosStart)+9;
            iValuePosEnd = payload.indexOf(",",iValuePosStart )-5;
            myWebData.strArrayForecastDateTime[iCounter] = payload.substring(iValuePosStart,iValuePosEnd);
            Serial.println("Data:"+myWebData.strArrayForecastDateTime[iCounter]);
          }
          
          
        } else {
          
          Serial.println("Error - Check HTML Request:");
          Serial.println(strRequest);
        } 
        
        
        
      }
      else {
        Serial.print("Response code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    return myWebData;
}

void loop() {

  static int iPosOld = -1;

  if ((millis() - lastTime) > timerDelay) {
    
    myWebData = getDataFromServer(strRequestDataFromServer);
    Serial.println("Current outdoor temperature: "+myWebData.strTemperature+"°C");
    Serial.println("Current wind speed "+myWebData.strWindspeed+"m/s");
    
    lastTime = millis();
  }

  if ((millis() - lastTimeForecast) > timerDelayForecast) {
    
    myWebData = getDataFromServer(strRequestDataForecastFromServer);
    //Serial.println("Current outdoor temperature: "+myWebData.strTemperature+"°C");
    //Serial.println("Current wind speed "+myWebData.strWindspeed+"m/s");
    
    lastTimeForecast = millis();
  }

  timeClient.update();

  // read the value from the analog channel (default: 12 bits (0 – 4095) resolution)
  iDigitalValue = analogRead(iPotPin);
  int iPos = iDigitalValue / 240;
  //Serial.print("digital value = ");
  //Serial.println(iPos);      

  if( iPosOld != iPos ) {
    lcd.clear();
  }

  if( iPos == 0 ){
    lcd.setCursor(4,0);             // Move the cursor to row 0, column 4
    lcd.print(timeClient.getFormattedTime()); 
    lcd.setCursor(0,1);             // Move the cursor to row 1, column 0 
    lcd.print(myWebData.strTemperature+"\xDF""C");
    lcd.setCursor(lcdColumns-myWebData.strWindspeed.length()-3,1);  // Align left          
    lcd.print(myWebData.strWindspeed+"m/s");
   } else if( iPos>0 && iPos<17 ) {
    lcd.setCursor(0,0);              
    lcd.print(myWebData.strArrayForecastDateTime[iPos-1]);
    lcd.setCursor(0,1);          
    lcd.print("Speed: "+myWebData.strArrayForecastWindspeed[iPos-1]+"m/s");
   } else if ( iPos==17 ) {
    lcd.setCursor(0,0);             // Move the cursor to row 0, column 4
    lcd.print("Calefactio v0.1"); 
    lcd.setCursor(0,1);             // Move the cursor to row 1, column 0          
    lcd.print(WiFi.localIP().toString()); 
   }

  iPosOld = iPos;
  delay(500);

}
