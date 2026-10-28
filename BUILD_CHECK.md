# Release validation — v1.1.0

Checked on 28 October 2026 using PlatformIO Core 6.2.0.

## Automated results

- All four environments (`cyd`, `demo`, `cyd_st7789`, `demo_st7789`) pass using the pinned platform and libraries.
- Host tests compile and pass for both normal and demo configurations on each display profile. They execute `src/main.cpp` with controlled sensor, clock and drawing interfaces.
- Sensor checks cover initial absence, delayed first conversion, 85 °C after conversion, −55/+125 °C endpoints, extrema, permanent wide-scale selection, disconnected/NaN/out-of-range readings, retry, reconnection, sensor replacement, unsupported family, parasite-power refusal, resolution/conversion request failure and timer rollover.
- Demo checks cover simulated readings, visible labelling, no sensor requests and direct drawing when sprite allocation fails.
- Packaging checks verify application and partition bytes at the documented offsets in all four complete images. SHA-256 checksums accompany the release.
- The four original commits and current source were reviewed before public release; no embedded credentials were identified.

Upstream OneWire 2.3.8 emits two extra-token preprocessor warnings. These do not prevent the builds. The project does not use touch.

- Profile checks cover the default ILI9341 target, driver isolation, BGR selection, inversion settings, shared GPIOs and demo flags. Host rendering checks confirm rotation 1 and per-profile inversion; a mixed-driver build is rejected.

## Physical checks still required

The new common CYD2USB ST7789 profile uses the board reference settings but still needs a physical check of its colours, orientation and panel compatibility. No board was flashed or physically tested during this release preparation. Automated drawing fakes check displayed messages, not pixel-perfect appearance or panel compatibility. Existing images are layout previews.

On the intended CYD, verify panel orientation and colours, the waiting screen without a sensor, a plausible live reading against a reference, disconnect/reconnect behaviour, and min/max reset after restart. Check sensor pinout before connecting power.

## Repeat the checks

Build all environments with `pio run -e cyd -e demo -e cyd_st7789 -e demo_st7789`. Run `python scripts/check_profiles.py` to verify the resolved, inherited driver and wiring flags. Run the two C++ test commands in the README, then `python scripts/package_release.py` from a Python environment containing PlatformIO. Packaging uses the esptool and framework installed by the pinned PlatformIO platform.
