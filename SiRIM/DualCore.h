#ifndef DualCore_h
#define DualCore_h

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "WiFiMQTT.h"
#include "IrrigationControl.h"

// Claves de los núcleos
#define NUCLEO_PRIMARIO 0X01
#define NUCLEO_SECUNDARIO 0X00

// Definición del intervalo de lectura en milisegundos
#define SENSOR_READ_INTERVAL 5000  // 5 segundos

struct SensorsData {
  float temperature;
  float himidity;
  float soilMoisture;
  uint32_t timestamp;
};

struct MQTTMessage {
    char message[256];  // Ajusta el tamaño según tus necesidades
};

WifiMqtt Wireless;
IrrigationControl iCtrl;

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
    static QueueHandle_t mqttQueue;

    static void WiFiMQTTTask( void * pvParameters );
    static void SendDataTask( void *pvParameters );
    static void ReciveDataTask( void * pvParameters );
    static void ReadSensorsTask( void *pvParameters );
};

// Inicializar la cola estática
QueueHandle_t DualCoreESP32::mqttQueue = NULL;

void DualCoreESP32 :: ConfigCores( void ){
  // Inicializar colas
  mqttQueue = xQueueCreate(10, sizeof(MQTTMessage));

  Serial.println("Entro a ConfigCores");

  // Conexión a Wifi y MQTT
  xTaskCreatePinnedToCore(
    this->WiFiMQTTTask,
    "WirelessConnections",
    10000,
    NULL,
    1,
    &WiFiMQTTTask_t,
    NUCLEO_PRIMARIO
  );

  // // Envío de datos al MQTT y guardado en MicroSD
  // xTaskCreatePinnedToCore(
  //   this->SendDataTask,
  //   "SendData",
  //   10000,
  //   NULL,
  //   1,
  //   &SendDataTask_t,
  //   NUCLEO_PRIMARIO
  // );

  // // Recibir datos a través de MQTT
  // xTaskCreatePinnedToCore(
  //   this->ReciveDataTask,
  //   "RecieveData",
  //   10000,
  //   NULL,
  //   1,
  //   &ReciveDataTask_t,
  //   NUCLEO_SECUNDARIO
  // );

  // Leer sensores y generar el JSON
  xTaskCreatePinnedToCore(
    this->ReadSensorsTask,
    "ReadSensors",
    10000,
    NULL,
    1,
    &ReadSensorsTask_t,
    NUCLEO_SECUNDARIO
  );

}

void DualCoreESP32 :: WiFiMQTTTask( void * pvParameters ){
  Serial.println("Entro a WiFiMQTTTask");
  Wireless.startConnections();

  // Buffer para recibir mensajes de la cola
  MQTTMessage receivedMessage;

  while(true){
    if(!Wireless.isWiFiConnected()){
      Serial.println("WiFi Desconectado");
      Wireless.reconnectWiFi();
    } else{
      if(!Wireless.isMQTTConnected()){
        Serial.println("MQTT Desconectado");
        Wireless.reconnectMQTT();
      } else {
        // Verificar si hay mensajes para publicar en la cola
        if(xQueueReceive(mqttQueue, &receivedMessage, 0) == pdTRUE) {
            // Publicar el mensaje si hay conexión MQTT
            Wireless.publishMessage(receivedMessage.message);
        }
      }
    //  Serial.println("Todo bien");
    }
    mqttClient.loop();
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void DualCoreESP32 :: ReadSensorsTask ( void * pvParameters){
  // Inicializar controlador de riego
  iCtrl.init();

  // Variable para almacenar el tiempo de la última lectura
  unsigned long lastReadTime = 0;

  // Estructura para mensaje MQTT
  MQTTMessage mqttMessage;

  while(true){
    // Obtener el tiempo actual
    unsigned long currentTime = millis();

    if(currentTime - lastReadTime >= SENSOR_READ_INTERVAL){
      lastReadTime = currentTime; 

      // Realizar la lectura de sensores
      iCtrl.readAllSensors();

      // Crear y guardar JSON
      String json = iCtrl.createJSON();
      iCtrl.saveDataInSD(json);

      // Evaluación si es hora de regar
      if(iCtrl.isManualIrrigationActivated()){
        if(iCtrl.irrigationStatus){
          // El usuario activó la opción de regar
          Serial.println("Activación manual");
        }
      } else {
        if(iCtrl.isTimerIrrigationActivated()){
          if(iCtrl.evaluateIfIsTimeToWater()){
            // Es hora de regar...
            Serial.println("Activación por alarma");
          }
        } else {
          if(iCtrl.evaluateIrrigationDecision()){
            // Se llegó a valores mínimos de los sensores
            Serial.println("Activación por parámetros de entrada");
          }
        }
      }
      // Copiar el JSON al mensaje MQTT
      strncpy(mqttMessage.message, json.c_str(), sizeof(mqttMessage.message) - 1);
      mqttMessage.message[sizeof(mqttMessage.message) - 1] = '\0';  // Asegurar terminación null
      
      // Enviar a la cola MQTT
      xQueueSend(mqttQueue, &mqttMessage, 0);
    }

    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}
// void DualCoreESP32 :: SendDataTask ( void * pvParameters){
   
   
//    while(true){
//     // Verificar si hay mensajes para publicar en la cola
//     if(xQueueReceive(mqttQueue, &receivedMessage, 0) == pdTRUE) {
//         // Publicar el mensaje si hay conexión MQTT
//         Wireless.publishMessage(receivedMessage.message);
//     }
//     vTaskDelay(100/portTICK_PERIOD_MS);
//   }
// }
// void DualCoreESP32 :: ReciveDataTask ( void * pvParameters){
//   while(true){
//     vTaskDelay(100/portTICK_PERIOD_MS);
//   }
// }
#endif