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
      Serial.println("[CONFIG_RESET]");
      config["ver"] = OTA_Version;
      config["ssid"] = ssid;
      config["pass"] = password;
      break;
    case CONFIG_RELOAD:
      Serial.println("[CONFIG_RELOAD]");
      break; 
  }
  return &config;
}

//**************************************************************************************************
void setup() {
  Serial.begin(115200);
  server.setConfigHandler(Config,true);
  server.begin();
  // Connect to Wifi
  Serial.print("Connecting to ");
  Serial.println((const char *)config["ssid"]);
  WiFi.begin((const char *)config["ssid"], (const char *)config["pass"]);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//**************************************************************************************************
void loop() {
  server.handleClient();
}
