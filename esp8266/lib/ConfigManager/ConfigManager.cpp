#include "ConfigManager.h"

ConfigManager::ConfigManager() {
  EEPROM.begin(EEPROM_SIZE);
}

bool ConfigManager::loadConfig() {
  EEPROM.get(CONFIG_ADDRESS, config);
  
  // Проверяем валидность конфигурации
  if (!config.configured || strlen(config.wifi_ssid) == 0) {
    // Загружаем значения по умолчанию
    strncpy(config.wifi_ssid, DEFAULT_WIFI_SSID, sizeof(config.wifi_ssid));
    strncpy(config.wifi_password, DEFAULT_WIFI_PASS, sizeof(config.wifi_password));
    strncpy(config.mqtt_broker, DEFAULT_MQTT_BROKER, sizeof(config.mqtt_broker));
    config.mqtt_port = DEFAULT_MQTT_PORT;
    String defaultName = "ESP8266-" + String(ESP.getChipId(), HEX);
    strncpy(config.device_name, defaultName.c_str(), sizeof(config.device_name));
    config.configured = false;
    config.wifi_enabled = true;
    
    return false;
  }
  return true;
}

bool ConfigManager::saveConfig() {
  config.configured = true;
  EEPROM.put(CONFIG_ADDRESS, config);
  bool result = EEPROM.commit();
  delay(100);
  return result;
}

void ConfigManager::resetConfig() {
  memset(&config, 0, sizeof(DeviceConfig));
  config.configured = false;
  saveConfig();
}

bool ConfigManager::isConfigured() {
  return config.configured;
}

void ConfigManager::printConfig() {
  Serial.println("=== Device Configuration ===");
  Serial.printf("Device: %s\n", config.device_name);
  Serial.printf("WiFi: %s\n", config.wifi_ssid);
  Serial.printf("MQTT: %s:%d\n", config.mqtt_broker, config.mqtt_port);
  Serial.printf("Configured: %s\n", config.configured ? "Yes" : "No");
  Serial.println("============================");
}