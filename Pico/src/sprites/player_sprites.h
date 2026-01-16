// sprites/player_sprites.h
// Player sprite pixel data - RGB565 format
// Each sprite is 8x8 pixels = 64 uint16_t values per frame
// Layout: Row-major order (left to right, top to bottom)
#ifndef PLAYER_SPRITES_H
#define PLAYER_SPRITES_H

#include <Arduino.h>
#include "sprite.h"

// =============================================================================
// GLOBAL COLORS
// =============================================================================
#define WH   0xFFFF  // White - Eye
#define BK   0x0000  // Black - Eye pupil
#define _T_  TRANSPARENT_COLOR  // Transparent

// =============================================================================
// PLAYER 1 SPRITE (Red theme)
// =============================================================================

// Color palette
#define B1   0x5F0B  // Body - Green (matching PLAYER1_COLOR)
#define D1   0x7005  // Dark - shadow/outline
#define L1   0xF80E  // Light - highlight
#define S1   0xFE73  // Skin - head (Unused in new design)

// Player 1 - Frame 0 (Left Arm Down, Right Arm Up)
const uint16_t PROGMEM P1_IDLE_F0[] = {
    // Row 0 
    _T_, _T_, B1,  B1,  B1,  B1,  _T_, _T_,
    // Row 1
    _T_, B1, B1,  WH,  BK,  B1,  B1, _T_,
    // Row 2
    _T_, B1,  B1,  BK,  BK,  B1,  B1,  _T_,
    // Row 3
    _T_,  _T_,  B1,  B1,  B1,  B1,  _T_,  B1,
    // Row 4
    B1,  B1,  B1,  B1,  B1,  B1,  B1,  B1,
    // Row 5
    B1, _T_,  B1,  B1,  B1,  B1,  _T_,  _T_,
    // Row 6
    _T_, _T_, B1,  _T_, _T_, B1,  B1,  _T_,
    // Row 7
    _T_, B1,  B1,  _T_, _T_, _T_, _T_, _T_,
};

// Player 1 - Frame 1 (Right Arm Down, Left Arm Up)
const uint16_t PROGMEM P1_IDLE_F1[] = {
    // Row 0 
    _T_, _T_, B1,  B1,  B1,  B1,  _T_, _T_,
    // Row 1
    _T_, B1, B1,  BK,  WH,  B1,  B1, _T_,
    // Row 2
    _T_, B1,  B1,  BK,  BK,  B1,  B1,  _T_,
    // Row 3
    B1,  _T_, B1,  B1,  B1,  B1,  _T_, _T_,
    // Row 4
    B1,  B1,  B1,  B1,  B1,  B1,  B1,  B1,
    // Row 5
    _T_, _T_, B1,  B1,  B1,  B1,  _T_, B1,
    // Row 6
    _T_, B1,  B1,  _T_, _T_, B1,  _T_, _T_,
    // Row 7
    _T_, _T_, _T_, _T_, _T_, B1,  B1,  _T_,
};

// Frame pointer array for Player 1 idle animation
const uint16_t* const PROGMEM P1_IDLE_FRAMES[] = {
    P1_IDLE_F0,
    P1_IDLE_F1
};

// =============================================================================
// PLAYER 2 SPRITE (Cyan theme)
// =============================================================================

// Color palette for Player 2
#define B2   0xF800  // Body - Red (matching PLAYER2_COLOR)
#define D2   0x0410  // Dark - shadow/outline
#define L2   0x7FFF  // Light - highlight
#define S2   0xFE73  // Skin - head (Unused)

// Player 2 - Frame 0 (Same shape as P1 F0)
const uint16_t PROGMEM P2_IDLE_F0[] = {
    // Row 0
    _T_, _T_, B2,  B2,  B2,  B2,  _T_, _T_,
    // Row 1
    _T_, B2, B2,  WH,  BK,  B2,  B2, _T_,
    // Row 2
    _T_, B2,  B2,  BK,  BK,  B2,  B2,  _T_,
    // Row 3
    _T_,  _T_,  B2,  B2,  B2,  B2,  _T_,  B2,
    // Row 4
    B2,  B2,  B2,  B2,  B2,  B2,  B2,  B2,
    // Row 5
    B2, _T_,  B2,  B2,  B2,  B2,  _T_,  _T_,
    // Row 6
    _T_, _T_, B2,  _T_, _T_, B2,  B2,  _T_,
    // Row 7
    _T_, B2,  B2,  _T_, _T_, _T_, _T_, _T_,
};

// Player 2 - Frame 1 (Same shape as P1 F1)
const uint16_t PROGMEM P2_IDLE_F1[] = {
    // Row 0
    _T_, _T_, B2,  B2,  B2,  B2,  _T_, _T_,
    // Row 1 - Mirrored
    _T_, B2, B2,  BK,  WH,  B2,  B2, _T_,
    // Row 2 - Mirrored (same)
    _T_, B2,  B2,  BK,  BK,  B2,  B2,  _T_,
    // Row 3 - Mirrored
    B2,  _T_, B2,  B2,  B2,  B2,  _T_, _T_,
    // Row 4 - Mirrored (same)
    B2,  B2,  B2,  B2,  B2,  B2,  B2,  B2,
    // Row 5 - Mirrored
    _T_, _T_, B2,  B2,  B2,  B2,  _T_, B2,
    // Row 6 - Mirrored
    _T_, B2,  B2,  _T_, _T_, B2,  _T_, _T_,
    // Row 7 - Mirrored
    _T_, _T_, _T_, _T_, _T_, B2,  B2,  _T_,
};

// Frame pointer array for Player 2 idle animation
const uint16_t* const PROGMEM P2_IDLE_FRAMES[] = {
    P2_IDLE_F0,
    P2_IDLE_F1
};

// =============================================================================
// SPRITE DEFINITIONS (Combine frames into complete definitions)
// =============================================================================

// Player 1 complete sprite definition
const SpriteDefinition PLAYER1_SPRITE PROGMEM = {
    .width = 8,
    .height = 8,
    .idle = {
        .frames = P1_IDLE_FRAMES,
        .frame_count = 2,
        .frame_duration_ms = 150  // Fast animation (0.3s cycle)
    }
};

// Player 2 complete sprite definition
const SpriteDefinition PLAYER2_SPRITE PROGMEM = {
    .width = 8,
    .height = 8,
    .idle = {
        .frames = P2_IDLE_FRAMES,
        .frame_count = 2,
        .frame_duration_ms = 150  // Fast animation (0.3s cycle)
    }
};

#endif // PLAYER_SPRITES_H
