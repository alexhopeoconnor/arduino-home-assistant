#include "ArduinoHALog.h"

static ArduinoHALogSink* g_arduinoHALogSink = nullptr;
static ArduinoHANetworkStatusFn g_networkStatusFn = nullptr;

#ifdef ARDUINOHA_DEBUG
static bool g_logEnabled = true;
static ArduinoHALogLevel g_logLevelMax = ArduinoHALogLevel::Debug;
#else
static bool g_logEnabled = false;
static ArduinoHALogLevel g_logLevelMax = ArduinoHALogLevel::Info;
#endif

void arduinoHASetLogSink(ArduinoHALogSink* sink) {
    g_arduinoHALogSink = sink;
}

ArduinoHALogSink* arduinoHAGetLogSink() {
    return g_arduinoHALogSink;
}

void arduinoHASetLogEnabled(bool enabled) {
    g_logEnabled = enabled;
}

bool arduinoHAIsLogEnabled() {
    return g_logEnabled;
}

void arduinoHASetLogLevel(ArduinoHALogLevel level) {
    g_logLevelMax = level;
}

ArduinoHALogLevel arduinoHAGetLogLevel() {
    return g_logLevelMax;
}

void arduinoHASetNetworkStatusFn(ArduinoHANetworkStatusFn fn) {
    g_networkStatusFn = fn;
}

int arduinoHANetworkStatusOptional(void) {
    if (!g_networkStatusFn) {
        return -1;
    }
    return g_networkStatusFn();
}

const char* arduinoHALogLevelTag(ArduinoHALogLevel level) {
    switch (level) {
        case ArduinoHALogLevel::Error:
            return "ERROR";
        case ArduinoHALogLevel::Warn:
            return "WARN";
        case ArduinoHALogLevel::Info:
            return "INFO";
        case ArduinoHALogLevel::Debug:
            return "DEBUG";
        case ArduinoHALogLevel::Trace:
            return "TRACE";
        default:
            return "?";
    }
}

void arduinoHALog(
    ArduinoHALogLevel level,
    const char* subsystem,
    const String& text
) {
    if (!g_logEnabled) {
        return;
    }

    if (static_cast<uint8_t>(level) > static_cast<uint8_t>(g_logLevelMax)) {
        return;
    }

    const char* sub = (subsystem && subsystem[0] != '\0') ? subsystem : "aha";

    ArduinoHALogMessage msg;
    msg.level = level;
    msg.subsystem = sub;
    msg.text = text;

    if (g_arduinoHALogSink) {
        g_arduinoHALogSink->log(msg);
        return;
    }

    Serial.print(F("["));
    Serial.print(arduinoHALogLevelTag(level));
    Serial.print(F("][aha."));
    Serial.print(sub);
    Serial.print(F("] "));
    Serial.println(text);
}
