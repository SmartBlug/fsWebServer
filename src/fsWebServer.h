#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <CRC32.h>

//#define DEBUG
#define SERIAL_ENABLED

typedef std::function<JsonDocument *(int)> THandlerConfig;

#define CONFIG_GET      0
#define CONFIG_RESET    1
#define CONFIG_SAVE     2
#define CONFIG_RELOAD   3

#define ONBOARD_BUTTON  0
#define RESET_DELAY     2

class fsWebServer : public ESP8266WebServer {
  public:
    fsWebServer(int port = 80);
    bool loadConfig(JsonDocument *config);
    bool saveConfig(JsonDocument *config);
    void resetConfig(void);
    String getContentType(String filename);
    bool handleFileRead(String path);
    void begin(THandlerConfig hConfig, bool autoWifi=false, bool autoReset=true);

  private:
    void handleRoot(void);
    void handleFS(void);
    void handleParams(void);
    void handleFiles(void);
    void handle200(void);
    void handleFileUpload(void);
    void handleFileUpgrade(void);
    void _handleFileUpload(String filename);
    void handleDelete(void);
    void handleFormat(void);
    void handleReset(void);
    void handleConfig(void);
    void handleSaveConfig(void);
    void handleNotFound(void);

    void Success(void);
    void Error(String txt);

    THandlerConfig ConfigHandler;
    File fsUploadFile;
};