#ifndef AHA_NATIVE_IPADDRESS_H
#define AHA_NATIVE_IPADDRESS_H

#include <Arduino.h>

class IPAddress
{
public:
    IPAddress() :
        _octets{0, 0, 0, 0}
    {

    }

    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) :
        _octets{a, b, c, d}
    {

    }

    String toString() const
    {
        return String(static_cast<unsigned int>(_octets[0])) + "." +
            String(static_cast<unsigned int>(_octets[1])) + "." +
            String(static_cast<unsigned int>(_octets[2])) + "." +
            String(static_cast<unsigned int>(_octets[3]));
    }

private:
    uint8_t _octets[4];
};

#endif
