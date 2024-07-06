#include "WiFiManager.h"
#include "EEPROMUtils.h"

WiFiManager::WiFiManager(BLECharacteristic *wifiConnectionCharacteristic, LED *wifiLED)
{
    this->wifiConnectionCharacteristic = wifiConnectionCharacteristic;
    this->wifiLED = wifiLED;

    char ssid[100];
    char password[100];

    readWiFiCredentialsFromEEPROM(ssid, password);

    if (strlen(ssid) == 0 || strlen(password) == 0)
    {
        Serial.println("No WiFi credentials found in EEPROM");
        return;
    }

    this->connectToWiFi(ssid, password);

    if (this->isConnected())
    {
        Serial.println("WiFi connected using credentials from EEPROM");
    }
    else
    {
        Serial.println("Failed to connect to WiFi using credentials from EEPROM");
    }
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
