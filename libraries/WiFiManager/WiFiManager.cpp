#include "WiFiManager.h"

WiFiManager::WiFiManager()
{
}

void WiFiManager::connectToWiFi(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    unsigned long startTime = millis();
    unsigned long timeout = 10000;

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime >= timeout)
        {
            Serial.println("Failed to connect to WiFi: Timeout");
            return;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

bool WiFiManager::isConnected()
{
    return WL_CONNECTED;
}
