#ifndef DEMOCHARACTERSET_H__
#define DEMOCHARACTERSET_H__

#include <Arduino.h>

class DemoCharacterSet
{
    public:
        DemoCharacterSet();

        void init();
        void update();
        void next();

    private:
        uint8_t m_charIdx;
};

#endif // DEMOCHARACTERSET_H__

