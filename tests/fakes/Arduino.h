#pragma once
#include <cstdint>
#include <cstdio>
#include <algorithm>
using byte = uint8_t;
static uint32_t fakeNow = 0;
inline uint32_t millis() { return fakeNow; }
inline void delay(uint32_t) {} // Test clock advances only through tick().
inline void pinMode(int,int) {}
inline void digitalWrite(int,int) {}
#define OUTPUT 1
#define HIGH 1
#define TFT_BL 21
#define TFT_BACKLIGHT_ON HIGH
#define constrain(v,lo,hi) std::min(std::max((v),(lo)),(hi))
struct SerialFake {
 void begin(int) {}
 template<class... A> void printf(const char*,A...) {}
 void println(const char*) {}
};
static SerialFake Serial;
