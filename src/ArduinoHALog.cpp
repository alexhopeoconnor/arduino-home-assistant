#include "ArduinoHALog.h"

static ArduinoHALogSink* g_arduinoHALogSink = nullptr;

void arduinoHASetLogSink(ArduinoHALogSink* sink) {
    g_arduinoHALogSink = sink;
}

ArduinoHALogSink* arduinoHAGetLogSink() {
    return g_arduinoHALogSink;
}
