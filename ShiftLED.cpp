// ShiftLED.cpp

#include "ShiftLED.h"

///---------------------------------------------------------------------------------------------------------------------
/// @brief Constructor for ShiftLED class.
///
ShiftLED::ShiftLED(String ledTypeString, uint16_t numLEDs, uint8_t dataPin, SPIClass& SPI_peripheral)
    : numLEDs(numLEDs), dataPin(dataPin), SPI_peripheral(SPI_peripheral), desiredGlobalBrightness(255),
      actualGlobalBrightness(255), maxAllowedPower_mW(0), maxPowerPerLED_mW(300) { // Default maxPowerPerLED_mW is 300
    // Parse the LED type and set configurations
    if (!parseLEDType(ledTypeString)) {
        Serial.println("Unsupported LED type. Please check the LED type string.");
    }

    // Allocate memory for LED data and brightness
    size_t colorComponentCount = colorOrder.size();
    this->ledData = new uint8_t[numLEDs * colorComponentCount];
    this->ledBrightness = new uint8_t[numLEDs]; // Per-LED brightness (0-255)
    memset(this->ledData, 0, numLEDs * colorComponentCount);
    memset(this->ledBrightness, 255, numLEDs); // Default brightness is 255
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Destructor for ShiftLED class.
///
ShiftLED::~ShiftLED() {
    delete[] this->ledData;
    delete[] this->ledBrightness;
    end();
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Parses the LED type string and sets up configurations.
///
bool ShiftLED::parseLEDType(String ledTypeString) {
    ledTypeString.toUpperCase();
    this->ledType = ledTypeString;

    colorOrder.clear();

    if (ledTypeString == "WS2812" || ledTypeString == "NEOPIXEL" || ledTypeString == "SK6812") {
        colorOrder = {GREEN, RED, BLUE}; // WS2812 and SK6812 use GRB order
        return true;
    } else {
        // Parse color order strings like "RGB", "GRBW", "RGBCHN"
        for (char &c : ledTypeString) {
            switch (c) {
                case 'R': colorOrder.push_back(RED); break;
                case 'G': colorOrder.push_back(GREEN); break;
                case 'B': colorOrder.push_back(BLUE); break;
                case 'W': colorOrder.push_back(WHITE); break;
                case 'C': colorOrder.push_back(COLD_WHITE); break;
                case 'H': colorOrder.push_back(WARM_WHITE); break;
                case 'N': colorOrder.push_back(NONE); break;
                default:
                    Serial.print("Unsupported color component: ");
                    Serial.println(c);
                    return false;
            }
        }

        if (colorOrder.empty()) {
            Serial.println("Invalid LED type or color order.");
            return false;
        }

        return true;
    }
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Initializes the ShiftLED object and begins SPI communication.
///
void ShiftLED::begin() {
    pinMode(this->dataPin, OUTPUT);
    SPI_peripheral.begin();
    // Start SPI transaction here
    SPI_peripheral.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Ends SPI communication and cleans up resources.
///
void ShiftLED::end() {
    // End SPI transaction here
    SPI_peripheral.endTransaction();
    SPI_peripheral.end();
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the color of a single LED using color components.
///
void ShiftLED::setLEDColor(uint16_t index, uint8_t red, uint8_t green, uint8_t blue,
                           uint8_t white, uint8_t warmWhite, uint8_t coldWhite,
                           uint8_t brightness) {
    if (index >= this->numLEDs) return;

    size_t colorComponentCount = colorOrder.size();
    uint8_t* p = &this->ledData[index * colorComponentCount];

    // Store colors in the order specified by colorOrder
    for (size_t i = 0; i < colorComponentCount; ++i) {
        switch (colorOrder[i]) {
            case RED:        p[i] = red; break;
            case GREEN:      p[i] = green; break;
            case BLUE:       p[i] = blue; break;
            case WHITE:      p[i] = white; break;
            case WARM_WHITE: p[i] = warmWhite; break;
            case COLD_WHITE: p[i] = coldWhite; break;
            case NONE:       p[i] = 0; break; // Data doesn't matter but should be in proper format
            default:         p[i] = 0; break;
        }
    }

    // Store per-LED brightness
    this->ledBrightness[index] = brightness;
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the color of a single LED using a color string.
///
void ShiftLED::setLEDColor(uint16_t index, const String& colorString, uint8_t brightness) {
    uint8_t red = 0, green = 0, blue = 0, white = 0, warmWhite = 0, coldWhite = 0;
    if (!parseColorString(colorString, red, green, blue, white, warmWhite, coldWhite)) {
        Serial.print("Failed to parse color string: ");
        Serial.println(colorString);
        return;
    }
    setLEDColor(index, red, green, blue, white, warmWhite, coldWhite, brightness);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the color of all LEDs using color components.
///
void ShiftLED::setAllLEDs(uint8_t red, uint8_t green, uint8_t blue,
                          uint8_t white, uint8_t warmWhite, uint8_t coldWhite,
                          uint8_t brightness) {
    for (uint16_t i = 0; i < this->numLEDs; i++) {
        setLEDColor(i, red, green, blue, white, warmWhite, coldWhite, brightness);
    }
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the color of all LEDs using a color string.
///
void ShiftLED::setAllLEDs(const String& colorString, uint8_t brightness) {
    uint8_t red = 0, green = 0, blue = 0, white = 0, warmWhite = 0, coldWhite = 0;
    if (!parseColorString(colorString, red, green, blue, white, warmWhite, coldWhite)) {
        Serial.print("Failed to parse color string: ");
        Serial.println(colorString);
        return;
    }
    setAllLEDs(red, green, blue, white, warmWhite, coldWhite, brightness);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the desired global brightness level.
///
void ShiftLED::setGlobalBrightness(uint8_t brightnessLevel) {
    desiredGlobalBrightness = brightnessLevel;
    // Update actual brightness in case max power is set
    updateActualBrightness();
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the maximum allowed power consumption in milliwatts (mW).
///
void ShiftLED::setMaxPower(uint32_t maxPower_mW) {
    maxAllowedPower_mW = maxPower_mW;
    // Update actual brightness based on new power limit
    updateActualBrightness();
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sets the maximum power consumption per LED at full brightness.
///
void ShiftLED::setMaxPowerPerLED(uint16_t maxPowerPerLED_mW) {
    this->maxPowerPerLED_mW = maxPowerPerLED_mW;
    // Update actual brightness in case this affects power consumption
    updateActualBrightness();
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Updates the actual global brightness based on estimated power consumption.
///
void ShiftLED::updateActualBrightness() {
    if (maxAllowedPower_mW == 0) {
        // No power limiting; use desired brightness
        actualGlobalBrightness = desiredGlobalBrightness;
        return;
    }

    // Estimate the current power consumption at desired brightness
    uint32_t estimatedPower = calculatePowerConsumption(desiredGlobalBrightness);

    if (estimatedPower > maxAllowedPower_mW) {
        // Reduce actual global brightness proportionally
        actualGlobalBrightness = (uint32_t)desiredGlobalBrightness * maxAllowedPower_mW / estimatedPower;
        actualGlobalBrightness = constrain(actualGlobalBrightness, 0, desiredGlobalBrightness);
    } else {
        // Use desired brightness
        actualGlobalBrightness = desiredGlobalBrightness;
    }
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Changes the number of LEDs at runtime.
///
void ShiftLED::setNumLEDs(uint16_t newNumLEDs) {
    delete[] this->ledData;
    delete[] this->ledBrightness;
    this->numLEDs = newNumLEDs;

    size_t colorComponentCount = colorOrder.size();
    this->ledData = new uint8_t[numLEDs * colorComponentCount];
    this->ledBrightness = new uint8_t[numLEDs]; // Per-LED brightness (0-255)
    memset(this->ledData, 0, numLEDs * colorComponentCount);
    memset(this->ledBrightness, 255, numLEDs); // Default brightness is 255
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Gets the current number of LEDs.
///
uint16_t ShiftLED::getNumLEDs() const {
    return this->numLEDs;
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Gets the actual global brightness level after power limiting.
///
uint8_t ShiftLED::getActualGlobalBrightness() const {
    return this->actualGlobalBrightness;
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Estimates the power consumption in milliwatts (mW) after brightness adjustment.
///
uint32_t ShiftLED::estimatePowerConsumption() const {
    return calculatePowerConsumption(actualGlobalBrightness);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Estimates the power consumption in milliwatts (mW) before brightness adjustment.
///
uint32_t ShiftLED::estimateDesiredPowerConsumption() const {
    return calculatePowerConsumption(desiredGlobalBrightness);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Calculates the power consumption in milliwatts (mW) using the specified global brightness.
///
uint32_t ShiftLED::calculatePowerConsumption(uint8_t globalBrightness) const {
    uint32_t totalPower_mW = 0;
    size_t colorComponentCount = colorOrder.size();

    for (uint16_t i = 0; i < numLEDs; i++) {
        uint8_t* p = &ledData[i * colorComponentCount];
        uint8_t brightness = ledBrightness[i];

        // Calculate per-LED brightness scaling (0.0 to 1.0)
        float ledBrightnessScale = ((float)brightness * globalBrightness) / (255.0 * 255.0);

        // Calculate average color intensity for the LED (0.0 to 1.0)
        float colorIntensity = 0.0;
        size_t activeComponents = 0;
        for (size_t j = 0; j < colorComponentCount; ++j) {
            // Exclude 'NONE' components from the calculation
            if (colorOrder[j] != NONE) {
                colorIntensity += p[j] / 255.0;
                activeComponents++;
            }
        }
        if (activeComponents > 0) {
            // Average color intensity across all active components
            colorIntensity /= activeComponents;
        }

        // Total intensity scaling
        float totalIntensity = ledBrightnessScale * colorIntensity;

        // Estimate power for this LED
        uint32_t ledPower_mW = (uint32_t)(maxPowerPerLED_mW * totalIntensity);
        totalPower_mW += ledPower_mW;
    }

    return totalPower_mW;
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Parses a color string and extracts color components.
///
bool ShiftLED::parseColorString(const String& colorString, uint8_t& red, uint8_t& green, uint8_t& blue,
                                uint8_t& white, uint8_t& warmWhite, uint8_t& coldWhite) {
    if (colorString.startsWith("#")) {
        // Hex color code
        if (colorString.length() == 7) { // Format: #RRGGBB
            red   = strtol(colorString.substring(1, 3).c_str(), NULL, 16);
            green = strtol(colorString.substring(3, 5).c_str(), NULL, 16);
            blue  = strtol(colorString.substring(5, 7).c_str(), NULL, 16);
            return true;
        } else if (colorString.length() == 4) { // Format: #RGB
            red   = strtol(colorString.substring(1, 2).c_str(), NULL, 16) * 17; // Duplicate hex digit
            green = strtol(colorString.substring(2, 3).c_str(), NULL, 16) * 17;
            blue  = strtol(colorString.substring(3, 4).c_str(), NULL, 16) * 17;
            return true;
        } else {
            Serial.println("Invalid hex color format.");
            return false;
        }
    } else if (colorString.endsWith("K") || colorString.endsWith("k")) {
        // Kelvin temperature
        uint16_t kelvin = colorString.substring(0, colorString.length() - 1).toInt();
        kelvinToWarmCold(kelvin, warmWhite, coldWhite);
        return true;
    } else {
        Serial.println("Unknown color format.");
        return false;
    }
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Converts a Kelvin temperature to warm and cold white components.
///
void ShiftLED::kelvinToWarmCold(uint16_t kelvin, uint8_t& warmWhite, uint8_t& coldWhite) {
    // Clamp kelvin to a reasonable range
    kelvin = constrain(kelvin, 2000, 9000);

    // Normalize kelvin to a 0-1 range
    float temp = (kelvin - 2000.0) / (9000.0 - 2000.0);

    // Calculate warm and cold white intensities
    warmWhite = (uint8_t)((1.0 - temp) * 255);
    coldWhite = (uint8_t)(temp * 255);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Updates the LEDs with the current color data.
///
void ShiftLED::update() {
    // Update actual global brightness based on power consumption
    updateActualBrightness();

    // Send initial reset code
    SPI_peripheral.transfer(0x00);
    delayMicroseconds(300); // Ensure data line is low for at least 50µs

    noInterrupts(); // Disable interrupts during data transmission

    size_t colorComponentCount = colorOrder.size();

    // Send pixel data
    for (uint16_t i = 0; i < numLEDs; i++) {
        uint8_t* p = &ledData[i * colorComponentCount];
        uint8_t brightness = ledBrightness[i];

        // Send the pixel data
        sendPixel(p, brightness);
    }

    endTransfer();

    interrupts(); // Re-enable interrupts
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sends the color data for a single pixel.
///
void ShiftLED::sendPixel(uint8_t* colors, uint8_t brightness) {
    size_t colorComponentCount = colorOrder.size();

    // Adjust brightness
    uint8_t adjustedColors[8] = {0}; // Supports up to 8 color components
    uint16_t totalBrightness = ((uint16_t)brightness * actualGlobalBrightness) / 255;
    for (size_t i = 0; i < colorComponentCount; ++i) {
        adjustedColors[i] = (colors[i] * totalBrightness) / 255;
    }

    // Send color components, including 'NONE' components
    for (size_t i = 0; i < colorComponentCount; ++i) {
        uint8_t color = adjustedColors[i];

        // Send each bit of the byte, MSB first
        for (int8_t bit = 7; bit >= 0; bit--) {
            if (color & (1 << bit)) {
                one();
            } else {
                zero();
            }
        }
    }
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sends a logic '1' bit to the LEDs via SPI.
///
void ShiftLED::one() {
    SPI_peripheral.transfer(0xF8);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Sends a logic '0' bit to the LEDs via SPI.
///
void ShiftLED::zero() {
    SPI_peripheral.transfer(0xC0);
}

///---------------------------------------------------------------------------------------------------------------------
/// @brief Ends the data transmission by sending a reset signal.
///
void ShiftLED::endTransfer() {
    // Send reset code
    SPI_peripheral.transfer(0x00);
    delayMicroseconds(300); // Ensure data line is low for at least 50µs
}
