// ShiftLED_Example.ino

#include <Arduino.h>
#include <ShiftLED.h>

//---------------------------------------------------------------------------------------------------------------------
// Configuration
//---------------------------------------------------------------------------------------------------------------------

const uint8_t DATA_PIN = MOSI;     // SPI data pin (MOSI, Pin 11 on Arduino Nano)
const String ledType = "GRB";   // LED type or color order (e.g., "WS2812", "GRBW")
const uint16_t TOTAL_LEDS = 10;     // Total number of LEDs

// Initialize the ShiftLED object with the LED type
ShiftLED leds(ledType, TOTAL_LEDS, DATA_PIN);

void setup() {
    Serial.begin(115200);
    delay(1000);

    leds.begin();

    // Clear all LEDs at the start
    leds.setAllLEDs(0, 0, 0);
    leds.update();

    leds.setMaxPowerPerLED(180);
    leds.setMaxPower(10000);
    leds.setGlobalBrightness(255);
}

void loop() {
    // Define the colors to cycle through
    String colors[] = {
    "#FF5733", // Vibrant Orange
    "#33FF57", // Bright Green
    "#3357FF", // Deep Blue
    "#FF33A8", // Hot Pink
    "#FFD700", // Gold
    "#00FFFF", // Cyan
    "#FF4500", // Orange Red
    "#800080", // Purple
    "#00FF7F", // Spring Green
    "#556B2F"  // Tokyo (Dark Olive Green-like tone)
};

    const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);

    for (uint8_t colorIndex = 0; colorIndex < numColors; colorIndex++) {
        // Light up LEDs one after another with the current color
        for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
            leds.setLEDColor(i, colors[colorIndex]);
            leds.update();
            delay(300); // Delay between lighting up each LED
        }
        Serial.println(leds.estimatePowerConsumption());
    }

    // Start the cycle again
}
