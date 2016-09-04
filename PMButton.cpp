#include "PMButton.h"

#define PMBUTTON_DELAY_VALUE 100

PMButton::PMButton(int pinNr) : m_pinNr(pinNr), m_counter(0), m_changed(false), m_pressed(false)
{
}

void PMButton::init()
{
    pinMode(m_pinNr, INPUT);
}

void PMButton::check()
{
    m_changed = false;

    if (0 == m_counter)
    {
        m_counter = PMBUTTON_DELAY_VALUE;

        bool buttonPressed;
        buttonPressed = (analogRead(m_pinNr) < 200);
        if (m_pressed != buttonPressed)
        {
            m_pressed = buttonPressed;
            m_changed = true;
        }
    }
    m_counter -= 1;
}

