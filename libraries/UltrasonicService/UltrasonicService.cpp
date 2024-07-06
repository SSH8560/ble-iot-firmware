#include "UltrasonicService.h"

BLECharacteristic *distanceCharacteristic;

void UltrasonicDistanceCharacteristicCallback::onRead(BLECharacteristic *pCharacteristic)
{
    float distance = ultrasonicSensor->getDistance();
    pCharacteristic->setValue(distance);

    Serial.print("Distance measured: ");
    Serial.println(distance);
}

void UltrasonicDistanceCharacteristicDescriptorCallback::onWrite(BLEDescriptor *pDescriptor)
{
    Serial.println("UltrasonicDistanceCharacteristicDescriptorCallback->onWrite: Called");
    uint8_t desc = (*(pDescriptor->getValue()));
    Serial.print("Descriptor value: ");
    Serial.println(desc);
    if (desc == 1)
    {
        Serial.println("Distance Notify on");
    }
    else
    {
        Serial.println("Distance Notify off");
    }
}

void setUpUltrasonicService(BLEServer *pServer, UltrasonicSensor *ultrasonicSensor)
{
    BLEService *service = pServer->createService(ULTRASONIC_SERVICE_UUID);

    distanceCharacteristic = service->createCharacteristic(
        ULTRASONIC_DISTANCE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    distanceCharacteristic->setCallbacks(new UltrasonicDistanceCharacteristicCallback(ultrasonicSensor));

    BLEDescriptor *pDistanceCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
    pDistanceCCCDescriptor->setCallbacks(new UltrasonicDistanceCharacteristicDescriptorCallback());
    distanceCharacteristic->addDescriptor(pDistanceCCCDescriptor);

    service->start();
}
