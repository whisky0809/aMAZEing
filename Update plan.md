# aMAZEing Turn-Based Mode Implementation Plan

## Overview
Implement proper turn-based gameplay where players alternate between maze moves and ProtoPie minigames, with bidirectional UART for move validation, button-based mode switching, and a dedicated status bar display.

## Requirements
1. **Turn-based flow**: Player 1 moves → minigame → Player 2 moves → minigame → repeat
2. **Invalid moves don't trigger swap**: Only valid maze moves should advance the turn
3. **Button triggers swap back**: Player presses button when minigame complete
4. **Built-in Arduino button as reset**: Reset entire system to starting state
5. **Status bar display**: Top 8 rows for text (turn indicator, button hints)
6. **Update all documentation**: README.md and PINOUT.md

---

## Architecture: Button-Based Control (Simple, No WiFi)

**Communication Flow:**
```
┌─────────────┐                                    ┌─────────────┐
│ R4 Arduino  │────── USB HID (keyboard/mouse) ───►│    iPad     │
│   (UNO R4   │                                    │  ProtoPie   │
│    WiFi)    │                                    └─────────────┘
└──────┬──────┘
       │ UART Bidirectional (GP16 TX ↔ GP17 RX)
       ▼
┌─────────────┐
│    Pico     │───► 64x64 LED Matrix
│  (Maze)     │
└─────────────┘
```

**Key Insight**: Instead of complex WiFi/Socket.IO integration, use a simple button press to signal "minigame done". This keeps the implementation straightforward and reliable.

---

## Implementation Plan

### Phase 1: Pico Firmware - Move Validation Response

**Files**: `Pico/src/main.cpp`, `Pico/src/game/game_state.cpp`

1. Initialize UART0 TX on GP16 (already wired to R4 RX)
2. After processing a move attempt, send single-byte response:
   - `V` = Valid move executed
   - `I` = Invalid move (hit wall/boundary)
   - `W` = Win condition (player reached goal)

**Changes needed:**
- Add `Serial1.begin()` or configure UART0 TX
- Modify move processing to return success/failure
- Send response byte after each move attempt

### Phase 2: R4 Firmware - Receive Move Validation

**File**: `R4/R4_PICO_LATEST.ino`

1. Add `Serial1.read()` to receive Pico responses
2. Only switch to minigame mode if move was VALID
3. Track current player (P1 or P2)

**New state machine:**
```
MINIGAME_MODE (ProtoPie mouse control, maze frozen)
    ↓ [BTN_A press - "take my turn"]
MAZE_ARMED (waiting for valid joystick direction)
    ↓ [joystick direction detected]
MAZE_PULSING (sending move pulse to Pico, 70ms)
    ↓ [wait for Pico response]
WAITING_ACK
    ├─► 'V' received: switch to MINIGAME_MODE, swap player
    ├─► 'I' received: stay in MAZE_ARMED (try again)
    └─► 'W' received: WIN state, display winner
```

### Phase 3: Button Mappings

**File**: `R4/R4_PICO_LATEST.ino`

| Button | Function |
|--------|----------|
| **BTN_A (GPIO 2)** | "Take my turn" - arm maze move |
| Joystick Switch (GPIO 6) | Reserved / unused (or toggle debug mode) |
| BTN_ENTER (GPIO 7) | Send ENTER to ProtoPie |
| Built-in USER | Full system reset |
| BTN_B/C/D | Send b/c/d keys to ProtoPie |

### Phase 4: Built-in Reset Button

**File**: `R4/R4_PICO_LATEST.ino`

The R4 WiFi has a USER button. Note: On R4 WiFi, this might be on a different pin than D13.

1. Identify correct pin for USER button
2. Add pin setup with INPUT_PULLUP
3. On press (debounced):
   - Send reset command `R` to Pico via UART
   - Send reset key `r` to ProtoPie
   - Reset R4 state: currentPlayer = P1, state = MINIGAME_MODE

### Phase 5: Pico Reset Handler

**File**: `Pico/src/main.cpp`

1. Listen for `R` byte on UART RX
2. When received, call `game.reset()` to restart maze

### Phase 6: Display Layout Change - Status Bar

**Files**: `Pico/src/config.h`, `Pico/src/game/game_state.cpp`

**New Layout**:
```
┌────────────────────────────────────────────────────────────────┐ Row 0
│                    STATUS BAR (8 pixels)                       │
│  "P1 TURN" / "P2 TURN" / "P1 MOVE!" / "MINIGAME" / etc.       │
├────────────────────────────────────────────────────────────────┤ Row 8
│                                                                │
│                    MAZE AREA (56 pixels)                       │
│                    8 columns × 7 rows                          │
│                    (cell size still 8×8)                       │
│                                                                │
└────────────────────────────────────────────────────────────────┘ Row 63
```

**Changes needed:**

1. **config.h** - Update constants:
   ```cpp
   #define STATUS_BAR_HEIGHT 8
   #define MAZE_OFFSET_Y 8       // Maze starts at row 8
   #define MAZE_HEIGHT 7         // 7 rows instead of 8
   #define MAZE_WIDTH 8          // 8 columns (unchanged)
   ```

