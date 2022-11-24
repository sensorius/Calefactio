
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>


//#define SDA 21    //Default SDA GPIO
//#define SCL 22    //Default SCL GPIO

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;


const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";


String strRequestDataTempFromServer = "http://dmigw.govcloud.dk/v2/climateData/collections/10kmGridValue/items?"\
                                  "cellId=10km_635_54&limit=1&parameterId=mean_temp&api-key="\
                                  "YOUR_APIKEY";
                              
String strRequestDataWindSpeedFromServer = "http://dmigw.govcloud.dk/v2/climateData/collections/10kmGridValue/items?"\
                                  "cellId=10km_635_54&limit=1&parameterId=mean_wind_speed&api-key="\
                                  "YOUR_APIKEY";

unsigned long timerDelay = 60000*5;  // 5 minutes
unsigned long lastTime = timerDelay;


LiquidCrystal_I2C lcd(0x27,lcdColumns,lcdRows); 

void setup() {
  Serial.begin(115200); 


  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("No Wifi...");     // The print content is displayed on the LCD

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi, my IP Address is: ");
  Serial.println(WiFi.localIP());
  
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("Connected");  
  lcd.setCursor(0,1);             // Move the cursor to row 1, column 0
  lcd.print(WiFi.localIP().toString());          

}

String getDataFromServer(String strRequest) {

  String strRetVal = "n/a";
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

        // Get numeric data we are interested in -> <SNIP> 00+01:00","value":6.3}}],"timeSta <SNIP>
        
        int iValuePosStart = payload.indexOf("value")+7;
        int iValuePosEnd = payload.indexOf("}}]",iValuePosStart );
        strRetVal = payload.substring(iValuePosStart,iValuePosEnd);
        
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
    return strRetVal;
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {

    String strVal = getDataFromServer(strRequestDataTempFromServer);
    Serial.println("Current outdoor temperature: "+strVal+"°C");
    lcd.setCursor(0,0);           
    lcd.print(strVal+"\xDF""C");          

    strVal = getDataFromServer(strRequestDataWindSpeedFromServer);
    Serial.println("Current wind speed "+strVal+"m/s");
    Serial.println(strVal);
    lcd.print(" "+strVal+"m/s");

    lastTime = millis();
  }
}
