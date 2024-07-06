#include "SettingService.h"
#include "WiFiManager.h"
#include <EEPROMUtils.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

BLECharacteristic *wifiCredentialCharacteristic;
BLECharacteristic *wifiConnectionCharacteristic;

void setUpSettingService(BLEServer *pServer, WiFiManager *wifiManager, const char *deviceUUID, const char *deviceType)
{
    BLEService *settingService = pServer->createService(SETTING_SERVICE_UUD);

    // Setting Wifi Credential
    wifiCredentialCharacteristic = settingService->createCharacteristic(
        SETTING_WIFI_CREDENTIAL_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_READ);
    wifiCredentialCharacteristic->setCallbacks(new SettingWifiCredentialCharacteristicCallback(wifiManager));

    BLEDescriptor *pSettingCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
    wifiCredentialCharacteristic->addDescriptor(pSettingCCCDescriptor);

    // Setting Wifi Connection Status
    wifiConnectionCharacteristic = settingService->createCharacteristic(
        SETTING_WIFI_CONNECTION_STATUS_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_READ);
    wifiConnectionCharacteristic->setCallbacks(new SettingWifiConnectionStatusCallBack(wifiManager));

    BLEDescriptor *pWifiConnectionCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
    wifiConnectionCharacteristic->addDescriptor(pWifiConnectionCCCDescriptor);

    // Setting Device Info
    BLECharacteristic *settingDeviceInfoCharacteristic = settingService->createCharacteristic(
        SETTING_DEVICE_INFO_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ);
    settingDeviceInfoCharacteristic->setCallbacks(new SettingDeviceInfoCallback(deviceUUID, deviceType));

    settingService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SETTING_SERVICE_UUD);
}

void SettingWifiCredentialCharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic)
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

void SettingWifiCredentialCharacteristicCallback::onRead(BLECharacteristic *pCharacteristic)
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

void SettingWifiConnectionStatusCallBack::onRead(BLECharacteristic *pCharacteristic)
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

void SettingDeviceInfoCallback::onRead(BLECharacteristic *pCharacteristic)
{
    pCharacteristic->setValue(String(deviceUUID) + "," + String(deviceType));
}
