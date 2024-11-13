#ifndef DualCore_h
#define DualCore_h

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "WiFiMQTT.h"

// Claves de los núcleos
#define NUCLEO_PRIMARIO 0X01
#define NUCLEO_SECUNDARIO 0X00

struct SensorsData {
  float temperature;
  float himidity;
  float soilMoisture;
  uint32_t timestamp;
};

WifiMqtt Wireless;

class DualCoreESP32{
  public:
    void ConfigCores( void ); // Creación de tareas xTaskCreatePinnedToCore
  
  private:

    // Tareas primer núcleo
    TaskHandle_t SendDataTask_t;
    TaskHandle_t ReciveDataTask_t;
    TaskHandle_t WiFiMQTTTask_t;

    // Tareas segundo núcleo
    TaskHandle_t ReadSensorsTask_t;

    // Queues
    QueueHandle_t transmitDataQueue;

    static void WiFiMQTTTask( void * pvParameters );
    static void SendDataTask( void *pvParameters );
    static void ReciveDataTask( void * pvParameters );
    static void ReadSensorsTask( void *pvParameters );
};

void DualCoreESP32 :: ConfigCores( void ){
  // Inicializar colas
  transmitDataQueue = xQueueCreate(10, sizeof(SensorsData));

  Serial.println("Entro a ConfigCores");

  // SendDataTask_t
  xTaskCreatePinnedToCore(
    this->WiFiMQTTTask,
    "WirelessConnections",
    10000,
    NULL,
    1,
    &WiFiMQTTTask_t,
    NUCLEO_PRIMARIO
  );

  // SendDataTask
  xTaskCreatePinnedToCore(
    this->SendDataTask,
    "SendData",
    10000,
    NULL,
    1,
    &SendDataTask_t,
    NUCLEO_SECUNDARIO
  );

  // LocalTask
  xTaskCreatePinnedToCore(
    this->ReadSensorsTask,
    "ReadSensors",
    10000,
    NULL,
    1,
    &ReadSensorsTask_t,
    NUCLEO_SECUNDARIO
  );

  // LocalTask
  xTaskCreatePinnedToCore(
    this->ReciveDataTask,
    "RecieveData",
    10000,
    NULL,
    1,
    &ReciveDataTask_t,
    NUCLEO_SECUNDARIO
  );
}

void DualCoreESP32 :: WiFiMQTTTask( void * pvParameters ){
  Serial.println("Entro a WiFiMQTTTask");
  Wireless.startConnections();

  while(true){
    if(!Wireless.isWiFiConnected()){
      Serial.println("WiFi Desconectado");
      Wireless.reconnectWiFi();
    } else{
      if(!Wireless.isMQTTConnected()){
        Serial.println("MQTT Desconectado");
        Wireless.reconnectMQTT();
      }
    //  Serial.println("Todo bien");
    }

    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void DualCoreESP32 :: SendDataTask ( void * pvParameters){
  while(true){
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}
void DualCoreESP32 :: ReciveDataTask ( void * pvParameters){
  while(true){
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}
void DualCoreESP32 :: ReadSensorsTask ( void * pvParameters){
  while(true){
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}
#endif