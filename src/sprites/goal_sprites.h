// sprites/goal_sprites.h
// Goal sprite pixel data - RGB565 format
// Animated doorway that changes color based on distance
#ifndef GOAL_SPRITES_H
#define GOAL_SPRITES_H

#include <Arduino.h>
#include "sprite.h"

// Color palette (will be replaced at runtime with distance-based colors)
#define GD   0xFFFF  // Goal Door frame (placeholder - will be tinted)
#define GI   0x2104  // Goal Interior (dark)
#define _T_  TRANSPARENT_COLOR  // Transparent

// Goal - Frame 0 (Door base)
const uint16_t PROGMEM GOAL_F0[] = {
    // Row 0 - Arch top
    _T_, _T_, GD,  GD,  GD,  GD,  _T_, _T_,
    // Row 1 - Arch sides
    _T_, GD,  GD,  GD,  GD,  GD,  GD,  _T_,
    // Row 2 - Door frame + interior
    GD,  GD,  GI,  GI,  GI,  GI,  GD,  GD,
    // Row 3 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 4 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 5 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 6 - Bottom frame
    GD,  GD,  GI,  GI,  GI,  GI,  GD,  GD,
    // Row 7 - Base
    GD,  GD,  GD,  GD,  GD,  GD,  GD,  GD,
};

// Goal - Frame 1 (Interior slightly brighter - subtle pulse)
const uint16_t PROGMEM GOAL_F1[] = {
    // Row 0 - Arch top
    _T_, _T_, GD,  GD,  GD,  GD,  _T_, _T_,
    // Row 1 - Arch sides
    _T_, GD,  GI,  GI,  GI,  GI,  GD,  _T_,
    // Row 2 - Door frame + interior (opens slightly)
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 3 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 4 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 5 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 6 - Bottom frame
    GD,  GD,  GI,  GI,  GI,  GI,  GD,  GD,
    // Row 7 - Base
    GD,  GD,  GD,  GD,  GD,  GD,  GD,  GD,
};

// Goal - Frame 2 (Interior brightest)
const uint16_t PROGMEM GOAL_F2[] = {
    // Row 0 - Arch top
    _T_, _T_, GD,  GD,  GD,  GD,  _T_, _T_,
    // Row 1 - Arch sides (more open)
    _T_, GD,  GI,  GI,  GI,  GI,  GD,  _T_,
    // Row 2 - Door frame + interior (more open)
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 3 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 4 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 5 - Interior
    GD,  GI,  GI,  GI,  GI,  GI,  GI,  GD,
    // Row 6 - Bottom frame
    GD,  GD,  GI,  GI,  GI,  GI,  GD,  GD,
    // Row 7 - Base
    GD,  GD,  GD,  GD,  GD,  GD,  GD,  GD,
};

// Frame pointer array for goal animation
const uint16_t* const PROGMEM GOAL_FRAMES[] = {
    GOAL_F0,
    GOAL_F1,
    GOAL_F2
};

// Goal sprite definition
const SpriteDefinition GOAL_SPRITE PROGMEM = {
    .width = 8,
    .height = 8,
    .idle = {
        .frames = GOAL_FRAMES,
        .frame_count = 3,
        .frame_duration_ms = 300  // Slower pulse (0.9s cycle)
    }
};

#endif // GOAL_SPRITES_H
