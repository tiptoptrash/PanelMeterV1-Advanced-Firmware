#include "DemoScroller.h"
#include "PMDisplay.h"

extern PMDisplay display;

DemoScroller::DemoScroller(const char *text, uint16_t scrollerSpeed) : m_text(text), m_scrollerPos(0), m_scrollerDelay(0), m_scrollerSpeed(scrollerSpeed)
{
}

void DemoScroller::init()
{
}

void DemoScroller::update()
{
    if (!m_scrollerDelay)
    {
        m_scrollerDelay = m_scrollerSpeed;

        char buf[5];
        memcpy(buf, &m_text[m_scrollerPos], 4);
        m_scrollerPos += 1;
        if (m_scrollerPos >= (strlen(m_text) - 3)) {
            m_scrollerPos = 0;
        }
        buf[4] = 0;
        display.writeUpperDisplayString(buf);
        display.writeLowerDisplayString(buf);
    }
    m_scrollerDelay -= 1;
}

void DemoScroller::changeSpeed()
{
    m_scrollerSpeed = (m_scrollerSpeed * 14) / 10;
    if (m_scrollerSpeed > 1000) {
        m_scrollerSpeed = 40;
    }
}
