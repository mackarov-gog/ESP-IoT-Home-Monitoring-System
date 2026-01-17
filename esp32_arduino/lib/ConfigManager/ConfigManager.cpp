#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

bool ConfigManager::loadConfig() {
  preferences.begin("iot-settings", true); // true - чтение
  
  config.configured = preferences.getBool("configured", false);
  config.wifi_enabled = preferences.getBool("wifi_on", true);
  
  if (config.configured) {
    preferences.getString("ssid", config.wifi_ssid, sizeof(config.wifi_ssid));
    preferences.getString("pass", config.wifi_password, sizeof(config.wifi_password));
    preferences.getString("mqtt_host", config.mqtt_broker, sizeof(config.mqtt_broker));
    config.mqtt_port = preferences.getInt("mqtt_port", 1883);
    preferences.getString("dev_name", config.device_name, sizeof(config.device_name));
  } else {
    // Дефолтные значения, если не настроено
    strncpy(config.wifi_ssid, DEFAULT_WIFI_SSID, sizeof(config.wifi_ssid));
    strncpy(config.wifi_password, DEFAULT_WIFI_PASS, sizeof(config.wifi_password));
    strncpy(config.mqtt_broker, DEFAULT_MQTT_BROKER, sizeof(config.mqtt_broker));
    config.mqtt_port = DEFAULT_MQTT_PORT;
    config.wifi_enabled = true;
    
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    String defaultName = "ESP32-" + String(chipId, HEX);
    strncpy(config.device_name, defaultName.c_str(), sizeof(config.device_name));
  }
  
  preferences.end();
  return config.configured;
}

bool ConfigManager::saveConfig() {
  preferences.begin("iot-settings", false); // false - запись
  
  preferences.putBool("configured", true);
  preferences.putBool("wifi_on", config.wifi_enabled);
  preferences.putString("ssid", config.wifi_ssid);
  preferences.putString("pass", config.wifi_password);
  preferences.putString("mqtt_host", config.mqtt_broker);
  preferences.putInt("mqtt_port", config.mqtt_port);
  preferences.putString("dev_name", config.device_name);
  
  preferences.end();
  config.configured = true;
  return true;
}

void ConfigManager::resetConfig() {
  preferences.begin("iot-settings", false);
  preferences.clear();
  preferences.end();
  config.configured = false;
}

bool ConfigManager::isConfigured() {
  return config.configured;
}