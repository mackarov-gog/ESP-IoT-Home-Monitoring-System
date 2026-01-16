#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>
#include "secrets.h"

struct DeviceConfig {
  char wifi_ssid[32];
  char wifi_password[64];
  char mqtt_broker[64];
  int mqtt_port;
  char device_name[32];
  bool configured;
};

class ConfigManager {
private:
  Preferences preferences;
  
public:
  DeviceConfig config;
  
  ConfigManager();
  bool loadConfig();
  bool saveConfig();
  void resetConfig();
  bool isConfigured();
};

#endif