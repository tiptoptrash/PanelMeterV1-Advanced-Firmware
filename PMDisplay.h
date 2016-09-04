#ifndef PMDISPLAY_H__
#define PMDISPLAY_H__

#include <Arduino.h>

#define UPPER_DISPLAY_NUM_DIGITS        4
#define LOWER_DISPLAY_NUM_DIGITS        5

class PMDisplay
{
    public:
        PMDisplay();

        void init();
        void update();
        uint8_t getNumberOfMappedCharacters();
        char getMappedCharacterAtIndex(uint8_t idx);

        void writeUpperDisplayString(const String &str);
        void writeUpperDisplayString(const char *str);

        void writeLowerDisplayString(const String &str);
        void writeLowerDisplayString(const char *str);

    private:
        void appendToDisplayBuffer(const char *str, uint8_t numDigits);
        void displayCharacter(uint8_t digitNr, uint8_t ch, bool enableDecimalPoint);

    private:
        bool    m_stringsLocked;
        String  m_upper7Seg; // the string written to the upper (large) display
        String  m_lower7Seg; // the string written to the lower (small) display
        char    m_displayBuffer[(UPPER_DISPLAY_NUM_DIGITS + LOWER_DISPLAY_NUM_DIGITS) * 2];
        uint8_t m_displayBufferIndex;
        uint8_t m_displayDigit;
};

#endif // PMDISPLAY_H__

