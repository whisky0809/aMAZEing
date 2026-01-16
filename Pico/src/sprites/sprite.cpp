// sprites/sprite.cpp
// Sprite renderer implementation
#include "sprite.h"

void SpriteRenderer::initInstance(SpriteInstance* instance, const SpriteDefinition* def) {
    instance->definition = def;
    instance->current_frame = 0;
    instance->last_frame_time = millis();
}

const uint16_t* SpriteRenderer::getCurrentFrame(const SpriteInstance* instance) {
    const SpriteAnimation* anim = &instance->definition->idle;

    // frames is a pointer to an array of pointers in PROGMEM
    const uint16_t* const* frame_array = anim->frames;
    return (const uint16_t*)pgm_read_ptr(&frame_array[instance->current_frame]);
}

void SpriteRenderer::updateAnimation(SpriteInstance* instance) {
    const SpriteAnimation* anim = &instance->definition->idle;
    uint32_t now = millis();


    if (now - instance->last_frame_time >= anim->frame_duration_ms) {
        instance->current_frame = (instance->current_frame + 1) % anim->frame_count;
        instance->last_frame_time = now;
    }
}

void SpriteRenderer::draw(DisplayManager* display, const SpriteInstance* instance,
                          int16_t x, int16_t y) {
    const uint16_t* frame = getCurrentFrame(instance);
    uint8_t w = instance->definition->width;
    uint8_t h = instance->definition->height;

    // Draw pixel by pixel with transparency check
    for (uint8_t py = 0; py < h; py++) {
        for (uint8_t px = 0; px < w; px++) {
            // Read color from PROGMEM
            uint16_t color = pgm_read_word(&frame[py * w + px]);

            // Skip transparent pixels
            if (color != TRANSPARENT_COLOR) {
                display->drawPixel(x + px, y + py, color);
            }
        }
    }
}

void SpriteRenderer::drawWithColorTint(DisplayManager* display, const SpriteInstance* instance,
                                        int16_t x, int16_t y, uint16_t tint_color) {
    const uint16_t* frame = getCurrentFrame(instance);
    uint8_t w = instance->definition->width;
    uint8_t h = instance->definition->height;

    // Draw pixel by pixel, replacing non-transparent pixels with tint color
    for (uint8_t py = 0; py < h; py++) {
        for (uint8_t px = 0; px < w; px++) {

            uint16_t pixel = pgm_read_word(&frame[py * w + px]);

            // Replace non-transparent pixels with tint color
            if (pixel != TRANSPARENT_COLOR) {
                display->drawPixel(x + px, y + py, tint_color);
            }
        }
    }
}
