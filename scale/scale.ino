#include <Secrets.h>
#include <EEPROMUtils.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <HX711.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <LED.h>

#define DEVICE_UUID "b4506cfd-135d-466a-b2dc-a85de6586b84"
#define DEVICE_TYPE "SCALE"

#define LOAD_CELL_SERVICE_UUID "68b6285c-df48-4809-9b0d-8ff8196996d8"
#define LOAD_CELL_WEIGHT_CHARACTERISTIC_UUID "8f46de3a-b1d6-4fa2-9298-a444f2e0f10d"
#define LOAD_CELL_TARE_CHARACTERISTIC_UUID "717a80d8-2e1e-42fb-bd94-ec7bdb345c65"
#define LOAD_CELL_CALIBRATION_CHARACTERISTIC_UUID "5ad21362-2a96-4d45-836b-dcb7e28dd1b8"
#define SETTING_SERVICE_UUD "3126d1ed-031f-4470-8906-3a3b90bc039a"
#define SETTING_WIFI_CREDENTIAL_CHARACTERISTIC_UUID "1683d984-ba48-4ad4-869c-fcff86e39ce5"
#define SETTING_WIFI_CONNECTION_STATUS_CHARACTERISTIC_UUID "6dfba204-77ac-437b-8b5a-194d2545c587"
#define SETTING_DEVICE_INFO_CHARACTERISTIC_UUID "b023daf1-980f-4c91-826d-0f0b0e3675c2"

#define DOUT 4
#define PD_SCK 2
#define WIFI_LED_PIN 18
#define DEFAULT_CALIBRATION 198.f

HX711 scale(DOUT, PD_SCK);
WiFiManager *wifiManager;
LED *wifiLED;
BLEServer *pServer;
BLECharacteristic *loadCellWeightCharacteristic;
BLECharacteristic *wifiCredentialCharacteristic;
BLECharacteristic *wifiConnectionCharacteristic;

bool client_is_connected = false;
bool load_cell_sampling_enabled = false;

unsigned long lastWeightPostTime = 0;
unsigned long lastWeightNotifyTime = 0;
const unsigned long weightPostInterval = 600000;
const unsigned long weightNotifyInterval = 500;

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
  void onRead(BLECharacteristic *pCharacteristic) override
  {
    Serial.println("offset");
    load_cell_sampling_enabled = false;
    delay(100);
    scale.get_units(5);
    scale.tare();
    load_cell_sampling_enabled = true;
  }
};
class LoadCellWeightCallBack : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    float weight = scale.get_units();
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
    Serial.println(floatValue);

    scale.set_scale(floatValue);
    saveCalibrationToEEPROM(floatValue);
  }

  void onRead(BLECharacteristic *pCharacteristic) override
  {
    float calibrationValue = readCalibrationFromEEPROM();

    if (calibrationValue == 0)
    {
      calibrationValue = DEFAULT_CALIBRATION;
    }
    pCharacteristic->setValue(calibrationValue);
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
class WifiConnectionDescriptorCallback : public BLEDescriptorCallbacks
{
  void onWrite(BLEDescriptor *pDescriptor)
  {
    u_int8_t desc = (*(pDescriptor->getValue()));
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
class SettingWifiCredentialCharacteristicCallback : public BLECharacteristicCallbacks
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

    wifiManager->connectToWiFi(ssid.c_str(), password.c_str());

    saveWiFiCredentialsToEEPROM(ssid.c_str(), password.c_str());
    pCharacteristic->setValue(value);
    pCharacteristic->notify();
  }
  void onRead(BLECharacteristic *pCharacteristic) override
  {
    char ssid[100] = {0};
    char password[100] = {0};

    readWiFiCredentialsFromEEPROM(ssid, password);

    if (strlen(ssid) == 0 || strlen(password) == 0)
    {
      Serial.println("No WiFi credentials found in EEPROM");
      pCharacteristic->setValue("No credentials");
    }
    else
    {
      String credentials = String(ssid) + "," + String(password);
      pCharacteristic->setValue(credentials.c_str());
      Serial.print("Read credentials from EEPROM: ");
      Serial.println(credentials);
    }
  }
};
class SettingWifiConnectionStatusCallBack : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic) override
  {
    if (wifiManager->isConnected())
    {
      pCharacteristic->setValue("connected");
    }
    else
    {
      pCharacteristic->setValue("disconnected");
    }
  }
};
class SettingDeviceInfoCallback : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic) override
  {
    pCharacteristic->setValue(String(DEVICE_UUID) + "," + String(DEVICE_TYPE));
  }
};

