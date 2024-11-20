#ifndef WiFiMQTT_h
#define WiFiMQTT_h

#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

#define MQTT_PORT 1883

// Crear un archivo llamado env.h con los valores y agregarlo:
// struct KeysEnv {
// 	char* ssid = "[ssid]";
//   char* password =  "[pswd]";
// 	char* mqtt_server = "[server]";
// } KeysEnv;
struct KeysEnv env;

const char* ssid = env.ssid;
const char* password =  env.password;
const char* mqtt_server = env.ssid;

// Conexiones
WiFiClient espClient;
PubSubClient mqttClient(espClient);

class WifiMqtt{
  public:
    static void startConnections( void );
    static void connectWiFi ( void );
    static void reconnectWiFi ( void );
    static bool isWiFiConnected ( void );
    static void connectMQTT( void );
    static void reconnectMQTT ( void );
    static bool isMQTTConnected ( void );
    static void publishMessage(const char* topic, const char* payload);
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    static void subscribeTopic(char* topic);
};

void WifiMqtt :: startConnections( void ){
    connectWiFi();
    connectMQTT();
    Serial.println("Iniciando conexiones MQTT y WiFI");
}

void WifiMqtt :: connectWiFi ( void ){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (!isWiFiConnected()) {
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool WifiMqtt :: isWiFiConnected ( void ){
  return WiFi.status() == WL_CONNECTED;
}

void WifiMqtt :: reconnectWiFi ( void ){
  WiFi.disconnect();
  if(!isWiFiConnected()){
    Serial.print("Reconectando a WiFi...");
    connectWiFi();
  }
}

void WifiMqtt :: connectMQTT( void ){
  mqttClient.setServer(mqtt_server, MQTT_PORT);
}

bool WifiMqtt :: isMQTTConnected ( void ){
  return mqttClient.connected();
}

void WifiMqtt :: reconnectMQTT ( void ){
    Serial.print("Attempting MQTT connection...");
    if (isMQTTConnected()) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      connectMQTT();
    }
}

void WifiMqtt :: publishMessage(const char* topic, const char* payload){
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

// Función de callback para manejar mensajes entrantes
void WifiMqtt :: mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en el topic: ");
    Serial.println(topic);

    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Contenido del mensaje: ");
    Serial.println(message);

    // if (String(topic) == topicWorked) {
    //     Serial.println("Procesando mensaje del topic ucol/iot...");
    // }
}

void WifiMqtt :: subscribeTopic(char* topic){
  mqttClient.subscribe(topic);
}

#endif
