#ifndef ULTRASONIC_SERVICE_H
#define ULTRASONIC_SERVICE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <UltrasonicSensor.h>

#define ULTRASONIC_SERVICE_UUID "735fd752-ca98-43c4-ba30-63170bacab0c"
#define ULTRASONIC_DISTANCE_CHARACTERISTIC_UUID "c0f26442-bba1-45ab-8ca1-201b3ea26cc9"

void setUpUltrasonicService(BLEServer *pServer, UltrasonicSensor *ultrasonicSensor);

class UltrasonicDistanceCharacteristicCallback : public BLECharacteristicCallbacks
{
public:
    UltrasonicDistanceCharacteristicCallback(UltrasonicSensor *ultrasonicSensor) : ultrasonicSensor(ultrasonicSensor) {}
    void onRead(BLECharacteristic *pCharacteristic) override;

private:
    UltrasonicSensor *ultrasonicSensor;
};

class UltrasonicDistanceCharacteristicDescriptorCallback : public BLEDescriptorCallbacks
{
public:
    void onWrite(BLEDescriptor *pDescriptor) override;
};

#endif
