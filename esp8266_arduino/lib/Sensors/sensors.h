#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <PubSubClient.h>

// Структура для хранения данных всех датчиков
struct SensorData {
  float temperature;
  float humidity;
  int gas;
  bool light;
  bool leak;
  bool motion;
  bool lightState;
};

class SensorManager {
private:
  SensorData data;
  unsigned long lastUpdate = 0;
  const unsigned long UPDATE_INTERVAL = 5000;  // Исправлено на unsigned long

public:
  SensorManager();
  void begin();
  void update();
  SensorData getData() const;
  void setLightState(bool state);
  void publishToMqtt(PubSubClient& mqttClient); // Для MQTT
  void appendToJson(String& json);              // Для Web-интерфейса
};


extern SensorManager sensorManager;

#endif