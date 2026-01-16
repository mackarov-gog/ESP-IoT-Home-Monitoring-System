#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "ConfigManager.h"
#include "WebServer.h"  
#include "sensors.h"


// MQTT Topics
#define TOPIC_TEMP "home/sensors/temperature"
#define TOPIC_HUM "home/sensors/humidity"
#define TOPIC_GAS "home/sensors/gas"
#define TOPIC_LIGHT "home/sensors/light"
#define TOPIC_LEAK "home/sensors/leak"
#define TOPIC_MOTION "home/sensors/motion"
#define TOPIC_LIGHT_CONTROL "home/actuators/light"
#define TOPIC_AVAILABILITY "home/sensors/availability"


void setupConfigMode();
void connectWiFi();
void publishSensorData();
void checkConnections();

ConfigManager configManager;
WebServer webServer(&configManager, &sensorManager);
WiFiClient espClient;
PubSubClient mqttClient(espClient);


unsigned long lastReconnectAttempt = 0;
unsigned long lastMQTTFailureTime = 0;
unsigned long wifiRetryStartTime = 0;
const unsigned long RECONNECT_INTERVAL = 10000; // 10 секунд для обычных попыток
const unsigned long MQTT_RETRY_INTERVAL = 180000; // 3 минуты для перезапуска
const unsigned long WIFI_RETRY_INTERVAL = 300000; // 5 минут для повторных попыток WiFi


bool wifiConnected = false;
bool mqttConnected = false;
bool configMode = false;
bool mqttRetryScheduled = false;
bool wifiRetryScheduled = false;


int wifiConnectionAttempts = 0;
int mqttConnectionAttempts = 0;
const int MAX_WIFI_ATTEMPTS = 3;
const int MAX_MQTT_ATTEMPTS = 3;


void setupWiFiAP();
void setupWiFiStation();
void disconnectAndStartAP();

void setupConfigMode();
void connectWiFi();
void publishSensorData();

void setupWiFiStation() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(configManager.config.wifi_ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(configManager.config.wifi_ssid, configManager.config.wifi_password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    wifiConnectionAttempts = 0;
    wifiRetryScheduled = false;
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnectionAttempts++;
    Serial.println("\nWiFi connection failed!");
    Serial.printf("Connection attempts: %d/%d\n", wifiConnectionAttempts, MAX_WIFI_ATTEMPTS);
  }
}

void setupWiFiAP() {
  Serial.println("Starting Configuration AP...");
  WiFi.mode(WIFI_AP);
  String apName = "IoT-Config-" + webServer.getDeviceID();
  WiFi.softAP(apName.c_str());
  
  Serial.println("Configuration AP Started");
  Serial.printf("AP: %s\n", apName.c_str());
  Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void disconnectAndStartAP() {
  Serial.println("MQTT connection failed multiple times. Disconnecting WiFi and starting AP for 3 minutes...");
  
  // Останавливаем MQTT клиент
  if (mqttClient.connected()) {
    mqttClient.disconnect();
  }
  
  // Отключаемся от WiFi
  WiFi.disconnect();
  delay(1000);
  
  // Переключаемся в режим AP
  wifiConnected = false;
  mqttConnected = false;
  configMode = true;
  mqttRetryScheduled = true;
  lastMQTTFailureTime = millis();
  mqttConnectionAttempts = 0;
  
  setupWiFiAP();
  
  Serial.println("Device is now in Configuration AP mode for 3 minutes");
  Serial.println("Connect to WiFi: " + String("IoT-Config-" + webServer.getDeviceID()));
  Serial.println("Then go to: http://192.168.4.1 to update MQTT settings");
}

void setupMQTT() {
  mqttClient.setServer(configManager.config.mqtt_broker, configManager.config.mqtt_port);
  mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }

    Serial.print("Message in topic [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    if (String(topic) == TOPIC_LIGHT_CONTROL) {
      if (message == "ON") {
        sensorManager.setLightState(true);
        mqttClient.publish(TOPIC_LIGHT, "1");
        Serial.println("Light turned ON");
      } else if (message == "OFF") {
        sensorManager.setLightState(false);
        mqttClient.publish(TOPIC_LIGHT, "0");
        Serial.println("Light turned OFF");
      }
    }
  });
  mqttClient.setBufferSize(1024);
}

