// sprites/sprite.h
// Sprite system with animation support for RGB565 displays
#ifndef SPRITE_H
#define SPRITE_H

#include <Arduino.h>
#include "../display/display_manager.h"

// Transparency marker - magenta
#define TRANSPARENT_COLOR 0xF81F  // RGB565 magenta (R=31, G=0, B=31)

// Single animation sequence
struct SpriteAnimation {
    const uint16_t* const* frames;  // Pointer to array of frame pointers (PROGMEM)
    uint8_t frame_count;            // Number of frames in this animation
    uint16_t frame_duration_ms;     // Milliseconds per frame
};

// Complete sprite definition
struct SpriteDefinition {
    uint8_t width;              // Sprite width in pixels (typically 8)
    uint8_t height;             // Sprite height in pixels (typically 8)
    SpriteAnimation idle;       // Idle animation
};

// Runtime sprite state (per-player instance)
struct SpriteInstance {
    const SpriteDefinition* definition;  // Pointer to sprite definition
    uint8_t current_frame;               // Current frame index
    uint32_t last_frame_time;            // millis() of last frame change
};

// Sprite renderer class
class SpriteRenderer {
public:
    static void initInstance(SpriteInstance* instance, const SpriteDefinition* def);

    // Update animation frame based on elapsed time
    static void updateAnimation(SpriteInstance* instance);

    // Render sprite at pixel coordinates (not cell coordinates)
    static void draw(DisplayManager* display, const SpriteInstance* instance,
                     int16_t x, int16_t y);

    // Render sprite with color tinting (replaces non-transparent pixels with tint_color)
    static void drawWithColorTint(DisplayManager* display, const SpriteInstance* instance,
                                   int16_t x, int16_t y, uint16_t tint_color);

private:
    // Get current frame pointer from animation
    static const uint16_t* getCurrentFrame(const SpriteInstance* instance);
};

#endif // SPRITE_H
