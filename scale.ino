#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <HX711.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include "secrets.h"

#define DEVICE_UUID "b4506cfd-135d-466a-b2dc-a85de6586b84"
#define DEVICE_TYPE "SCALE"

#define SERVICE_UUID "68b6285c-df48-4809-9b0d-8ff8196996d8"
#define CHARACTERISTIC_UUID "8f46de3a-b1d6-4fa2-9298-a444f2e0f10d"
#define LOAD_CELL_TARE_CHARACTERISTIC_UUID "717a80d8-2e1e-42fb-bd94-ec7bdb345c65"
#define LOAD_CELL_CALIBRATION_CHARACTERISTIC_UUID "5ad21362-2a96-4d45-836b-dcb7e28dd1b8"
#define SETTING_SERVICE_UUD "3126d1ed-031f-4470-8906-3a3b90bc039a"
#define SETTING_CHARACTERISTIC_UUID "1683d984-ba48-4ad4-869c-fcff86e39ce5"

#define EEPROM_SIZE 512
#define EEPROM_SSID_START 0
#define EEPROM_PASS_START 100

#define DOUT 4
#define PD_SCK 2
HX711 scale(DOUT, PD_SCK);
BLEServer *pServer;
BLECharacteristic *loadCellCharacteristic;
BLECharacteristic *settingCharacteristic;

bool client_is_connected = false;
bool wifi_is_connected = false;
bool load_cell_sampling_enabled = false;

unsigned long lastWeightPostTime = 0;
unsigned long lastWeightNotifyTime = 0;
const unsigned long weightPostInterval = 600000;
const unsigned long weightNotifyInterval = 500;
float scale_factor = 198.f;

class BaseBLEServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    client_is_connected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer)
  {
    client_is_connected = false;
    Serial.println("Device disconnected");
    pServer->getAdvertising()->start();
  }
};
class LoadCellTareCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic) override
  {
    Serial.println("offset");
    load_cell_sampling_enabled = false;
    delay(100);
    scale.get_units(5);
    scale.tare();
    load_cell_sampling_enabled = true;
  }
};
class SampleLoadCellCallback : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    Serial.println("SampleLoadCellCallback->onRead: Called");
    float weight = scale.get_units();
    Serial.print("Weight: ");
    Serial.print(weight, 1);
    Serial.println(" kg");
    std::string result = "{weight: " + std::to_string(weight) + "}";
    pCharacteristic->setValue(weight);
  }
};
class LoadCellCalibrationCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic) override
  {
    String value = pCharacteristic->getValue();
    Serial.print("Received Data: ");
    Serial.println(value.c_str());

    float floatValue = atof(value.c_str());
    Serial.print("Converted Float Value: ");
    scale.set_scale(floatValue);
  }
  void onRead(BLECharacteristic *pCharacteristic) override
  {
  }
};
class LoadCellDescriptorCallback : public BLEDescriptorCallbacks
{
  void onWrite(BLEDescriptor *pDescriptor)
  {
    Serial.println("LoadCellDescriptorCallback->onWrite: Called");
    u_int8_t desc = (*(pDescriptor->getValue()));
    Serial.println(std::to_string(desc).c_str());
    if (desc == 1)
    {
      Serial.println("Notify on");
      load_cell_sampling_enabled = true;
    }
    else
    {
      Serial.println("Notify off");
      load_cell_sampling_enabled = false;
    }
  }
};
class SettingDescriptorCallback : public BLEDescriptorCallbacks
{
  void onWrite(BLEDescriptor *pDescriptor)
  {
    Serial.println("SettingDescriptorCallback->onWrite: Called");
    u_int8_t desc = (*(pDescriptor->getValue()));
    Serial.println(std::to_string(desc).c_str());
    if (desc == 1)
    {
      Serial.println("Notify on");
    }
    else
    {
      Serial.println("Notify off");
    }
  }
};

