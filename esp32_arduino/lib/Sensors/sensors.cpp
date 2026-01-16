#include "sensors.h"

SensorManager sensorManager;

SensorManager::SensorManager() {
  data = {0, 0, 0, false, false, false};
}

void SensorManager::begin() {
  pinMode(PIN_MOTION, INPUT);
  pinMode(PIN_LEAK, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  
  // Настройка АЦП ESP32 (0-4095)
  analogReadResolution(12); 
  Serial.println("Sensors initialized on ESP32");
}

void SensorManager::update() {
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();

    // Чтение цифровых датчиков
    data.motion = digitalRead(PIN_MOTION);
    
    // Чтение аналоговых датчиков (на ESP32 ADC1: пины 32-39)
    data.gas = analogRead(PIN_GAS); 
    
    // Пример обработки протечки (если датчик аналоговый)
    data.leak = (analogRead(PIN_LEAK) > 2000); 

    // Эмуляция температуры для примера (или чтение DHT)
    data.temperature = 22.0 + (random(-10, 10) / 10.0);
    data.humidity = 50.0 + (random(-20, 20) / 10.0);
  }
}

void SensorManager::setRelay(bool state) {
  data.relayState = state;
  digitalWrite(PIN_RELAY, state ? HIGH : LOW);
}

void SensorManager::publishToMqtt(PubSubClient& mqttClient, const char* baseTopic) {
  if (!mqttClient.connected()) return;
  
  char buf[10];
  dtostrf(data.temperature, 4, 1, buf);
  mqttClient.publish((String(baseTopic) + "/temp").c_str(), buf);
  
  mqttClient.publish((String(baseTopic) + "/motion").c_str(), data.motion ? "ON" : "OFF");
  mqttClient.publish((String(baseTopic) + "/gas").c_str(), String(data.gas).c_str());
}

void SensorManager::appendToJson(String& json) {
  json += "\"temp\":" + String(data.temperature) + ",";
  json += "\"hum\":" + String(data.humidity) + ",";
  json += "\"gas\":" + String(data.gas) + ",";
  json += "\"leak\":" + String(data.leak ? "true" : "false") + ",";
  json += "\"motion\":" + String(data.motion ? "true" : "false") + ",";
  json += "\"relay\":" + String(data.relayState ? "true" : "false");
}