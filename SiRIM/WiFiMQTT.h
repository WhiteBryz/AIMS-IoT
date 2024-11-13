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
PubSubClient client(espClient);

class WifiMqtt{
  public:
    static void startConnections( void );
    static void connectWiFi ( void );
    static void reconnectWiFi ( void );
    bool isWiFiConnected ( void );
    static void reconnectMQTT ( void );
    bool isMQTTConnected ( void );
};

void WifiMqtt :: startConnections( void ){
    connectWiFi();
    client.setServer(mqtt_server, MQTT_PORT);
    Serial.println("Entro a startConnections y salió");
}

void WifiMqtt :: connectWiFi ( void ){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool WifiMqtt :: isWiFiConnected ( void ){
  return WiFi.isConnected();
}

void WifiMqtt :: reconnectWiFi ( void ){
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  if(!WiFi.status() != WL_CONNECTED){
    Serial.print(".");
  }
}

bool WifiMqtt :: isMQTTConnected ( void ){
  return WiFi.isConnected();
}

void WifiMqtt :: reconnectMQTT ( void ){
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
    }
}



#endif
