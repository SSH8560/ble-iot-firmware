#include "LoadCellService.h"

BLECharacteristic *loadCellWeightCharacteristic;
bool load_cell_sampling_enabled = false;
// LoadCellTareCharacteristicCallback 클래스의 onRead 메소드 구현
void LoadCellTareCharacteristicCallback::onRead(BLECharacteristic *pCharacteristic)
{
    Serial.println("offset");
    // 로드셀 샘플링을 일시 중지하고 테어링을 수행합니다.
    load_cell_sampling_enabled = false;
    delay(100);
    loadCellManager->getWeight();
    loadCellManager->tare();
    load_cell_sampling_enabled = true;
}

// LoadCellWeightCharacteristicCallBack 클래스의 onRead 메소드 구현
void LoadCellWeightCharacteristicCallBack::onRead(BLECharacteristic *pCharacteristic)
{
    Serial.println("Read Weight");
    float weight = loadCellManager->getWeight(5);
    pCharacteristic->setValue(weight);
    Serial.println(weight);
}

// LoadCellCalibrationCharacteristicCallback 클래스의 onWrite 메소드 구현
void LoadCellCalibrationCharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic)
{
    String value = pCharacteristic->getValue();
    float floatValue = atof(value.c_str());
    loadCellManager->setCalibration(floatValue);
    saveCalibrationToEEPROM(floatValue);
}

// LoadCellCalibrationCharacteristicCallback 클래스의 onRead 메소드 구현
void LoadCellCalibrationCharacteristicCallback::onRead(BLECharacteristic *pCharacteristic)
{
    float calibrationValue = readCalibrationFromEEPROM();

    if (calibrationValue == 0)
    {
        calibrationValue = DEFAULT_CALIBRATION;
    }
    pCharacteristic->setValue(calibrationValue);
}

void LoadCellWeightCharacteristicDescriptorCallback::onWrite(BLEDescriptor *pDescriptor)
{
    Serial.println("LoadCellDescriptorCallback->onWrite: Called");
    u_int8_t desc = (*(pDescriptor->getValue()));
    if (desc == 1)
    {
        Serial.println("Weight Notify on");
        load_cell_sampling_enabled = true;
    }
    else
    {
        Serial.println("Weight Notify off");
        load_cell_sampling_enabled = false;
    }
}

// LoadCell 서비스 설정 함수
void setUpLoadCellService(BLEServer *pServer, LoadCellManager *loadCellManager)
{
    BLEService *service = pServer->createService(LOAD_CELL_SERVICE_UUID);

    // Weight Characteristic
    loadCellWeightCharacteristic = service->createCharacteristic(
        LOAD_CELL_WEIGHT_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    loadCellWeightCharacteristic->setCallbacks(new LoadCellWeightCharacteristicCallBack(loadCellManager));

    BLEDescriptor *pLoadCellCCCDescriptor = new BLEDescriptor((uint16_t)0x2902);
    pLoadCellCCCDescriptor->setCallbacks(new LoadCellWeightCharacteristicDescriptorCallback());
    loadCellWeightCharacteristic->addDescriptor(pLoadCellCCCDescriptor);

    // Tare Characteristic
    BLECharacteristic *loadCellTareCharacteristic = service->createCharacteristic(
        LOAD_CELL_TARE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ);
    loadCellTareCharacteristic->setCallbacks(new LoadCellTareCharacteristicCallback(loadCellManager));

    // Calibration Characteristic
    BLECharacteristic *loadCellCalibrationCharacteristic = service->createCharacteristic(
        LOAD_CELL_CALIBRATION_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
    loadCellCalibrationCharacteristic->setCallbacks(new LoadCellCalibrationCharacteristicCallback(loadCellManager));

    service->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(LOAD_CELL_SERVICE_UUID);
}
