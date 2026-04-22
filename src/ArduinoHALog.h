#ifndef ARDUINOHA_LOG_H
#define ARDUINOHA_LOG_H

#include <Arduino.h>

/**
 * Optional sink for ArduinoHA debug output (ARDUINOHA_DEBUG).
 * When set, ARDUINOHA_DEBUG_* macros route here; otherwise Serial is used.
 */
class ArduinoHALogSink {
public:
    virtual void print(const String& s) = 0;
    virtual void println(const String& s) = 0;
    virtual ~ArduinoHALogSink() = default;
};

void arduinoHASetLogSink(ArduinoHALogSink* sink);
ArduinoHALogSink* arduinoHAGetLogSink();

#endif
