#include <Arduino.h>
#include <TFT_eSPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <string.h>

#ifndef THERMOMETER_DEMO
#define THERMOMETER_DEMO 0
#endif
#ifndef ONE_WIRE_PIN
#define ONE_WIRE_PIN 27
#endif
#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 1 // Set to 0 if your panel displays the correct colours without inversion.
#endif

namespace {
constexpr uint16_t BACKGROUND = 0x0866;
constexpr uint16_t CARD = 0x1194;
constexpr uint16_t MUTED = 0xC6DF;
constexpr uint16_t ACCENT = 0x07FF;
constexpr uint16_t AMBER = 0xFE48;
constexpr uint16_t DIGITS = 0xFFE0;
constexpr uint16_t PINK = 0xFB5F;
constexpr uint32_t CONVERSION_MS = 800; // DS18B20 12-bit conversion: up to 750 ms.
constexpr uint32_t SAMPLE_MS = 1000;
constexpr uint32_t RETRY_MS = 3000;

TFT_eSPI display;
TFT_eSprite frame(&display);
bool buffered = false;
OneWire wire(ONE_WIRE_PIN);
DallasTemperature sensor(&wire);
DeviceAddress address = {};
DeviceAddress previousAddress = {};
bool havePreviousAddress = false;

enum class Status { Missing, Starting, Live, Error, Power, Demo };
Status status = Status::Missing;
bool connected = false;
bool converting = false;
bool haveReading = false;
bool haveHistory = false;
float temperature = 0;
float minimum = 0;
float maximum = 0;
uint32_t conversionStarted = 0;
uint32_t lastScan = 0;
uint32_t lastGoodReading = 0;
uint32_t lastPaint = 0;
bool dirty = true;
bool wideScale = false;

template <typename Surface>
void label(Surface &screen, const char *text, int x, int y, uint16_t colour, uint16_t background = CARD) {
    screen.setTextDatum(TL_DATUM);
    screen.setTextColor(colour, background);
    screen.drawString(text, x, y, 2);
}

template <typename Surface>
void drawScreen(Surface &screen, uint32_t now) {
    screen.fillScreen(BACKGROUND);
    screen.setTextSize(1);
    screen.setTextDatum(TL_DATUM);
    screen.setTextColor(ACCENT, BACKGROUND);
    screen.drawString("THERMOMETER", 12, 11, 2);

    const bool live = status == Status::Live || status == Status::Demo;
    const uint16_t statusColour = live ? ACCENT : AMBER;
    const char *badge = status == Status::Demo ? "DEMO" : live ? "LIVE" : "WAIT";
    screen.fillCircle(244, 20, 3, statusColour);
    screen.setTextColor(statusColour, BACKGROUND);
    screen.drawString(badge, 255, 11, 2);
    screen.drawFastHLine(12, 34, 98, ACCENT);
    screen.drawFastHLine(110, 34, 99, PINK);
    screen.drawFastHLine(209, 34, 99, DIGITS);

    screen.fillRoundRect(12, 42, 296, 124, 10, CARD);
    screen.drawRoundRect(12, 42, 296, 124, 10, ACCENT);

    // Normal scale suits everyday temperatures. Once exceeded, the wider
    // scale stays selected until restart so the dial cannot jump back and forth.
    const float low = wideScale ? -60.0f : -20.0f;
    const float high = wideScale ? 140.0f : 60.0f;
    constexpr int cx = 160, cy = 140;
    constexpr float pi = 3.14159265f;
    auto xAt = [&](float fraction, int radius) {
        return cx + static_cast<int>(lroundf(cosf(pi * (1.0f - fraction)) * radius));
    };
    auto yAt = [&](float fraction, int radius) {
        return cy - static_cast<int>(lroundf(sinf(pi * (1.0f - fraction)) * radius));
    };
    // Colour bands are visual decoration, not warning thresholds.
    for (int i = 0; i <= 180; ++i) {
        const float fraction = i / 180.0f;
        const uint16_t band = i < 45 ? TFT_BLUE : i < 90 ? ACCENT : i < 135 ? DIGITS : PINK;
        screen.drawLine(xAt(fraction, 90), yAt(fraction, 90), xAt(fraction, 94), yAt(fraction, 94), band);
    }
    char text[32];
    for (int i = 0; i <= 20; ++i) {
        const float fraction = i / 20.0f;
        const bool major = i % 5 == 0;
        screen.drawLine(xAt(fraction, 87), yAt(fraction, 87), xAt(fraction, major ? 78 : 83), yAt(fraction, major ? 78 : 83), TFT_WHITE);
        if (major) {
            snprintf(text, sizeof(text), "%.0f", low + fraction * (high - low));
            screen.setTextDatum(MC_DATUM);
            screen.setTextColor(MUTED, CARD);
            screen.drawString(text, xAt(fraction, 65), yAt(fraction, 65), 2);
        }
    }
    if (haveReading) {
        const float fraction = constrain((temperature - low) / (high - low), 0.0f, 1.0f);
        const float angle = pi * (1.0f - fraction);
        const int tipX = xAt(fraction, 57), tipY = yAt(fraction, 57);
        const int dx = static_cast<int>(lroundf(sinf(angle) * 3));
        const int dy = static_cast<int>(lroundf(cosf(angle) * 3));
        screen.fillTriangle(tipX, tipY, cx - dx, cy - dy, cx + dx, cy + dy, PINK);
    }
    screen.fillCircle(cx, cy, 5, haveReading ? DIGITS : MUTED);

    const char *message = "Sensor not connected";
    if (status == Status::Starting) message = "Taking first reading...";
    else if (status == Status::Live) message = "TEMPERATURE / C";
    else if (status == Status::Error) message = "Sensor read failed";
    else if (status == Status::Power) message = "Use 3-wire sensor power";
    else if (status == Status::Demo) message = "SIMULATED / C";
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(live ? ACCENT : AMBER, CARD);
    screen.drawString(message, 160, 149, 2);

    screen.fillRoundRect(12, 170, 296, 50, 8, CARD);
    if (haveReading) snprintf(text, sizeof(text), "%.2f", temperature);
    else snprintf(text, sizeof(text), "--.--");
    screen.setTextColor(haveReading ? DIGITS : MUTED, CARD);
    const int readingWidth = screen.textWidth(text, 7);
    const int left = (320 - readingWidth - 28) / 2;
    screen.setTextDatum(TL_DATUM);
    screen.drawString(text, left, 171, 7);
    screen.drawCircle(left + readingWidth + 8, 187, 2, DIGITS);
    label(screen, "C", left + readingWidth + 15, 183, DIGITS);

    if (haveHistory) snprintf(text, sizeof(text), "MIN %.2f C", minimum);
    else snprintf(text, sizeof(text), "MIN --.-- C");
    label(screen, text, 12, 223, ACCENT, BACKGROUND);
    if (haveHistory) snprintf(text, sizeof(text), "MAX %.2f C", maximum);
    else snprintf(text, sizeof(text), "MAX --.-- C");
    screen.setTextDatum(TR_DATUM);
    screen.setTextColor(PINK, BACKGROUND);
    screen.drawString(text, 308, 223, 2);
}

void paint(uint32_t now) {
    if (buffered) {
        drawScreen(frame, now);
        frame.pushSprite(0, 0);
    } else {
        drawScreen(display, now); // Graceful fallback if a frame buffer is unavailable.
    }
    dirty = false;
    lastPaint = now;
}

void acceptReading(float value) {
    temperature = value;
    if (value < -20.0f || value > 60.0f) wideScale = true;
    haveReading = true;
    if (!haveHistory) {
        minimum = maximum = value;
        haveHistory = true;
    } else {
        minimum = fminf(minimum, value);
        maximum = fmaxf(maximum, value);
    }
    lastGoodReading = millis();
    status = THERMOMETER_DEMO ? Status::Demo : Status::Live;
    dirty = true;
    Serial.printf("Temperature: %.2f C | min %.2f | max %.2f\n", value, minimum, maximum);
}

void beginConversion() {
    const auto request = sensor.requestTemperaturesByAddress(address);
    if (!request.result) {
        connected = false;
        haveReading = false;
        status = Status::Error;
        lastScan = millis();
        dirty = true;
        return;
    }
    conversionStarted = millis();
    converting = true;
}

void scanSensor() {
    lastScan = millis();
    sensor.begin();
    sensor.setWaitForConversion(false);
    DeviceAddress found;
    for (uint8_t i = 0; i < sensor.getDeviceCount(); ++i) {
        if (!sensor.getAddress(found, i) || found[0] != 0x28) continue;
        if (!sensor.isConnected(found)) continue;
        // This project is for the externally powered 3-wire wiring discussed.
        if (sensor.readPowerSupply(found)) {
            status = Status::Power;
            dirty = true;
            return;
        }
        if (!sensor.setResolution(found, 12)) continue;
        if (!havePreviousAddress || memcmp(found, previousAddress, sizeof(DeviceAddress)) != 0) {
            haveHistory = false; // Never combine extrema from two different sensors.
            memcpy(previousAddress, found, sizeof(DeviceAddress));
            havePreviousAddress = true;
        }
        memcpy(address, found, sizeof(DeviceAddress));
        connected = true;
        haveReading = false;
        status = Status::Starting;
        dirty = true;
        Serial.println("DS18B20 found on GPIO27; starting conversion.");
        beginConversion();
        return;
    }
    // Retain an error indication until a successful reconnection.
    if (status != Status::Error) status = Status::Missing;
    dirty = true;
}
} // namespace

