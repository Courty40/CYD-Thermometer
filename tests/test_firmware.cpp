#include <cassert>
#include <limits>
#include <iostream>
#include "../src/main.cpp"
bool contains(const char* text) {
 for (const auto& s:drawn) if(s==text) return true;
 return false;
}
void tick(uint32_t ms) { fakeNow += ms; loop(); }
int main() {
 fakeSpriteAvailable=false;
 setup();
 assert(fakeRotation==1);
 assert(fakeDisplayInverted==(DISPLAY_INVERT!=0));
#if defined(ST7789_DRIVER)
 assert(DISPLAY_INVERT==0);
 assert(std::string(THERMOMETER_DISPLAY_NAME)=="ST7789 CYD2USB");
#else
 assert(DISPLAY_INVERT==1);
 assert(std::string(THERMOMETER_DISPLAY_NAME)=="ILI9341 CYD");
#endif
 assert(!buffered); // Exercise real direct-render fallback.
#if THERMOMETER_DEMO
 assert(status==Status::Demo && haveReading && haveHistory);
 assert(contains("DEMO") && contains("SIMULATED / C"));
 tick(1000);
 assert(sensor.requests==0 && status==Status::Demo);
 assert(temperature>=22.2f && temperature<=24.6f);
 std::cout << "Demo isolation and fallback display checks passed\n";
#else
 assert(status==Status::Missing && !haveReading);
 assert(contains("Sensor not connected") && contains("--.--"));
 sensor.present=true; tick(3000);
 assert(connected && converting && !haveReading && !sensor.wait);
 tick(799); assert(!haveReading); // Never display the power-on value before conversion.
 tick(1); assert(haveReading && status==Status::Live && temperature==21.5f);
 assert(minimum==21.5f && maximum==21.5f);
 auto read=[&](float value) {sensor.value=value; tick(1000); tick(800);};
 read(85); assert(temperature==85 && wideScale && maximum==85);
 read(-55); assert(minimum==-55 && wideScale);
 read(125); assert(maximum==125);
 read(20); assert(wideScale && minimum==-55 && maximum==125);
 read(DEVICE_DISCONNECTED_C);
 assert(!connected && !haveReading && haveHistory && status==Status::Error);
 assert(contains("Sensor read failed") && contains("--.--"));
 sensor.present=false; tick(3000); assert(status==Status::Error);
 sensor.present=true; sensor.value=18; tick(3000); tick(800);
 assert(haveReading && minimum==-55 && maximum==125); // Same sensor keeps extrema.
 sensor.identity=2; connected=false; scanSensor();
 assert(!haveHistory && !haveReading); tick(800);
 assert(minimum==18 && maximum==18);
 read(std::numeric_limits<float>::quiet_NaN()); assert(!haveReading && !connected);
 scanSensor(); tick(800); assert(!haveReading);
 sensor.value=126; scanSensor(); tick(800); assert(!haveReading);
 sensor.value=-56; scanSensor(); tick(800); assert(!haveReading);
 sensor.parasite=true; scanSensor(); assert(status==Status::Power && !connected);
 paint(fakeNow); assert(contains("Use 3-wire sensor power"));
 sensor.parasite=false; sensor.resolutionOK=false; scanSensor(); assert(!connected);
 sensor.resolutionOK=true; sensor.requestOK=false; scanSensor();
 assert(status==Status::Error && !converting && !connected);
 sensor.requestOK=true; sensor.family=0x10; scanSensor(); assert(!connected);
 sensor.family=0x28; sensor.value=22;
 fakeNow=0xFFFFFF00u; scanSensor(); tick(800);
 assert(haveReading && temperature==22); // Conversion timer survives millis rollover.
 std::cout << "Sensor startup, extrema, scale, failures, recovery, replacement, power, family and rollover checks passed\n";
#endif
}
