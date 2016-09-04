#ifndef PMMEASUREMENT_H__
#define PMMEASUREMENT_H__

#include <Arduino.h>

class PMMeasurement
{
    public:
        typedef enum _ENMeasurementMode
        {
            MM_Voltage = 0,
            MM_Current,
            MM__Count
        } ENMeasurementMode;

        struct ModeCalibrationData {
            float  offset;
            float  slope;
            int8_t noNegative;
        };

        struct CalibrationData {
            ModeCalibrationData voltage;
            ModeCalibrationData current;
            uint16_t averaging;
        };

    public:
        PMMeasurement();

        void setCalibrationData(const CalibrationData &data);
        inline const CalibrationData &getCalibrationData() {
            return m_calibrationData;
        }

        void setMode(ENMeasurementMode mode);
        void setNextMode();
        inline ENMeasurementMode getMode() {
            return m_mode;
        }

        inline float getValue() {
            return m_measurementValue;
        }

        void init();
        bool update();

    private:
        void restart();

    private:
        CalibrationData m_calibrationData;
        ENMeasurementMode m_mode;
        float m_measurementValue;
        float m_averagingMeasValueSum;
        uint16_t m_averagingCounter;
        uint8_t m_initialDelayCounter;
};

#endif // PMMEASUREMENT_H__
