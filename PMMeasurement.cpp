#include "PMMeasurement.h"
#include "PMDisplay.h"
#include <Wire.h>

extern PMDisplay display;

////////---ADC CurrSense---////////
#define ADC_ADDRESS 0x48  // hardware address of the MCP3221AOT-E/OT used in the PM_V1 board.
#define PIN_C1      A0
#define PIN_C2      A1

/////////---I2C---//////////////////
#define PIN_SDA     A4
#define PIN_SCL     A5

PMMeasurement::PMMeasurement() : m_mode(MM_Voltage), m_measurementValue(0.0f), m_averagingMeasValueSum(0.0f), m_averagingCounter(0), m_initialDelayCounter(0)
{
    memset(&m_calibrationData, 0, sizeof(m_calibrationData));
}

void PMMeasurement::init()
{
    ////////////---I2C---/////////////////////
    pinMode(PIN_SDA, INPUT);           // set pin to input
    digitalWrite(SDA, HIGH);       // enable 20k internal pullup resistor
    pinMode(PIN_SDA, INPUT);           // set pin to input
    digitalWrite(SDA, HIGH);       // enable 20k internal pullup resistor

    ////////////---Multiplexer control signals ---/////////////////
    pinMode(PIN_C1, OUTPUT);  //C1, C2 control whether the ADC's input is VIN or the current sensor's one.
    pinMode(PIN_C2, OUTPUT);

    Wire.begin(); //starts I2C communication used to aquire voltage from A/D converter

    setMode(m_mode);
}

void PMMeasurement::setCalibrationData(const PMMeasurement::CalibrationData &data)
{
    m_calibrationData = data;
    restart();
}

void PMMeasurement::setMode(ENMeasurementMode mode)
{
    m_mode = mode;
    switch (m_mode)
    {
        case MM_Current:
            digitalWrite(PIN_C1, LOW);//C1: LOW-->current sense, HIGH --> ADC input
            digitalWrite(PIN_C2, HIGH);  //C2:  the reverse from above
            break;
        case MM_Voltage:
            digitalWrite(PIN_C1, HIGH);//C1: LOW-->current sense, HIGH --> ADC input
            digitalWrite(PIN_C2, LOW);  //C2:  the reverse from above
            break;
    }
    restart();
}

void PMMeasurement::setNextMode()
{
    setMode((ENMeasurementMode) ((m_mode + 1) % ((int)MM__Count)));
}

bool PMMeasurement::update()
{
    bool measurementValueWasUpdated = false;

    uint16_t timeoutCounter = 0x500;

    Wire.requestFrom(ADC_ADDRESS, 2); // requests 2 bytes
    while (Wire.available() < 2) // while two bytes to receive
    {
        timeoutCounter--;
        if (0 == timeoutCounter)
        {
            break;
        }
    }

    if (m_initialDelayCounter)
    {
        m_initialDelayCounter--; // wait until hardware has applied the MUX changes
    }
    else if (timeoutCounter)
    {
        float singleMeasValue = 0.0f;

        uint16_t adcRaw;
        adcRaw  = Wire.read() << 8;
        adcRaw |= Wire.read();
        m_averagingMeasValueSum += ((float)adcRaw);

        if (m_averagingCounter) {
            m_averagingCounter--;
        }

        if (!m_averagingCounter)
        {
            m_averagingCounter = m_calibrationData.averaging;
            m_measurementValue = m_averagingMeasValueSum;
            m_averagingMeasValueSum = 0;
            if (m_calibrationData.averaging) {
                m_measurementValue /= ((float)m_calibrationData.averaging);
            }

            bool limitToZero = false;

            switch (m_mode)
            {
                case MM_Current:
                    m_measurementValue = (m_measurementValue - m_calibrationData.current.offset) * m_calibrationData.current.slope;
                    limitToZero = (1 == m_calibrationData.current.noNegative);
                    break;
                case MM_Voltage:
                    m_measurementValue = (m_measurementValue - m_calibrationData.voltage.offset) * m_calibrationData.voltage.slope;
                    limitToZero = (1 == m_calibrationData.voltage.noNegative);
                    break;
                default:
                    singleMeasValue = 0.0f;
                    break;
            }

            if (limitToZero && (m_measurementValue < 0.0f)) {
                m_measurementValue = 0.0f;
            }
            measurementValueWasUpdated = true;
        }
    }

    return measurementValueWasUpdated;
}

void PMMeasurement::restart()
{
    m_averagingMeasValueSum = 0.0f;
    m_measurementValue = 0.0f;
    m_averagingCounter = m_calibrationData.averaging;
    m_initialDelayCounter = 10;
}

