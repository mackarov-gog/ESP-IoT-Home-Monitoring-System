#include "MyWebServer.h"

// Объявляем extern переменные
extern bool wifiConnected;
extern bool mqttConnected;
extern bool configMode;


MyWebServer::MyWebServer(ConfigManager* cm, SensorManager* sm) : server(80), configManager(cm), sensorManager(sm) {}

void MyWebServer::begin() {
  setupRoutes();
  server.begin();
  Serial.println("Web Server started");
}


void MyWebServer::setupRoutes() {
  // Главная страница - всегда мониторинг данных
  server.on("/", [this]() {
    server.send(200, "text/html", generateMonitorHTML());
  });
  
  // Страница конфигурации
  server.on("/config", [this]() {
    server.send(200, "text/html", generateConfigHTML());
  });
  
  // API для данных в JSON формате
  server.on("/api/status", [this]() {
    server.send(200, "application/json", generateStatusJSON());
  });
  
  // Сохранение конфигурации
  server.on("/configure", HTTP_POST, [this]() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String mqtt_broker = server.arg("mqtt_broker");
    String mqtt_port_str = server.arg("mqtt_port");

    // Clean unwanted characters
    ssid.replace("\r", ""); ssid.replace("\n", "");
    password.replace("\r", ""); password.replace("\n", "");
    mqtt_broker.replace("\r", ""); mqtt_broker.replace("\n", "");


    // Проверка наличия аргумента чекбокса
  //configManager->config.wifi_enabled = server.hasArg("wifi_enabled");
  
  //configManager->saveConfig();
    
    // Save configuration
    strncpy(configManager->config.wifi_ssid, ssid.c_str(), sizeof(configManager->config.wifi_ssid));
    strncpy(configManager->config.wifi_password, password.c_str(), sizeof(configManager->config.wifi_password));
    strncpy(configManager->config.mqtt_broker, mqtt_broker.c_str(), sizeof(configManager->config.mqtt_broker));
    
    configManager->config.mqtt_port = mqtt_port_str.toInt();
    if (configManager->config.mqtt_port <= 0) configManager->config.mqtt_port = 1883;
    
    if (configManager->saveConfig()) {
      String successPage = R"raw(
<!DOCTYPE html><html><head>
<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
<style>
body{font-family:system-ui,-apple-system,sans-serif; margin:20px; background:#f5f5f5; display:flex; justify-content:center; align-items:center; min-height:100vh;}
.container{background:white; padding:30px; border-radius:15px; box-shadow:0 5px 20px rgba(0,0,0,0.1); max-width:400px; width:100%; text-align:center;}
.success{color:#4CAF50; font-size:24px; margin-bottom:15px;}
.loader{margin:20px auto; width:40px; height:40px; border:4px solid #f3f3f3; border-top:4px solid #007cba; border-radius:50%; animation:spin 1s linear infinite;}
@keyframes spin{0%{transform:rotate(0deg);}100%{transform:rotate(360deg);}}
</style></head>
<body>
<div class='container'>
<div class='success'>✅ Settings Saved!</div>
<p>Device will reboot in 3 seconds...</p>
<div class='loader'></div>
</div>
<script>setTimeout(()=>{window.location.href='/';},3000);</script>
</body></html>
)raw";
      server.send(200, "text/html", successPage);
      delay(3000);
      ESP.restart();
    } else {
      server.send(500, "text/html", "<h1>Save Error!</h1>");
    }
  });

  server.on("/reset", [this]() {
    configManager->resetConfig();
    server.send(200, "text/html", 
      "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:system-ui,-apple-system,sans-serif; margin:20px; background:#f5f5f5; display:flex; justify-content:center; align-items:center; min-height:100vh;}.container{background:white; padding:30px; border-radius:15px; box-shadow:0 5px 20px rgba(0,0,0,0.1); max-width:400px; width:100%; text-align:center;}.loader{margin:20px auto; width:40px; height:40px; border:4px solid #f3f3f3; border-top:4px solid #dc3545; border-radius:50%; animation:spin 1s linear infinite;}@keyframes spin{0%{transform:rotate(0deg);}100%{transform:rotate(360deg);}}</style></head><body><div class='container'><h1>🔄 Configuration Reset</h1><p>Device will reboot in 3 seconds...</p><div class='loader'></div></div><script>setTimeout(()=>window.location.href='/',3000)</script></body></html>");
    delay(3000);
    ESP.restart();
  });
}

String MyWebServer::generateConfigHTML() {
  String cleanSsid = String(configManager->config.wifi_ssid);
  String cleanPassword = String(configManager->config.wifi_password);
  String cleanBroker = String(configManager->config.mqtt_broker);
  
String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>IoT Device Configuration</title>";
  html += "<meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>";
  
   html += "<style>";
  html += "* { box-sizing: border-box; margin: 0; padding: 0; }";
  html += "body { ";
  html += "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;";
  html += "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);";
  html += "  margin: 0;";
  html += "  padding: 20px;";
  html += "  min-height: 100vh;";
  html += "  display: flex;";
  html += "  justify-content: center;";
  html += "  align-items: center;";
  html += "  line-height: 1.6;";
  html += "}";
  
  html += ".container { ";
  html += "  background: rgba(255, 255, 255, 0.95);";
  html += "  backdrop-filter: blur(10px);";
  html += "  padding: 25px;";
  html += "  border-radius: 20px;";
  html += "  box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1);";
  html += "  width: 100%;";
  html += "  max-width: 500px;";
  html += "  margin: 20px auto;";
  html += "  border: 1px solid rgba(255, 255, 255, 0.2);";
  html += "}";
  
  html += "h1 { ";
  html += "  text-align: center;";
  html += "  color: #2d3748;";
  html += "  margin-bottom: 25px;";
  html += "  font-size: clamp(1.5rem, 4vw, 2rem);";
  html += "  font-weight: 700;";
  html += "}";
  
  html += ".info { ";
  html += "  text-align: center;";
  html += "  margin: 20px 0;";
  html += "  padding: 15px;";
  html += "  background: linear-gradient(135deg, #e3f2fd, #bbdefb);";
  html += "  border-radius: 12px;";
  html += "  font-size: clamp(0.875rem, 2.5vw, 1rem);";
  html += "  border-left: 4px solid #2196f3;";
  html += "}";
  
  html += ".device-id { ";
  html += "  font-weight: 700;";
  html += "  color: #1976d2;";
  html += "  font-family: 'Courier New', monospace;";
  html += "  background: rgba(25, 118, 210, 0.1);";
  html += "  padding: 2px 6px;";
  html += "  border-radius: 4px;";
  html += "}";
  
  html += ".form-group { ";
  html += "  margin: 20px 0;";
  html += "}";
  
  html += "label { ";
  html += "  display: block;";
  html += "  margin: 8px 0;";
  html += "  font-weight: 600;";
  html += "  color: #2d3748;";
  html += "  font-size: clamp(0.9rem, 2.5vw, 1rem);";
  html += "}";
  
  html += "input { ";
  html += "  width: 100%;";
  html += "  padding: 15px;";
  html += "  border: 2px solid #e2e8f0;";
  html += "  border-radius: 12px;";
  html += "  font-size: 16px;";
  html += "  transition: all 0.3s ease;";
  html += "  background: white;";
  html += "}";
  
  html += "input:focus { ";
  html += "  border-color: #4299e1;";
  html += "  outline: none;";
  html += "  box-shadow: 0 0 0 3px rgba(66, 153, 225, 0.1);";
  html += "  transform: translateY(-2px);";
  html += "}";
  
  html += ".btn-group { ";
  html += "  display: flex;";
  html += "  flex-direction: column;";
  html += "  gap: 12px;";
  html += "  margin-top: 25px;";
  html += "}";
  
  html += "button, .btn { ";
  html += "  padding: 16px;";
  html += "  border: none;";
  html += "  border-radius: 12px;";
  html += "  font-size: 16px;";
  html += "  font-weight: 600;";
  html += "  cursor: pointer;";
  html += "  transition: all 0.3s ease;";
  html += "  display: flex;";
  html += "  align-items: center;";
  html += "  justify-content: center;";
  html += "  gap: 8px;";
  html += "  text-decoration: none;";
  html += "  text-align: center;";
  html += "}";

html += ".switch-container { display: flex; align-items: center; margin: 20px 0; }";
html += ".switch { position: relative; display: inline-block; width: 46px; height: 24px; margin-right: 12px; }";
html += ".switch input { opacity: 0; width: 0; height: 0; }";
html += ".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #555; transition: .3s; border-radius: 24px; }";
html += ".slider:before { position: absolute; content: ''; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; }";
html += "input:checked + .slider { background-color: #2ecc71; }";
html += "input:checked + .slider:before { transform: translateX(22px); }";

// Стиль для кнопки сохранения
html += ".save-btn { width: 100%; padding: 12px; background: #3498db; border: none; border-radius: 5px; color: white; font-weight: bold; cursor: pointer; transition: 0.3s; margin-top: 20px; }";
html += ".save-btn:hover { background: #2980b9; }";
html += ".save-btn:active { transform: scale(0.98); }";
  
  html += ".save-btn { ";
  html += "  background: linear-gradient(135deg, #48bb78, #38a169);";
  html += "  color: white;";
  html += "}";
  
  html += ".save-btn:hover { ";
  html += "  background: linear-gradient(135deg, #38a169, #2f855a);";
  html += "  transform: translateY(-2px);";
  html += "  box-shadow: 0 5px 15px rgba(72, 187, 120, 0.3);";
  html += "}";
  
  html += ".clear-btn { ";
  html += "  background: linear-gradient(135deg, #ed8936, #dd6b20);";
  html += "  color: white;";
  html += "}";
  
  html += ".clear-btn:hover { ";
  html += "  background: linear-gradient(135deg, #dd6b20, #c05621);";
  html += "  transform: translateY(-2px);";
  html += "  box-shadow: 0 5px 15px rgba(237, 137, 54, 0.3);";
  html += "}";
  
  html += ".reset-btn { ";
  html += "  background: linear-gradient(135deg, #f56565, #e53e3e);";
  html += "  color: white;";
  html += "}";
  
  html += ".reset-btn:hover { ";
  html += "  background: linear-gradient(135deg, #e53e3e, #c53030);";
  html += "  transform: translateY(-2px);";
  html += "  box-shadow: 0 5px 15px rgba(245, 101, 101, 0.3);";
  html += "}";
  
  html += ".monitor-btn { ";
  html += "  background: linear-gradient(135deg, #4299e1, #3182ce);";
  html += "  color: white;";
  html += "}";
  
  html += ".monitor-btn:hover { ";
  html += "  background: linear-gradient(135deg, #3182ce, #2c5aa0);";
  html += "  transform: translateY(-2px);";
  html += "  box-shadow: 0 5px 15px rgba(66, 153, 225, 0.3);";
  html += "}";
  
  html += "</style>";
  
  html += "<style>body{font-family:sans-serif;padding:20px;background:#f0f2f5}.container{background:white;padding:20px;border-radius:10px;max-width:500px;margin:auto;box-shadow:0 2px 10px rgba(0,0,0,0.1)}input{width:100%;padding:10px;margin:5px 0;border:1px solid #ddd;border-radius:5px}button{width:100%;padding:10px;background:#007cba;color:white;border:none;border-radius:5px;cursor:pointer;margin-top:10px}.reset-btn{background:#e00}</style>";
  html += "</head>";
  
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>⚙️ Device Configuration</h1>";
  
  html += "<div class='info'>";
  html += "Device ID: <b>" + getDeviceID() + "</b>";
  html += "<br>Current Mode: " + String(configMode ? "Configuration AP" : "Station Mode");
  html += "</div>";
  
  html += "<form action='/configure' method='POST' id='configForm'>";
  
  html += "<label>📶 WiFi SSID</label>";
  html += "<input type='text' name='ssid' value='" + cleanSsid + "' required>";
  
  html += "<label>🔑 WiFi Password</label>";
  html += "<input type='password' name='password' value='" + cleanPassword + "'>";
  
  html += "<label>📡 MQTT Broker</label>";
  html += "<input type='text' name='mqtt_broker' value='" + cleanBroker + "' required>";
  
  html += "<label>🔢 MQTT Port</label>";
  html += "<input type='number' name='mqtt_port' value='" + String(configManager->config.mqtt_port) + "'>";

 html += "<div class='input-group'>";
 html += "    <label for='wifi_enabled'>Enable Wi-Fi Connection</label>";
 html += "    <input type='checkbox' id='wifi_enabled' name='wifi_enabled' ${configManager->config.wifi_enabled ? 'checked' : ''}>";
 html += "</div>";

  html += "<button type='submit'>💾 Save Settings</button>";
  html += "</form>";
  
  html += "<form action='/reset' style='margin-top:20px' onsubmit='return confirm(\"Reset all settings?\")'>";
  html += "<button class='reset-btn'>🔄 Reset to Factory</button>";
  html += "</form>";
  
  html += "<div style='text-align:center;margin-top:20px'><a href='/' style='text-decoration:none;color:#007cba'>📊 View Sensor Data</a></div>";
  
  html += "</div></body></html>";
  
  return html;
}

String MyWebServer::generateMonitorHTML() {
  // ИСПРАВЛЕНИЕ: В JavaScript изменены имена полей (temp, hum, relay), чтобы они совпадали с sensors.cpp
  String html = R"raw(
<!DOCTYPE html>
<html>
<head>
    <title>IoT Home Monitor</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0;
            padding: 20px;
            min-height: 100vh;
        }
        .container { 
            max-width: 800px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            padding: 25px;
            border-radius: 20px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1);
        }
        .header { 
            text-align: center;
            margin-bottom: 30px;
        }
        .status-bar {
            display: flex;
            justify-content: space-between;
            margin-bottom: 20px;
            flex-wrap: wrap;
            gap: 10px;
        }
        .status-item {
            padding: 10px 15px;
            border-radius: 10px;
            color: white;
            font-weight: bold;
        }
        .wifi-connected { background: #4CAF50; }
        .wifi-disconnected { background: #f44336; }
        .mqtt-connected { background: #2196F3; }
        .mqtt-disconnected { background: #ff9800; }
        .config-mode { background: #9C27B0; }
        .sensors-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 25px;
        }
        .sensor-card {
            background: white;
            padding: 20px;
            border-radius: 15px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
            text-align: center;
            border-left: 5px solid #007cba;
        }
        .sensor-value {
            font-size: 24px;
            font-weight: bold;
            margin: 10px 0;
        }
        .sensor-unit {
            color: #666;
            font-size: 14px;
        }
        .alert { border-left-color: #f44336; background: #ffebee; }
        .normal { border-left-color: #4CAF50; background: #e8f5e8; }
        .warning { border-left-color: #ff9800; background: #fff3e0; }
        .controls {
            text-align: center;
            margin-top: 25px;
        }
        .btn {
            padding: 12px 24px;
            margin: 0 10px;
            border: none;
            border-radius: 8px;
            background: #007cba;
            color: white;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }
        .config-btn {
            background: #9C27B0;
        }
        .json-btn {
            background: #FF9800;
        }
        @media (max-width: 600px) {
            .container { padding: 15px; }
            .sensors-grid { grid-template-columns: 1fr; }
            .status-bar { flex-direction: column; }
            .btn { display: block; margin: 10px auto; width: 80%; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🏠 IoT Home Monitor</h1>
            <p>Real-time Sensor Data</p>
        </div>
        
        <div class="status-bar">
            <div class="status-item )raw" + String(wifiConnected ? "wifi-connected" : "wifi-disconnected") + R"raw(">
                WiFi: )raw" + String(wifiConnected ? "Connected" : "Disconnected") + R"raw(
            </div>
            <div class="status-item )raw" + String(mqttConnected ? "mqtt-connected" : "mqtt-disconnected") + R"raw(">
                MQTT: )raw" + String(mqttConnected ? "Connected" : "Disconnected") + R"raw(
            </div>
            )raw";
            
  if (configMode) {
    html += R"raw(<div class="status-item config-mode">Mode: Configuration AP</div>)raw";
  }
  
  html += R"raw(
        </div>
        
        <div class="sensors-grid" id="sensors-grid">
            <div class="sensor-card normal">Loading data...</div>
        </div>
        
        <div class="controls">
            <a href="/config" class="btn config-btn">⚙️ Configuration</a>
            <a href="/api/status" class="btn json-btn">📊 JSON Data</a>
        </div>
    </div>

    <script>
        function updateSensorData() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // console.log(data); // Для отладки
                    const grid = document.getElementById('sensors-grid');
                    grid.innerHTML = `
                        <div class="sensor-card ${data.temp > 30 ? 'alert' : 'normal'}">
                            <h3>🌡️ Temperature</h3>
                            <div class="sensor-value">${data.temp ? data.temp.toFixed(1) : '--'}<span class="sensor-unit">°C</span></div>
                            ${data.temp > 30 ? '<div style="color: #f44336;">High Temperature!</div>' : ''}
                        </div>
                        <div class="sensor-card ${data.hum > 80 || data.hum < 30 ? 'warning' : 'normal'}">
                            <h3>💧 Humidity</h3>
                            <div class="sensor-value">${data.hum ? data.hum.toFixed(1) : '--'}<span class="sensor-unit">%</span></div>
                            ${data.hum > 80 ? '<div style="color: #ff9800;">High Humidity!</div>' : ''}
                        </div>
                        <div class="sensor-card ${data.gas > 300 ? 'alert' : data.gas > 200 ? 'warning' : 'normal'}">
                            <h3>⚠️ Gas Level</h3>
                            <div class="sensor-value">${data.gas}</div>
                            ${data.gas > 300 ? '<div style="color: #f44336;">DANGER!</div>' : ''}
                        </div>
                        <div class="sensor-card ${data.relay ? 'normal' : 'warning'}">
                            <h3>💡 Light</h3>
                            <div class="sensor-value">${data.relay ? 'ON' : 'OFF'}</div>
                        </div>
                        <div class="sensor-card ${data.leak ? 'alert' : 'normal'}">
                            <h3>💦 Water Leak</h3>
                            <div class="sensor-value">${data.leak ? 'DETECTED!' : 'No'}</div>
                            ${data.leak ? '<div style="color: #f44336;">ALERT!</div>' : ''}
                        </div>
                        <div class="sensor-card ${data.motion ? 'warning' : 'normal'}">
                            <h3>🚶 Motion</h3>
                            <div class="sensor-value">${data.motion ? 'Detected' : 'No'}</div>
                        </div>
                    `;
                })
                .catch(error => {
                    console.error('Error:', error);
                    // document.getElementById('sensors-grid').innerHTML = 'Error loading data';
                });
        }

        // Обновляем данные каждые 2 секунды
        setInterval(updateSensorData, 2000);
        
        // Загружаем данные сразу при открытии страницы
        updateSensorData();
    </script>
</body>
</html>
)raw";
  return html;
}

String MyWebServer::generateStatusJSON() {
  String json = "{";
  
  // ИСПРАВЛЕНИЕ: Добавляем данные с сенсоров в JSON!
  sensorManager->appendToJson(json); 
  
  // Добавляем запятую перед wifi, так как appendToJson не ставит запятую в конце, но добавляет поля
  json += ",\"wifi\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"mqtt\":" + String(mqttConnected ? "true" : "false"); 
  
  json += "}";
  return json;
}


void MyWebServer::stop() {
  server.stop();
}

void MyWebServer::handleClient() {
  server.handleClient();
}

String MyWebServer::getDeviceID() {
  uint32_t chipId = (uint32_t)ESP.getEfuseMac(); 
  return String(chipId, HEX);
}