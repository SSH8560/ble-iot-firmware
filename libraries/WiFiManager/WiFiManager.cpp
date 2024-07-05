#include "WiFiManager.h"

WiFiManager::WiFiManager(BLECharacteristic *wifiConnectionCharacteristic, LED *wifiLED)
{
    this->wifiConnectionCharacteristic = wifiConnectionCharacteristic;
    this->wifiLED = wifiLED;
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
            onDisconnect();
            return;
        }
        delay(500);
        Serial.print(".");
    }
    onConnect();
    Serial.println("Connected to WiFi");
}

bool WiFiManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::onConnect()
{
    if (wifiConnectionCharacteristic)
    {
        wifiConnectionCharacteristic->setValue("connected");
        wifiConnectionCharacteristic->notify();
    }
    if (wifiLED)
    {
        wifiLED->on();
    }
}

void WiFiManager::onDisconnect()
{
    if (wifiConnectionCharacteristic)
    {
        wifiConnectionCharacteristic->setValue("disconnected");
        wifiConnectionCharacteristic->notify();
    }
    if (wifiLED)
    {
        wifiLED->off();
    }
}
