#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <PubSubClient.h>

// Назначение пинов для ESP32
#define PIN_TEMP_HUM 4    // DHT22 (Digital)
#define PIN_GAS      34   // MQ Sensor (Analog - только ADC1)
#define PIN_LEAK     35   // Water Leak (Analog/Digital)
#define PIN_MOTION   13   // PIR Sensor (Digital)
#define PIN_RELAY    12   // Нагрузка/Свет (Digital Out)

struct SensorData {
  float temperature;
  float humidity;
  int gas;
  bool leak;
  bool motion;
  bool relayState;
};

class SensorManager {
private:
  SensorData data;
  unsigned long lastUpdate = 0;
  const unsigned long UPDATE_INTERVAL = 2000;

public:
  SensorManager();
  void begin();
  void update();
  void setRelay(bool state);
  void publishToMqtt(PubSubClient& mqttClient, const char* baseTopic);
  void appendToJson(String& json);
  SensorData getData() { return data; }
};

extern SensorManager sensorManager;

#endif