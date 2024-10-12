# ShiftLED Library

ShiftLED is an Arduino library for controlling addressable LEDs using SPI, with built-in power limiting features.

## Features

- Control various types of addressable LEDs.
- Set per-LED colors and brightness.
- Global brightness control with automatic power limiting.
- Power consumption estimation.
- Support for color strings (e.g., "#FF0000", "2700K").

## Installation

### Manual Installation

1. Download or clone the repository.
2. Copy the `ShiftLED` folder into your `Arduino/libraries` directory.
3. Restart the Arduino IDE.

### Install via ZIP File

1. Download the `ShiftLED.zip` file.
2. Open the Arduino IDE.
3. Go to `Sketch` -> `Include Library` -> `Add .ZIP Library...`.
4. Select the `ShiftLED.zip` file.

## Usage

```cpp
#include <ShiftLED.h>

// Create a ShiftLED object
ShiftLED leds("RGBCH", NUM_LEDS, DATA_PIN);

void setup() {
    leds.begin();
    leds.setGlobalBrightness(200);
    leds.setMaxPower(2000); // Max power in mW
    leds.setMaxPowerPerLED(400); // Max power per LED in mW
}

void loop() {
    // Set colors and update LEDs
    leds.setAllLEDs("#FF0000"); // Set all LEDs to red
    leds.update();
}
