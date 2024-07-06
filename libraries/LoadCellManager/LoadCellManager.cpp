#include "LoadCellManager.h"
#include "EEPROMUtils.h"

LoadCellManager::LoadCellManager(int dout, int pd_sck, float defaultCalibration) : scale(dout, pd_sck)
{
    float calibrationValue = readCalibrationFromEEPROM();
    if (calibrationValue != 0)
    {
        scale.set_scale(calibrationValue);
    }
    else
    {
        scale.set_scale(defaultCalibration);
    }
    scale.tare();

    // Debugging output
    Serial.println("LoadCellManager initialized.");
    Serial.print("Calibration value: ");
    Serial.println(scale.get_scale());
}

float LoadCellManager::getWeight(int times)
{
    float weight = scale.get_units(5);
    Serial.print("Calibration value: ");
    Serial.println(scale.get_scale());
    return weight;
}

void LoadCellManager::tare()
{
    delay(500);
    scale.tare();
}

void LoadCellManager::setCalibration(float calibrationValue)
{
    scale.set_scale(calibrationValue);
}
