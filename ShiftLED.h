// ShiftLED.h

#ifndef SHIFT_LED_H
#define SHIFT_LED_H

#include <Arduino.h>
#include <SPI.h>
#include <vector>

/// @brief Enumeration for color components.
enum ColorComponent {
    RED,
    GREEN,
    BLUE,
    WHITE,
    WARM_WHITE,
    COLD_WHITE,
    NONE
};

/// @brief ShiftLED class for controlling addressable LEDs using SPI.
class ShiftLED {
  public:
    // Constructor
    ShiftLED(String ledTypeString, uint16_t numLEDs, uint8_t dataPin, SPIClass& SPI_peripheral = SPI);

    // Destructor
    ~ShiftLED();

    // Initializes the ShiftLED object and begins SPI communication.
    void begin();

    // Ends SPI communication and cleans up resources.
    void end();

    // Sets the color of a single LED using color components.
    void setLEDColor(uint16_t index, uint8_t red, uint8_t green, uint8_t blue,
                     uint8_t white = 0, uint8_t warmWhite = 0, uint8_t coldWhite = 0,
                     uint8_t brightness = 255);

    // Sets the color of a single LED using a color string.
    void setLEDColor(uint16_t index, const String& colorString, uint8_t brightness = 255);

    // Sets the color of all LEDs using color components.
    void setAllLEDs(uint8_t red, uint8_t green, uint8_t blue,
                    uint8_t white = 0, uint8_t warmWhite = 0, uint8_t coldWhite = 0,
                    uint8_t brightness = 255);

    // Sets the color of all LEDs using a color string.
    void setAllLEDs(const String& colorString, uint8_t brightness = 255);

    // Sets the desired global brightness level (0-255).
    void setGlobalBrightness(uint8_t brightnessLevel);

    // Sets the maximum allowed power consumption in milliwatts (mW).
    void setMaxPower(uint32_t maxPower_mW);

    // Sets the maximum power consumption per LED at full brightness.
    void setMaxPowerPerLED(uint16_t maxPowerPerLED_mW);

    // Updates the LEDs with the current color data.
    void update();

    // Changes the number of LEDs at runtime.
    void setNumLEDs(uint16_t newNumLEDs);

    // Gets the current number of LEDs.
    uint16_t getNumLEDs() const;

    // Gets the actual global brightness level after power limiting.
    uint8_t getActualGlobalBrightness() const;

    // Estimates the power consumption in milliwatts (mW) after brightness adjustment.
    uint32_t estimatePowerConsumption() const;

    // Estimates the power consumption in milliwatts (mW) before brightness adjustment.
    uint32_t estimateDesiredPowerConsumption() const;

  private:
    uint16_t numLEDs;
    uint8_t dataPin;
    uint8_t desiredGlobalBrightness; // Desired global brightness (0-255)
    uint8_t actualGlobalBrightness;  // Actual global brightness (0-255)
    uint8_t* ledData;                // Stores color values sequentially
    uint8_t* ledBrightness;          // Stores per-LED brightness levels (0-255)
    SPIClass& SPI_peripheral;

    String ledType;
    std::vector<ColorComponent> colorOrder;

    // Maximum allowed power consumption in milliwatts (mW).
    uint32_t maxAllowedPower_mW;

    // Maximum power consumption per LED at full brightness (default is 300 mW).
    uint16_t maxPowerPerLED_mW;

    // Parses the LED type string and sets up configurations.
    bool parseLEDType(String ledTypeString);

    // Parses a color string and extracts color components.
    bool parseColorString(const String& colorString, uint8_t& red, uint8_t& green, uint8_t& blue,
                          uint8_t& white, uint8_t& warmWhite, uint8_t& coldWhite);

    // Converts a Kelvin temperature to warm and cold white components.
    void kelvinToWarmCold(uint16_t kelvin, uint8_t& warmWhite, uint8_t& coldWhite);

    // Sends a logic '1' bit to the LEDs via SPI.
    void one();

    // Sends a logic '0' bit to the LEDs via SPI.
    void zero();

    // Sends the color data for a single pixel.
    void sendPixel(uint8_t* colors, uint8_t brightness);

    // Ends the data transmission by sending a reset signal.
    void endTransfer();

    // Updates the actual global brightness based on estimated power consumption.
    void updateActualBrightness();

    // Calculates the power consumption in milliwatts (mW) using the specified global brightness.
    uint32_t calculatePowerConsumption(uint8_t globalBrightness) const;
};

#endif // SHIFT_LED_H
