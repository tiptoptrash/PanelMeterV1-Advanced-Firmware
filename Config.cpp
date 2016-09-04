#include "Config.h"
#include "Console.h"
#include "PMDisplay.h"
#include "PMMeasurement.h"
#include <Wire.h>
#include <EEPROM.h>

extern PMDisplay display;
extern PMMeasurement measurement;

#define CONFIGDATA_STRUCTURE_VER  0x01

static const EEPROMConfigData defaultData = {
    CONFIGDATA_STRUCTURE_VER,
    0.0f, 1.0f, 0,
    0.0f, 1.0f, 0,
    2000,
};

struct EEPROMConfigItem {
    const char *displayName;
    const char *serialName;
    uint8_t     structOffset;
    uint8_t     valueType; // ENValueType
};

static const EEPROMConfigItem configItems[] = {
    { "VOFF", "V_offset ", offsetof(EEPROMConfigData, calibration.voltage.offset),     static_cast<uint8_t>(VT_float) },
    { "VSLO", "V_slope  ", offsetof(EEPROMConfigData, calibration.voltage.slope),      static_cast<uint8_t>(VT_float) },
    { "VnoN", "V_noNeg  ", offsetof(EEPROMConfigData, calibration.voltage.noNegative), static_cast<uint8_t>(VT_uint8) },
    { "IOFF", "I_offset ", offsetof(EEPROMConfigData, calibration.current.offset),     static_cast<uint8_t>(VT_float) },
    { "ISLO", "I_slope  ", offsetof(EEPROMConfigData, calibration.current.slope),      static_cast<uint8_t>(VT_float) },
    { "InoN", "I_noNeg  ", offsetof(EEPROMConfigData, calibration.current.noNegative), static_cast<uint8_t>(VT_uint8) },
    { "AVG",  "Averaging", offsetof(EEPROMConfigData, calibration.averaging),          static_cast<uint8_t>(VT_uint16) },
};

#define CONFIG_ITEM_COUNT (sizeof(configItems)/sizeof(configItems[0]))

static bool g_displayMeasurementAuto = false;
static int  g_displayedEEPROMDataNr = 0;

void readEEPROM(EEPROMConfigData &data)
{
    int eeAddress = 0;
    memset(&data, 0, sizeof(data));
    EEPROM.get(eeAddress, data);
    if (CONFIGDATA_STRUCTURE_VER != data.structureVersion) {
        data = defaultData;
    }
}

static bool calibrationItemInteraction(const char *itemName, void *valuePtr, ENValueType valueType, bool doWrite)
{
    bool ok = true;
    Serial.print(itemName);
    Serial.print(" = ");
    Serial.print(stringFromValue(valuePtr, valueType));
    Serial.println("");
    if (doWrite) {
        ok = false;
        Serial.print("Enter new value [<ENTER>: keep current, <ESC>: abort]: ");
        String str;
        ok = serialReadLine(str);
        if (str.length() > 0) {
            setValueFromString(valuePtr, valueType, str);
            Serial.print("--> ");
            Serial.print(itemName);
            Serial.print(" = ");
            Serial.print(stringFromValue(valuePtr, valueType));
            Serial.println("\r\n");
        }
        else {
            Serial.println("");
        }
    }
    return ok;
}

bool serialCalibrationMenu(bool doWrite)
{
    bool showAgain = false;
    EEPROMConfigData configData;
    readEEPROM(configData);

    bool ok = true;

    Serial.println("\r\n--- Active calibration data ---");

    for (uint8_t i = 0; i < CONFIG_ITEM_COUNT; i++)
    {
        const EEPROMConfigItem &item = configItems[i];
        if (!calibrationItemInteraction(item.serialName, &reinterpret_cast<uint8_t *>(&configData)[item.structOffset], static_cast<ENValueType>(item.valueType), doWrite))
        {
            ok = false;
            break;
        }
    }

    if (ok && doWrite) {
        Serial.print("Writing to EEPROM... ");
        int eeAddress = 0;
        EEPROM.put(eeAddress, configData);
        Serial.println("DONE!");
    }
    else if (doWrite) {
        Serial.println("*** ABORTED!\r\n");
    }
    else {
        Serial.println("");
    }

    return ok && doWrite;
}

void displayConfigDataItem()
{
    EEPROMConfigData configData;
    readEEPROM(configData);
    const EEPROMConfigItem &item = configItems[g_displayedEEPROMDataNr];
    String str = stringFromValue(&reinterpret_cast<uint8_t *>(&configData)[item.structOffset], static_cast<ENValueType>(item.valueType));
    while (str.length() < 4) {
        str = " " + str;
    }
    display.writeUpperDisplayString(str);
    display.writeLowerDisplayString(item.displayName);
}

void displayNextConfigDataItem()
{
    g_displayedEEPROMDataNr++;
    g_displayedEEPROMDataNr %= CONFIG_ITEM_COUNT;
    displayConfigDataItem();
}

