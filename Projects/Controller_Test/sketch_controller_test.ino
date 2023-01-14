

#include <WiFi.h>
#include <HTTPClient.h>

// Webserver
#include "WebServer.h"



#define LED_BUILTIN  2  // Blue LED on ESP32 board


// WLAN credentials
const char* pSSID = "YOUR_SSID";
const char* pPassword = "YOUR_PASSWORD";

String strSpotPrice = "1.09";              
String strWindSpeed = "4.2";
String strOutdoorTemp = "-5.3";
String strHeatingBufferTemp = "63.1";
String strHeatingBufferFakeTemp = "0.0";

String strHeatingBufferSize = "1000";
String strElectricityImportCharge = "1.5";


float fProbabilityWindElectricity = -1.0;



// Our webserver on port 80
WebServer server(80);


void setup() {
  Serial.begin(115200); 

  // Initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);


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
  
  
  Serial.print("Wifi RSSI=");
  Serial.println(WiFi.RSSI());

  delay(2000);


  server.on("/", wshandle_OnConnect);
  server.on("/calc", wshandle_OnCalc);
  server.onNotFound(wshandle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  delay(1000);

}




void wshandle_OnConnect() {
  Serial.println("WebServerHandle: OnConnect");

  String strHTML = "<!DOCTYPE html> <html>"\
  "<head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>"\
  "<title>Calefactio</title>"\
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"\
  "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}"\
  "p {font-size: 14px;color: #888;margin-bottom: 10px;}</style>"\
  "\n</head><body>"\
  "<h1>Controller Test</h1>";


  strHTML +="<h4><form action=\"/calc\">";
  strHTML +="Heating buffer size [l]: <input type=number name=\"heatingBufferSize\" value="+strHeatingBufferSize+" min=0 max=10000 step=1><br>";
  strHTML +="Electricity import charge [kr&#47;kWh]: <input type=number name=\"electricityImportCharge\" value="+strElectricityImportCharge+" min=0 max=100 step=0.01><br>";
  strHTML +="<br>Spot price [kr]: <input type=number name=\"spotPrice\" value="+strSpotPrice+" min=0 max=100 step=0.01><br>";
  strHTML +="Wind speed [m&#47;s]: <input type=number name=\"windSpeed\"  value="+strWindSpeed+" min=0 max=130 step=0.1><br>";
  strHTML +="Outdoor temperature [&#176;C]: <input type=number name=\"outdoorTemp\" value="+strOutdoorTemp+" min=-50 max=50 step=0.1><br>";
  strHTML +="<br>Heating buffer temperature [&#176;C]: <input type=number name=\"heatingBufferTemp\" value="+strHeatingBufferTemp+" min=0 max=100 step=0.1><br>";
  strHTML +="<br><input type=\"submit\" value=\"Calculate\">";
  strHTML +="</form></h4><br>";

  if( strHeatingBufferFakeTemp == "0.0" ) {
    strHTML += "";
    }
  else {
    strHTML += "Calculated data:<br>";
    strHTML += "<br>Probability of wind electricity: "+ String(fProbabilityWindElectricity,1);
    strHTML += "<br>Temperature of heating buffer: "+ strHeatingBufferFakeTemp + "&#176;C";
    }


  strHTML += "</body></html>"; 
  
  server.send(200, "text/html", strHTML);
}

void wshandle_OnCalc() {

  Serial.print("ARGS: ");
  Serial.println(server.args());
  Serial.println("Data: " + server.arg("spotPrice"));
  Serial.println("Data: " + server.arg("windSpeed"));
  Serial.println("Data: " + server.arg("outdoorTemp"));
  Serial.println("Data: " + server.arg("heatingBufferTemp"));

  strWindSpeed = server.arg("windSpeed");
  strSpotPrice = server.arg("spotPrice");
  strOutdoorTemp = server.arg("outdoorTemp");
  strHeatingBufferTemp = server.arg("heatingBufferTemp");

  strHeatingBufferSize = server.arg("heatingBufferSize");
  strElectricityImportCharge = server.arg("electricityImportCharge");
 
  float fHeatingBufferTemp = 76.4;

  strHeatingBufferFakeTemp =  String(fHeatingBufferTemp,1);

  fProbabilityWindElectricity = calcProbabilityWindElectricity(strWindSpeed);

  server.sendHeader("Location", "/",true);  
  server.send(302, "text/plain", "");
  
}


void wshandle_NotFound(){
  server.send(404, "text/plain", "Page not found");
}



float calcProbabilityWindElectricity(String strWindSpeed)
{
  float fWindSpeed = strWindSpeed.toFloat();
  float fRetVal = -1.0;

  if( fWindSpeed < 5.0 )
    fRetVal = 0.0;
  else if ( fWindSpeed == 5.0 )
    fRetVal = 0.5;
  else
    fRetVal = 1.0;
  
  return fRetVal;
}


// Main loop
void loop() {
  server.handleClient();
  delay(100);
}
