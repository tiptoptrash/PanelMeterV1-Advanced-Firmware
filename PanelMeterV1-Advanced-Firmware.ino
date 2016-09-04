/*
  PanelMeter V1 Advanced Firmware
  -------------------------------

  This firmware is intended for Panel Meter V1 which was developed by Sotirios Zorbas.
  For more information on this project and where to buy the devices, visit the following link:
  https://www.tindie.com/products/sotos/panel-meter-v1/

  -------------------------------

  This software is a full rewrite of the original firmware and
  tries to provide
  - easy calibration
  - easy customization
  - data logging support through the serial port (e.g. for connecting to a Raspberry Pi)
  - modular programming structure
  - a big number of displayable characters and symbols for the 7-segment displays
  - enhanced data processing and filtering
  
  -------------------------------

  Copyright (c) 2016 Stefan Robl <stefan@qdev.de>

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  associated documentation files (the "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial
  portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
  LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  -------------------------------

  Change log:
  - V1.0.08 -- 2016-09-04
    Initial release.
  
*/
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "TimerOne.h"
#include "PMDisplay.h"
#include "PMButton.h"
#include "PMMeasurement.h"
#include "DemoScroller.h"
#include "DemoCharacterSet.h"
#include "Config.h"

#define VERSION_STRING            "1.0.08"
#define CONFIGDATA_STRUCTURE_VER  0x02

///////---Buttons---///////////////
#define PIN_BT1   A7
#define PIN_BT2   A6

///////---LED---//////////////////
#define PIN_LED   13

//#define SHOWS_DEMO_SCROLLER1
#define SHOWS_DEMO_SCROLLER2
//#define SHOWS_DEMO_CHARSET
//#define SHOWS_CONFIG_VALUES
#define AUTO_MEAS_CHANGE_DELAY_VALUE 10

typedef enum _ENDisplayMode
{
    DM_Measurement = 0,
#ifdef SHOWS_DEMO_CHARSET
    DM_CharacterDemonstration,
#endif
#ifdef SHOWS_DEMO_SCROLLER1
    DM_Scroller1,
#endif
#ifdef SHOWS_DEMO_SCROLLER2
    DM_Scroller2,
#endif
#ifdef SHOWS_CONFIG_VALUES
    DM_ConfigValues,
#endif
    DM_Version,
    DM__Count
} ENDisplayMode;

typedef enum _ENDisplayMeasurementMode
{
    DMM_Auto = 0,
    DMM_Voltage,
    DMM_Current,
    DMM_Power,
    DMM__Count
} ENDisplayMeasurementMode;

struct STMeasValue {
    String string;
    float  value;
    bool   valid;
};



PMDisplay display;
static PMMeasurement    measurement;
static PMButton         buttonUpper(PIN_BT2);
static PMButton         buttonLower(PIN_BT1);
#ifdef SHOWS_DEMO_SCROLLER1
static DemoScroller     demoScroller1("THIS IS A TEST FOR THE DISPLAY 1234567890ABCDEF     ", 220);
#endif
#ifdef SHOWS_DEMO_SCROLLER2
static DemoScroller     demoScroller2("\x80" "\x81" "\x82" "\x83" "\x84" "\x85" "\x80" "\x81" "\x82", 75);
#endif
#ifdef SHOWS_DEMO_CHARSET
static DemoCharacterSet demoCharacterSet;
#endif
static ENDisplayMode            g_displayMode = DM_Measurement;
static ENDisplayMeasurementMode g_displayMeasurementMode = DMM_Auto;
static ENDisplayMeasurementMode g_displayedMeasurement   = DMM_Voltage;
static uint16_t                 g_updateMeasurementDisplayDelayCounter = 0;
static uint16_t                 g_displayAutoMeasurementDelayCounter = AUTO_MEAS_CHANGE_DELAY_VALUE;
static bool                     g_displayMeasurementAuto = false;
static STMeasValue              g_measVal_U;
static STMeasValue              g_measVal_I;
static STMeasValue              g_measVal_P;

void timerCallback(void);       // this function is called when the timer overflows
void handleButtons(void);       // this function checks the tact buttons
void setDisplayMode(ENDisplayMode mode);
void updateMeasurementDisplayMode();
bool serialReadLine(String &str);
void performReset();
void formatMeasValue(STMeasValue &v);
void initMeasValue(STMeasValue &v);

