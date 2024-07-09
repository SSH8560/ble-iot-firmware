#ifndef LOADCELL_SERVICE_H
#define LOADCELL_SERVICE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <UltrasonicSensor.h>
#include <EEPROMUtils.h>

#define DISTANCE_SERVICE_UUID "ed675cf4-ff51-4be8-96e1-4c89b78e0fa0"
#define DISTANCE_CHARACTERISTIC_UUID "3e5df93e-36a6-4f01-a9d0-021d6cd599a4"
#define DISTANCE_CHANGE_THRESHOLD_CHARACTERISTIC_UUID "2da860d3-536f-4e84-afcb-27e0178d3102"

void setUpDistanceService(BLEServer *pServer, UltrasonicSensor *sensor);

class DistanceCharacteristicCallback : public BLECharacteristicCallbacks
{
public:
    DistanceCharacteristicCallback(UltrasonicSensor *sensor) : sensor(sensor) {}
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    UltrasonicSensor *sensor;
};

class DistanceChangeThresholdCharacteristicCallback : public BLECharacteristicCallbacks
{
public:
    void onRead(BLECharacteristic *pCharacteristic) override;
    void onWrite(BLECharacteristic *pCharacteristic) override;
};

class DistanceCharacteristicDescriptorCallback : public BLEDescriptorCallbacks
{
public:
    void onWrite(BLEDescriptor *pDescriptor) override;
};

#endif
