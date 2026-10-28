# Changelog

## 1.1.0 — 28 October 2026

- Adds a separate common CYD2USB ST7789 profile with BGR colour order and inversion off.
- Retains ILI9341 as the default Build/Upload target.
- Provides controller-specific sensor, application-only and demo binaries.
- Reports the display profile at serial startup and checks profile isolation in CI.
- ST7789 panel orientation and colours still require physical confirmation.

## 1.0.0 — 28 October 2026

First packaged G1ROK public release.

- Retains the analogue dial, digital Celsius reading, session extrema and corrected display colours.
- Adds version identification to serial startup and reports the configured sensor GPIO correctly.
- Provides complete USB-flashable sensor and demo images, an application-only binary and SHA-256 checksums.
- Rewrites setup, board compatibility, wiring, sensor accuracy and troubleshooting documentation.
- Adds application-logic regression tests, automated builds and repeatable release packaging.

## Earlier development

Initial dial and DS18B20 implementation, source folder correction and display colour inversion fix. See the Git history for the original commits.
