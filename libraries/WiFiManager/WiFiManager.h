#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <BLECharacteristic.h>

class WiFiManager
{
public:
    WiFiManager();
    void connectToWiFi(const char *ssid, const char *password);
    bool isConnected();
};

#endif