bool reconnectMQTT() {
  Serial.print("Connecting to MQTT...");
  
  // Пробуем подключиться с device_name как client ID
  if (mqttClient.connect(configManager.config.device_name)) {
    Serial.println("connected");
    mqttClient.subscribe(TOPIC_LIGHT_CONTROL);
    mqttClient.publish(TOPIC_AVAILABILITY, "online");
    mqttConnectionAttempts = 0;
    return true;
  }
  
  Serial.print("failed, rc=");
  Serial.println(mqttClient.state());
  
  mqttConnectionAttempts++;
  Serial.printf("MQTT connection attempts: %d/%d\n", mqttConnectionAttempts, MAX_MQTT_ATTEMPTS);
  
  return false;
}



void checkConnections() {
  unsigned long currentMillis = millis();
  
  // Если мы в режиме конфигурации из-за MQTT, обрабатываем ожидание
  if (configMode && mqttRetryScheduled) {
    // Безопасное вычисление оставшегося времени с учетом переполнения
    unsigned long elapsed = currentMillis - lastMQTTFailureTime;
    unsigned long timeLeft = (elapsed >= MQTT_RETRY_INTERVAL) ? 0 : (MQTT_RETRY_INTERVAL - elapsed);
    
    // Если прошло 3 минуты - пытаемся подключиться снова
    if (timeLeft == 0) {
      Serial.println("3 minutes passed. Attempting to reconnect to WiFi and MQTT...");
      mqttRetryScheduled = false;
      configMode = false;
      
      // Останавливаем AP и переключаемся в STA режим
      WiFi.softAPdisconnect(true);
      delay(1000);
      
      // Пытаемся подключиться к WiFi
      setupWiFiStation();
      
      if (wifiConnected) {
        setupMQTT();
        lastReconnectAttempt = currentMillis;
      } else {
        // Если WiFi не подключился, снова переходим в AP режим на 3 минуты
        disconnectAndStartAP();
      }
    } else {
      // Показываем оставшееся время каждые 30 секунд
      static unsigned long lastStatusPrint = 0;
      if (currentMillis - lastStatusPrint > 30000) {
        lastStatusPrint = currentMillis;
        Serial.printf("MQTT AP Mode - Next connection attempt in: %lu seconds\n", timeLeft / 1000);
      }
    }
    return;
  }
  
  // Если мы в режиме конфигурации из-за WiFi, обрабатываем ожидание
  if (configMode && wifiRetryScheduled) {
    // Безопасное вычисление оставшегося времени с учетом переполнения
    unsigned long elapsed = currentMillis - wifiRetryStartTime;
    unsigned long timeLeft = (elapsed >= WIFI_RETRY_INTERVAL) ? 0 : (WIFI_RETRY_INTERVAL - elapsed);
    
    // Если прошло 5 минут - пытаемся подключиться снова
    if (timeLeft == 0) {
      Serial.println("5 minutes passed. Attempting to reconnect to WiFi...");
      wifiRetryScheduled = false;
      configMode = false;
      
      // Останавливаем AP и переключаемся в STA режим
      WiFi.softAPdisconnect(true);
      delay(1000);
      
      // Пытаемся подключиться к WiFi
      setupWiFiStation();
      
      if (wifiConnected) {
        setupMQTT();
        lastReconnectAttempt = currentMillis;
      } else {
        // Если WiFi не подключился, снова переходим в AP режим на 5 минут
        wifiRetryScheduled = true;
        configMode = true;
        wifiRetryStartTime = currentMillis;
        setupWiFiAP();
      }
    } else {
      // Показываем оставшееся время каждые 30 секунд
      static unsigned long lastStatusPrint = 0;
      if (currentMillis - lastStatusPrint > 30000) {
        lastStatusPrint = currentMillis;
        Serial.printf("WiFi AP Mode - Next connection attempt in: %lu seconds\n", timeLeft / 1000);
      }
    }
    return;
  }
  
  // Если мы в обычном режиме конфигурации (первый запуск), не пытаемся подключаться
  if (configMode) {
    return;
  }
  
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("WiFi connection lost!");
      wifiConnected = false;
      mqttConnected = false;
    }
    
    if (currentMillis - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = currentMillis;
      
      if (wifiConnectionAttempts < MAX_WIFI_ATTEMPTS) {
        if (configManager.isConfigured()) {
          setupWiFiStation();
        }
      } else {
        // Превышено количество попыток - переходим в AP режим на 5 минут
        Serial.println("Max WiFi connection attempts reached. Starting Configuration AP for 5 minutes...");
        configMode = true;
        wifiRetryScheduled = true;
        wifiRetryStartTime = currentMillis;
        setupWiFiAP();
      }
    }
    return;
  } else if (!wifiConnected) {
    wifiConnected = true;
    wifiConnectionAttempts = 0;
    wifiRetryScheduled = false;
    Serial.println("WiFi reconnected");
    if (configManager.isConfigured()) {
      setupMQTT();
    }
  }
  
  
  if (wifiConnected && !mqttClient.connected() && configManager.isConfigured()) {
    if (mqttConnected) {
      Serial.println("MQTT connection lost!");
      mqttConnected = false;
    }
    
    if (currentMillis - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = currentMillis;
      if (reconnectMQTT()) {
        mqttConnected = true;
      } else {
        
        if (mqttConnectionAttempts >= MAX_MQTT_ATTEMPTS) {
          disconnectAndStartAP();
        }
      }
    }
  } else if (!mqttConnected && mqttClient.connected()) {
    mqttConnected = true;
    Serial.println("MQTT reconnected");
  }
}

