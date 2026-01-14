# R4 Controller Firmware

This firmware runs on the **Arduino UNO R4 WiFi** and acts as the central controller for the aMAZEing project.

## Purpose
1.  **USB HID**: Acts as a Keyboard/Mouse for the iPad (ProtoPie).
2.  **Input Handling**: Reads Joystick and Buttons.
3.  **Game Logic**: Manages hold-to-control state machine.
4.  **Gateway**: Forwards moves to the Pico via UART and receives validation.

## Pinout

| Component | R4 Pin | Notes |
|-----------|--------|-------|
| **Jx** | A0 | Joystick X-axis analog |
| **Jy** | A1 | Joystick Y-axis analog |
| **Jsw** | D6 | Joystick switch, left click in mouse mode |
| **A** | D2 | Send 'A' to ProtoPie |
| **B** | D3 | Send 'B' to ProtoPie |
| **C** | D4 | Send 'C' to ProtoPie |
| **D** | D5 | Send 'D' to ProtoPie |
| **Enter** | D7 | Start game + send ENTER to ProtoPie |
| **Power Switch** | D8 | Win trigger on OFF, Reset on ON |
| **Toggle** | D9 | Hold for maze control |
| **UART TX** | D1 | To Pico GP17 (RX) |
| **UART RX** | D0 | From Pico GP16 (TX) |

## Control Modes

The controller has two modes based on D9 button state:

### PROTOPIE_MODE (D9 Released)
- Joystick controls mouse cursor on iPad
- D6 (joystick switch) acts as left click
- Maze is frozen (neutral packets sent to Pico)
- Status bar shows "MiniWait"

### MAZE_HELD (D9 Held)
- Mouse is frozen
- Joystick controls maze movement
- Only **one move** allowed per hold cycle
- Status bar shows "P1 GO!" or "P2 GO!"

## Game Flow

1. **Start**: Press D7 (ENTER) to start the game (sends `s` to Pico)
2. **Hold D9**: Enter maze control mode (sends `H` to Pico)
3. **Move**: Tilt joystick to move player (one move only)
4. **Release D9**: Exit maze mode (sends `U` to Pico, `t` to ProtoPie if move was made)
5. **Turn switches**: Other player's turn begins
6. **Repeat**: Other player holds D9 and makes their move

## UART Protocol

**R4 -> Pico (Single Byte)**:
- `s`: Start game
- `R`: Reset game
- `o`: Win trigger (power switch OFF)
- `H`: D9 held
- `U`: D9 released

**R4 -> Pico (8-byte Packet)**:
- `[0xAA][0x55][Xlo][Xhi][Ylo][Yhi][mask][checksum]`

**Pico -> R4 (Single Byte)**:
- `V`: Valid move
- `I`: Invalid move
- `G`: Goal reached

## Setup
1.  Open `R4_aMAZEing/R4_aMAZEing.ino` in Arduino IDE.
2.  Select Board: **Arduino UNO R4 WiFi**.
3.  Upload.
