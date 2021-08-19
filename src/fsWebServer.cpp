#include "fsWebServer.h"

void fsWebServer_handleFS();

//**************************************************************************************************
void fsWebServer::Success(void) {
  this->send(200, "application/json", "{\"success\":true}");
}
//**************************************************************************************************
void fsWebServer::Error(String txt) {
  #ifdef DEBUG
    Serial.printf("Error : %s",txt.c_str());
  #endif

  this->send(500, "application/json", "{\"success\":false,\"Error\":\""+txt+"\"}");
}

//**************************************************************************************************
String fsWebServer::getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//**************************************************************************************************
bool fsWebServer::handleFileRead(String path) { // send the right file to the client (if it exists)
  #ifdef DEBUG
    Serial.println("handleFileRead: " + path);
  #endif
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = this->getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = ESP8266WebServer::streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    #ifdef DEBUG
      Serial.println(String("\tSent file: ") + path);
    #endif
    return true;
  }
  #ifdef DEBUG
    Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  #endif
  return false;
}

//**************************************************************************************************
void fsWebServer::handleRoot() {
  if (SPIFFS.exists("/index.html")) this->handleFS();
  else if (this->handleFileRead("/fs.html")) return;
       else this->handleNotFound();
  //String result = "<html><body><h1>Title</h1></body></html>";
  //this->send(200, "text/html", result);
}

//**************************************************************************************************
void fsWebServer::handleFS() {

  if (SPIFFS.exists("/fs.html")) {
    if (this->handleFileRead("/fs.html")) {
      return;
    } 
  }

  String result = "<html><body>";
  // Display File System
  Dir dir = SPIFFS.openDir("/");
  #ifdef DEBUG
    Serial.println("SPIFFS directory {/} :");
  #endif
  while (dir.next()) {
    result = result + dir.fileName();
    result = result + "<br>";
  }
  result = result + "<form action=\"upload\" method=\"post\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Upload\"></form></body></html>";

  this->send(200, "text/html", result);
}

//**************************************************************************************************
void fsWebServer::handleParams() {
  this->handleFileRead("/params.html");
}

//**************************************************************************************************
void fsWebServer::handleFiles(void) {
  bool first = true;
  String result = "[";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    if (first) first=false;
          else result = result + ",";
    result = result + "{\"name\":\"";
    result = result + dir.fileName();
    result = result + "\",\"size\":";
    File f = dir.openFile("r");
    result = result + f.size();
    result = result + "}";
    f.close();
  }
  result = result + "]";
  this->send(200, "application/json", result);
}

//**************************************************************************************************
void fsWebServer::handle200(void) {
  this->send(200, "text/plain", "");
}

//**************************************************************************************************
void fsWebServer::handleFileUpload(void){ // upload a new file to the SPIFFS
  #ifdef DEBUG
    Serial.println("handleFileUpload");
  #endif
  this->_handleFileUpload("");
}

//**************************************************************************************************
void fsWebServer::handleFileUpgrade(void){ // upload a new file to the SPIFFS
  //#ifdef DEBUG
    Serial.println("handleFileUpgrade");
  //#endif
  this->_handleFileUpload(".upgrade");
}

//**************************************************************************************************
void fsWebServer::_handleFileUpload(String filename){ // upload a new file to the SPIFFS
  StaticJsonDocument<64> data;
  #ifdef DEBUG
    Serial.println("handleFileUpload");
  #endif
  HTTPUpload& upload = this->upload();
    if (filename=="") filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
  if(upload.status == UPLOAD_FILE_START){
    //String filename = upload.filename;
    #ifdef DEBUG
      Serial.print("handleFileUpload Name: "); Serial.println(filename);
    #endif
    this->fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(this->fsUploadFile)
      this->fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(this->fsUploadFile) {                                    // If the file was successfully created
      this->fsUploadFile.close();                               // Close the file again
      //#ifdef DEBUG
        Serial.print("handleFileUpload Size: "); 
        Serial.println(upload.totalSize);
      //#endif
      //server.sendHeader("Location","/fs");      // Redirect the client to the success page
      //server.send(303);
      if (filename=="/.upgrade") {
        char buffer[64];
        const uint32_t KNOWN_CHECKSUM = 0x4A17B156;
        CRC32 crc;

        this->fsUploadFile = SPIFFS.open(filename, "r");
        //while (this->fsUploadFile.available()) {
          int l = this->fsUploadFile.readBytesUntil('\n', buffer, sizeof(buffer));
          buffer[l] = 0;
          Serial.println(buffer);
        //}
        char chars[2];
        while (this->fsUploadFile.available()) {
          fsUploadFile.readBytes(chars,1);
          crc.update(chars[0]);
        }
        uint32_t checksum = crc.finalize();
        snprintf(buffer, sizeof(buffer), "%ld", checksum);
        Serial.println(buffer);

        this->fsUploadFile.close();    

        DeserializationError error = deserializeJson(data, buffer);
        //this->send(200, "application/json", "{\"success\":true,\"filename\":\""+filename+"\",\"version\":\""+(const char *)data["version"]+"\"}");
        this->send(200, "application/json", "{\"success\":true,\"filename\":\""+filename+"\",\"version\":\""+(const char *)data["version"]+"\"}");
      }
      else this->Success();
    } else {
      //this->send(500, "text/plain", "{\"success\":false}");
      this->Error("File Error");
    }
  }
}
//**************************************************************************************************
void fsWebServer::handleDelete(void) {
  StaticJsonDocument<50> data;
  DeserializationError error = deserializeJson(data, this->arg("plain"));
  if (SPIFFS.remove((const char *)data["name"])) {
    //this->send(200, "application/json", "{\"success\":true}");
    this->Success();
  } else {
    //this->send(500, "text/plain", "{\"success\":false}");
    this->Error("not removed");
  }
}


