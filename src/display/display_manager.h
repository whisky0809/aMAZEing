// display/display_manager.h
// Display abstraction layer for RP2040Matrix library
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "GFXMatrix.h"

class DisplayManager {
private:
    GFXMatrix* matrix;

public:
    DisplayManager();
    ~DisplayManager();
    bool init();
    void clear();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    // Text support
    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t c);
    void setTextSize(uint8_t s);
    void print(const char* text);
    void print(int n);

    void update();  // Refresh display
    void setBrightness(uint8_t brightness);

    // Access to underlying GFX object for advanced drawing
    Adafruit_GFX* getGFX();
};

#endif // DISPLAY_MANAGER_H
