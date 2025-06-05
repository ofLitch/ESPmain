#ifndef ESP_MAIN_NOW_H
#define ESP_MAIN_NOW_H

// Librerías necesarias
#include <freertos/semphr.h>
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "sensorData.h"
#include "thresholds.h"
#include "esp_wifi.h"

// Definiciones
#define DELAY_ESP_NOW 800  ///< Tiempo de espera entre envíos (en milisegundos)
const uint8_t BROADCAST_ADDRESS[] = {0xEC, 0xE3, 0x34, 0x8A, 0x55, 0xA0};  ///< MAC ESPactuadores EC:E3:34:8A:55:A0
extern SemaphoreHandle_t mutex;

/**
 * @file 
 * @brief 
 *
 * @param pvParameters 
 */
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
        memcpy(&data, incomingData, sizeof(data));
        xSemaphoreGive(mutex);
    }else {
        Serial.println("⚠️ No se pudo obtener el mutex en OnDataRecv");
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "📩 Entrega Exitosa a ESPactuadores" : "⚠️ Error de Entrega a ESPactuadores");
}
esp_err_t esp_wifi_config_espnow_channel(uint8_t channel) {
    esp_err_t err = esp_wifi_set_promiscuous(true);
    if (err != ESP_OK) {
        return err;
    }
    err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    if (err != ESP_OK) {
        return err;
    }
    err = esp_wifi_set_promiscuous(false);
    return err;
}

void taskEspNow(void *pvParameters){
    // Inicializar ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    int wifi_channel = WiFi.channel();
    int espnow_channel = (wifi_channel + 6) % 14;
    Serial.print("Using WiFi channel: ");
    Serial.println(wifi_channel);
    Serial.print("Using ESP-NOW channel: ");
    Serial.println(espnow_channel);
    esp_wifi_config_espnow_channel(espnow_channel);
    // Registrar el callback para recibir datos
    esp_err_t cb_recv_result = esp_now_register_recv_cb(OnDataRecv);
    if (cb_recv_result != ESP_OK) {
        Serial.print("Error registering recv callback: ");
        Serial.println(esp_err_to_name(cb_recv_result));
        vTaskDelete(NULL); // Eliminar la tarea si falla el registro del callback
        return;
    };

    // Registrar callback para el estado del envío
    esp_err_t cb_send_result = esp_now_register_send_cb(OnDataSent);
    if (cb_send_result != ESP_OK) {
        Serial.print("Error registering send callback: ");
        Serial.println(esp_err_to_name(cb_send_result));
        vTaskDelete(NULL); // Eliminar la tarea si falla el registro del callback
        return;
    }

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, BROADCAST_ADDRESS, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    // Agregar par
    esp_err_t add_peer_result = esp_now_add_peer(&peerInfo);
    if (add_peer_result != ESP_OK){
        Serial.print("Failed to add peer: ");
        Serial.println(esp_err_to_name(add_peer_result));
        vTaskDelete(NULL);
        return;
    }

    // Variables para enviar datos, inicializar en "apagado"
    unsigned short int sendData[3] = {0,0,0};

    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            getThresholdsForSend(data, sendData);
            xSemaphoreGive(mutex);
        }else {
            Serial.println("⚠️ No se pudo obtener el mutex en taskEspNow");
        }

        // Enviar datos a través de ESP-NOW
        esp_err_t result = esp_now_send(BROADCAST_ADDRESS, (uint8_t *) &sendData, sizeof(sendData));
        if (result == ESP_OK) {
            Serial.println("📤 Data enviada correctamente a ESPactuadores");
        } else {
            Serial.print("⚠️ Error al enviar datos: ");
            Serial.println(esp_err_to_name(result));
        }

        // Esperar un tiempo antes de enviar nuevamente
        vTaskDelay(pdMS_TO_TICKS(DELAY_ESP_NOW));
    }
}

#endif