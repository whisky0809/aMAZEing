# Two-Player Turn-Based Maze Game - Implementation Plan

## Overview

This plan details how to extend the Arduino maze game to support two players competing simultaneously in a turn-based manner. Players will share WASD controls, take turns making moves, have independent fog-of-war, but remain always visible to each other as they race to the same goal.

## User Requirements

- **Input**: Shared WASD controls with turn-based gameplay
- **Visibility**: Independent fog-of-war per player, but both players always visible
- **Win Condition**: Both players race to the same goal - first to reach wins
- **Turn Rules**: Only valid moves switch turns (invalid moves don't consume a turn)

## Key Technical Challenge

The maze uses a **memoryless procedural generation** design - it only stores `current_cell_dirs` (available exits) for the current position. Since two players can be at different positions simultaneously, each player needs their own `current_cell_dirs` state stored with their Player struct.

## Architecture Changes

### 1. Data Structure Modifications

#### config.h - New Constants
Add color constants for two-player mode:
```cpp
// Two-player colors (RGB565)
#define PLAYER1_COLOR  0xF800  // Red (reuse existing PLAYER_COLOR)
#define PLAYER2_COLOR  0x07FF  // Cyan (high contrast with red)
#define P1_FOG_COLOR   0x001F  // Blue (reuse PATH_COLOR)
#define P2_FOG_COLOR   0x780F  // Purple (distinct from blue)
#define ACTIVE_HIGHLIGHT 0xFFFF // White border for active player

// Turn indicator UI position (top-right corner)
#define TURN_INDICATOR_X 56
#define TURN_INDICATOR_Y 2
```

#### game_state.h - Enhanced Player Structure
Modify Player struct to include per-player maze state and move counter:
```cpp
struct Player {
    uint8_t x, y;              // Position (unchanged)
    uint16_t color;            // Player color (unchanged)
    uint8_t current_cell_dirs; // NEW: Per-player maze state (bitfield)
    uint16_t moves;            // NEW: Per-player move counter
};
```

#### game_state.h - GameState Class Updates
Replace single player with array and add turn management:
```cpp
private:
    Player players[2];         // NEW: Array of two players
    uint8_t active_player;     // NEW: 0 or 1 (whose turn)
    uint8_t winner;            // NEW: 0, 1, or 255 (no winner)
    bool two_player_mode;      // NEW: Mode toggle flag
    // Remove: Player player, uint16_t moves

public:
    void setTwoPlayerMode(bool enable);
    uint8_t getActivePlayer();
    // Modify getMoves() to getPlayerMoves(uint8_t player_id)
```

### 2. Game Logic Changes

#### game_state.cpp - resetGame()
Initialize both players at the same starting position:
```cpp
void GameState::resetGame() {
    maze.init();

    // Randomize start position
    uint8_t sx = random(0, MAZE_SIZE);
    uint8_t sy = random(0, MAZE_SIZE);

    // Initialize Player 1
    players[0].x = sx;
    players[0].y = sy;
    players[0].color = PLAYER1_COLOR;
    players[0].moves = 0;
    maze.generateNewDirections(sx, sy);
    players[0].current_cell_dirs = maze.getCurrentDirections();

    if (two_player_mode) {
        // Initialize Player 2 at same position
        players[1].x = sx;
        players[1].y = sy;
        players[1].color = PLAYER2_COLOR;
        players[1].moves = 0;
        players[1].current_cell_dirs = players[0].current_cell_dirs;
    }

    // Set goal (far from start) - unchanged logic
    // ... existing goal randomization code ...

    active_player = 0;  // Player 1 starts
    winner = 255;       // No winner yet
    state = STATE_PLAYING;
}
```

#### game_state.cpp - handleInput() Refactoring
Split into mode-specific handlers:
```cpp
void GameState::handleInput(Direction dir) {
    if (dir == DIR_NONE) return;

    switch (state) {
        case STATE_START:
            resetGame();
            break;

        case STATE_PLAYING:
            if (two_player_mode) {
                handleTwoPlayerMove(dir);
            } else {
                handleSinglePlayerMove(dir);
            }
            break;

        case STATE_WIN:
            state = STATE_START;
            break;
    }
}
```

#### New Helper Functions

**handleSinglePlayerMove()** - Extract existing logic:
```cpp
void GameState::handleSinglePlayerMove(Direction dir) {
    Player& p = players[0];

    if (isValidMove(p, dir)) {
        movePlayer(p, dir);
        p.moves++;
        maze.generateNewDirections(p.x, p.y);
        p.current_cell_dirs = maze.getCurrentDirections();

        if (maze.isGoal(p.x, p.y)) {
            state = STATE_WIN;
            winner = 0;
        }
    }
}
```

**handleTwoPlayerMove()** - Turn-based logic:
```cpp
void GameState::handleTwoPlayerMove(Direction dir) {
    Player& p = players[active_player];

    if (isValidMove(p, dir)) {
        // Valid move: execute and switch turns
        movePlayer(p, dir);
        p.moves++;
        maze.generateNewDirections(p.x, p.y);
        p.current_cell_dirs = maze.getCurrentDirections();

        if (maze.isGoal(p.x, p.y)) {
            state = STATE_WIN;
            winner = active_player;
            return;
        }

        // Switch turns (toggle between 0 and 1)
        active_player = 1 - active_player;
    }
    // Invalid moves are ignored - no turn switch
}
```

**isValidMove()** - Check bounds and maze state:
```cpp
bool GameState::isValidMove(const Player& p, Direction dir) {
    // Check grid bounds
    switch (dir) {
        case NORTH: if (p.y == 0) return false; break;
        case SOUTH: if (p.y >= MAZE_SIZE - 1) return false; break;
        case EAST:  if (p.x >= MAZE_SIZE - 1) return false; break;
        case WEST:  if (p.x == 0) return false; break;
        default: return false;
    }

    // Check player's current cell allows this direction
    return (p.current_cell_dirs & (1 << dir)) != 0;
}
```

**movePlayer()** - Apply movement:
```cpp
void GameState::movePlayer(Player& p, Direction dir) {
    switch (dir) {
        case NORTH: p.y--; break;
        case SOUTH: p.y++; break;
        case EAST:  p.x++; break;
        case WEST:  p.x--; break;
    }
}
```

### 3. Rendering System

#### game_state.cpp - render() Routing
```cpp
void GameState::render(DisplayManager* display) {
    display->clear();

    if (state == STATE_START) {
        renderStartScreen(display);
        return;
    }

    if (state == STATE_WIN) {
        renderWinScreen(display);
        return;
    }

    // STATE_PLAYING
    if (two_player_mode) {
        renderTwoPlayer(display);
    } else {
        renderSinglePlayer(display);  // Existing logic
    }

    display->update();
}
```

#### renderTwoPlayer() - Independent Fog-of-War
```cpp
void GameState::renderTwoPlayer(DisplayManager* display) {
    // 1. Render Player 1's fog-of-war (blue)
    renderPlayerFog(display, players[0], P1_FOG_COLOR);

    // 2. Render Player 2's fog-of-war (purple)
    renderPlayerFog(display, players[1], P2_FOG_COLOR);

    // 3. Draw goal (always visible, shared)
    renderGoal(display);

    // 4. Draw both players (always visible)
    display->fillRect(players[0].x * 2, players[0].y * 2, 2, 2, players[0].color);
    display->fillRect(players[1].x * 2, players[1].y * 2, 2, 2, players[1].color);

    // 5. Draw turn indicator (corner square)
    display->fillRect(TURN_INDICATOR_X, TURN_INDICATOR_Y, 4, 4,
                      players[active_player].color);
}
```

#### renderPlayerFog() - Draw Available Paths
```cpp
void GameState::renderPlayerFog(DisplayManager* display, const Player& p,
                                 uint16_t fog_color) {
    uint8_t dirs = p.current_cell_dirs;
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();

    // Draw each valid adjacent cell (don't overdraw goal)
    if ((dirs & (1 << NORTH)) && p.y > 0) {
        if (!(p.x == gx && p.y - 1 == gy))
            display->fillRect(p.x * 2, (p.y - 1) * 2, 2, 2, fog_color);
    }
    if ((dirs & (1 << SOUTH)) && p.y < MAZE_SIZE - 1) {
        if (!(p.x == gx && p.y + 1 == gy))
            display->fillRect(p.x * 2, (p.y + 1) * 2, 2, 2, fog_color);
    }
    if ((dirs & (1 << EAST)) && p.x < MAZE_SIZE - 1) {
        if (!(p.x + 1 == gx && p.y == gy))
            display->fillRect((p.x + 1) * 2, p.y * 2, 2, 2, fog_color);
    }
    if ((dirs & (1 << WEST)) && p.x > 0) {
        if (!(p.x - 1 == gx && p.y == gy))
            display->fillRect((p.x - 1) * 2, p.y * 2, 2, 2, fog_color);
    }
}
```

#### renderGoal() - Shared Goal with Dual Adjacency Check
Extract goal rendering logic and check both players for adjacency:
```cpp
void GameState::renderGoal(DisplayManager* display) {
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();

    bool p1_can_reach = false;
    bool p2_can_reach = false;

    // Check if Player 1 can reach goal
    if (isAdjacent(players[0], gx, gy)) {
        p1_can_reach = canReachGoal(players[0], gx, gy);
    }

    // Check if Player 2 can reach goal (if in two-player mode)
    if (two_player_mode && isAdjacent(players[1], gx, gy)) {
        p2_can_reach = canReachGoal(players[1], gx, gy);
    }

    if (p1_can_reach || p2_can_reach) {
        // Checkerboard pattern (reachable)
        display->drawPixel(gx * 2, gy * 2, GOAL_COLOR);
        display->drawPixel(gx * 2 + 1, gy * 2, PATH_COLOR);
        display->drawPixel(gx * 2, gy * 2 + 1, PATH_COLOR);
        display->drawPixel(gx * 2 + 1, gy * 2 + 1, GOAL_COLOR);
    } else {
        // Solid green (visible but not adjacent)
        display->fillRect(gx * 2, gy * 2, 2, 2, GOAL_COLOR);
    }
}

bool GameState::isAdjacent(const Player& p, uint8_t gx, uint8_t gy) {
    int dx = abs((int)p.x - (int)gx);
    int dy = abs((int)p.y - (int)gy);
    return (dx + dy == 1);
}

bool GameState::canReachGoal(const Player& p, uint8_t gx, uint8_t gy) {
    uint8_t dirs = p.current_cell_dirs;
    if (gx > p.x && (dirs & (1 << EAST))) return true;
    if (gx < p.x && (dirs & (1 << WEST))) return true;
    if (gy > p.y && (dirs & (1 << SOUTH))) return true;
    if (gy < p.y && (dirs & (1 << NORTH))) return true;
    return false;
}
```

#### renderWinScreen() - Show Winner and Move Counts
```cpp
void GameState::renderWinScreen(DisplayManager* display) {
    if (!two_player_mode) {
        // Existing single-player win screen
        display->setTextColor(GOAL_COLOR);
        display->setCursor(10, 10);
        display->print("YOU WIN!");
        display->setTextColor(0xFFFF);
        display->setCursor(10, 25);
        display->print("MOVES:");
        display->print(players[0].moves);
    } else {
        // Two-player win screen
        display->setTextColor(GOAL_COLOR);
        display->setCursor(4, 5);
        display->print("PLAYER ");
        display->print(winner + 1);
        display->setCursor(10, 18);
        display->print("WINS!");

        // Show both move counts
        display->setTextColor(PLAYER1_COLOR);
        display->setCursor(4, 32);
        display->print("P1:");
        display->print(players[0].moves);

        display->setTextColor(PLAYER2_COLOR);
        display->setCursor(4, 42);
        display->print("P2:");
        display->print(players[1].moves);
    }

    display->setTextColor(PLAYER_COLOR);
    display->setCursor(4, 54);
    display->print("PRESS KEY");
}
```

### 4. Input Handling

#### serial_input.h - Add Mode Toggle
```cpp
class SerialInput {
private:
    bool reset_requested;
    bool mode_toggle_requested;  // NEW

public:
    SerialInput();
    Direction getCommand();
    bool isResetRequested();
    bool isToggleModeRequested();  // NEW
};
```

#### serial_input.cpp - Handle 'T' Key
```cpp
SerialInput::SerialInput() {
    reset_requested = false;
    mode_toggle_requested = false;  // NEW
}

Direction SerialInput::getCommand() {
    if (!Serial.available()) return DIR_NONE;

    char cmd = toupper(Serial.read());
    while (Serial.available()) Serial.read(); // Flush

    switch (cmd) {
        case 'W': return NORTH;
        case 'A': return WEST;
        case 'S': return SOUTH;
        case 'D': return EAST;
        case 'R':
            reset_requested = true;
            return DIR_NONE;
        case 'T':  // NEW: Toggle two-player mode
            mode_toggle_requested = true;
            return DIR_NONE;
        default: return DIR_NONE;
    }
}

bool SerialInput::isToggleModeRequested() {
    if (mode_toggle_requested) {
        mode_toggle_requested = false;
        return true;
    }
    return false;
}
```

#### main.cpp - Mode Toggle in Main Loop
```cpp
void loop() {
    Direction dir = input.getCommand();

    if (dir != DIR_NONE) {
        game.handleInput(dir);
    }

    if (input.isResetRequested()) {
        Serial.println("========== GAME RESET ==========");
        game.init();
    }

    // NEW: Handle mode toggle
    if (input.isToggleModeRequested()) {
        static bool two_player = false;
        two_player = !two_player;
        game.setTwoPlayerMode(two_player);
        Serial.print("Two-player mode: ");
        Serial.println(two_player ? "ON" : "OFF");
        game.init();  // Restart with new mode
    }

    game.update();
    game.render(&display);

    delay(16); // ~60 FPS
}
```

## Implementation Sequence

1. **config.h** - Add new color constants and UI position defines
2. **game_state.h** - Modify Player struct, update GameState class members
3. **game_state.cpp** - Core logic refactoring:
   - Update resetGame() for two players
   - Refactor handleInput() and create helper functions
   - Extract renderSinglePlayer() from existing render()
   - Create renderTwoPlayer() and all fog-of-war helpers
   - Update win screen rendering
4. **serial_input.h/cpp** - Add mode toggle flag and 'T' key handling
5. **main.cpp** - Add mode toggle check in loop()

## Testing Checklist

### Single-Player (Regression)
- [ ] Game starts in single-player mode by default
- [ ] WASD movement works
- [ ] Fog-of-war shows adjacent paths
- [ ] Win detection works
- [ ] Move counter accurate

### Two-Player Mode
- [ ] Toggle with 'T' key works
- [ ] Both players start at same position
- [ ] Players have different colors (red vs cyan)
- [ ] Turn indicator shows active player
- [ ] Valid moves switch turns
- [ ] Invalid moves don't switch turns
- [ ] Each player has independent fog colors
- [ ] Both players always visible
- [ ] Goal always visible
- [ ] First to goal wins
- [ ] Win screen shows correct winner and both move counts

## Memory Impact

- **Before**: ~22 bytes (single Player struct + game state)
- **After**: ~51 bytes (Player[2] with maze state + turn management)
- **Net increase**: ~29 bytes (negligible on RP2040's 264KB RAM)

## Files to Modify

1. `src/config.h` - +8 lines (color constants)
2. `src/game/game_state.h` - Modify Player struct, update class (15 additions, 5 mods)
3. `src/game/game_state.cpp` - Major refactoring (~200 lines added/modified)
4. `src/input/serial_input.h` - +1 line (method declaration)
5. `src/input/serial_input.cpp` - +10 lines (toggle handling)
6. `src/main.cpp` - +8 lines (mode toggle in loop)

## Color Scheme

- **Player 1**: Red (0xF800)
- **Player 2**: Cyan (0x07FF) - high contrast
- **P1 Fog**: Blue (0x001F)
- **P2 Fog**: Purple (0x780F) - distinct but complementary
- **Goal**: Green (0x07E0) - unchanged
- **Turn Indicator Border**: White (0xFFFF)
