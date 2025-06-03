#ifndef ESP_MAIN_NOW_H
#define ESP_MAIN_NOW_H

// Librerías necesarias
#include <freertos/semphr.h>
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "sensorData.h"

/**
 * @file 
 * @brief 
 *
 * @param pvParameters 
 */
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.print("Bytes received: ");
    Serial.println(len);
    // Aquí puedes copiar los datos a tu estructura global 'data'
    data.ppm = incomingReadings.ppm;
    data.temperature = incomingReadings.temperature;
    data.humidity = incomingReadings.humidity;
    data.soilWet = incomingReadings.soilWet;
    data.light = incomingReadings.light;
    //data.dateTime = incomingReadings.dateTime; //Cuidado con esto, toca definirlo bien
}

void taskEspNow(void *pvParameters){
    // Inicializar ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(5000)); // Esperar cinco segundos para estabilizar
    // Registrar el callback para recibir datos
    esp_now_register_recv_cb(OnDataRecv);
    vTaskDelete(NULL);
}

#endif
