#ifndef MQTT_h
#define MQTT_h

#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

#define MQTT_PORT 1883

// Estructura para credenciales WiFi y MQTT
struct KeysEnv env;

const char* ssid = env.ssid;
const char* password = env.password;
const char* mqtt_server = env.mqtt_server;

// WiFi y MQTT Clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char* topicRX = "ucol/iot/config";

class MQTTHandler {
  public:
    static void startConnections();
    static void connectWiFi();
    static void reconnectWiFi();
    static bool isWiFiConnected();
    static void reconnectMQTT();
    static bool isMQTTConnected();
    static void publishMessage(const char* topic, const char* payload);
};

void MQTTHandler::startConnections() {
    connectWiFi();
    mqttClient.setServer(mqtt_server, MQTT_PORT);
    Serial.println("Iniciando conexiones MQTT y WiFi...");
}

void MQTTHandler::connectWiFi() {
    Serial.print("Conectando a WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nWiFi conectado.");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
}

bool MQTTHandler::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void MQTTHandler::reconnectWiFi() {
    if (!isWiFiConnected()) {
        Serial.println("Reconectando a WiFi...");
        connectWiFi();
    }
}

bool MQTTHandler::isMQTTConnected() {
    return mqttClient.connected();
}

void MQTTHandler::reconnectMQTT() {
    if (!isMQTTConnected()) {
        Serial.print("Intentando conectar a MQTT...");
        if (mqttClient.connect("IoTRiegoAutoMKUltra")) {
            Serial.println("Conectado a MQTT.");
            mqttClient.subscribe(topicRX);
            Serial.println("Suscrito al topic ucol/iot/confs.");
        } else {
            Serial.print("Error de conexión MQTT. Código: ");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}

void MQTTHandler::publishMessage(const char* topic, const char* payload) {
    if (isMQTTConnected()) {
        mqttClient.publish(topic, payload);
        Serial.print("Mensaje publicado en el topic ");
        Serial.print(topic);
        Serial.print(": ");
        Serial.println(payload);
    } else {
        Serial.println("No se puede publicar. MQTT no está conectado.");
    }
}

#endif