void connectToWiFi(const char *ssid, const char *password)
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
      wifi_is_connected = false;
      settingCharacteristic->setValue("fail");
      settingCharacteristic->notify();
      return;
    }
    delay(500);
    Serial.print(".");
  }
  wifi_is_connected = true;
  settingCharacteristic->setValue("success");
  settingCharacteristic->notify();
  Serial.println("Connected to WiFi");
}
void saveWiFiCredentialsToEEPROM(const char *ssid, const char *password)
{
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 100; ++i)
  {
    EEPROM.write(EEPROM_SSID_START + i, ssid[i]);
    EEPROM.write(EEPROM_PASS_START + i, password[i]);
    if (ssid[i] == '\0' && password[i] == '\0')
      break;
  }
  EEPROM.commit();
  EEPROM.end();
  Serial.println("WiFi credentials saved to EEPROM");
}
void readWiFiCredentialsFromEEPROM(char *ssid, char *password)
{
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 100; ++i)
  {
    ssid[i] = EEPROM.read(EEPROM_SSID_START + i);
    password[i] = EEPROM.read(EEPROM_PASS_START + i);
    if (ssid[i] == '\0' && password[i] == '\0')
      break;
  }
  EEPROM.end();
}
class SettingCharacteristicCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic) override
  {
    String value = pCharacteristic->getValue();
    Serial.print("Received Data: ");
    Serial.println(value.c_str());

    int firstCommaIndex = value.indexOf(',');

    if (firstCommaIndex == -1)
    {
      Serial.println("Invalid format. Expected 'ssid,password'");
      return;
    }

    String ssid = value.substring(0, firstCommaIndex);
    String password = value.substring(firstCommaIndex + 1, value.length());

    Serial.print("Parsed SSID: ");
    Serial.println(ssid);
    Serial.print("Parsed Password: ");
    Serial.println(password);

    connectToWiFi(ssid.c_str(), password.c_str());

    saveWiFiCredentialsToEEPROM(ssid.c_str(), password.c_str());
  }
  void onRead(BLECharacteristic *pCharacteristic) override
  {
    Serial.println("SampleLoadCellCallback->onRead: Called");
    pCharacteristic->setValue(String(DEVICE_UUID)+","+String(DEVICE_TYPE));
  }
};

void setup()
{
  Serial.begin(115200);

  setUpBLEServer();
  setUpBLEService();
  setupAdvertisementData();
  setUpScale();
  setupWiFi();

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

  if (wifi_is_connected)
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
  float weight = scale.get_units(5);
  loadCellCharacteristic->setValue(weight);
  loadCellCharacteristic->notify();
}
void postWeight()
{
  if (!wifi_is_connected)
  {
    Serial.println("WiFi is not connected. Cannot send data.");
    return;
  }

  float weight = scale.get_units(5);
  int maxRetries = 5;
  int retryCount = 0;
  bool success = false;

  while (retryCount < maxRetries && !success)
  {
    HTTPClient http;
    http.begin(SERVICE_URL);
    http.addHeader("apikey", API_KEY);
    http.addHeader("Authorization", "Bearer " API_KEY);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Prefer", "return=minimal");
    String payload =  "{ \"value\": \"" + String(weight) + "\", " +
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

void setUpBLEServer()
{
  BLEDevice::init(String(DEVICE_TYPE)+" "+String(DEVICE_UUID));
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BaseBLEServerCallbacks());
}
void setUpBLEService()
{
  BLEService *service = pServer->createService(SERVICE_UUID);
  BLEService *settingService = pServer->createService(SETTING_SERVICE_UUD);

  // Weight/Load Cell Characteristic
  loadCellCharacteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);
  loadCellCharacteristic->setCallbacks(new SampleLoadCellCallback());

  BLEDescriptor *pLoadCellCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
  pLoadCellCCCDescriptor->setCallbacks(new LoadCellDescriptorCallback());
  loadCellCharacteristic->addDescriptor(pLoadCellCCCDescriptor);

  // Tare
  BLECharacteristic *loadCellTareCharacteristic = service->createCharacteristic(
      LOAD_CELL_TARE_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE);
  loadCellTareCharacteristic->setCallbacks(new LoadCellTareCallback());

  // Calibration
  BLECharacteristic *loadCellCalibrationCharacteristic = service->createCharacteristic(
      LOAD_CELL_CALIBRATION_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_READ);
  loadCellCalibrationCharacteristic->setCallbacks(new LoadCellCalibrationCallback());

  // Setting
  settingCharacteristic = settingService->createCharacteristic(
      SETTING_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_READ);
  settingCharacteristic->setCallbacks(new SettingCharacteristicCallback());
  settingCharacteristic->setValue(String(wifi_is_connected));

  BLEDescriptor *pSettingCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
  pSettingCCCDescriptor->setCallbacks(new SettingDescriptorCallback());
  settingCharacteristic->addDescriptor(pSettingCCCDescriptor);

  settingService->start();
  service->start();
}
void setupAdvertisementData(void)
{
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SETTING_SERVICE_UUD);
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}
void setupWiFi()
{
  char ssid[100];
  char password[100];

  readWiFiCredentialsFromEEPROM(ssid, password);

  if (strlen(ssid) == 0 || strlen(password) == 0)
  {
    Serial.println("No WiFi credentials found in EEPROM");
    return;
  }

  connectToWiFi(ssid, password);

  if (wifi_is_connected)
  {
    Serial.println("WiFi connected using credentials from EEPROM");
  }
  else
  {
    Serial.println("Failed to connect to WiFi using credentials from EEPROM");
  }
}
void setUpScale()
{
  scale.set_scale(scale_factor);
  scale.tare();
}
