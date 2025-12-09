// display/display_manager.cpp
#include "display_manager.h"
#include "../config.h"

extern "C" {
    #include "hub75.h"
}

DisplayManager::DisplayManager() {
    // Create GFXMatrix object for 64x64 display
    matrix = new GFXMatrix(MATRIX_WIDTH, MATRIX_HEIGHT);
}

DisplayManager::~DisplayManager() {
    if (matrix != nullptr) {
        delete matrix;
    }
}

bool DisplayManager::init() {
    if (matrix == nullptr) {
        Serial.println("ERROR: Matrix object is null!");
        return false;
    }

    Serial.println("Initializing GFXMatrix display...");

    // Initialize the HUB75 hardware
    matrix->begin();

    // Clear the display
    matrix->clear();
    matrix->display();

    Serial.println("Display initialized successfully!");
    Serial.println("Pin Configuration (PCB_LAYOUT_V1):");
    Serial.println("  Data pins: GP0-GP5 (R0,G0,B0,R1,G1,B1)");
    Serial.println("  Row select: GP6-GP10 (A,B,C,D,E)");
    Serial.println("  LATCH: GP11, OE: GP12, CLK: GP13");

    return true;
}

void DisplayManager::clear() {
    if (matrix != nullptr) {
        matrix->clear();
    }
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // Bounds checking
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return;
    }

    if (matrix != nullptr) {
        matrix->drawPixel(x, y, color);
    }
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (matrix != nullptr) {
        matrix->fillRect(x, y, w, h, color);
    }
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (matrix != nullptr) {
        matrix->drawRect(x, y, w, h, color);
    }
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
    if (matrix != nullptr) matrix->setCursor(x, y);
}

void DisplayManager::setTextColor(uint16_t c) {
    if (matrix != nullptr) matrix->setTextColor(c);
}

void DisplayManager::setTextSize(uint8_t s) {
    if (matrix != nullptr) matrix->setTextSize(s);
}

void DisplayManager::print(const char* text) {
    if (matrix != nullptr) matrix->print(text);
}

void DisplayManager::print(int n) {
    if (matrix != nullptr) matrix->print(n);
}

void DisplayManager::update() {
    // Refresh the physical display
    if (matrix != nullptr) {
        matrix->display();
    }
}

void DisplayManager::setBrightness(uint8_t brightness) {
    // Note: GFXMatrix uses hub75_set_masterbrightness() from hub75.h
    // Range is 0 to DISPLAY_WIDTH-4 (0-60 for 64x64)
    // Map 0-255 to 0-60
    int hub75_brightness = (brightness * 60) / 255;
    
    Serial.print("Setting brightness: Input=");
    Serial.print(brightness);
    Serial.print(" -> HUB75=");
    Serial.println(hub75_brightness);

    hub75_set_masterbrightness(hub75_brightness);
    
    // Force update to apply new brightness bits to framebuffer
    update();
}

Adafruit_GFX* DisplayManager::getGFX() {
    return matrix;
}
