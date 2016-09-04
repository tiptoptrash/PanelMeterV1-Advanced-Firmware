#ifndef CONSOLE_H__
#define CONSOLE_H__

#include <Arduino.h>

typedef enum _ENValueType
{
    VT_int8 = 0,
    VT_uint8,
    VT_int16,
    VT_uint16,
    VT_int32,
    VT_uint32,
    VT_float
} ENValueType;

bool serialReadLine(String &str);
String stringFromValue(void *valuePtr, ENValueType valueType);
void setValueFromString(void *valuePtr, ENValueType valueType, const String &str);

#endif // CONSOLE_H__

