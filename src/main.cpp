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
#include "telegram_WiFi.h"
#include "esp_main_now.h"

/* =========== Definiciones ============ */
#define WIFI_SSID             "El pocas"                        ///< Nombre de la red WiFi
#define WIFI_PASSWORD         "nosequeponer"                 ///< Contraseña de la red WiFi
#define BOT_TOKEN "7836875521:AAGTRDghaQIzQcjYiftXSKeHS17xkPnClLs" ///< Token del bot de Telegram
#define CHAT_ID "1559018507"                                       ///< ID del chat de Telegram   
SemaphoreHandle_t mutex; ///< Mutex para sincronización entre tareas;     

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
  const char *botToken = BOT_TOKEN;
  const char *chatID = CHAT_ID;
  
  // Crear el mutex
  mutex = xSemaphoreCreateMutex();

  // Inicializar
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Parámetros para tasks
  void *paramTelegram_WiFi[2] = {(void *)botToken, (void *)chatID};

  // Tasks para conexiones
  xTaskCreate(taskTelegram_WiFi, "Telegram WiFi", 8192, paramTelegram_WiFi, 2, NULL);
  xTaskCreate(taskEspNow, "ESP-NOW", 8192, NULL, 2, NULL);
}

void loop() {}