#include <WiFi.h>
#include "time.h"
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Adafruit_Sensor.h>

#define DHTPIN 23
#define DHTTYPE DHT11
#define P1 200
#define P2 201
#define ESPERA1 202
#define ESPERA2 203
#define SUMAGMT 204
#define RESTAGMT 205
#define BOTON1 35
#define BOTON2 34
const char *ssid = "ORT-IoT";
const char *password = "NuevaIOT$25";
const char *ntpServer = "pool.ntp.org";
const int daylightOffset_sec = 0;

int boton1;
int boton2;
int gmt = -3 * 3600; //empieza en arg  
int estado;

WiFiServer server(80);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset = */ U8X8_PIN_NONE);

void Maquina(int EstadoBoton1, int EstadoBoton2);

void setup() {
  Serial.begin(9600);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  u8g2.begin();
  WiFi.begin(ssid, password);
  delay(100);
  configTime(gmt, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);  // Espera hasta que se sincronice la hora
  }

  estado = P1;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  boton1 = digitalRead(BOTON1);
  boton2 = digitalRead(BOTON2);
  Maquina(boton1, boton2);
  delay(100);
}

void Maquina(int EstadoBoton1, int EstadoBoton2) {
  char stringgmt[4];
  char stringhora[6];
  struct tm timeinfo;

  switch (estado) {
    case P1:
      getLocalTime(&timeinfo);
      strftime(stringhora, sizeof(stringhora), "%H:%M", &timeinfo);
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(15, 15, "Hora:");
      u8g2.drawStr(60, 15, stringhora);
      u8g2.sendBuffer();

      if (EstadoBoton1 == LOW && EstadoBoton2 == LOW) {
        estado = ESPERA1;
      }
      break;

    case ESPERA1:
      if (EstadoBoton1 == HIGH && EstadoBoton2 == HIGH) {
        estado = P2;
      }
      break;

    case ESPERA2:
      if (EstadoBoton1 == HIGH && EstadoBoton2 == HIGH) {
        estado = P1;
      }
      break;

    case P2:
      sprintf(stringgmt, "GMT%+d", gmt / 3600);
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(15, 15, "GMT:");
      u8g2.drawStr(60, 15, stringgmt);
      u8g2.sendBuffer();

      if (EstadoBoton1 == LOW && EstadoBoton2 == LOW) {
        estado = ESPERA2;
      }
      if (EstadoBoton1 == LOW && EstadoBoton2 == HIGH) {
        estado = SUMAGMT;
      }
      if (EstadoBoton2 == LOW && EstadoBoton1 == HIGH) {
        estado = RESTAGMT;
      }
      break;

    case SUMAGMT:
      if (EstadoBoton1 == HIGH) {
        if (gmt < 43200) {  // +12 horas máx
          gmt += 3600;
          configTime(gmt, daylightOffset_sec, ntpServer);
        }
        estado = P2;
      }
      if (EstadoBoton2 == LOW && EstadoBoton1 == LOW) {
        estado = ESPERA2;
      }
      break;

    case RESTAGMT:
      if (EstadoBoton2 == HIGH) {
        if (gmt > -43200) {  // -12 horas mín
          gmt -= 3600;
          configTime(gmt, daylightOffset_sec, ntpServer);
        }
        estado = P2;
      }
      if (EstadoBoton2 == LOW && EstadoBoton1 == LOW) {
        estado = ESPERA2;
      }
      break;
  }
}
