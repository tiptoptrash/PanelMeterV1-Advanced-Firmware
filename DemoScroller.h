#ifndef DEMOSCROLLER_H__
#define DEMOSCROLLER_H__

#include <Arduino.h>

class DemoScroller
{
    public:
        DemoScroller(const char *text, uint16_t scrollerSpeed);

        void init();
        void update();
        void changeSpeed();

    private:
        const char *m_text;
        uint8_t m_scrollerPos = 0;
        uint16_t m_scrollerDelay = 0;
        uint16_t m_scrollerSpeed = 75;
};

#endif // DEMOSCROLLER_H__

