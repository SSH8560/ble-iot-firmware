#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <Secrets.h>
#include <EEPROMUtils.h>
#include <WiFiManager.h>
#include <UltrasonicSensor.h>
#include <LED.h>
#include <SettingService.h>
#include <UltrasonicService.h>
#include <BLEServerSetup.h>

#define DEVICE_UUID "2344fd60-1f17-4c1a-8e92-2f67a313fd9a"
#define DEVICE_TYPE "DIST"

#define TRIG_PIN 2
#define ECHO_PIN 4
#define WIFI_LED_PIN 18
#define BLE_LED_PIN 19
#define DEFAULT_CALIBRATION 198.f

WiFiManager *wifiManager;
UltrasonicSensor *ultrasonicSensor;
LED *wifiLED;
LED *bleLED;
extern BLEServer *pServer;
extern BLECharacteristic *distanceCharacteristic;

extern bool client_is_connected;

unsigned long lastDistanceNotifyTime = 0;
const unsigned long distanceNotifyInterval = 500;

void setup()
{
    Serial.begin(115200);
    wifiLED = new LED(WIFI_LED_PIN);
    bleLED = new LED(BLE_LED_PIN);
    wifiManager = new WiFiManager(wifiConnectionCharacteristic, wifiLED);
    ultrasonicSensor = new UltrasonicSensor(TRIG_PIN, ECHO_PIN);
    setUpBLEServer(DEVICE_UUID, DEVICE_TYPE, bleLED);
    setUpSettingService(pServer, wifiManager, DEVICE_UUID, DEVICE_TYPE);
    setUpUltrasonicService(pServer, ultrasonicSensor);
    startAdvertisement();
    Serial.println("Setup Complete");
}

void loop()
{
    unsigned long currentTime = millis();
    if (client_is_connected)
    {
        if (currentTime - lastDistanceNotifyTime >= distanceNotifyInterval)
        {
            lastDistanceNotifyTime = currentTime;
            notifyDistance();
        }
    }
}

void notifyDistance()
{
    distanceCharacteristic->setValue(String(ultrasonicSensor->getDistance()));
    distanceCharacteristic->notify();
}
