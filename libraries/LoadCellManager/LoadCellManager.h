#ifndef LOAD_CELL_MANAGER_H
#define LOAD_CELL_MANAGER_H

#include <HX711.h>

class LoadCellManager
{
public:
    LoadCellManager(int dout, int pd_sck, float defaultCalibration);
    float getWeight(int times = 5);
    void tare();
    void setCalibration(float calibrationValue);

private:
    HX711 scale;
};

#endif
