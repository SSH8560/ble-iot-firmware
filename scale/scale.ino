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
#include <LoadCellManager.h>
#include <LED.h>
#include <SettingService.h>
#include <LoadCellService.h>
#include <BLEServerSetup.h>

#define DEVICE_UUID "b4506cfd-135d-466a-b2dc-a85de6586b84"
#define DEVICE_TYPE "SCALE"

#define DOUT 4
#define PD_SCK 2
#define WIFI_LED_PIN 18
#define BLE_LED_PIN 19
#define DEFAULT_CALIBRATION 198.f

WiFiManager *wifiManager;
LoadCellManager *loadCellManager;
LED *wifiLED;
LED *bleLED;
extern BLEServer *pServer;
extern BLECharacteristic *loadCellWeightCharacteristic;

extern bool client_is_connected;
extern bool load_cell_sampling_enabled;

unsigned long lastWeightPostTime = 0;
unsigned long lastWeightNotifyTime = 0;
const unsigned long weightPostInterval = 600000;
const unsigned long weightNotifyInterval = 500;

void setup()
{
  Serial.begin(115200);
  wifiLED = new LED(WIFI_LED_PIN);
  bleLED = new LED(BLE_LED_PIN);
  wifiManager = new WiFiManager(wifiConnectionCharacteristic, wifiLED);
  loadCellManager = new LoadCellManager(DOUT, PD_SCK, DEFAULT_CALIBRATION);
  setUpBLEServer(DEVICE_UUID, DEVICE_TYPE, bleLED);
  setUpSettingService(pServer, wifiManager, DEVICE_UUID, DEVICE_TYPE);
  setUpLoadCellService(pServer, loadCellManager);
  startAdvertisement();
  Serial.println("Setup Complete");
}

void loop()
{
  unsigned long currentTime = millis();
  if (client_is_connected & load_cell_sampling_enabled)
  {
    if (currentTime - lastWeightNotifyTime >= weightNotifyInterval)
    {
      lastWeightNotifyTime = currentTime;
      notifyWeight();
    }
  }

  if (wifiManager->isConnected())
  {
    if (currentTime - lastWeightPostTime >= weightPostInterval)
    {
      lastWeightPostTime = currentTime;
      postWeight();
    }
  }
}

void notifyWeight(void)
{
  float weight = loadCellManager->getWeight();
  loadCellWeightCharacteristic->setValue(weight);
  loadCellWeightCharacteristic->notify();
}

void postWeight()
{
  if (!wifiManager->isConnected())
  {
    Serial.println("WiFi is not connected. Cannot send data.");
    return;
  }

  float weight = loadCellManager->getWeight();
  int maxRetries = 5;
  int retryCount = 0;
  bool success = false;

  while (retryCount < maxRetries && !success)
  {
    HTTPClient http;
    http.begin(SCALE_SERVICE_URL);
    http.addHeader("apikey", API_KEY);
    http.addHeader("Authorization", "Bearer " API_KEY);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Prefer", "return=minimal");
    String payload = "{ \"value\": \"" + String(weight) + "\", " +
                     "\"device_id\":\"" + DEVICE_UUID + "\" }";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      success = true;
    }
    else
    {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
      retryCount++;
      if (retryCount < maxRetries)
      {
        Serial.println("Retrying... (" + String(retryCount) + "/" + String(maxRetries) + ")");
        delay(1000);
      }
    }

    http.end();
  }

  if (!success)
  {
    Serial.println("Failed to send data after " + String(maxRetries) + " attempts.");
  }
}
