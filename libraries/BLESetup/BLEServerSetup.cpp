#include "BLEServerSetup.h"

bool client_is_connected = false;
BLEServer *pServer;

void BaseBLEServerCallbacks::onConnect(BLEServer *pServer)
{
    client_is_connected = true;
    bleLED->on();
}

void BaseBLEServerCallbacks::onDisconnect(BLEServer *pServer)
{
    client_is_connected = false;
    bleLED->off();
    pServer->getAdvertising()->start();
}

void setUpBLEServer(const char *deviceUUID, const char *deviceType, LED *bleLED)
{
    int dashIndex = String(deviceUUID).indexOf('-');
    String shortUUID = String(deviceUUID).substring(0, dashIndex);
    String deviceName = String(deviceType) + "_" + shortUUID;

    BLEDevice::init(deviceName.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BaseBLEServerCallbacks(bleLED));
}

void startAdvertisement()
{
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}
