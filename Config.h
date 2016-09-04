#ifndef CONFIG_H__
#define CONFIG_H__

#include <Arduino.h>
#include "PMMeasurement.h"

struct EEPROMConfigData {
    uint8_t                        structureVersion; // 0xff = uninitialized
    PMMeasurement::CalibrationData calibration;
};

void readEEPROM(EEPROMConfigData &data);
bool serialCalibrationMenu(bool doWrite);
void displayConfigDataItem();
void displayNextConfigDataItem();

#endif // CONFIG_H__

