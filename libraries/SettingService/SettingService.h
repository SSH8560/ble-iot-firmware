#ifndef SETTING_SERVICE_H
#define SETTING_SERVICE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SETTING_SERVICE_UUD "3126d1ed-031f-4470-8906-3a3b90bc039a"
#define SETTING_WIFI_CREDENTIAL_CHARACTERISTIC_UUID "1683d984-ba48-4ad4-869c-fcff86e39ce5"
#define SETTING_WIFI_CONNECTION_STATUS_CHARACTERISTIC_UUID "6dfba204-77ac-437b-8b5a-194d2545c587"
#define SETTING_DEVICE_INFO_CHARACTERISTIC_UUID "b023daf1-980f-4c91-826d-0f0b0e3675c2"

class WiFiManager;

extern BLECharacteristic *wifiCredentialCharacteristic;
extern BLECharacteristic *wifiConnectionCharacteristic;

void setUpSettingService(BLEServer *pServer, WiFiManager *wifiManager, const char *deviceUUID, const char *deviceType);
void setupAdvertisementData(void);

class SettingWifiCredentialCharacteristicCallback : public BLECharacteristicCallbacks
{
public:
    SettingWifiCredentialCharacteristicCallback(WiFiManager *wifiManager) : wifiManager(wifiManager) {}
    void onWrite(BLECharacteristic *pCharacteristic) override;
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    WiFiManager *wifiManager;
};

class SettingWifiConnectionStatusCallBack : public BLECharacteristicCallbacks
{
public:
    SettingWifiConnectionStatusCallBack(WiFiManager *wifiManager) : wifiManager(wifiManager) {}
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    WiFiManager *wifiManager;
};

class SettingDeviceInfoCallback : public BLECharacteristicCallbacks
{
public:
    SettingDeviceInfoCallback(const char *deviceUUID, const char *deviceType) : deviceUUID(deviceUUID), deviceType(deviceType) {}
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    const char *deviceUUID;
    const char *deviceType;
};

class WifiConnectionCharacteristicDescriptorCallback : public BLEDescriptorCallbacks
{
public:
    void onWrite(BLEDescriptor *pDescriptor) override;
};

class WifiCredentialCharacteristicDescriptorCallback : public BLEDescriptorCallbacks
{
public:
    void onWrite(BLEDescriptor *pDescriptor) override;
};

#endif
