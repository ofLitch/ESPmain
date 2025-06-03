#ifndef SENSOR_TELEGRAM_WIFI_H
#define SENSOR_TELEGRAM_WIFI_H

// Librerías necesarias
#include <freertos/semphr.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "sensorData.h"
#include "thresholds.h"

// Certificado raíz de Telegram (debes definirlo si no lo tienes)
extern const char TELEGRAM_CERTIFICATE_ROOT[];

// Definiciones
#define DELAY_TIME 2200 ///< Tiempo de espera entre envíos (en milisegundos)

// Funciones
String checkThresholds();
void handleTelegramCommands(UniversalTelegramBot &bot, const String &chatId, SensorData buffer);

/**
 * @brief Tarea que se encarga de conectarse a WiFi y enviar datos al bot de Telegram.
 * 
 * @param pvParameters Parámetros que incluyen: [0] mutex, [1] SSID, [2] password, [3] botToken, [4] chatID
 */
void taskTelegram_WiFi(void *pvParameters) {
    // Extraer los parámetros
    void **params = (void **)pvParameters;
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)params[0];   
    const char *wifiSSID = (const char *)params[1];
    const char *wifiPassword = (const char *)params[2];
    const char *botToken = (const char *)params[3];
    const char *chatID = (const char *)params[4];

    // Conexión a WiFi
    WiFi.begin(wifiSSID, wifiPassword);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    Serial.println("\nConectado a WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("\n✅ WiFi conectado");

    // Inicializar cliente seguro y bot de Telegram
    WiFiClientSecure client;
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    UniversalTelegramBot bot(botToken, client);

    // Notificar inicio
    bot.sendMessage(chatID, "🤖 *Sistema VerdeVital conectado a WiFi y listo para recibir datos*", "Markdown");
    bot.sendMessage(chatID, "🌱 Instrucciones de funcionamiento usa /verDatos para ver los datos actuales, /verUmbrales para ver los umbrales actuales, /modificarUmbral <parametro> <valor> para modificar un umbral :3", "Markdown");
    SensorData buffer;
    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            // Verificar umbrales y notificar si se superan
            String alert = checkThresholds();
            if (alert.length() > 0) bot.sendMessage(chatID, alert, "Markdown");
            
            // Cambiar el estado del bot si se recibe un comando
            buffer = data;
            handleTelegramCommands(bot, chatID, buffer);
            xSemaphoreGive(mutex); // Liberar el mutex
        } else {
            Serial.println("⚠️ No se pudo obtener el mutex para enviar datos");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_TIME));
    }
}

String checkThresholds() {
    String alert = "";

    if (data.light < thresholds.light)                  alert += "🔅 Luz baja: " + String(data.light) + " lx\n";
    if (data.temperature > thresholds.temperature)      alert += "🌡 Temperatura alta: " + String(data.temperature) + " °C\n";
    if (data.humidity > thresholds.humidity)            alert += "💧 Humedad alta: " + String(data.humidity) + " %\n";
    if (data.ppm > thresholds.ppm)                      alert += "🌫 PPM alto: " + String(data.ppm) + "\n";
    if (data.soilWet < thresholds.soilWet)              alert += "🌱 Suelo seco: " + String(data.soilWet) + "\n";  
    
    return alert;
}

void handleTelegramCommands(UniversalTelegramBot &bot, const String &chatId, SensorData buffer) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            String msg = bot.messages[i].text;
            String sender = bot.messages[i].chat_id;

            if (sender != chatId) continue;

            if (msg.startsWith("/modificarUmbral")) {
                int space1 = msg.indexOf(' ', 17);
                if (space1 > 0) {
                    String param = msg.substring(17, space1);
                    String valueStr = msg.substring(space1 + 1);
                    float value = valueStr.toFloat();

                    if (isnan(value) && value < 0) {
                        bot.sendMessage(chatId, "❌ Valor no válido.", "");
                        continue;
                    }

                    if (param == "luz") thresholds.light = value;
                    else if (param == "temp") thresholds.temperature = value;
                    else if (param == "hum") thresholds.humidity = value;
                    else if (param == "ppm") thresholds.ppm = value;
                    else if (param == "suelo") thresholds.soilWet = value;
                    else {
                        bot.sendMessage(chatId, "❌ Parámetro no válido.", "");
                        continue;
                    }

                    bot.sendMessage(chatId, "✅ Nuevo umbral de " + param + ": " + String(value), "");
                } else {
                    bot.sendMessage(chatId, "❌ Formato de comando incorrecto.", "");
                }
            }

            // Ver umbrales actuales
            if (msg == "/verDatos") {
                String report = "📊 *Datos actuales:*\n";
                report += "🔅 Luz: " + String(buffer.light) + " lx\n"
                        + "🌡 Temp: " + String(buffer.temperature) + " °C\n"
                        + "💧 Hum: " + String(buffer.humidity) + " %\n"
                        + "🌫 PPM: " + String(buffer.ppm) + "\n"
                        + "🌱 Suelo: " + String(buffer.soilWet);
                bot.sendMessage(chatId, report, "Markdown");
            }

            // Ver umbrales actuales
            if (msg == "/verUmbrales") {
                String report = "📊 *Umbrales actuales:*\n";
                report += "🔅 Luz < " + String(thresholds.light) + "\n";
                report += "🌡 Temp > " + String(thresholds.temperature) + "\n";
                report += "💧 Hum > " + String(thresholds.humidity) + "\n";
                report += "🌫 PPM > " + String(thresholds.ppm) + "\n";
                report += "🌱 Suelo < " + String(thresholds.soilWet);
                bot.sendMessage(chatId, report, "Markdown");
            }
        }

        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

#endif // SENSOR_TELEGRAM_WIFI_H
