#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <BLECharacteristic.h>
#include <LED.h>

class WiFiManager
{
public:
    WiFiManager(BLECharacteristic *wifiConnectionCharacteristic, LED *wifiLED);
    void connectToWiFi(const char *ssid, const char *password);
    bool isConnected();

private:
    LED *wifiLED;
    BLECharacteristic *wifiConnectionCharacteristic;
    void onConnect();
    void onDisconnect();
};

#endif
