// TODO: Hace falta
// Manejo de errores, manejo de excepciones
/**
 * @file main.cpp
 * @brief 
 * 
 * @details 
 * 
 * @author
 *          Valentina Muñoz Arcos
 *          Luis Miguel Gómez Muñoz
 *          David Alejandro Ortega Flórez
 * 
 * @version 3.2
 * @date 2025-06-02
 */

/* =========== Librerías ============ */
#include <ThreeWire.h> 
#include <RtcDS1302.h>

#include "thresholds.h"
#include "sensorData.h"
#include "printData.h"
#include "telegram_WiFi.h"

/* =========== Definiciones ============ */
#define WIFI_SSID             "Pomona Altos"                        ///< Nombre de la red WiFi
#define WIFI_PASSWORD         "altosdepomona2525AP"                 ///< Contraseña de la red WiFi
#define BOT_TOKEN "7836875521:AAGTRDghaQIzQcjYiftXSKeHS17xkPnClLs" ///< Token del bot de Telegram
#define CHAT_ID "1559018507"                                       ///< ID del chat de Telegram        

/* =========== Configuración FreeRTOS ============== */
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0; ///< CPU en el que se ejecutará la tarea (0 para un solo núcleo)
#else
  static const BaseType_t app_cpu = 1; ///< CPU en el que se ejecutará la tarea (1 para múltiples núcleos)
#endif


/**
 * @brief 
 * @note 
 * @details
 * @warning 
*/
void setup() {
  // Constantes de Pines y Parámetros
  const char *wifiSSID = WIFI_SSID;
  const char *wifiPassword = WIFI_PASSWORD;
  const char *botToken = BOT_TOKEN;
  const char *chatID = CHAT_ID;

  // Instancias
  SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

  // Inicializar
  Serial.begin(115200);

  // Parámetros para tasks
  void *paramData[1] = {mutex};
  void *paramTelegram_WiFi[5] = {mutex, (void *)wifiSSID, (void *)wifiPassword, (void *)botToken, (void *)chatID};

  // Tasks para conexiones
  xTaskCreate(taskTelegram_WiFi, "Telegram WiFi", 8192, paramTelegram_WiFi, 2, NULL);
  // Tarea para imprimir datos
  xTaskCreate(taskPrintData, "Print Sensor Data", 2048, paramData, 2, NULL);
}

void loop() {}