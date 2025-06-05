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
#define TELEGRAM_DELAY_TIME 3000 ///< Tiempo de espera entre envíos (en milisegundos)
#define COMMANDS_DELAY_TIME 500 ///< Tiempo de espera entre envíos (en milisegundos)
extern SemaphoreHandle_t mutex;

// Funciones
void handleTelegramCommands(UniversalTelegramBot &bot, const String &chatID, SensorData buffer);

/**
 * @brief Tarea que se encarga de conectarse a WiFi y enviar datos al bot de Telegram.
 * 
 * @param pvParameters Parámetros que incluyen: [0] mutex, [1] SSID, [2] password, [3] botToken, [4] chatID
 */
void taskTelegram_WiFi(void *pvParameters) {
    // Extraer los parámetros
    void **params = (void **)pvParameters;  
    const char *botToken = (const char *)params[0];
    const char *chatID = (const char *)params[1];

    // Conexión a WiFi
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    Serial.println("\n🛜 WiFi conectado");

    // Inicializar cliente seguro y bot de Telegram
    WiFiClientSecure client;
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    UniversalTelegramBot bot(botToken, client);

    // Notificar inicio
    bot.sendMessage(chatID, "🤖 *Sistema VerdeVital conectado a WiFi y listo para recibir datos*", "Markdown");
    bot.sendMessage(chatID, "🌱 Instrucciones de funcionamiento usa /verDatos para ver los datos actuales, /verUmbrales para ver los umbrales actuales, /modificarUmbral <parametro> <valor> para modificar un umbral :3", "Markdown");
    bot.setMyCommands(
        "[{\"command\":\"/verDatos\",\"description\":\"Ver datos actuales\"},"
        "{\"command\":\"/verUmbrales\",\"description\":\"Ver umbrales actuales\"},"
        "{\"command\":\"/modificarUmbral <parametro> <valor>\",\"description\":\"Modificar un umbral\"}]"
    );

    SensorData buffer;
    buffer.ppm = 0;
    buffer.temperature = 0;
    buffer.humidity = 0;
    buffer.soilWet = 0;
    buffer.light = 0;
    buffer.dateTime = RtcDateTime(0);

    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            buffer = data;
            xSemaphoreGive(mutex);
        } else {
            Serial.println("⚠️ No se pudo obtener el mutex para enviar datos");
        }

        // Atender comandos de Telegram
        handleTelegramCommands(bot, chatID, buffer);
        // Verificar umbrales y notificar si se superan
        String alert = checkThresholds(buffer);
        if (alert.length() > 0) bot.sendMessage(chatID, alert, "Markdown");
            
        vTaskDelay(pdMS_TO_TICKS(TELEGRAM_DELAY_TIME));
    }
}

void handleTelegramCommands(UniversalTelegramBot &bot, const String &chatID, SensorData buffer) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            String msg = bot.messages[i].text;
            String sender = bot.messages[i].chat_id;

            if (sender != chatID) continue;

            if (msg.startsWith("/modificarUmbral")) {
                int space1 = msg.indexOf(' ', 17);
                if (space1 > 0) {
                    String param = msg.substring(17, space1);
                    String valueStr = msg.substring(space1 + 1);
                    float value = valueStr.toFloat();

                    if (isnan(value) && value < 0) {
                        bot.sendMessage(chatID, "❌ Valor no válido.", "");
                        continue;
                    }

                    if (param == "luz") thresholds.light = value;
                    else if (param == "temp") thresholds.temperature = value;
                    else if (param == "hum") thresholds.humidity = value;
                    else if (param == "ppm") thresholds.ppm = value;
                    else if (param == "suelo") thresholds.soilWet = value;
                    else {
                        bot.sendMessage(chatID, "❌ Parámetro no válido.", "");
                        continue;
                    }

                    bot.sendMessage(chatID, "✅ Nuevo umbral de " + param + ": " + String(value), "");
                } else {
                    bot.sendMessage(chatID, "❌ Formato de comando incorrecto.", "");
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
                bot.sendMessage(chatID, report, "Markdown");
            }

            // Ver umbrales actuales
            if (msg == "/verUmbrales") {
                String report = "📊 *Umbrales actuales:*\n";
                report += "🔅 Luz < " + String(thresholds.light) + "\n";
                report += "🌡 Temp > " + String(thresholds.temperature) + "\n";
                report += "💧 Hum > " + String(thresholds.humidity) + "\n";
                report += "🌫 PPM > " + String(thresholds.ppm) + "\n";
                report += "🌱 Suelo < " + String(thresholds.soilWet);
                bot.sendMessage(chatID, report, "Markdown");
            }
        }

        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

#endif // SENSOR_TELEGRAM_WIFI_H
