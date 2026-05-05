#ifndef ARDUINOHA_LOG_H
#define ARDUINOHA_LOG_H

#include <Arduino.h>

/**
 * Log severity for ArduinoHA. Numeric order is used for filtering: only messages with
 * level <= the configured maximum (see arduinoHASetLogLevel) are emitted.
 */
enum class ArduinoHALogLevel : uint8_t {
    Error = 1,
    Warn,
    Info,
    Debug,
    Trace,
};

struct ArduinoHALogMessage {
    ArduinoHALogLevel level;
    const char* subsystem;
    String text;
};

/**
 * Structured log sink (mirrors WiFiManager-style log callbacks).
 * Install with arduinoHASetLogSink; when null, arduinoHALog writes to Serial if enabled.
 */
class ArduinoHALogSink {
public:
    virtual void log(const ArduinoHALogMessage& msg) = 0;
    virtual ~ArduinoHALogSink() = default;
};

void arduinoHASetLogSink(ArduinoHALogSink* sink);
ArduinoHALogSink* arduinoHAGetLogSink();

void arduinoHASetLogEnabled(bool enabled);
bool arduinoHAIsLogEnabled();

/**
 * Maximum verbosity to print: Error..Trace. Messages with level <= this value are shown.
 */
void arduinoHASetLogLevel(ArduinoHALogLevel level);
ArduinoHALogLevel arduinoHAGetLogLevel();

void arduinoHALog(
    ArduinoHALogLevel level,
    const char* subsystem,
    const String& text
);

const char* arduinoHALogLevelTag(ArduinoHALogLevel level);

/**
 * Optional hook for transport diagnostics (e.g. return WiFi.status()).
 * When unset, publish failure logs omit network fields.
 */
typedef int (*ArduinoHANetworkStatusFn)(void);

void arduinoHASetNetworkStatusFn(ArduinoHANetworkStatusFn fn);

/** Returns -1 when no hook is installed. */
int arduinoHANetworkStatusOptional(void);

#endif
