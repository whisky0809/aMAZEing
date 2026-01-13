# aMAZEing Turn-Based Mode Implementation Plan

## Overview
Implement proper turn-based gameplay where players alternate between maze moves and ProtoPie minigames, with bidirectional UART for move validation, button-based mode switching, and a dedicated status bar display.

## Requirements
- [x] 1. **Turn-based flow**: Player 1 moves → minigame → Player 2 moves → minigame → repeat
- [x] 2. **Invalid moves don't trigger swap**: Only valid maze moves should advance the turn
- [x] 3. **Button triggers swap back**: Player presses button when minigame complete
- [x] 4. **Built-in Arduino button as reset**: Reset entire system to starting state
- [x] 5. **Status bar display**: Top 8 rows for text (turn indicator, button hints)
- [x] 6. **Update all documentation**: README.md and PINOUT.md

---

## Implementation Plan

### Phase 1: Pico Firmware - Move Validation Response
**Status: [x] COMPLETE**
- [x] Initialize UART0 TX on GP16 (already wired to R4 RX)
- [x] After processing a move attempt, send single-byte response:
   - `V` = Valid move executed
   - `I` = Invalid move (hit wall/boundary)
   - `W` = Win condition (player reached goal)

### Phase 2: R4 Firmware - Receive Move Validation
**Status: [x] COMPLETE**
- [x] Add `Serial1.read()` to receive Pico responses
- [x] Only switch to minigame mode if move was VALID
- [x] Track current player (P1 or P2)

### Phase 3: Button Mappings
**Status: [x] COMPLETE**
- [x] Update button mappings in R4 firmware

### Phase 4: Built-in Reset Button
**Status: [x] COMPLETE**
- [x] Identify correct pin for USER button (Pin 13)
- [x] Add pin setup with INPUT_PULLUP
- [x] On press (debounced):
   - Send reset command `R` to Pico via UART
   - Send reset key `r` to ProtoPie
   - Reset R4 state: currentPlayer = P1, state = MINIGAME_MODE

### Phase 5: Pico Reset Handler
**Status: [x] COMPLETE**
- [x] Listen for `R` byte on UART RX
- [x] When received, call `game.reset()` to restart maze

### Phase 6: Display Layout Change - Status Bar
**Status: [x] COMPLETE**
- [x] **config.h**: Update constants (STATUS_BAR_HEIGHT, MAZE_OFFSET_Y, MAZE_HEIGHT)
- [x] **game_state.cpp**: Implement `renderStatusBar()`
- [x] **Maze rendering**: Apply `MAZE_OFFSET_Y` to all Y coordinates (sprites, goal, fog, barriers)
- [x] **Maze generation**: Use `MAZE_HEIGHT` (7) for Y bounds

### Phase 7: Documentation Updates
**Status: [x] COMPLETE**
- [x] **`aMAZEing/README.md`**: Top-level project overview
- [x] **`Pico/README.md`**: Updated with turn-based flow, UART protocol, display layout
- [x] **`Pico/PINOUT.md`**: Added GP16 TX → R4 RX connection
- [x] **`R4/README.md`**: New R4 firmware documentation
- [x] **Cleanup**: Deleted `R4/R4_PICO.ino`

---

## Files Modified/Created

| File | Status |
|------|--------|
| `R4/R4_PICO_LATEST.ino` | **Modified** |
| `Pico/src/main.cpp` | **Modified** |
| `Pico/src/config.h` | **Modified** |
| `Pico/src/game/game_state.h` | **Modified** |
| `Pico/src/game/game_state.cpp` | **Modified** |
| `Pico/src/game/maze_generator.cpp` | **Modified** |
| `Pico/README.md` | **Modified** |
| `Pico/PINOUT.md` | **Modified** |
| `aMAZEing/README.md` | **Created** |
| `R4/README.md` | **Created** |
| `R4/R4_PICO.ino` | **Deleted** |

---

## Verification Plan
- [ ] 1. **Test Pico TX → R4 RX**: Send test byte from Pico, verify R4 receives it via Serial monitor
- [ ] 2. **Test invalid move**: Try moving into wall, verify R4 stays in MAZE_ARMED (no player swap)
- [ ] 3. **Test valid move**: Make valid move, verify R4 switches player and enters MINIGAME_MODE
- [ ] 4. **Test full turn cycle**: P1 move → minigame → P2 arm turn → P2 move → minigame
- [ ] 5. **Test reset button**: Press USER button, verify maze resets and R4 returns to P1/MINIGAME_MODE
- [ ] 6. **Test win condition**: Reach goal, verify 'W' sent and ProtoPie receives 'w' key
- [ ] 7. **Test status bar**: Verify text displays correctly in top 8 rows (turn indicator, button hints)
- [ ] 8. **Test maze bounds**: Verify players cannot move beyond row 6 (7 rows: 0-6), maze renders with Y offset