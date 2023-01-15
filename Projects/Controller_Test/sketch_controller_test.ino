

#include <WiFi.h>
#include <HTTPClient.h>

#include <WiFiUdp.h>

// Webserver
#include "WebServer.h"



#define LED_BUILTIN  2  // Blue LED on ESP32 board
#define MAX_FORECAST_COUNT 10

// WLAN credentials
const char* pSSID = "YOUR_SSID";
const char* pPassword = "YOUR_PASSWORD";


#define MAX_FORECAST_COUNT 10

typedef struct {
  String strSpotPrice = "0";
  String strWindSpeed = "0";
  String strOutdoorTemp = "0";
} forecastDataType;

forecastDataType forecastData[MAX_FORECAST_COUNT];

String strSpotPriceInit[MAX_FORECAST_COUNT] = { "0.74", "0.86", "1.07", "1.44", "1.68", "1.73", "1.42", "1.21", "1.12", "1.04"};
String strWindSpeedInit[MAX_FORECAST_COUNT] = { "6", "6", "5", "5", "4", "4", "5", "5", "6", "6"};
String strOutdoorTempInit[MAX_FORECAST_COUNT] = { "9", "8", "7", "6", "6", "5", "5", "4", "4", "4"};



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

  for( int i=0; i<MAX_FORECAST_COUNT; i++ ) {
    forecastData[i].strSpotPrice = strSpotPriceInit[i];
    forecastData[i].strWindSpeed = strWindSpeedInit[i];
    forecastData[i].strOutdoorTemp = strOutdoorTempInit[i];
    Serial.println("i="+String(i)+":"+forecastData[i].strSpotPrice);
    }

}




void wshandle_OnConnect() {
  Serial.println("WebServerHandle: OnConnect");

  String strHTML = "<!DOCTYPE html> <html>"\
  "<head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>"\
  "<title>Calefactio</title>"\
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }"\
  "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}"\
  "p {font-size: 14px;color: #888;margin-bottom: 10px;}</style>"\
  "\n</head><body>"\
  "<h1>Controller Test</h1>";

  strHTML += "<center>";
  strHTML +="<h4><form action=\"/calc\">";


  strHTML +="Heating buffer size [l]: <input type=number name=\"heatingBufferSize\" value="+strHeatingBufferSize+" min=0 max=10000 step=1>";
  strHTML +="<br>Electricity import charge [kr&#47;kWh]: <input type=number name=\"electricityImportCharge\" value="+strElectricityImportCharge+" min=0 max=100 step=0.01>";
  strHTML +="<br>Heating buffer temp. [&#176;C]: <input type=number name=\"heatingBufferTemp\" value="+strHeatingBufferTemp+" min=0 max=100 step=0.1><br>";
  
  strHTML += "<br><table>";
  strHTML += "<tr><th>Data set</th>";
  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    strHTML += "<th>"+String(i)+"</th>";
  } strHTML += "</tr>";


  strHTML += "<tr><td>Wind speed [m&#47;s]:</td>";
  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    String strName = "windSpeed_"+String(i);
    strHTML += "<td><input type=number name=\""+strName+"\" value="+forecastData[i].strWindSpeed+" min=0 max=130 step=0.1></td>";
  } strHTML += "</tr>";


  strHTML += "<tr><td>Outdoor temp. [&#176;C]:</td>";
  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    String strName = "outdoorTemp_"+String(i);
    strHTML += "<td><input type=number name=\""+strName+"\" value="+forecastData[i].strOutdoorTemp+" min=-50 max=50 step=0.1></td>";
  } strHTML += "</tr>";


  strHTML += "<tr><td>Spot price [kr]:</td>";
  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    String strName = "spotPrice_"+String(i);
    strHTML += "<td><input type=number name=\""+strName+"\" value="+forecastData[i].strSpotPrice+" min=0 max=100 step=0.01></td>";
  } strHTML += "</tr>";

  strHTML += "</table>";



  strHTML +="<br><input type=\"submit\" value=\"Calculate...\">";
  strHTML +="</form></h4><br>";

  

  strHTML += "<br><table>";
  strHTML += "<tr><th>Forecast set</th>";
  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    strHTML += "<th>"+String(i)+"</th>";
  } strHTML += "</tr>";


  strHTML += "<tr><td>Probability wind electricity:</td>";
  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    String strValue = String(calcProbabilityWindElectricity(forecastData[i].strWindSpeed),1);
    strHTML += "<td>"+strValue+"</td>";
  } strHTML += "</tr>";

  strHTML += "</table>";


  strHTML += "</center>";

  strHTML += "</body></html>"; 
  
  server.send(200, "text/html", strHTML);
}

void wshandle_OnCalc() {

  Serial.print("ARGS: ");
  Serial.println(server.args());

  strHeatingBufferTemp = server.arg("heatingBufferTemp");
  strHeatingBufferSize = server.arg("heatingBufferSize");
  strElectricityImportCharge = server.arg("electricityImportCharge");

  for(int i=0; i<MAX_FORECAST_COUNT; i++) {
    String strName = "outdoorTemp_"+String(i); forecastData[i].strOutdoorTemp = server.arg(strName);
    strName = "windSpeed_"+String(i); forecastData[i].strWindSpeed = server.arg(strName);
    strName = "spotPrice_"+String(i); forecastData[i].strSpotPrice = server.arg(strName);
  }
 
  float fHeatingBufferTemp = 76.4;
  strHeatingBufferFakeTemp =  String(fHeatingBufferTemp,1);

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
