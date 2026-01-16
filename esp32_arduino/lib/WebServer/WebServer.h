#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>          
#include <WebServer.h>     
#include "ConfigManager.h"
#include "sensors.h"

class WebServer {
private:
    ::WebServer server;    
    ConfigManager* configManager;
    SensorManager* sensorManager;

    void handleRoot();
    void handleSave();
    void handleStatus();
    String getDeviceID();

public:
    WebServer(ConfigManager* cm, SensorManager* sm);
    void begin();
    void handleClient();
};

#endif