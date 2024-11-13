#ifndef RTC_h
#define RTC_h

#include <Wire.h> /* Librería para el uso del protocolo I2C. */
#include <ArduinoJson.h>
#include <RTClib.h>

/*RTClib.h, requiere de la instalación de dos librerías:
RTClib: Permite el uso de varios relojes del fabricante Adafruit.
Adafruit BusIO: Es necesaria cargarla para configurar ciertos elementos de los componentes electrónicos de Adafruit.*/

RTC_DS1307 rtc;

class DS1307_RTC
{
  public:
      uint8_t hora = 0, minuto = 0, segundo = 0, dia = 0, mes = 0;
      uint16_t ano = 0;
      String fecha = "", tiempo = "";

  public:
      void rtcInit(void);
      void getTime(void);
      String formatDate(void);
      String formatTime(void);
      void showTime(void);
      void showTimeJson(void);
};

/* Función que configura lo necesario para que el reloj se inicialice */
void DS1307_RTC::rtcInit(void)
{

    while (!rtc.begin())
    { /* Mientras que el reloj no se ha iniciado. */
        Serial.println(F("Hay un error de reconocimiento del RTC, revisa conexiones y alimentación"));
        Serial.flush(); /* Espera a que se transmita la información serial correctamente. */
        delay(3000);
        // abort ( );
    }
    if (rtc.isrunning())
    { /* Si el reloj no está ejecutándose*/

        /* Esta instrucción solo debe usarse una sola vez, para que el reloj actualice la hora, una vez hecho esto, se puede comentar y volver a subir el código. */
        Serial.println(F("El reloj se está configurando con la fecha y hora de la computadora"));
        // __DATE__ y __TIME__ Son macros que retornan la fecha y hora de la computadora.
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); /* Función que ajusta automáticamente la fecha y hora. */
    }
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));  // año, mes, día, hora, minuto, segundo.                  /* Función que ajusta manualmente la fecha y hora. */
}

/* Función que configura lo necesario para que el reloj se inicialice */
void DS1307_RTC::getTime(void)
{
    DateTime now = rtc.now();
    segundo = now.second();
    minuto = now.minute();
    hora = now.hour();
    dia = now.day();
    mes = now.month();
    ano = now.year();
}

/* Función que le da un formato legible a la fecha. */
String DS1307_RTC::formatDate(void)
{

    fecha = "";
    if (dia < 10)
        fecha += '0';
    fecha += dia;
    fecha += '/';
    if (mes < 10)
        fecha += '0';
    fecha += mes;
    fecha += '/';
    fecha += ano;
    return fecha;
}

/* Función que le da un formato legible al tiempo. */
String DS1307_RTC::formatTime(void)
{

    tiempo = "";
    if (hora < 10)
        tiempo += '0';
    tiempo += hora;
    tiempo += ':';

    if (minuto < 10)
        tiempo += '0';
    tiempo += minuto;
    tiempo += ':';

    if (segundo < 10)
        tiempo += '0';
    tiempo += segundo;
    return tiempo;
}

/* Función que muestra en la consola serial la fecha y la hora. */
void DS1307_RTC ::showTime(void)
{

    formatTime(); /* Dar formato al tiempo */
    Serial.print(F("Hora: "));
    Serial.println(tiempo);
    formatDate();
    Serial.print(F("Fecha: "));
    delay(200);
    Serial.println(fecha); /* Dar formato a la fecha */
}

void DS1307_RTC ::showTimeJson(void)
{
}

#endif