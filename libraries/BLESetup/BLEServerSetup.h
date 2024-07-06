#ifndef BLE_SERVER_SETUP_H
#define BLE_SERVER_SETUP_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFiManager.h>
#include <LoadCellManager.h>
#include <SettingService.h>
#include <LoadCellService.h>
#include <LED.h>

class BaseBLEServerCallbacks : public BLEServerCallbacks
{
public:
    BaseBLEServerCallbacks(LED *bleLED) : bleLED(bleLED){};
    void onConnect(BLEServer *pServer) override;
    void onDisconnect(BLEServer *pServer) override;

private:
    LED *bleLED;
};

void setUpBLEServer(const char *deviceUUID, const char *deviceType, LED *bleLED);

void startAdvertisement();

#endif
