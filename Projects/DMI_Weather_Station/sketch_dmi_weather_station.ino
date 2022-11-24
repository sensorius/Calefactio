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
                                  "forecast?q=L%C3%B8kken,DK&units=metric&"\
                                  "appid=YOUR_APPID";
                          

unsigned long timerDelay = 60000*5;  // 5 minutes
unsigned long lastTime = timerDelay;


struct webDataType {
  String strTemperature = "?";
  String strWindspeed = "?";
};

  
struct webDataType myWebData;

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
  
  delay(3000);
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
        
        int iValuePosStart = payload.indexOf("temp")+6;
        int iValuePosEnd = payload.indexOf(",",iValuePosStart );
        myWebData.strTemperature = payload.substring(iValuePosStart,iValuePosEnd);

        iValuePosStart = payload.indexOf("speed")+7;
        iValuePosEnd = payload.indexOf(",",iValuePosStart );
        myWebData.strWindspeed = payload.substring(iValuePosStart,iValuePosEnd);
        
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
  //Send an HTTP POST request every 5 minutes
  if ((millis() - lastTime) > timerDelay) {
    
    myWebData = getDataFromServer(strRequestDataFromServer);
    Serial.println("Current outdoor temperature: "+myWebData.strTemperature+"°C");
    Serial.println("Current wind speed "+myWebData.strWindspeed+"m/s");
    
    lastTime = millis();
  }
  
  timeClient.update();

  lcd.setCursor(4,0);             // Move the cursor to row 0, column 4
  lcd.print(timeClient.getFormattedTime()); 
  lcd.setCursor(0,1);             // Move the cursor to row 1, column 0 
  lcd.print(myWebData.strTemperature+"\xDF""C");
  lcd.setCursor(lcdColumns-myWebData.strWindspeed.length()-3,1);  // Align left          
  lcd.print(myWebData.strWindspeed+"m/s");

  delay(1000);

}