// 1. Реализация режима точки доступа
void setupConfigMode() { // Имя должно быть в точности таким
  WiFi.mode(WIFI_AP);
  String apName = "IoT-Config-" + String(ESP.getChipId(), HEX);
  WiFi.softAP(apName.c_str());
  Serial.println("Access Point Started: " + apName);
  configMode = true;
}

// 2. Реализация подключения к роутеру
void connectWiFi() { // Имя должно быть в точности таким
  WiFi.mode(WIFI_STA);
  WiFi.begin(configManager.config.wifi_ssid, configManager.config.wifi_password);
  Serial.println("Connecting to WiFi...");
  configMode = false;
}


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  
  configManager.loadConfig();
  sensorManager.begin();

  // Логика: если WiFi выключен ИЛИ устройство не настроено
  if (!configManager.config.wifi_enabled || !configManager.isConfigured()) {
    configMode = true;
    setupConfigMode(); // Запуск точки доступа (AP)
  } else {
    configMode = false;
    connectWiFi();     // Попытка подключения к роутеру
  }
  
  webServer.start();
  
  Serial.println("Setup completed");
}
 

void publishSensorData() {
  sensorManager.publishToMqtt(mqttClient);
}

void loop() {

  sensorManager.update();
  
  webServer.handleClient();
  

  
  
  static unsigned long lastPublish = 0;
  unsigned long currentMillis = millis();
  if (!configMode && mqttConnected && (currentMillis - lastPublish > 10000)) {
    publishSensorData();
    lastPublish = currentMillis;
  }
  
  
  checkConnections();
  
  if (mqttConnected) {
    mqttClient.loop();
  }
  
  
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (currentMillis - lastBlink > 1000) {
    lastBlink = currentMillis;
    
    if (configMode) {
      if (mqttRetryScheduled) {
        
        digitalWrite(LED_BUILTIN, ledState);
        ledState = !ledState;
      } else if (wifiRetryScheduled) {
        
        digitalWrite(LED_BUILTIN, ledState);
        ledState = !ledState;
      } else {
        
        digitalWrite(LED_BUILTIN, ledState);
        ledState = !ledState;
      }
    } else if (!wifiConnected) {
      
      digitalWrite(LED_BUILTIN, ledState);
      ledState = !ledState;
    } else if (!mqttConnected && configManager.isConfigured()) {
      
      digitalWrite(LED_BUILTIN, ledState);
      ledState = !ledState;
    } else if (wifiConnected && mqttConnected) {
      
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  
  delay(100);
}