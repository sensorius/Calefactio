
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_PASSWORD";


String strRequestDataFromServer = "http://dmigw.govcloud.dk/v2/climateData/collections/10kmGridValue/items?"\
                                  "cellId=10km_635_54&limit=1&parameterId=mean_temp&api-key="\
                                  "YOUR_API_KEY";
                              

unsigned long timerDelay = 60000*5;  // 5 minutes
unsigned long lastTime = timerDelay;


void setup() {
  Serial.begin(115200); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi, my IP Address is: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      // Get data from server
      http.begin(strRequestDataFromServer.c_str());
     
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();

        // Get numeric data we are interested in -> <SNIP> 00+01:00","value":6.3}}],"timeSta <SNIP>
        
        int iValuePosStart = payload.indexOf("value")+7;
        int iValuePosEnd = payload.indexOf("}}]",iValuePosStart );
        String strBuffer = payload.substring(iValuePosStart,iValuePosEnd);
        
        Serial.println("Current outdoor temperature: "+strBuffer+"°C");
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
    lastTime = millis();
  }
}