2. **game_state.cpp** - Status bar rendering:
   ```cpp
   void GameState::renderStatusBar(DisplayManager* display) {
     display->fillRect(0, 0, 64, STATUS_BAR_HEIGHT, 0x0000);  // Clear bar
     display->setTextSize(1);

     if (state == STATE_PLAYING) {
       // Show whose turn + hint
       if (/* waiting for BTN_A */) {
         display->setTextColor(players[active_player].color);
         display->setCursor(2, 0);
         display->print("P");
         display->print(active_player + 1);
         display->print(" PRESS A");
       } else if (/* armed for move */) {
         display->setCursor(2, 0);
         display->print("P");
         display->print(active_player + 1);
         display->print(" MOVE!");
       }
     }
   }
   ```

3. **Maze rendering offset** - Add `MAZE_OFFSET_Y` to all Y coordinates:
   - Player sprites: `p.y * CELL_SIZE + MAZE_OFFSET_Y`
   - Goal: `gy * CELL_SIZE + MAZE_OFFSET_Y`
   - Fog, barriers, etc.

4. **Maze generation** - Use `MAZE_HEIGHT` (7) instead of `MAZE_SIZE` for Y bounds

### Phase 7: Documentation Updates

**Files to create/update:**

1. **`aMAZEing/README.md`** (NEW - top-level project overview):
   - Project description & gameplay overview
   - System architecture diagram (R4 ↔ Pico ↔ iPad)
   - Hardware requirements (all components)
   - Quick start guide (setup order)
   - Folder structure explanation
   - Links to component-specific READMEs

2. **`Pico/README.md`** - Update with:
   - New turn-based flow
   - UART protocol (V/I/W responses, R reset command)
   - Display layout (8px status bar + 8×7 maze)
   - Button controls from R4 perspective

3. **`Pico/PINOUT.md`** - Add:
   - GP16 TX → R4 RX connection for bidirectional UART

4. **`R4/README.md`** (NEW):
   - R4 firmware purpose & setup
   - Button mappings table
   - State machine explanation
   - Serial debug commands

---

## Detailed Code Changes

### R4_PICO_LATEST.ino - Key Changes

```cpp
// NEW: Reset button (verify pin for R4 WiFi USER button)
static const int BTN_RESET = 13;  // May need adjustment
static uint32_t lastReset = 0;

// NEW: Player tracking
enum Player { PLAYER_1, PLAYER_2 };
static Player currentPlayer = PLAYER_1;

// UPDATED: Enhanced state machine
enum ControlState {
  MINIGAME_MODE,     // ProtoPie mouse control, maze frozen, waiting for BTN_A
  MAZE_ARMED,        // BTN_A pressed, waiting for joystick direction
  MAZE_PULSING,      // Sending move pulse to Pico
  WAITING_ACK        // Waiting for Pico's V/I/W response
};
static ControlState state = MINIGAME_MODE;

// BTN_A triggers arm (instead of sending 'a' to ProtoPie in MINIGAME_MODE)
void handleBtnA(uint32_t now) {
  if (digitalRead(BTN_A) == LOW && (now - lastA >= BTN_DEBOUNCE_MS)) {
    if (state == MINIGAME_MODE) {
      state = MAZE_ARMED;
      Serial.print("[ARMED] Player ");
      Serial.println(currentPlayer == PLAYER_1 ? "1" : "2");
    }
    lastA = now;
  }
}

// NEW: Check for Pico response
void checkPicoResponse() {
  while (Serial1.available()) {
    char response = Serial1.read();
    if (state == WAITING_ACK) {
      if (response == 'V') {
        // Valid move - switch player, go to minigame
        currentPlayer = (currentPlayer == PLAYER_1) ? PLAYER_2 : PLAYER_1;
        Keyboard.write('v');  // Tell ProtoPie: valid move, start minigame
        state = MINIGAME_MODE;
        Serial.print("[VALID] Now Player ");
        Serial.println(currentPlayer == PLAYER_1 ? "1" : "2");
      } else if (response == 'I') {
        // Invalid move - stay armed, player can retry
        state = MAZE_ARMED;
        Serial.println("[INVALID] Try another direction");
      } else if (response == 'W') {
        // Win condition
        Keyboard.write('w');  // Tell ProtoPie: game won
        state = MINIGAME_MODE;
        Serial.println("[WIN!]");
      }
    }
  }
}

// NEW: Handle reset button
void checkResetButton(uint32_t now) {
  if (digitalRead(BTN_RESET) == LOW && (now - lastReset >= BTN_DEBOUNCE_MS)) {
    // Send reset to Pico
    Serial1.write('R');
    // Send reset to ProtoPie
    Keyboard.write('r');
    // Reset local state
    currentPlayer = PLAYER_1;
    state = MINIGAME_MODE;
    lastReset = now;
    Serial.println("[RESET] Full system reset");
  }
}
```

### Pico Changes