//**************************************************************************************************
void fsWebServer::handleFormat(void) {
  #ifdef DEBUG
    Serial.print("Formating...");
  #endif
  SPIFFS.format();
  #ifdef DEBUG
    Serial.println("OK !");
  #endif

  if (this->ConfigHandler) this->saveConfig(this->ConfigHandler(CONFIG_RESET));
  this->Success();
}

//**************************************************************************************************
void fsWebServer::handleReset(void) {
  ESP.restart();
}

//**************************************************************************************************
void fsWebServer::handleNotFound(void) {
  if (this->handleFileRead(this->uri())) {
    return;
  }
  this->send(404, "text/plain", "Not found");
}

//**************************************************************************************************
void fsWebServer::handleConfig(void) {
/*
  #ifdef DEBUG
    Serial.println("handleConfig");
  #endif
  //this->send_P(200, "application/json", NULL);
  WiFiClient client = this->client();
  JsonDocument * config;
  config = this->ConfigHandler(CONFIG_GET);
*/
  //serializeJson(*config, Serial);
  //this->send(200, "application/json","");
  //this->send(200, "application/json", "configtxt");
  //this->send_P(200, "application/json", NULL);
  //this->sendHeader("Content-Type","application/json");
  serializeJson(*this->ConfigHandler(CONFIG_GET), this->client());
/*  return;

  String configtxt;

  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    #ifdef DEBUG
      Serial.println("file open failed");
    #endif
    this->Error("can't open config");
  }
  configtxt = file.readString();
  //#ifdef OTA_Version
  //  configtxt = configtxt.substring(configtxt.length()-1) + ",\"ver\":\"" + OTA_Version + "\"}";
  //#endif
  file.close();
  this->send(200, "application/json", configtxt);*/
}

//**************************************************************************************************
void fsWebServer::handleSaveConfig() {

  if (!this->ConfigHandler) this->Error("No Config");
  //Serial.println(this->arg("plain"));
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    #ifdef DEBUG
      Serial.println("Failed to write to config file");
    #endif
    //this->send ( 200, "text/json", "{success:false}");
    this->Error("Failed to write");
    return;
  }
  file.println(this->arg("plain"));
  file.close();
  // On recharge
  if (SPIFFS.exists("/config.json")) loadConfig(this->ConfigHandler(CONFIG_GET)); //this->ConfigHandler(CONFIG_RELOAD);

  //this->send ( 200, "text/json", "{success:true}" );
  //this->Success();
  this->handleConfig();
}

//**************************************************************************************************
//void fsWebServer::handleResetConfig() {
void fsWebServer::resetConfig() {
  //JsonDocument * config;
  #ifdef DEBUG
    Serial.println("Reseting config file...");
  #endif
  if (!this->ConfigHandler) this->Error("No Config");

  //config = this->ConfigHandler(CONFIG_GET);
  this->ConfigHandler(CONFIG_GET)->clear();
  //config->clear();
  this->saveConfig(this->ConfigHandler(CONFIG_RESET));
  /*if (SPIFFS.exists("/config.json")) {
    loadConfig(*config);
    this->ConfigHandler(CONFIG_RELOAD);
  }*/
  this->handleConfig();
}

