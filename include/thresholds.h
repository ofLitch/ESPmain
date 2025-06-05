#ifndef THRESHOLDS_H
#define THRESHOLDS_H

#include "sensorData.h"

String checkThresholds(SensorData buffer);
void getThresholdsForSend(SensorData buffer, unsigned short int sendData[3]);

struct Thresholds {
    float light = 1000.0;
    float temperature = 24.0;
    float humidity = 80.0;
    float ppm = 100.0;
    float soilWet = 500.0;
} thresholds;

String checkThresholds(SensorData buffer) {
    String alert = "";
    if (buffer.light < thresholds.light)                  alert += "🔅 Luz baja: " + String(buffer.light) + " lx\n";
    if (buffer.temperature > thresholds.temperature)      alert += "🌡 Temperatura alta: " + String(buffer.temperature) + " °C\n";
    if (buffer.humidity > thresholds.humidity)            alert += "💧 Humedad alta: " + String(buffer.humidity) + " %\n";
    if (buffer.ppm > thresholds.ppm)                      alert += "🌫 PPM alto: " + String(buffer.ppm) + "\n";
    if (buffer.soilWet < thresholds.soilWet)              alert += "🌱 Suelo seco: " + String(buffer.soilWet) + "\n";  
    return alert;
}

void getThresholdsForSend(SensorData buffer, unsigned short int sendData[3]){
    // [0] ventilador, [1] bomba, [2] leds
    // Inicializar en 0
    sendData[0] = 0;
    sendData[1] = 0;
    sendData[2] = 0;

    if (buffer.temperature > thresholds.temperature) sendData[0] = 1; // ventilador
    if (buffer.soilWet < thresholds.soilWet) sendData[1] = 1;         // bomba
    if (buffer.light < thresholds.light) sendData[2] = 1;             // leds
}

#endif
