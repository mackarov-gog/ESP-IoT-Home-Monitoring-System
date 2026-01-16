#include "sensors.h"

// Создаем глобальный экземпляр ТОЛЬКО здесь
SensorManager sensorManager;

SensorManager::SensorManager() {
  // Инициализация начальных значений
  data.temperature = 20.0;
  data.humidity = 45.0;
  data.gas = 100;
  data.light = false;
  data.leak = false;
  data.motion = false;
  data.lightState = false;
}

void SensorManager::begin() {
  // Здесь можно добавить инициализацию реальных датчиков
  Serial.println("Sensor Manager initialized");
}

void SensorManager::update() {
  unsigned long currentMillis = millis();
  
  // Исправлено сравнение с unsigned long
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = currentMillis;
    
    // Генерация тестовых данных
    data.temperature = 20.0 + random(-5, 6) / 10.0;
    data.humidity = 45.0 + random(-10, 11) / 10.0;
    data.humidity = constrain(data.humidity, 30, 60);
    data.gas = random(50, 150);
    data.light = random(0, 2);
    data.leak = random(0, 100) > 95;
    data.motion = random(0, 100) > 80;
  }
}

SensorData SensorManager::getData() const {
  return data;
}

void SensorManager::setLightState(bool state) {
  data.lightState = state;
}


void SensorManager::publishToMqtt(PubSubClient& mqttClient) {
  // Все MQTT публикации теперь живут здесь
  mqttClient.publish("home/sensors/temp", String(data.temperature).c_str());
  mqttClient.publish("home/sensors/hum", String(data.humidity).c_str());
  mqttClient.publish("home/sensors/gas", String(data.gas).c_str());
  // Добавили новый датчик? Просто допишите строку здесь.
}

void SensorManager::appendToJson(String& json) {
  // Ключи должны быть полными, как их ждет веб-интерфейс
  json += "\"temperature\":" + String(data.temperature, 1) + ",";
  json += "\"humidity\":" + String(data.humidity, 1) + ",";
  json += "\"gas\":" + String(data.gas) + ",";
  json += "\"light\":" + String(data.light ? "true" : "false") + ",";
  json += "\"leak\":" + String(data.leak ? "true" : "false") + ",";
  json += "\"motion\":" + String(data.motion ? "true" : "false") + ","; // Запятая в конце важна
}