#ifndef IrrigationControl_h
#define IrrigationControl_h

// Librerías
//#include "RTC.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h>
#include <ArduinoJson.h>

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

// Instancias de las clases
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT11);
RTC_DS1307 rtc;

struct ChangeConfiguration{
  String setIrrigationTime;
  int setLightThreshold;
  int setSoilMoistureThreshold;
  bool setManualIrrigationMode;
  bool setIrrigationStatus;
};

class IrrigationControl {
  public:
    // Sensores
    String currentDate;
    int s_airHumidity;
    int s_airTemperature;
    int s_soilMoisture;
    int s_waterLevel;
    int s_lightIntensity;

    // Parámetros de configuración inicial para gestionar el riego
    String irrigationTime = "";
    bool irrigationConfigWithTimer = false;

    int minLightThreshold = 4;
    int minSoilMoistureThreshold = 500;
    
    bool manualIrrigationMode = false;
    bool irrigationStatus = false;

    /*--- Funciones para la Lógica de riego ---*/

    // Inicializar todos los pines y componentes
    static void init();

    // Leer datos de sensores
    static int readSoilMoisture ( int pinSensor );
    static int readLightIntensity ( int pinSensor );
    static float readAirHumidity( void );
    static float readAirTemperature ( void );
    static float readWaterLevel ( void );

    // Funciones adicionales
    static void saveDataInSD(const String& data);
    void ChangeConfigurationParameters( ChangeConfiguration newConfig );
};

void IrrigationControl :: init( void ){
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

void IrrigationControl :: ChangeConfigurationParameters ( ChangeConfiguration newConfig ){
  irrigationTime = newConfig.setIrrigationTime;
  minLightThreshold = newConfig.setLightThreshold;
  minSoilMoistureThreshold = newConfig.setSoilMoistureThreshold;
  manualIrrigationMode = newConfig.setManualIrrigationMode;
  irrigationStatus = newConfig.setIrrigationStatus;
}

float IrrigationControl :: readAirHumidity ( void ){
  // Código para obtener la humedad del ambiente
  return dht.readHumidity();
}
float IrrigationControl :: readAirTemperature ( void ){
  // Código para obtener la temperatura del ambiente
  return dht.readTemperature();
}
int IrrigationControl :: readSoilMoisture ( int pinSensor ){
  // Código para obtener la humedad del suelo
  return analogRead(pinSensor);
}
float IrrigationControl :: readWaterLevel ( void ){
  // Código para obtener el nivel del agua
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER, LOW);

  return (pulseIn(ECHO, HIGH) * VELOCIDAD_SONIDO / 2);
}
int IrrigationControl :: readLightIntensity ( int pinSensor ){
  return analogRead(pinSensor);
}

void saveDataInSD(const String& data){
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

#endif