void initMeasValue(STMeasValue &v)
{
    v.string = " . . . .";
    v.value  = 0.0f;
    v.valid  = false;
}

void performReset()
{
    cli();
    wdt_enable(WDTO_15MS); // activate watchdog
    while (1) {
        // cause watchdog to reset the system...
    }
}



void applyStoredConfig()
{
    EEPROMConfigData configData;
    readEEPROM(configData);
    measurement.setCalibrationData(configData.calibration);
    serialCalibrationMenu(false); // display the calibration values stored in EEPROM
}

void serialMenu()
{
    if (Serial.available() > 0) {
        char ch = (char)Serial.read();
        switch (ch)
        {
            case 'r':
                serialCalibrationMenu(false);
                break;
            case 'w':
                if (serialCalibrationMenu(true)) {
                    applyStoredConfig();
                }
                break;
            case 'x':
                performReset();
                break;
        }
    }
}

void updateMeasurementDisplayMode()
{
    switch (g_displayMeasurementMode)
    {
        case DMM_Auto:
            if (!g_displayAutoMeasurementDelayCounter)
            {
                g_displayAutoMeasurementDelayCounter = AUTO_MEAS_CHANGE_DELAY_VALUE;
                switch (g_displayedMeasurement)
                {
                    case DMM_Voltage:
                        g_displayedMeasurement = DMM_Current;
                        break;
                    case DMM_Current:
                        g_displayedMeasurement = DMM_Power;
                        break;
                    case DMM_Power:
                        g_displayedMeasurement = DMM_Voltage;
                        break;
                }
            }
            g_displayAutoMeasurementDelayCounter--;
            break;
        case DMM_Voltage:
        case DMM_Current:
        case DMM_Power:
            g_displayedMeasurement = g_displayMeasurementMode;
            break;
    }

    if (DMM_Auto == g_displayMeasurementMode) {
        digitalWrite(PIN_LED, HIGH);
    }
    else {
        digitalWrite(PIN_LED, LOW);
    }

    switch (g_displayedMeasurement)
    {
        case DMM_Voltage:
            display.writeUpperDisplayString(g_measVal_U.string);
            display.writeLowerDisplayString("VoLt");
            break;
        case DMM_Current:
            display.writeUpperDisplayString(g_measVal_I.string);
            display.writeLowerDisplayString(" A*P.");
            break;
        case DMM_Power:
            display.writeUpperDisplayString(g_measVal_P.string);
            display.writeLowerDisplayString("*ATT");
            break;
    }
}

void setDisplayMode(ENDisplayMode mode)
{
    g_displayMode = mode;

    switch (g_displayMode)
    {
        case DM_Measurement:
            updateMeasurementDisplayMode();
            break;
#ifdef SHOWS_DEMO_CHARSET
        case DM_CharacterDemonstration:
            demoCharacterSet.update();
            break;
#endif
#ifdef SHOWS_CONFIG_VALUES
        case DM_ConfigValues:
            displayConfigDataItem();
            break;
#endif
        case DM_Version:
            display.writeUpperDisplayString("VERS");
            display.writeLowerDisplayString(VERSION_STRING);
        default:
            break;
    }
}

void formatMeasValue(STMeasValue &v)
{
    float measVal = v.value;
    if (v.valid)
    {
        if (measVal >= 0.0f)
        {
            if (measVal < 1000.0f) {
                v.string = String(measVal, 3); // 3 decimal places
            }
            else if (measVal < 10000.0f) {
                v.string = String((uint16_t)measVal); // no decimal places
            }
            else {
                v.string = "OVER"; // value is too large
            }
        }
        else
        {
            if (measVal > -100.0f) {
                v.string = String(measVal, 2); // -x.yy: 2 decimal places
            }
            else if (measVal > -1000.0f) {
                v.string = "-" + String((uint16_t)(-measVal)); // -xxx: no decimal places
            }
            else {
                v.string = "OVER"; // value is too large
            }
        }
        while (v.string.length() < 4) {
            v.string = " " + v.string;
        }
    }
    else
    {
        v.string = " . . . .";
    }
}

