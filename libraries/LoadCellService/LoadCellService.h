#ifndef LOADCELL_SERVICE_H
#define LOADCELL_SERVICE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <LoadCellManager.h>
#include <EEPROMUtils.h>
#include <LoadCellManager.h>

#define LOAD_CELL_SERVICE_UUID "68b6285c-df48-4809-9b0d-8ff8196996d8"
#define LOAD_CELL_WEIGHT_CHARACTERISTIC_UUID "8f46de3a-b1d6-4fa2-9298-a444f2e0f10d"
#define LOAD_CELL_TARE_CHARACTERISTIC_UUID "717a80d8-2e1e-42fb-bd94-ec7bdb345c65"
#define LOAD_CELL_CALIBRATION_CHARACTERISTIC_UUID "5ad21362-2a96-4d45-836b-dcb7e28dd1b8"

#ifndef DEFAULT_CALIBRATION
#define DEFAULT_CALIBRATION 198.f
#endif

extern BLECharacteristic *loadCellWeightCharacteristic;

void setUpLoadCellService(BLEServer *pServer, LoadCellManager *loadCellManager);

class LoadCellTareCharacteristicCallback : public BLECharacteristicCallbacks
{
public:
    LoadCellTareCharacteristicCallback(LoadCellManager *loadCellManager) : loadCellManager(loadCellManager) {}
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    LoadCellManager *loadCellManager;
};

class LoadCellWeightCharacteristicCallBack : public BLECharacteristicCallbacks
{
public:
    LoadCellWeightCharacteristicCallBack(LoadCellManager *loadCellManager) : loadCellManager(loadCellManager) {}
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    LoadCellManager *loadCellManager;
};

class LoadCellCalibrationCharacteristicCallback : public BLECharacteristicCallbacks
{
public:
    LoadCellCalibrationCharacteristicCallback(LoadCellManager *loadCellManager) : loadCellManager(loadCellManager) {}
    void onRead(BLECharacteristic *pCharacteristic) override;
    void onWrite(BLECharacteristic *pCharacteristic) override;

private:
    LoadCellManager *loadCellManager;
};

class LoadCellWeightCharacteristicDescriptorCallback : public BLEDescriptorCallbacks
{
public:
    void onWrite(BLEDescriptor *pDescriptor) override;
};

#endif
