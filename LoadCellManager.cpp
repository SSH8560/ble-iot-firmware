#include "LoadCellManager.h"
#include "EEPROMUtils.h"

LoadCellManager::LoadCellManager(int dout, int pd_sck) : scale(dout, pd_sck) {}

void LoadCellManager::setup(float calibrationValue) {
    scale.set_scale(calibrationValue);
    delay(500);
    scale.tare();
}

float LoadCellManager::getWeight(int times) {
    return scale.get_units(times);
}

void LoadCellManager::tare() {
    delay(500);
    scale.tare();
}

void LoadCellManager::setCalibration(float calibrationValue) {
    scale.set_scale(calibrationValue);
}
