#ifndef MY_WEB_SERVER_H
#define MY_WEB_SERVER_H

#include <WiFi.h>
#include <WebServer.h> // Теперь это точно подключит системную библиотеку
#include "ConfigManager.h"
#include "sensors.h"

class MyWebServer {
private:
  ::WebServer server; // Системный класс
  ConfigManager* configManager;
  SensorManager* sensorManager;
  
  void setupRoutes();
  String generateConfigHTML();
  String generateMonitorHTML();
  String generateStatusJSON();
  
public:
  MyWebServer(ConfigManager* cm, SensorManager* sm);
  void begin(); // Используем begin, как принято в Arduino
  void stop();
  void handleClient();
  String getDeviceID();
};

#endif