void setup()
{
    MCUSR = 0; // clear any flags of prior resets
    wdt_enable(WDTO_8S); // activate watchdog

    display.init();
#ifdef SHOWS_DEMO_SCROLLER1
    demoScroller1.init();
#endif
#ifdef SHOWS_DEMO_SCROLLER2
    demoScroller2.init();
#endif
#ifdef SHOWS_DEMO_CHARSET
    demoCharacterSet.init();
#endif
    buttonUpper.init();
    buttonLower.init();
    measurement.init();

    Serial.begin(115200);

    Serial.print("\r\n\r\nPanelMeter V1\r\nFirmware: V");
    Serial.print(VERSION_STRING);
    Serial.print("\r\n\r\n");

    Timer1.initialize(500); // timer overflow value (= callback period in µs)
    Timer1.attachInterrupt(timerCallback);

    initMeasValue(g_measVal_U);
    initMeasValue(g_measVal_I);
    initMeasValue(g_measVal_P);

    applyStoredConfig();

    setDisplayMode(g_displayMode); // initial refresh of the display
}

void loop()
{
    buttonUpper.check();
    buttonLower.check();

    if (buttonLower.getChanged() && buttonLower.getPressed())
    {
        switch (g_displayMode)
        {
            case DM_Measurement:
                g_displayMeasurementMode = (ENDisplayMeasurementMode) ((g_displayMeasurementMode + 1) % ((int)DMM__Count));
                updateMeasurementDisplayMode();
                break;
#ifdef SHOWS_DEMO_CHARSET
            case DM_CharacterDemonstration:
                demoCharacterSet.next();
                break;
#endif
#ifdef SHOWS_CONFIG_VALUES
            case DM_ConfigValues:
                displayNextConfigDataItem();
                break;
#endif
#ifdef SHOWS_DEMO_SCROLLER1
            case DM_Scroller1:
                demoScroller1.changeSpeed();
                break;
#endif
#ifdef SHOWS_DEMO_SCROLLER2
            case DM_Scroller2:
                demoScroller2.changeSpeed();
                break;
#endif
            default:
                break;
        }
    }

    if (buttonUpper.getChanged() && buttonUpper.getPressed())
    {
        setDisplayMode((ENDisplayMode) ((g_displayMode + 1) % ((int)DM__Count)));
    }

    if ((buttonLower.getChanged() || buttonUpper.getChanged()) && buttonLower.getPressed() && buttonUpper.getPressed())
    {
        setDisplayMode(g_displayMode);
    }


    if (measurement.update())
    {
        switch (measurement.getMode())
        {
            case PMMeasurement::MM_Current:
                g_measVal_I.value = measurement.getValue();
                g_measVal_I.valid = true;
                formatMeasValue(g_measVal_I);
                break;
            case PMMeasurement::MM_Voltage:
                g_measVal_U.value = measurement.getValue();
                g_measVal_U.valid = true;
                formatMeasValue(g_measVal_U);
                break;
        }

        if (g_measVal_U.valid && g_measVal_I.valid) {
            g_measVal_P.value = g_measVal_U.value * g_measVal_I.value;
            g_measVal_P.valid = true;
            formatMeasValue(g_measVal_P);

            String outStr = "!U=";
            outStr += String(g_measVal_U.value, 6);
            outStr += ";I=";
            outStr += String(g_measVal_I.value, 6);
            outStr += "#";
            Serial.flush();
            Serial.println(outStr);
            Serial.flush();
        }

        measurement.setNextMode();
    }

    if (DM_Measurement == g_displayMode) {
        if (!g_updateMeasurementDisplayDelayCounter) {
            updateMeasurementDisplayMode();
            g_updateMeasurementDisplayDelayCounter = 400;
        }
        g_updateMeasurementDisplayDelayCounter--;
    }

    serialMenu();

    wdt_reset(); // trigger WD
}

void timerCallback() // this is the function called when the timer overflows
{
    switch (g_displayMode)
    {
#ifdef SHOWS_DEMO_SCROLLER1
        case DM_Scroller1:
            demoScroller1.update();
            break;
#endif
#ifdef SHOWS_DEMO_SCROLLER2
        case DM_Scroller2:
            demoScroller2.update();
            break;
#endif
        default:
            break;
    }

    display.update();
}

