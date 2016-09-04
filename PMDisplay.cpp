#include "PMDisplay.h"

/////////---Display Module---//////////
#define PIN_DISP_SEG_A    11
#define PIN_DISP_SEG_B    12
#define PIN_DISP_SEG_C    2
#define PIN_DISP_SEG_D    3
#define PIN_DISP_SEG_E    4
#define PIN_DISP_SEG_F    5
#define PIN_DISP_SEG_G    6
#define PIN_DISP_SEG_DP   7
#define PIN_DISP_SPECIAL  8
#define PIN_DISP_MUX_A    10
#define PIN_DISP_MUX_B    A3
#define PIN_DISP_MUX_C    A2
#define PIN_DISP_MUX_EN   9

#define SEGMENT_BITS(a,b,c,d,e,f,g,dp)  ( (a<<0) | (b<<1) | (c<<2) | (d<<3) | (e<<4) | (f<<5) | (g<<6) | (dp<<7) )
#define TABLE_LENGTH(table)             (sizeof(table)/sizeof(table[0]))
#define CHAR_SPECIAL_START              ((uint8_t)0x80)
#define CHAR_SPECIAL_SEG_START          ((uint8_t)0xf0)

typedef uint8_t segmentbits_t;
struct SegmentCharMap {
    uint8_t       ch;
    segmentbits_t segments;
};

static const SegmentCharMap segmentCharMap[] = {
    //ch                a  b  c  d  e  f  g  p
    { '0', SEGMENT_BITS(1, 1, 1, 1, 1, 1, 0, 0) }, //  aaaaa
    { '1', SEGMENT_BITS(0, 1, 1, 0, 0, 0, 0, 0) }, // f     b
    { '2', SEGMENT_BITS(1, 1, 0, 1, 1, 0, 1, 0) }, // f     b
    { '3', SEGMENT_BITS(1, 1, 1, 1, 0, 0, 1, 0) }, // f     b
    { '4', SEGMENT_BITS(0, 1, 1, 0, 0, 1, 1, 0) }, //  ggggg
    { '5', SEGMENT_BITS(1, 0, 1, 1, 0, 1, 1, 0) }, // e     c
    { '6', SEGMENT_BITS(1, 0, 1, 1, 1, 1, 1, 0) }, // e     c
    { '7', SEGMENT_BITS(1, 1, 1, 0, 0, 0, 0, 0) }, // e     c
    { '8', SEGMENT_BITS(1, 1, 1, 1, 1, 1, 1, 0) }, //  ddddd   p
    { '9', SEGMENT_BITS(1, 1, 1, 1, 0, 1, 1, 0) },
    { ' ', SEGMENT_BITS(0, 0, 0, 0, 0, 0, 0, 0) },
    { 'A', SEGMENT_BITS(1, 1, 1, 0, 1, 1, 1, 0) },
    { 'B', SEGMENT_BITS(0, 0, 1, 1, 1, 1, 1, 0) },
    { 'b', SEGMENT_BITS(0, 0, 1, 1, 1, 1, 1, 0) },
    { 'C', SEGMENT_BITS(1, 0, 0, 1, 1, 1, 0, 0) },
    { 'c', SEGMENT_BITS(0, 0, 0, 1, 1, 0, 1, 0) },
    { 'D', SEGMENT_BITS(0, 1, 1, 1, 1, 0, 1, 0) },
    { 'd', SEGMENT_BITS(0, 1, 1, 1, 1, 0, 1, 0) },
    { 'E', SEGMENT_BITS(1, 0, 0, 1, 1, 1, 1, 0) },
    { 'F', SEGMENT_BITS(1, 0, 0, 0, 1, 1, 1, 0) },
    { 'G', SEGMENT_BITS(1, 0, 1, 1, 1, 1, 0, 1) },
    { 'H', SEGMENT_BITS(0, 1, 1, 0, 1, 1, 1, 0) },
    { 'h', SEGMENT_BITS(0, 0, 1, 0, 1, 1, 1, 0) },
    { 'I', SEGMENT_BITS(0, 1, 1, 0, 0, 0, 0, 0) },
    { 'J', SEGMENT_BITS(0, 1, 1, 1, 1, 0, 0, 0) },
    { 'L', SEGMENT_BITS(0, 0, 0, 1, 1, 1, 0, 0) },
    { 'l', SEGMENT_BITS(0, 0, 0, 1, 1, 1, 0, 0) },
    { 'N', SEGMENT_BITS(1, 1, 1, 0, 1, 1, 0, 0) },
    { 'n', SEGMENT_BITS(0, 0, 1, 0, 1, 0, 1, 0) },
    { 'O', SEGMENT_BITS(1, 1, 1, 1, 1, 1, 0, 0) },
    { 'o', SEGMENT_BITS(0, 0, 1, 1, 1, 0, 1, 0) },
    { 'P', SEGMENT_BITS(1, 1, 0, 0, 1, 1, 1, 0) },
    { 'R', SEGMENT_BITS(0, 0, 0, 0, 1, 0, 1, 0) },
    { 'r', SEGMENT_BITS(0, 0, 0, 0, 1, 0, 1, 0) },
    { 'S', SEGMENT_BITS(1, 0, 1, 1, 0, 1, 1, 0) },
    { 'T', SEGMENT_BITS(0, 0, 0, 1, 1, 1, 1, 0) },
    { 't', SEGMENT_BITS(0, 0, 0, 1, 1, 1, 1, 0) },
    { 'U', SEGMENT_BITS(0, 1, 1, 1, 1, 1, 0, 0) },
    { 'u', SEGMENT_BITS(0, 0, 1, 1, 1, 0, 0, 0) },
    { 'V', SEGMENT_BITS(0, 1, 1, 1, 1, 1, 0, 1) },
    { 'v', SEGMENT_BITS(0, 0, 1, 1, 1, 0, 0, 1) },
    { 'Y', SEGMENT_BITS(0, 1, 1, 1, 0, 1, 1, 0) },
    { '*', SEGMENT_BITS(0, 1, 1, 0, 1, 1, 0, 0) },
    { '-', SEGMENT_BITS(0, 0, 0, 0, 0, 0, 1, 0) },
    { '_', SEGMENT_BITS(0, 0, 0, 1, 0, 0, 0, 0) },
    { CHAR_SPECIAL_START     + 0, SEGMENT_BITS(1, 0, 0, 0, 0, 0, 0, 0) }, // 0x80  seg a
    { CHAR_SPECIAL_START     + 1, SEGMENT_BITS(0, 1, 0, 0, 0, 0, 0, 0) }, // 0x81  seg b
    { CHAR_SPECIAL_START     + 2, SEGMENT_BITS(0, 0, 1, 0, 0, 0, 0, 0) }, // 0x82  seg c
    { CHAR_SPECIAL_START     + 3, SEGMENT_BITS(0, 0, 0, 1, 0, 0, 0, 0) }, // 0x83  seg d
    { CHAR_SPECIAL_START     + 4, SEGMENT_BITS(0, 0, 0, 0, 1, 0, 0, 0) }, // 0x84  seg e
    { CHAR_SPECIAL_START     + 5, SEGMENT_BITS(0, 0, 0, 0, 0, 1, 0, 0) }, // 0x85  seg f
    { CHAR_SPECIAL_START     + 6, SEGMENT_BITS(0, 0, 0, 0, 0, 0, 1, 0) }, // 0x86  seg g
    { CHAR_SPECIAL_START     + 7, SEGMENT_BITS(0, 0, 0, 0, 0, 0, 0, 1) }, // 0x87  dot
    { CHAR_SPECIAL_SEG_START + 0, SEGMENT_BITS(1, 1, 0, 0, 0, 0, 0, 0) }, // 0xf0  :
    { CHAR_SPECIAL_SEG_START + 1, SEGMENT_BITS(0, 0, 1, 0, 0, 0, 0, 0) }, // 0xf1  ^
    { CHAR_SPECIAL_SEG_START + 2, SEGMENT_BITS(1, 1, 1, 0, 0, 0, 0, 0) }, // 0xf2  : and ^
};
static const uint8_t segmentPins[] = { PIN_DISP_SEG_A, PIN_DISP_SEG_B, PIN_DISP_SEG_C, PIN_DISP_SEG_D, PIN_DISP_SEG_E, PIN_DISP_SEG_F, PIN_DISP_SEG_G, PIN_DISP_SEG_DP };

