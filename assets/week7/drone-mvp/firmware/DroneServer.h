#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "index.h"
#include "ServoController.h"

class DroneServer {
public:
  DroneServer(const char* ssid, const char* password, IPAddress ip, ServoController& servos)
    : _ssid(ssid), _password(password), _ip(ip), _servos(servos),
      _http(80), _ws(81) {
    _instance = this;
  }

  void begin() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(_ip, _ip, IPAddress(255, 255, 255, 0));
    WiFi.softAP(_ssid, _password);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    _http.on("/", HTTP_GET, []() {
      _instance->_http.send_P(200, "text/html", INDEX_HTML);
    });
    _http.begin();
    Serial.println("HTTP server started on port 80");

    _ws.onEvent(_wsEvent);
    _ws.begin();
    Serial.println("WebSocket server started on port 81");
  }

  void loop() {
    _http.handleClient();
    _ws.loop();
  }

private:
  inline static DroneServer* _instance = nullptr;

  static void _wsEvent(uint8_t clientId, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
      _instance->_handleMessage(String((char*)payload));
    } else if (type == WStype_CONNECTED) {
      IPAddress ip = _instance->_ws.remoteIP(clientId);
      Serial.printf("WS client %d connected from %s\n", clientId, ip.toString().c_str());
    } else if (type == WStype_DISCONNECTED) {
      Serial.printf("WS client %d disconnected\n", clientId);
    }
  }

  void _handleMessage(String msg) {
    int sep = msg.indexOf(':');
    if (sep < 1) return;
    int index = msg.substring(0, sep).toInt();
    int angle = msg.substring(sep + 1).toInt();
    _servos.move(index, angle);
  }

  const char*      _ssid;
  const char*      _password;
  IPAddress        _ip;
  ServoController& _servos;
  WebServer        _http;
  WebSocketsServer _ws;
};
