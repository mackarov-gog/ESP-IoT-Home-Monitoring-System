#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include "ConfigManager.h"
#include "sensors.h"

class WebServer {
private:
  ESP8266WebServer server;
  ConfigManager* configManager;
  SensorManager* sensorManager;
  
  void setupRoutes();
  String generateConfigHTML();
  String generateMonitorHTML();
  String generateStatusJSON();
  
public:
  WebServer(ConfigManager* cm, SensorManager* sm);
  void start();
  void stop();
  void handleClient();
  String getDeviceID();
};

#endif