PMDisplay::PMDisplay() : m_stringsLocked(false), m_displayBufferIndex(0), m_displayDigit(0)
{

}

void PMDisplay::init()
{
    /////////---Display Module---//////////
    pinMode(PIN_DISP_SEG_A, OUTPUT);
    pinMode(PIN_DISP_SEG_B, OUTPUT);
    pinMode(PIN_DISP_SEG_C, OUTPUT);
    pinMode(PIN_DISP_SEG_D, OUTPUT);
    pinMode(PIN_DISP_SEG_E, OUTPUT);
    pinMode(PIN_DISP_SEG_F, OUTPUT);
    pinMode(PIN_DISP_SEG_G, OUTPUT);
    pinMode(PIN_DISP_SEG_DP, OUTPUT);
    pinMode(PIN_DISP_SPECIAL, OUTPUT);
    pinMode(PIN_DISP_MUX_A, OUTPUT);
    pinMode(PIN_DISP_MUX_B, OUTPUT);
    pinMode(PIN_DISP_MUX_C, OUTPUT);
    pinMode(PIN_DISP_MUX_EN, OUTPUT);
}

/*
  The following function is the main printing function.
  Upper7Seg and  Lower7Seg are two global variables of type String.
  As the name implies the first on is for writing to the big (upper) screen and the other to the small one.
Note:
  - If you want special characters to appear (':' and/or the upper dot) place it at the 5th character of the Lower7Seg buffer
Examples:
  - to upper display "3.1"                : Upper7Seg = "3.1";
  - to upper display "3.148"              : Upper7Seg = "3.148";
  - to lower display "18:15"              : Lower7Seg = "1815" "\xf0";
  - to lower display "A1:5C" + upper dot  : Lower7Seg = "A15C" "\xf2";
  - to upper display only segments a      : Upper7Seg = "\x80" "\x80" "\x80" "\x80";
*/
void PMDisplay::update()
{
    if (m_displayBufferIndex == 0)
    {
        if (!m_stringsLocked)
        {
            m_displayBufferIndex = 0;
#if 1
            appendToDisplayBuffer(m_upper7Seg.c_str(), UPPER_DISPLAY_NUM_DIGITS);
            appendToDisplayBuffer(m_lower7Seg.c_str(), LOWER_DISPLAY_NUM_DIGITS);
#else
            // for testing
            appendToDisplayBufferndToDisplayBuffer("1.2.", UPPER_DISPLAY_NUM_DIGITS);
            appendToDisplayBuffer("A15C" "\x82", LOWER_DISPLAY_NUM_DIGITS);
#endif
        }
        m_displayBufferIndex = 0;
    }

    char ch = m_displayBuffer[m_displayBufferIndex++];
    bool showDecimalPoint = false;
    if (ch)
    {
        if (m_displayBuffer[m_displayBufferIndex] == '.')
        {
            showDecimalPoint = true;
            m_displayBufferIndex++;
        }
    }

    displayCharacter(m_displayDigit, (uint8_t)ch, showDecimalPoint);
    m_displayDigit++;
    if (m_displayDigit >= (UPPER_DISPLAY_NUM_DIGITS + LOWER_DISPLAY_NUM_DIGITS))
    {
        m_displayDigit = 0;
        m_displayBufferIndex = 0;
    }
}

