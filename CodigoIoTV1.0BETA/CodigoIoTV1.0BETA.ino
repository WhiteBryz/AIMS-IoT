#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include "MQTT.h"

// Configuración de MQTT
const char* topicTX = "ucol/iot/sensores";

// Pines y configuración de dispositivos
#define TRIGGER 26
#define ECHO 25
#define VELOCIDAD_SONIDO 0.034
#define LDR_PIN 35
#define RELAY1_PIN 27
#define DHT_PIN 16
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define BUT_BOMBA 0
#define BUT_CARRUSEL 2

bool bombaManual = false;
String modo = "manual"; // Por defecto, inicia en modo manual
int pantallaActual = 0;
unsigned long lastButtonPress = 0;
unsigned long lastPublishTime = 0; // Variable para controlar el tiempo de publicación

// Variables para modo automático
int nivelLuzUmbral = 50;  // Valor por defecto si no se recibe JSON
int humedadSueloUmbral = 50; // Valor por defecto si no se recibe JSON

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT11);
RTC_DS1307 rtc;

int ldrValueRaw;
float temperature;
float humidity;
float distance;
float ldrValue;
float nivelAgua;

void setup() {
    Serial.begin(115200);

    // Iniciar conexiones WiFi y MQTT
    MQTTHandler::startConnections();

    // Establecer el callback
    mqttClient.setCallback(callback);

    // Inicialización de componentes
    lcd.init();
    lcd.backlight();
    lcd.print("Iniciando...");

    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(BUT_BOMBA, INPUT_PULLUP);
    pinMode(BUT_CARRUSEL, INPUT_PULLUP);

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
    delay(2000); // Puede ser optimizado o removido en versiones futuras
}

void loop() {
    // Mantener conexión MQTT
    if (!MQTTHandler::isMQTTConnected()) {
        MQTTHandler::reconnectMQTT();
    }

    mqttClient.loop();

    // Actualizar lecturas de sensores
    ldrValueRaw = analogRead(LDR_PIN);
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    distance = obtener_distancia();
    ldrValue = map(ldrValueRaw, 0, 4096, 0, 100);
    nivelAgua = map(distance, 17, 6, 0, 100);

    // Enviar datos
    enviarDatos();

    // Control del riego
    controlarRiego(); // Ahora incluye la nueva lógica

    // Control del carrusel de pantallas
    manejarCarrusel();

    delay(100); // Ajusta si es necesario
}

// Función de callback MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.println("Error al parsear JSON");
        return;
    }

    if (doc["modo"] == "manual") {
        modo = "manual";
        bombaManual = false; // Reinicia estado de la bomba en manual
    } else if (doc["modo"] == "auto") {
        modo = "auto";
        if (doc.containsKey("nivelLuz")) {
            nivelLuzUmbral = doc["nivelLuz"];
        }
        if (doc.containsKey("humedadSuelo")) {
            humedadSueloUmbral = doc["humedadSuelo"];
        }
        Serial.println("Umbrales actualizados:");
        Serial.println("Nivel de luz: " + String(nivelLuzUmbral));
        Serial.println("Humedad del suelo: " + String(humedadSueloUmbral));
    }

    Serial.println("Modo actualizado: " + modo);
}

// Control del riego con condición base
void controlarRiego() {
    // Condición base: si el nivel de agua es menor al 20%, no activa la bomba
    if (nivelAgua < 20) {
        digitalWrite(RELAY1_PIN, LOW); // Asegura que la bomba esté apagada
        return; // Sale de la función
    }

    // Condicionales para los modos manual y automático
    if (modo == "manual") {
        bombaManualboton(); // Controla el riego con el botón
        digitalWrite(RELAY1_PIN, bombaManual ? HIGH : LOW);
    } else if (modo == "auto") {
        if (ldrValue < nivelLuzUmbral || humidity < humedadSueloUmbral) {
            digitalWrite(RELAY1_PIN, HIGH);
        } else {
            digitalWrite(RELAY1_PIN, LOW);
        }
    }
}