void setup() {
    Serial.begin(115200);
    display.init();
    display.setRotation(1);
    display.invertDisplay(DISPLAY_INVERT != 0);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    frame.setColorDepth(8); // 76.8 KB full-screen buffer; avoids visible redraw flicker.
    buffered = frame.createSprite(320, 240) != nullptr;
    if (THERMOMETER_DEMO) {
        status = Status::Demo;
        acceptReading(23.4f);
    }
    paint(millis());
    if (!THERMOMETER_DEMO) scanSensor();
}

void loop() {
    const uint32_t now = millis();
    if (THERMOMETER_DEMO) {
        if (now - lastGoodReading >= SAMPLE_MS) acceptReading(23.4f + 1.2f * sinf(now / 15000.0f));
    } else if (!connected) {
        if (now - lastScan >= RETRY_MS) scanSensor();
    } else if (converting && now - conversionStarted >= CONVERSION_MS) {
        converting = false;
        const float value = sensor.getTempC(address); // Includes scratchpad CRC validation.
        if (value == DEVICE_DISCONNECTED_C || !isfinite(value) || value < -55 || value > 125) {
            connected = false;
            haveReading = false; // Do not present a stale value as a live temperature.
            status = Status::Error;
            lastScan = millis();
            dirty = true;
            Serial.println("Sensor read failed; retrying connection.");
        } else acceptReading(value);
    } else if (!converting && now - conversionStarted >= SAMPLE_MS) {
        beginConversion();
    }
    if (dirty || millis() - lastPaint >= SAMPLE_MS) paint(millis());
    delay(1);
}

