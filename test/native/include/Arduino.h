#ifndef AHA_NATIVE_ARDUINO_H
#define AHA_NATIVE_ARDUINO_H

// Minimal Arduino API shim for the host-only PlatformIO native test target.
// It is deliberately test-only: production builds continue to use each
// platform's Arduino core and PROGMEM implementation.

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>

typedef uint8_t byte;

class __FlashStringHelper;

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef PGM_P
typedef const char* PGM_P;
#endif

#define F(value) reinterpret_cast<const __FlashStringHelper*>(value)
#ifndef pgm_read_byte
#define pgm_read_byte(address) (*reinterpret_cast<const uint8_t*>(address))
#endif

#ifndef strlen_P
inline size_t strlen_P(PGM_P value)
{
    return value ? strlen(value) : 0;
}
#endif

#ifndef strcpy_P
inline char* strcpy_P(char* destination, PGM_P source)
{
    return strcpy(destination, source);
}
#endif

#ifndef strncpy_P
inline char* strncpy_P(char* destination, PGM_P source, size_t count)
{
    return strncpy(destination, source, count);
}
#endif

#ifndef strcat_P
inline char* strcat_P(char* destination, PGM_P source)
{
    return strcat(destination, source);
}
#endif

#ifndef strcmp_P
inline int strcmp_P(const char* left, PGM_P right)
{
    return strcmp(left, right);
}
#endif

#ifndef memcpy_P
inline void* memcpy_P(void* destination, PGM_P source, size_t count)
{
    return memcpy(destination, source, count);
}
#endif

class String
{
public:
    String() = default;

    String(const char* value) :
        _value(value ? value : "")
    {

    }

    String(const __FlashStringHelper* value) :
        _value(value ? reinterpret_cast<const char*>(value) : "")
    {

    }

    String(const std::string& value) :
        _value(value)
    {

    }

    String(char value) :
        _value(1, value)
    {

    }

    String(bool value) :
        _value(value ? "1" : "0")
    {

    }

    String(int value) : _value(std::to_string(value)) { }
    String(unsigned int value) : _value(std::to_string(value)) { }
    String(long value) : _value(std::to_string(value)) { }
    String(unsigned long value) : _value(std::to_string(value)) { }
    String(long long value) : _value(std::to_string(value)) { }
    String(unsigned long long value) : _value(std::to_string(value)) { }
    String(float value) : _value(std::to_string(value)) { }
    String(double value) : _value(std::to_string(value)) { }

    const char* c_str() const
    {
        return _value.c_str();
    }

    size_t length() const
    {
        return _value.length();
    }

    String& operator+=(const String& value)
    {
        _value += value._value;
        return *this;
    }

    String& operator+=(const char* value)
    {
        _value += value ? value : "";
        return *this;
    }

    String& operator+=(const __FlashStringHelper* value)
    {
        _value += value ? reinterpret_cast<const char*>(value) : "";
        return *this;
    }

    String operator+(const String& value) const
    {
        return String(_value + value._value);
    }

    String operator+(const char* value) const
    {
        return String(_value + (value ? value : ""));
    }

    String operator+(const __FlashStringHelper* value) const
    {
        return String(_value + (value ? reinterpret_cast<const char*>(value) : ""));
    }

private:
    std::string _value;
};

inline String operator+(const char* left, const String& right)
{
    return String(left) + right;
}

inline String operator+(const __FlashStringHelper* left, const String& right)
{
    return String(left) + right;
}

class NativeSerial
{
public:
    void begin(unsigned long) { }

    template<typename T>
    size_t print(const T& value)
    {
        std::cout << value;
        return 1;
    }

    size_t print(const __FlashStringHelper* value)
    {
        std::cout << reinterpret_cast<const char*>(value);
        return 1;
    }

    size_t print(const String& value)
    {
        std::cout << value.c_str();
        return value.length();
    }

    template<typename T>
    size_t println(const T& value)
    {
        print(value);
        std::cout << '\n';
        return 1;
    }

    size_t println()
    {
        std::cout << '\n';
        return 1;
    }
};

static NativeSerial Serial;

inline uint32_t& nativeArduinoMillisStorage()
{
    static uint32_t value = 0;
    return value;
}

inline unsigned long millis()
{
    return nativeArduinoMillisStorage();
}

inline void delay(unsigned long duration)
{
    nativeArduinoMillisStorage() += static_cast<uint32_t>(duration);
}

inline void yield() { }

#endif
