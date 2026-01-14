// config.h - Pin definitions and game constants
#ifndef CONFIG_H
#define CONFIG_H

// Debug mode - uncomment to enable verbose serial output
// #define DEBUG_MODE

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

// UART pins for R4 communication (UART0 on GP16/GP17)
#define UART_TX_PIN  16      // GP16 Sends back to R4
#define UART_RX_PIN  17      // GP17 receives from R4 TX
#define UART_BAUD    115200

// R4 binary packet protocol
#define UART_HEADER_0   0xAA
#define UART_HEADER_1   0x55
#define UART_PACKET_LEN 8

// Joystick thresholds (raw ADC 0-1023, center ~512)
#define JOY_CENTER      512
#define JOY_DEADZONE    100  // Must exceed this from center to register direction

// Button mask bits in packet byte 6
#define BTN_MASK_JSW    (1 << 0)  // Joystick switch
#define BTN_MASK_MAIN   (1 << 1)  // Main button

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
#define STATUS_BAR_HEIGHT 8
#define MAZE_OFFSET_Y     8

#define MAZE_WIDTH        (MATRIX_WIDTH / CELL_SIZE)   // 64/8 = 8 cells
#define MAZE_HEIGHT       7                            // 7 cells (56 pixels high)

#define START_X        0
#define START_Y        0

// Two-player colors (RGB565)
#define PLAYER1_COLOR  0x5F0B  // Green
#define PLAYER2_COLOR  0xF800  // Magenta/Red
#define P1_FOG_COLOR   0x001F  // Blue
#define P2_FOG_COLOR   0x780F  // Purple
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

// UI Text Constants (Start Screen)
#define TEXT_TITLE_L1       "It's"
#define TEXT_TITLE_L2_PRE   "a"
#define TEXT_TITLE_L2_MAIN  "MAZE"
#define TEXT_TITLE_L2_SUF   "ing"
#define TEXT_PRESS_KEY      "Press red"

// Goal Message Text (shown when goal is reached)
#define TEXT_GOAL_L1        "A little"
#define TEXT_GOAL_L2        "bit more"

// Win Screen Text (shown when power switch triggers win)
#define TEXT_WIN_ESCAPED    " ESCAPED!"  // Prefixed with P1/P2
#define TEXT_WIN_WANTS      " wants"     // Prefixed with P2/P1 (other player)
#define TEXT_WIN_L3         "one more"
#define TEXT_WIN_L4         "rabbit"
#define TEXT_WIN_L5         "hole..."

// Minigame Prompt Text (shown after valid move)
#define TEXT_MINIGAME_L1    "Press"
#define TEXT_MINIGAME_L2    "round"
#define TEXT_MINIGAME_L3    "green"
#define TEXT_MINIGAME_L4    "button"

#endif // CONFIG_H