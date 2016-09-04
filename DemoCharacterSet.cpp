#include "DemoCharacterSet.h"
#include "PMDisplay.h"

extern PMDisplay display;

DemoCharacterSet::DemoCharacterSet() : m_charIdx(0)
{
}

void DemoCharacterSet::init()
{
}

void DemoCharacterSet::update()
{
    char buf[6];
    memset(buf, display.getMappedCharacterAtIndex(m_charIdx), 5);
    buf[5] = 0;
    display.writeUpperDisplayString(buf);
    display.writeLowerDisplayString(buf);
}

void DemoCharacterSet::next()
{
    m_charIdx += 1;
    m_charIdx = m_charIdx % display.getNumberOfMappedCharacters();
    update();
}

