// config.h - Pin definitions and game constants
#ifndef CONFIG_H
#define CONFIG_H

// Debug mode - uncomment to enable verbose serial output
#define DEBUG_MODE

// Matrix dimensions
#define MATRIX_WIDTH  64
#define MATRIX_HEIGHT 64

// Pin mapping for HUB75 interface (PCB_LAYOUT_V1)
// These pins are hardcoded in the GFXMatrix library
// Data pins (consecutive): GP0-GP5
//   GP0 = R0, GP1 = G0, GP2 = B0
//   GP3 = R1, GP4 = G1, GP5 = B1
// Row select pins (consecutive): GP6-GP10
//   GP6 = A, GP7 = B, GP8 = C, GP9 = D, GP10 = E
// Control pins (must be consecutive):
//   GP11 = LATCH, GP12 = OE (Output Enable), GP13 = CLK (Clock)

// Joystick Pins (HW-504 module)
#define JOY_X_PIN    26  // ADC0
#define JOY_Y_PIN    27  // ADC1
#define JOY_BTN_PIN  22  // Button (active LOW)

// Joystick ADC thresholds (10-bit ADC: 0-1023, center ~512)
#define JOY_CENTER       512    // ADC center value
#define JOY_CENTER_MIN   400    // Dead zone lower bound
#define JOY_CENTER_MAX   624    // Dead zone upper bound
#define JOY_DEBOUNCE_MS  150    // Minimum ms between direction inputs
#define JOY_BTN_DEBOUNCE 50     // Button debounce time (ms)
#define JOY_LONG_PRESS   500    // Long press threshold for mode toggle (ms)

// Game Constants - RGB565 color format
#define PLAYER_COLOR   0xF800  // Red
#define GOAL_COLOR     0x07E0  // Green
#define PATH_COLOR     0x001F  // Blue
#define WALL_COLOR     0x0000  // Black
#define VISITED_COLOR  0x2104  // Dim gray

// Game settings
// Cell size in pixels (each maze cell is CELL_SIZE x CELL_SIZE pixels on screen)
#define CELL_SIZE      8

// Maze dimensions (derived from display size)
#ifdef MAZE_SIZE
#undef MAZE_SIZE
#endif
#define MAZE_SIZE      (MATRIX_WIDTH / CELL_SIZE)  // 64/8 = 8 cells
#define START_X        0
#define START_Y        0

// Two-player colors (RGB565)
#define PLAYER1_COLOR  0xF800  // Red (reuse existing PLAYER_COLOR)
#define PLAYER2_COLOR  0x07FF  // Cyan (high contrast with red)
#define P1_FOG_COLOR   0x001F  // Blue (reuse PATH_COLOR)
#define P2_FOG_COLOR   0x780F  // Purple (distinct from blue)
#define ACTIVE_HIGHLIGHT 0xFFFF // White border for active player

// Goal distance color progression (RGB565) - heat map: red=far, green=close
#define GOAL_DIST_FAR      0xF800  // Red - far away (>= 10 cells)
#define GOAL_DIST_MEDIUM   0xFD20  // Orange - medium distance (5-9 cells)
#define GOAL_DIST_CLOSE    0xFFE0  // Yellow - close (2-4 cells)
#define GOAL_DIST_ADJACENT 0x07E0  // Green - adjacent (1 cell)

// Blocked direction and goal accessibility indicators
#define BLOCKED_COLOR      0xF800  // Red - blocked path indicator
#define GOAL_ACCESSIBLE    0x001F  // Blue - goal is accessible (open door)

// Turn indicator UI position (top-right corner)
#define TURN_INDICATOR_X 56
#define TURN_INDICATOR_Y 2

// UI Text Constants
#define TEXT_TITLE_L1       "It's"
#define TEXT_TITLE_L2_PRE   "a"
#define TEXT_TITLE_L2_MAIN  "MAZE"
#define TEXT_TITLE_L2_SUF   "ing"

#define TEXT_MODE_1P        "1 PLAYER"
#define TEXT_MODE_2P        "2 PLAYER"
#define TEXT_TOGGLE_BTN     "T: MODE"

#define TEXT_PRESS_KEY      "WASD=START"
#define TEXT_TO_START       "TO START"
#define TEXT_YOU_WIN        "YOU WIN!"
#define TEXT_MOVES_LABEL    "MOVES:"
#define TEXT_PLAYER_PREFIX  "PLAYER "
#define TEXT_WINS_SUFFIX    "WINS!"
#define TEXT_P1_LABEL       "P1:"
#define TEXT_P2_LABEL       "P2:"

#endif // CONFIG_H