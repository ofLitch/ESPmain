#ifndef SENSOR_PRINTDATA_H
#define SENSOR_PRINTDATA_H

// Librerías necesarias
#include <freertos/semphr.h>
#include <Arduino.h>
#include "sensorData.h"

/**
 * @file printData.h
 * @brief Definición de la tarea para imprimir datos de sensores en el puerto serial.
 *
 * @param pvParameters Puntero a los parámetros que incluye el mutex.
 */
void taskPrintData(void *pvParameters) { 
    void **params = (void **)pvParameters;
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)params[0];   
    vTaskDelay(pdMS_TO_TICKS(1000));

    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            Serial.print("LDR: ");
            Serial.println(data.light);
            Serial.print("Soil Wet: ");
            Serial.println(data.soilWet);
            Serial.print("MQ135 PPM: ");
            Serial.println(data.ppm);
            Serial.print("Temperature: ");
            Serial.println(data.temperature);
            Serial.print("Humidity: ");
            Serial.println(data.humidity);

            xSemaphoreGive(mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

#endif
