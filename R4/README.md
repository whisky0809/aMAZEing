# R4 Controller Firmware

This firmware runs on the **Arduino UNO R4 WiFi** and acts as the central controller for the aMAZEing project.

## Purpose
1.  **USB HID**: Acts as a Keyboard/Mouse for the iPad (ProtoPie).
2.  **Input Handling**: Reads Joystick and Buttons.
3.  **Game Logic**: Manages the turn-based state machine.
4.  **Gateway**: Forwards moves to the Pico via UART and receives validation.

## Pinout

| Component | R4 Pin | Notes |
|-----------|--------|-------|
| **Joystick X** | A0 | Analog |
| **Joystick Y** | A1 | Analog |
| **Joystick SW**| 9 | Digital Pullup (Unused in current logic) |
| **BTN_A** | 2 | "Take Turn" (Arm Maze) |
| **BTN_B** | 3 | Send 'b' to ProtoPie |
| **BTN_C** | 4 | Send 'c' to ProtoPie |
| **BTN_D** | 5 | Send 'd' to ProtoPie |
| **BTN_ENTER** | 7 | Send ENTER to ProtoPie |
| **SW_POWER** | 8 | Send 'o' on OFF edge |
| **UART TX** | 1 (TX) | To Pico RX (GP17) |
| **UART RX** | 0 (RX) | To Pico TX (GP16) |
| **RESET** | 13 | USER Button (Soft Reset) |

## State Machine

The controller cycles through these states:

1.  **MINIGAME_MODE**:
    -   Joystick controls Mouse on iPad.
    -   Maze is frozen.
    -   Waiting for **BTN_A** press.
2.  **MAZE_ARMED**:
    -   Mouse is frozen.
    -   Waiting for Joystick directional input.
3.  **MAZE_PULSING**:
    -   Sends the direction to the Pico for a fixed duration (70ms).
4.  **WAITING_ACK**:
    -   Waits for response from Pico:
        -   `V` (Valid): Switch player, go to MINIGAME_MODE.
        -   `I` (Invalid): Return to MAZE_ARMED (try again).
        -   `W` (Win): Send 'w' to iPad, Reset to Start.

## Setup
1.  Open `R4_PICO_LATEST.ino` in Arduino IDE.
2.  Select Board: **Arduino UNO R4 WiFi**.
3.  Upload.