uint8_t PMDisplay::getNumberOfMappedCharacters()
{
    return TABLE_LENGTH(segmentCharMap);
}

char PMDisplay::getMappedCharacterAtIndex(uint8_t idx)
{
    char ch = 0;
    if (idx < TABLE_LENGTH(segmentCharMap))
    {
        ch = segmentCharMap[idx].ch;
    }
    return ch;
}

void PMDisplay::writeUpperDisplayString(const String &str)
{
    m_stringsLocked = true;
    m_upper7Seg = str;
    m_stringsLocked = false;
}

void PMDisplay::writeUpperDisplayString(const char *str)
{
    m_stringsLocked = true;
    m_upper7Seg = str;
    m_stringsLocked = false;
}

void PMDisplay::writeLowerDisplayString(const String &str)
{
    m_stringsLocked = true;
    m_lower7Seg = str;
    m_stringsLocked = false;
}

void PMDisplay::writeLowerDisplayString(const char *str)
{
    m_stringsLocked = true;
    m_lower7Seg = str;
    m_stringsLocked = false;
}

void PMDisplay::displayCharacter(uint8_t digitNr, uint8_t ch, bool enableDecimalPoint)
{
    segmentbits_t segmentBits = 0;

    digitalWrite(PIN_DISP_MUX_EN, HIGH);
    digitalWrite(PIN_DISP_SPECIAL, LOW);

    for (uint8_t i = 0; i < TABLE_LENGTH(segmentCharMap); i++)
    {
        if (segmentCharMap[i].ch == ch)
        {
            segmentBits = segmentCharMap[i].segments;
            break;
        }
    }

    if (enableDecimalPoint)
    {
        segmentBits |= SEGMENT_BITS(0, 0, 0, 0, 0, 0, 0, 1);
    }

    for (uint8_t segNr = 0; segNr < TABLE_LENGTH(segmentPins); segNr++)
    {
        digitalWrite(segmentPins[segNr], segmentBits & 1);
        segmentBits = segmentBits >> 1;
    }

    if (ch >= CHAR_SPECIAL_SEG_START)
    {
        if (digitNr == 8)
        {
            digitalWrite(PIN_DISP_SPECIAL, HIGH);
        }
    }
    else
    {
        if (digitNr < 8)
        {
            digitalWrite(PIN_DISP_MUX_C, (digitNr >> 2) & 1);
            digitalWrite(PIN_DISP_MUX_B, (digitNr >> 1) & 1);
            digitalWrite(PIN_DISP_MUX_A, (digitNr >> 0) & 1);
            digitalWrite(PIN_DISP_MUX_EN, LOW);
        }
    }
}

void PMDisplay::appendToDisplayBuffer(const char *str, uint8_t numDigits)
{
    uint8_t i = 0;
    uint8_t digits = 0;

    while (digits < numDigits)
    {
        char ch = str[i];
        i++;
        if (!ch)
        {
            break;
        }

        m_displayBuffer[m_displayBufferIndex++] = ch;
        if (str[i] == '.')
        {
            m_displayBuffer[m_displayBufferIndex++] = '.';
            while (str[i] == '.')
            {
                i++;
            }
        }
        digits++;
    }

    while (digits < numDigits)
    {
        m_displayBuffer[m_displayBufferIndex++] = ' ';
        digits++;
    }
}


