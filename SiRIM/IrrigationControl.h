#ifndef IrrigationControl_h
#define IrrigationControl_h

// Librerías
#include "RTC.h"

// Pines

// Instancias de clases de librerias
DS1307_RTC RTC;

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

    // Parámetros de configuración para realizar el riego
    String irrigationTime = "";
    int minLightThreshold = 4;         // Verificar valor de luz
    int minSoilMoistureThreshold = 500;  // Verificar valor de luz
    bool manualIrrigationMode = false;
    bool irrigationStatus = false;
  public:
    static void init();
    int readAirHumidity( void );
    int readAirTemperature ( void );
    int readSoilMoisture ( void );
    int readWaterLevel ( void );
    int readLightInensity ( void );
    void ChangeConfigurationParameters( ChangeConfiguration newConfig );
};

void IrrigationControl :: init( void ){
  RTC.rtcInit();
  //pinMode();

}

void IrrigationControl :: ChangeConfigurationParameters ( ChangeConfiguration newConfig ){
  irrigationTime = newConfig.setIrrigationTime;
  minLightThreshold = newConfig.setLightThreshold;
  minSoilMoistureThreshold = newConfig.setSoilMoistureThreshold;
  manualIrrigationMode = newConfig.setManualIrrigationMode;
  irrigationStatus = newConfig.setIrrigationStatus;
}

int IrrigationControl :: readAirHumidity ( void ){
  // Código para obtener la humedad del ambiente
}
int IrrigationControl :: readAirTemperature ( void ){
  // Código para obtener la temperatura del ambiente
}
int IrrigationControl :: readSoilMoisture ( void ){
  // Código para obtener la humedad del suelo
}
int IrrigationControl :: readWaterLevel ( void ){
  // Código para obtener el nivel del agua
}
int IrrigationControl :: readLightInensity ( void ){
  // Código para obtener la intensidad de la luz
}

#endif