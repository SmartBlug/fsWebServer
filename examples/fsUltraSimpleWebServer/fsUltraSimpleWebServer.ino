/*
    This sketch connect to WiFi and setup a web server using simple File System and json config file
    Access Interface to http://ipaddress/ and upload files from SPIFFS directory except test.py
*/

#include <fsWebServer.h>
#include <ArduinoJson.h>

#define OTA_Version "0.0.1"

const char* ssid     = "........";
const char* password = "........";

fsWebServer server(80);
DynamicJsonDocument config(500);

//**************************************************************************************************
JsonDocument * Config(int Mode) {
  switch (Mode) {
    case CONFIG_RESET:
      config["ver"] = OTA_Version;
      config["ssid"] = ssid;
      config["pass"] = password;
      break;
    case CONFIG_RELOAD:
      config["ver"] = OTA_Version;
      break; 
  }
  return &config;
}

//**************************************************************************************************
void setup() {
  Serial.begin(115200);
  server.begin(Config,true);  // Start server with Wifi autoconnect using config["ssid"] and config["pass"]
}

//**************************************************************************************************
void loop() {
  server.handleClient();
}