//**************************************************************************************************
/*bool fsWebServer::loadConfig(DynamicJsonDocument *config) {
  #ifdef DEBUG
    Serial.println("Loading config file...");
  #endif
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    #ifdef DEBUG
      Serial.println("Failed to open config file");
    #endif
    if (this->ConfigHandler) this->ConfigHandler(CONFIG_RESET);
    return false;
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(*config, file);
  if (error) {
    #ifdef DEBUG
      Serial.println(F("Failed to read file, using default configuration"));
    #endif
    //initConfig(false);
    return false;
  }
  file.close();
  
  return true;
}
*/
//**************************************************************************************************
bool fsWebServer::loadConfig(JsonDocument *config) {
  #ifdef DEBUG
    Serial.println("Loading config file...");
  #endif
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    #ifdef DEBUG
      Serial.println("Failed to open config file");
    #endif
    this->resetConfig();
    /*if (this->ConfigHandler) {
      config->clear();
      this->saveConfig(this->ConfigHandler(CONFIG_RESET));
    
    return false;}
    */
  }

  /*bool first = true;
  String result = "[";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    if (first) first=false;
          else result = result + ",";
    result = result + "{\"name\":\"";
    result = result + dir.fileName();
    result = result + "\",\"size\":";
    File f = dir.openFile("r");
    result = result + f.size();
    result = result + "}";
    f.close();
  }
  result = result + "]";
  Serial.println(result);*/

  /*#ifdef DEBUG
    Serial.println("Before:");
    serializeJson(*config, Serial);
  #endif*/  
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(*config, file);
  this->ConfigHandler(CONFIG_RELOAD);
  /*#ifdef DEBUG
    DynamicJsonDocument tst = *config;
    Serial.println("After:");
    Serial.println((const char *)tst["ver"]);
    serializeJson(*config, Serial);
  #endif*/  

  if (error) {
    #ifdef DEBUG
      Serial.println(F("Failed to read file, using default configuration"));
    #endif
    //initConfig(false);
    return false;
  }
  file.close();
  
  return true;
}

//**************************************************************************************************
bool fsWebServer::saveConfig(JsonDocument *config) {
  #ifdef DEBUG
    Serial.println("Saving config file...");
  #endif
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    #ifdef DEBUG
      Serial.println("Failed to open config file for writing");
    #endif
    return false;
  }

  // Serialize JSON to file
  if (serializeJson(*config, file) == 0) {
    #ifdef DEBUG
      Serial.println(F("Failed to write to file"));
    #endif
    file.close();
    return false;
  }
  file.close();
  return true;
}

//**************************************************************************************************
void fsWebServer::setConfigHandler(THandlerConfig Config, bool autoReset) {
  this->ConfigHandler = Config;
  if (autoReset) {
    Serial.println(F("\nPress button to reset config..."));
    delay(1000*RESET_DELAY);
    pinMode(ONBOARD_BUTTON, INPUT);
    if (digitalRead(ONBOARD_BUTTON)==0) {
      #ifdef DEBUG
        Serial.println("Button pressed, reseting config file...");
      #endif
      this->ConfigHandler(CONFIG_GET)->clear();
      this->saveConfig(this->ConfigHandler(CONFIG_RESET));
    }
  }

  if (SPIFFS.exists("/config.json")) this->loadConfig(this->ConfigHandler(CONFIG_GET));
  else this->saveConfig(this->ConfigHandler(CONFIG_RESET));
}

//**************************************************************************************************
fsWebServer::fsWebServer(int port = 80):ESP8266WebServer(port) {

  SPIFFS.begin();
  this->ConfigHandler = 0;
  
  ESP8266WebServer::on("/", HTTP_GET, std::bind(&fsWebServer::handleRoot, this));
  ESP8266WebServer::on("/fs", HTTP_GET, std::bind(&fsWebServer::handleFS, this));
  ESP8266WebServer::on("/params", HTTP_GET, std::bind(&fsWebServer::handleParams, this));
  ESP8266WebServer::on("/files", HTTP_GET, std::bind(&fsWebServer::handleFiles, this));
  ESP8266WebServer::on("/delete", HTTP_POST, std::bind(&fsWebServer::handleDelete, this));
  ESP8266WebServer::on("/format", HTTP_GET, std::bind(&fsWebServer::handleFormat, this));
  ESP8266WebServer::on("/reset", HTTP_GET, std::bind(&fsWebServer::handleReset, this));
  ESP8266WebServer::on("/upload", HTTP_POST, std::bind(&fsWebServer::handle200, this),std::bind(&fsWebServer::handleFileUpload,this));
  ESP8266WebServer::on("/upgrade", HTTP_POST, std::bind(&fsWebServer::handle200, this),std::bind(&fsWebServer::handleFileUpgrade,this));
  ESP8266WebServer::on("/config", HTTP_GET, std::bind(&fsWebServer::handleConfig, this));
  ESP8266WebServer::on("/saveConfig", HTTP_POST, std::bind(&fsWebServer::handleSaveConfig, this));
  //ESP8266WebServer::on("/resetConfig", HTTP_GET, std::bind(&fsWebServer::handleResetConfig, this));
  ESP8266WebServer::on("/resetConfig", HTTP_GET, std::bind(&fsWebServer::resetConfig, this));
  ESP8266WebServer::onNotFound(std::bind(&fsWebServer::handleNotFound,this));  
}