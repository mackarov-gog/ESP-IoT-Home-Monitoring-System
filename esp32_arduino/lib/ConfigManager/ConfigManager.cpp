#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

bool ConfigManager::loadConfig() {
  preferences.begin("iot-settings", true); // true - режим только чтение
  
  // Если ключ "configured" не найден, вернет false
  config.configured = preferences.getBool("configured", false);
  
  if (config.configured) {
    preferences.getString("ssid", config.wifi_ssid, sizeof(config.wifi_ssid));
    preferences.getString("pass", config.wifi_password, sizeof(config.wifi_password));
    preferences.getString("mqtt_host", config.mqtt_broker, sizeof(config.mqtt_broker));
    config.mqtt_port = preferences.getInt("mqtt_port", 1883);
    preferences.getString("dev_name", config.device_name, sizeof(config.device_name));
  } else {
    // Значения по умолчанию
    strncpy(config.wifi_ssid, DEFAULT_WIFI_SSID, sizeof(config.wifi_ssid));
    strncpy(config.wifi_password, DEFAULT_WIFI_PASS, sizeof(config.wifi_password));
    strncpy(config.mqtt_broker, DEFAULT_MQTT_BROKER, sizeof(config.mqtt_broker));
    config.mqtt_port = DEFAULT_MQTT_PORT;
    
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    String defaultName = "ESP32-" + String(chipId, HEX);
    strncpy(config.device_name, defaultName.c_str(), sizeof(config.device_name));
  }
  
  preferences.end();
  return config.configured;
}

bool ConfigManager::saveConfig() {
  preferences.begin("iot-settings", false); // false - режим записи
  
  preferences.putString("ssid", config.wifi_ssid);
  preferences.putString("pass", config.wifi_password);
  preferences.putString("mqtt_host", config.mqtt_broker);
  preferences.putInt("mqtt_port", config.mqtt_port);
  preferences.putString("dev_name", config.device_name);
  preferences.putBool("configured", true);
  
  preferences.end();
  return true;
}

void ConfigManager::resetConfig() {
  preferences.begin("iot-settings", false);
  preferences.clear(); // Удаляет все ключи в этом пространстве
  preferences.end();
}

bool ConfigManager::isConfigured() {
  return config.configured;
}