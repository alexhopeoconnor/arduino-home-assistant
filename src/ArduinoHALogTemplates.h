#ifndef ARDUINOHA_LOG_TEMPLATES_H
#define ARDUINOHA_LOG_TEMPLATES_H

#include "ArduinoHALog.h"
#include <IPAddress.h>

inline String ahaLogStringifyArg(const __FlashStringHelper* v) {
    return String(v);
}

inline String ahaLogStringifyArg(const IPAddress& v) {
    return v.toString();
}

template<typename T>
inline String ahaLogStringifyArg(const T& v) {
    return String(v);
}

template<typename T>
inline void arduinoHALogf(
    ArduinoHALogLevel level,
    const char* subsystem,
    T&& text
) {
    arduinoHALog(level, subsystem, ahaLogStringifyArg(text));
}

template<typename T, typename U>
inline void arduinoHALogf(
    ArduinoHALogLevel level,
    const char* subsystem,
    T&& a,
    U&& b
) {
    arduinoHALog(
        level,
        subsystem,
        ahaLogStringifyArg(a) + ahaLogStringifyArg(b)
    );
}

template<typename T, typename U, typename V>
inline void arduinoHALogf(
    ArduinoHALogLevel level,
    const char* subsystem,
    T&& a,
    U&& b,
    V&& c
) {
    arduinoHALog(
        level,
        subsystem,
        ahaLogStringifyArg(a) + ahaLogStringifyArg(b) + ahaLogStringifyArg(c)
    );
}

#endif
