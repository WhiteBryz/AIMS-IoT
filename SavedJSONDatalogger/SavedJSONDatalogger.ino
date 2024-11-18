#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include "MQTT.h"

// Configuración de MQTT
const char* topicWorked = "ucol/iot";

// Pines y configuración de dispositivos
#define TRIGGER 26
#define ECHO 25
#define VELOCIDAD_SONIDO 0.034
#define LDR_PIN 35
#define SOIL_MOISTURE1_PIN 34
#define SOIL_MOISTURE2_PIN 33
#define RELAY1_PIN 32
#define RELAY2_PIN 27
#define DHT_PIN 16
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT11);
RTC_DS1307 rtc;

// Función de callback para manejar mensajes entrantes
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en el topic: ");
    Serial.println(topic);

    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Contenido del mensaje: ");
    Serial.println(message);

    if (String(topic) == topicWorked) {
        Serial.println("Procesando mensaje del topic ucol/iot...");
    }
}

// Función para guardar datos en la SD
void guardarEnSD(const String& data) {
    // Abrir el archivo en modo escritura/apéndice
    File file = SD.open("/datalog.txt", FILE_APPEND);
    if (file) {
        file.println(data); // Escribir datos en el archivo
        file.close();       // Cerrar el archivo
        Serial.println("Datos guardados en datalog.txt:");
        Serial.println(data);
    } else {
        Serial.println("Error al abrir datalog.txt para escritura.");
    }
}

void setup() {
    Serial.begin(115200);

    // Iniciar conexiones WiFi y MQTT
    MQTTHandler::startConnections();

    // Establecer el callback
    mqttClient.setCallback(mqttCallback);

    // Suscribirse al topic
    if (MQTTHandler::isMQTTConnected()) {
        mqttClient.subscribe(topicWorked);
        Serial.println("Suscrito al topic ucol/iot.");
    }

    // Inicialización de componentes
    lcd.init();
    lcd.backlight();
    lcd.print("Iniciando...");

    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(SOIL_MOISTURE1_PIN, INPUT);
    pinMode(SOIL_MOISTURE2_PIN, INPUT);
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);

    dht.begin();
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    if (!SD.begin(SD_CS)) {
        Serial.println("Error inicializando tarjeta SD");
        while (true);
    }

    if (!rtc.begin()) {
        Serial.println("Error inicializando RTC");
        while (true);
    }

    if (!rtc.isrunning()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    lcd.clear();
    lcd.print("Sistema Listo");
    delay(2000);
}

void loop() {
    // Verificar conexión WiFi y MQTT
    if (!MQTTHandler::isMQTTConnected()) {
        MQTTHandler::reconnectMQTT();
    }
    mqttClient.loop();

    // Publicar datos al topic cada 10 segundos
    static unsigned long lastPublishTime = 0;
    if (millis() - lastPublishTime > 10000) {
        // Obtener lecturas de sensores
        int ldrValue = analogRead(LDR_PIN);
        int soilMoisture1 = analogRead(SOIL_MOISTURE1_PIN);
        int soilMoisture2 = analogRead(SOIL_MOISTURE2_PIN);
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        float distance = obtener_distancia();
        DateTime now = rtc.now();

        // Validar lecturas de sensores
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Error leyendo sensor DHT.");
            return;
        }

        // Crear JSON con estructura deseada
        DynamicJsonDocument doc(512);
        doc["fecha"] = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
        doc["hora"] = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
        doc["temperaturaAmbiente"] = temperature;
        doc["humedadAmbiente"] = humidity;
        doc["humedadSuelo"]["sensor1"] = soilMoisture1;
        doc["humedadSuelo"]["sensor2"] = soilMoisture2;
        doc["iluminacion"] = ldrValue;
        doc["riegoManual"] = false; // Cambiar a `true` si controlas riego manual
        doc["nivelAgua"] = distance;

        // Convertir JSON a cadena
        String jsonString;
        serializeJson(doc, jsonString);

        // Publicar JSON en el topic MQTT
        if (MQTTHandler::isMQTTConnected()) {
            MQTTHandler::publishMessage(topicWorked, jsonString.c_str());
            Serial.println("Datos publicados en MQTT:");
            Serial.println(jsonString);
        } else {
            Serial.println("No se pudo publicar. MQTT no está conectado.");
        }

        // Guardar en la tarjeta SD
        guardarEnSD(jsonString);

        lastPublishTime = millis();
    }

    delay(1000);
}

// Código para sensor de distancia HC-SR04
float obtener_distancia() {
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIGGER, LOW);

    return (pulseIn(ECHO, HIGH) * VELOCIDAD_SONIDO / 2);
}
