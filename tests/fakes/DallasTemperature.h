#pragma once
#include <cstring>
#include "OneWire.h"
using DeviceAddress = uint8_t[8];
#define DEVICE_DISCONNECTED_C -127.0f
class DallasTemperature {
public:
 explicit DallasTemperature(OneWire*) {}
 bool present = false, parasite = false, resolutionOK = true, requestOK = true;
 uint8_t family = 0x28, identity = 1;
 float value = 21.5f;
 int requests = 0;
 bool wait = true;
 void begin() {}
 void setWaitForConversion(bool v) { wait=v; }
 uint8_t getDeviceCount() { return present ? 1 : 0; }
 bool getAddress(uint8_t* a,uint8_t) { std::memset(a,0,8); a[0]=family; a[7]=identity; return present; }
 bool isConnected(const uint8_t*) { return present; }
 bool readPowerSupply(const uint8_t*) { return parasite; }
 bool setResolution(const uint8_t*,uint8_t v) { return resolutionOK && v==12; }
 struct Request { bool result; };
 Request requestTemperaturesByAddress(const uint8_t*) { ++requests; return {requestOK}; }
 float getTempC(const uint8_t*) { return value; }
};
