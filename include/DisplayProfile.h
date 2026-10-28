#pragma once
#if defined(ST7789_DRIVER) && (defined(ILI9341_DRIVER) || defined(ILI9341_2_DRIVER))
#error "Choose one display driver: ILI9341 or ST7789."
#endif
#if defined(ST7789_DRIVER)
#define THERMOMETER_DISPLAY_NAME "ST7789 CYD2USB"
#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 0
#endif
#else
#define THERMOMETER_DISPLAY_NAME "ILI9341 CYD"
#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 1
#endif
#endif