**src/main.cpp** - Add UART TX and response sending:
```cpp
// In setup():
Serial1.begin(115200);  // TX on GP16, RX on GP17

// NEW: Send move result to R4
void sendMoveResponse(char response) {
  Serial1.write(response);
}

// NEW: Check for reset command
void checkResetCommand() {
  if (Serial1.available()) {
    char cmd = Serial1.read();
    if (cmd == 'R') {
      game.reset();
      Serial.println("[RESET] Game reset via UART");
    }
  }
}
```

**src/game/game_state.h** - Add method to check last move result:
```cpp
// Add to GameState class public section:
enum MoveResult { MOVE_NONE, MOVE_VALID, MOVE_INVALID, MOVE_WIN };
MoveResult getLastMoveResult();
```

**src/game/game_state.cpp** - Track and report move result:
```cpp
// Add member variable:
MoveResult lastMoveResult = MOVE_NONE;

// Modify handleTwoPlayerMove() (line 226):
void GameState::handleTwoPlayerMove(Direction dir) {
    Player& p = players[active_player];

    if (isValidMove(p, dir)) {
        movePlayer(p, dir);
        p.moves++;
        maze.generateNewDirections(p.x, p.y);
        p.current_cell_dirs = maze.getCurrentDirections();

        if (maze.isGoal(p.x, p.y)) {
            state = STATE_WIN;
            winner = active_player;
            lastMoveResult = MOVE_WIN;  // NEW
            return;
        }
        active_player = 1 - active_player;
        lastMoveResult = MOVE_VALID;  // NEW
    } else {
        lastMoveResult = MOVE_INVALID;  // NEW
    }
}

MoveResult GameState::getLastMoveResult() {
    MoveResult result = lastMoveResult;
    lastMoveResult = MOVE_NONE;  // Clear after read
    return result;
}
```

---

## Files to Modify

| File | Changes |
|------|---------|
| `R4/R4_PICO_LATEST.ino` | Player tracking, reset button, new state machine, Pico ACK handling |
| `Pico/src/main.cpp` | UART TX init, reset command handler, send move responses |
| `Pico/src/config.h` | Add STATUS_BAR_HEIGHT, MAZE_OFFSET_Y, change MAZE_HEIGHT to 7 |
| `Pico/src/game/game_state.h` | Add MoveResult enum, getLastMoveResult(), renderStatusBar() |
| `Pico/src/game/game_state.cpp` | Track move result, add status bar rendering, apply Y offset to all maze rendering |
| `Pico/src/game/maze_generator.cpp` | Use MAZE_HEIGHT (7) for Y bounds instead of MAZE_SIZE |
| `Pico/README.md` | Document turn-based flow, button controls, UART protocol, new display layout |
| `Pico/PINOUT.md` | Add GP16 TX → R4 RX connection documentation |

## Files to Create

| File | Purpose |
|------|---------|
| `aMAZEing/README.md` | **Top-level project overview**: system architecture, component relationships, quick start guide, hardware requirements |
| `R4/README.md` | R4 firmware documentation, button mappings, setup guide |

## Files to Delete

| File | Reason |
|------|--------|
| `R4/R4_PICO.ino` | Superseded by R4_PICO_LATEST.ino |

---

## Verification Plan

1. **Test Pico TX → R4 RX**: Send test byte from Pico, verify R4 receives it via Serial monitor
2. **Test invalid move**: Try moving into wall, verify R4 stays in MAZE_ARMED (no player swap)
3. **Test valid move**: Make valid move, verify R4 switches player and enters MINIGAME_MODE
4. **Test full turn cycle**: P1 move → minigame → P2 arm turn → P2 move → minigame
5. **Test reset button**: Press USER button, verify maze resets and R4 returns to P1/MINIGAME_MODE
6. **Test win condition**: Reach goal, verify 'W' sent and ProtoPie receives 'w' key
7. **Test status bar**: Verify text displays correctly in top 8 rows (turn indicator, button hints)
8. **Test maze bounds**: Verify players cannot move beyond row 6 (7 rows: 0-6), maze renders with Y offset

---

## UART Protocol Summary

### R4 → Pico (existing binary protocol)
- 8-byte packets: `AA 55 Xlo Xhi Ylo Yhi mask checksum`
- Joystick position and button state

### R4 → Pico (new single-byte commands)
| Byte | Meaning |
|------|---------|
| `R`  | Reset game |

### Pico → R4 (new single-byte responses)
| Byte | Meaning |
|------|---------|
| `V`  | Valid move executed |
| `I`  | Invalid move (wall) |
| `W`  | Win! Player reached goal |

---

## Button Quick Reference (Final)

| Button | Location | Action |
|--------|----------|--------|
| **BTN_A** | GPIO 2 | **Arm maze turn** (MINIGAME → MAZE_ARMED) |
| BTN_B | GPIO 3 | Send 'b' to ProtoPie |
| BTN_C | GPIO 4 | Send 'c' to ProtoPie |
| BTN_D | GPIO 5 | Send 'd' to ProtoPie |
| Joystick Switch | GPIO 6 | Reserved (unused in normal play) |
| BTN_ENTER | GPIO 7 | Send ENTER to ProtoPie |
| Power Switch | GPIO 8 | Send 'o' on OFF edge |
| **USER Button** | Built-in | **Full system reset** |
