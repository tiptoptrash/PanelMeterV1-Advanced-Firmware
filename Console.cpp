#include "Console.h"
#include "PMDisplay.h"
#include <avr/wdt.h>

bool serialReadLine(String &str)
{
    bool ok = false;
    while (1) {
        wdt_reset(); // trigger WD
        if (Serial.available() > 0) {
            char ch = Serial.read();
            if ((0 == ch) || ('\n' == ch)) {
                // do nothing
            }
            else if ('\x1b' == ch) {
                ok = false;
                Serial.print("<ESC>\r\n");
                break;
            }
            else if ('\r' == ch) {
                ok = true;
                Serial.print("\r\n");
                break;
            }
            else {
                Serial.print(ch);
                str += ch;
            }
        }
    }
    return ok;
}

String stringFromValue(void *valuePtr, ENValueType valueType)
{
    String str;
    if (valuePtr)
    {
        switch (valueType)
        {
            case VT_int8:
                str = String(*reinterpret_cast<int8_t *>(valuePtr));
                break;
            case VT_uint8:
                str = String(*reinterpret_cast<uint8_t *>(valuePtr));
                break;
            case VT_int16:
                str = String(*reinterpret_cast<int16_t *>(valuePtr));
                break;
            case VT_uint16:
                str = String(*reinterpret_cast<uint16_t *>(valuePtr));
                break;
            case VT_int32:
                str = String(*reinterpret_cast<int32_t *>(valuePtr));
                break;
            case VT_uint32:
                str = String(*reinterpret_cast<uint32_t *>(valuePtr));
                break;
            case VT_float:
                str = String(*reinterpret_cast<float *>(valuePtr), 15);
                break;
        }
    }
    return str;
}

void setValueFromString(void *valuePtr, ENValueType valueType, const String &str)
{
    if (valuePtr)
    {
        switch (valueType)
        {
            case VT_int8:
                *reinterpret_cast<int8_t *>(valuePtr) = static_cast<int8_t>(str.toInt());
                break;
            case VT_uint8:
                *reinterpret_cast<uint8_t *>(valuePtr) = static_cast<uint8_t>(str.toInt());
                break;
            case VT_int16:
                *reinterpret_cast<int16_t *>(valuePtr) = static_cast<int16_t>(str.toInt());
                break;
            case VT_uint16:
                *reinterpret_cast<uint16_t *>(valuePtr) = static_cast<uint16_t>(str.toInt());
                break;
            case VT_int32:
                *reinterpret_cast<int32_t *>(valuePtr) = static_cast<int32_t>(str.toInt());
                break;
            case VT_uint32:
                *reinterpret_cast<uint32_t *>(valuePtr) = static_cast<uint32_t>(str.toInt());
                break;
            case VT_float:
                *reinterpret_cast<float *>(valuePtr) = static_cast<float>(str.toFloat());
                break;
        }
    }
}

