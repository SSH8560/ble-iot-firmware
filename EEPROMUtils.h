#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <EEPROM.h>

#define EEPROM_SIZE 512
#define EEPROM_SSID_START 0
#define EEPROM_PASS_START 100
#define EEPROM_CALIBRATION 200

void saveWiFiCredentialsToEEPROM(const char *ssid, const char *password);
void readWiFiCredentialsFromEEPROM(char *ssid, char *password);
void saveCalibrationToEEPROM(float calibrationValue);
float readCalibrationFromEEPROM();

#endif