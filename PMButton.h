#ifndef PMBUTTON_H__
#define PMBUTTON_H__

#include <Arduino.h>

class PMButton
{
    public:
        PMButton(int pinNr);

        void init();

        inline bool getChanged() {
            return m_changed;
        }
        inline bool getPressed() {
            return m_pressed;
        }

        void check();

    private:
        int m_pinNr;
        uint8_t m_counter;
        bool m_changed;
        bool m_pressed;
};

#endif // PMBUTTON_H__
