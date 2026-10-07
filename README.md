# CYD analog thermometer

A 320x240 landscape thermometer for the CYD display configuration already tested on your board. Colourful semicircular analog meter, pink needle, a large yellow digital Celsius reading underneath and session min/max. All digital readings show two decimal places. The DS18B20 is explicitly set to 12-bit mode (0.0625 C measurement increments; decimal formatting does not increase sensor accuracy). Screen updates about once a second using a frame buffer. No touch controls or network connection required.

## Upload

Extract the ZIP, open the folder containing platformio.ini in VS Code, connect the CYD, and use PlatformIO Build then Upload for the **cyd** environment. The display shows placeholders and "Sensor not connected" until an externally powered DS18B20 is found. It checks again every three seconds and connects automatically. You can upload before wiring the sensor; power off while wiring.

## Wiring

Use CN1 labels, rather than assuming connector pin order:

| DS18B20 | CYD CN1 |
|---|---|
| VDD | 3.3V |
| GND | GND |
| DQ | GPIO27 |

Fit a 4.7 kohm resistor between DQ and 3.3V unless your sensor module already has one. Check the actual sensor lead assignments. This project uses external 3-wire power; parasite power is not supported.

## Before your sensor is ready

An optional **demo** environment shows a simulated temperature and visibly labels the display DEMO. In PlatformIO select demo before Build/Upload, or run `pio run -e demo -t upload`. Use **cyd** (`pio run -e cyd -t upload`) again when the real sensor is connected. Default builds select cyd. The preview images illustrate both states; they are layout previews, not photographs of a physical display.

## Behaviour

- Uses the first detected DS18B20 (family 0x28), with a completed 12-bit conversion before displaying data.
- Rejects failed CRC/read results and values outside the sensor range. Genuine 85 C is permitted after a completed conversion.
- On disconnection/read failure, clears the current temperature and retries. Min/max remain the historical extrema for that sensor during this powered session.
- Replacing the sensor resets its min/max. Restarting the board resets all history.
- Logs readings to Serial Monitor at 115200 baud.
- The dial starts at -20 to 60 C. If a reading exceeds that range, it switches to -60 to 140 C until restart, covering the entire DS18B20 range. Tick labels update with the scale. Colour bands are decorative, not alarms. The needle is hidden when there is no valid current reading.
- A TFT_eSPI warning about TOUCH_CS is expected: touch is not used, and CYD touch has a separate SPI connection.

Change ONE_WIRE_PIN in platformio.ini if you later choose a different suitable GPIO. Screen colours/layout are at the top of src/main.cpp. Display pins and driver are unchanged from the working Hello World project.

References: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/PINS.md and https://github.com/milesburton/Arduino-Temperature-Control-Library
