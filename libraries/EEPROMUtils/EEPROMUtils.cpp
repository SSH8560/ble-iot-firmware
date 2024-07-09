#include "EEPROMUtils.h"
#include <Arduino.h>

void saveWiFiCredentialsToEEPROM(const char *ssid, const char *password)
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < 100; ++i)
    {
        EEPROM.write(EEPROM_SSID_START + i, ssid[i]);
        EEPROM.write(EEPROM_PASS_START + i, password[i]);
        if (ssid[i] == '\0' && password[i] == '\0')
            break;
    }
    EEPROM.commit();
    EEPROM.end();
    Serial.println("WiFi credentials saved to EEPROM");
}

void readWiFiCredentialsFromEEPROM(char *ssid, char *password)
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < 100; ++i)
    {
        ssid[i] = EEPROM.read(EEPROM_SSID_START + i);
        password[i] = EEPROM.read(EEPROM_PASS_START + i);
        if (ssid[i] == '\0' && password[i] == '\0')
            break;
    }
    EEPROM.end();
}

void saveCalibrationToEEPROM(float calibrationValue)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_CALIBRATION, calibrationValue);
    EEPROM.commit();
    EEPROM.end();
    Serial.println("Calibration value saved to EEPROM");
}

float readCalibrationFromEEPROM()
{
    EEPROM.begin(EEPROM_SIZE);
    float calibrationValue = 0;
    EEPROM.get(EEPROM_CALIBRATION, calibrationValue);
    EEPROM.end();

    if (isnan(calibrationValue) || calibrationValue > 1e6 || calibrationValue < -1e6)
    {
        Serial.println("Invalid calibration value read from EEPROM");
        return 0;
    }

    Serial.print("Calibration value read from EEPROM: ");
    Serial.println(calibrationValue);
    return calibrationValue;
}

void writeDistanceChangeThreshold(int threshold)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_DISTANCE_THRESHOLD, threshold);
    EEPROM.commit();
    EEPROM.end();
    Serial.print("Distance change threshold saved to EEPROM: ");
    Serial.println(threshold);
}

int readDistanceChangeThreshold()
{
    EEPROM.begin(EEPROM_SIZE);
    int threshold = 0;
    EEPROM.get(EEPROM_DISTANCE_THRESHOLD, threshold);
    EEPROM.end();

    Serial.print("Distance change threshold read from EEPROM: ");
    Serial.println(threshold);
    return threshold;
}