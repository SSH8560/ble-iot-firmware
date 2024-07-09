#include "DistanceService.h"

BLECharacteristic *distanceCharacteristic;
BLECharacteristic *distanceChangeThresholdCharacteristic;

void setUpDistanceService(BLEServer *pServer, UltrasonicSensor *sensor)
{
    BLEService *distanceService = pServer->createService(DISTANCE_SERVICE_UUID);

    distanceCharacteristic = distanceService->createCharacteristic(
        DISTANCE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    distanceCharacteristic->setCallbacks(new DistanceCharacteristicCallback(sensor));

    BLEDescriptor *pDistanceCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
    pDistanceCCCDescriptor->setCallbacks(new DistanceCharacteristicDescriptorCallback());
    distanceCharacteristic->addDescriptor(pDistanceCCCDescriptor);

    distanceChangeThresholdCharacteristic = distanceService->createCharacteristic(
        DISTANCE_CHANGE_THRESHOLD_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    distanceChangeThresholdCharacteristic->setCallbacks(new DistanceChangeThresholdCharacteristicCallback());

    distanceService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(DISTANCE_SERVICE_UUID);
}

void DistanceCharacteristicCallback::onRead(BLECharacteristic *pCharacteristic)
{
    int distance = sensor->getDistance();
    pCharacteristic->setValue(String(distance));
}

void DistanceChangeThresholdCharacteristicCallback::onRead(BLECharacteristic *pCharacteristic)
{
    int threshold = readDistanceChangeThreshold();
    pCharacteristic->setValue(String(threshold));
}

void DistanceChangeThresholdCharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic)
{
    String value = pCharacteristic->getValue();
    if (value.length() > 0)
    {
        int threshold = value.toInt();
        writeDistanceChangeThreshold(threshold);
    }
}

void DistanceCharacteristicDescriptorCallback::onWrite(BLEDescriptor *pDescriptor)
{
    uint8_t desc = (*(pDescriptor->getValue()));
    if (desc == 1)
    {
        Serial.println("Distance Notify on");
    }
    else
    {
        Serial.println("Distance Notify off");
    }
}