void manejarCarrusel() {
    if (!digitalRead(BUT_CARRUSEL)) {
        if (millis() - lastButtonPress > 300) { // Evita rebotes
            pantallaActual = (pantallaActual + 1) % 5; // Cambia entre 5 pantallas
            lastButtonPress = millis();
        }
    }

    lcd.clear();

    switch (pantallaActual) {
        case 0:
            lcd.setCursor(0, 0);
            lcd.print("Temp: ");
            lcd.print(temperature);
            lcd.print(" C");
            lcd.setCursor(0, 1);
            lcd.print("Hum. Amb: ");
            lcd.print(humidity);
            lcd.print("%");
            break;
        case 1:
            lcd.setCursor(0, 0);
            lcd.print("Luz: ");
            lcd.print(ldrValue);
            lcd.print("%");
            lcd.setCursor(0, 1);
            lcd.print("N. agua: ");
            lcd.print(nivelAgua);
            lcd.print("%");
            break;
        case 2:
            lcd.setCursor(0, 0);
            lcd.print("Modo: ");
            lcd.print(modo);
            lcd.setCursor(0, 1);
            lcd.print("Bomba: ");
            lcd.print(digitalRead(RELAY1_PIN) ? "ON" : "OFF");
            break;
        case 3:
            lcd.setCursor(0, 0);
            lcd.print("Luz Umbral: ");
            lcd.print(nivelLuzUmbral);
            lcd.setCursor(0, 1);
            lcd.print("Hum. Umbral: ");
            lcd.print(humedadSueloUmbral);
            break;
        case 4: {
            DateTime now = rtc.now();
            lcd.setCursor(0, 0);
            lcd.print("Hora: ");
            lcd.print(now.hour());
            lcd.print(":");
            if (now.minute() < 10) lcd.print("0"); // Formato HH:MM
            lcd.print(now.minute());
            lcd.setCursor(0, 1);
            lcd.print("Dia: ");
            lcd.print(now.day());
            lcd.print("/");
            lcd.print(now.month());
            lcd.print("/");
            lcd.print(now.year());
            break;
        }
    }
}


// Función para manejar el botón de bomba en modo manual
void bombaManualboton() {
    if (!digitalRead(BUT_BOMBA)) {
        if (millis() - lastButtonPress > 300) { // Evita rebotes
            bombaManual = !bombaManual;
            Serial.println(bombaManual ? "Bomba Activada" : "Bomba Desactivada");
            lastButtonPress = millis();
        }
    }
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

// Función para guardar datos en la SD
void guardarEnSD(const String& jsonString) {
    File file = SD.open("/datalog.json", FILE_APPEND);
    if (file) {
        file.println(jsonString);
        file.close();
        Serial.println("Datos guardados en datalog.json");
    } else {
        Serial.println("Error al abrir datalog.json para escritura.");
    }
}

// Función para enviar datos vía MQTT
void enviarDatos() {
    if (millis() - lastPublishTime > 5000) { // Publicar cada 5 segundos
        DateTime now = rtc.now();

        StaticJsonDocument<512> doc;
        doc["fecha"] = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
        doc["hora"] = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
        doc["timestamp"] = now.unixtime();
        doc["temperaturaAmbiente"] = temperature;
        doc["humedadAmbiente"] = humidity;
        doc["humedadSuelo"] = random(30, 70);
        doc["iluminacion"] = ldrValue;
        doc["nivelAgua"] = nivelAgua;

        String jsonString;
        serializeJson(doc, jsonString);

        if (MQTTHandler::isMQTTConnected()) {
            MQTTHandler::publishMessage(topicTX, jsonString.c_str());
            Serial.println("Datos publicados en MQTT");
        } else {
            Serial.println("No se pudo publicar. MQTT no está conectado.");
        }

        guardarEnSD(jsonString);

        lastPublishTime = millis();
    }
}