void setup()
{
  Serial.begin(115200);
  wifiLED = new LED(WIFI_LED_PIN);
  wifiManager = new WiFiManager(wifiConnectionCharacteristic, wifiLED);

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
  float weight = scale.get_units(5);
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

void setUpBLEServer()
{
  int dashIndex = String(DEVICE_UUID).indexOf('-');
  String shortUUID = String(DEVICE_UUID).substring(0, dashIndex);
  String deviceName = String(DEVICE_TYPE) + "_" + shortUUID;

  BLEDevice::init(deviceName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BaseBLEServerCallbacks());
}
void setUpBLEService()
{
  BLEService *service = pServer->createService(LOAD_CELL_SERVICE_UUID);
  BLEService *settingService = pServer->createService(SETTING_SERVICE_UUD);

  // Weigh
  loadCellWeightCharacteristic = service->createCharacteristic(
      LOAD_CELL_WEIGHT_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);
  loadCellWeightCharacteristic->setCallbacks(new LoadCellWeightCallBack());

  BLEDescriptor *pLoadCellCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
  pLoadCellCCCDescriptor->setCallbacks(new LoadCellDescriptorCallback());
  loadCellWeightCharacteristic->addDescriptor(pLoadCellCCCDescriptor);

  // Tare
  BLECharacteristic *loadCellTareCharacteristic = service->createCharacteristic(
      LOAD_CELL_TARE_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ);
  loadCellTareCharacteristic->setCallbacks(new LoadCellTareCallback());

  // Calibration
  BLECharacteristic *loadCellCalibrationCharacteristic = service->createCharacteristic(
      LOAD_CELL_CALIBRATION_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_READ);
  loadCellCalibrationCharacteristic->setCallbacks(new LoadCellCalibrationCallback());

  // Setting Wifi Credential
  wifiCredentialCharacteristic = settingService->createCharacteristic(
      SETTING_WIFI_CREDENTIAL_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_READ);
  wifiCredentialCharacteristic->setCallbacks(new SettingWifiCredentialCharacteristicCallback());

  BLEDescriptor *pSettingCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
  pSettingCCCDescriptor->setCallbacks(new SettingDescriptorCallback());
  wifiCredentialCharacteristic->addDescriptor(pSettingCCCDescriptor);

  // Setting Wifi Connection Status
  wifiConnectionCharacteristic = settingService->createCharacteristic(
      SETTING_WIFI_CONNECTION_STATUS_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_READ);
  wifiConnectionCharacteristic->setCallbacks(new SettingWifiConnectionStatusCallBack());

  BLEDescriptor *pWifiConnectionCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
  pWifiConnectionCCCDescriptor->setCallbacks(new WifiConnectionDescriptorCallback());
  wifiConnectionCharacteristic->addDescriptor(pWifiConnectionCCCDescriptor);

  // Setting Device Info
  BLECharacteristic *settingDeviceInfoCharacteristic = settingService->createCharacteristic(
      SETTING_DEVICE_INFO_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ);
  settingDeviceInfoCharacteristic->setCallbacks(new SettingDeviceInfoCallback());

  settingService->start();
  service->start();
}
void setupAdvertisementData(void)
{
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SETTING_SERVICE_UUD);
  pAdvertising->addServiceUUID(LOAD_CELL_SERVICE_UUID);
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

  wifiManager->connectToWiFi(ssid, password);

  if (wifiManager->isConnected())
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
  float calibrationValue = readCalibrationFromEEPROM();
  if (calibrationValue != 0)
  {
    Serial.println("Using calibration value from EEPROM");
    scale.set_scale(calibrationValue);
  }
  else
  {
    Serial.println("Using default calibration value");
    scale.set_scale(DEFAULT_CALIBRATION);
  }
  scale.tare();
}
