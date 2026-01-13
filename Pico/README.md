# Maze Game - 64x64 LED Matrix (Pico Engine)

Procedurally-generated maze navigation game for Raspberry Pi Pico W, designed to be controlled by an external Arduino UNO R4 WiFi via UART.

## Features
- **Turn-Based Gameplay**: Players alternate turns (Move -> Minigame -> Move).
- **Status Bar**: Top 8 pixels show current player turn and hints.
- **Bidirectional UART**: Sends move validation (Valid/Invalid/Win) back to controller.
- **64x64 HUB75 Matrix**: High-refresh rate display.

## Display Layout

The 64x64 matrix is divided into two regions:

```
Row 0-7:   STATUS BAR (Text: "P1 TURN", "P2 TURN", etc.)
Row 8-63:  MAZE AREA (7x8 Grid, 8x8 pixels per cell)
```

## Communication Protocol (UART0)

**Pins**:
- **TX**: GP16 (Connects to R4 RX)
- **RX**: GP17 (Connects to R4 TX)
- **Baud**: 115200

**R4 -> Pico Commands (Single Byte)**:
- `R`: Reset Game (Restart, P1 start)

**R4 -> Pico Commands (Binary Packet)**:
- 8-byte packet with Joystick X/Y and Button Mask.

**Pico -> R4 Responses (Single Byte)**:
- `V`: Move Valid (Player moved successfully)
- `I`: Move Invalid (Hit wall/boundary)
- `W`: Win (Player reached goal)

## Hardware Setup

See `PINOUT.md` for detailed wiring.

## Building & Uploading

1.  Open this folder in VS Code with PlatformIO.
2.  Click **Build** (Alien head -> Build).
3.  Hold BOOTSEL on Pico, connect USB.
4.  Click **Upload**.

## Project Structure

-   `src/main.cpp`: Entry point, UART init, Main Loop.
-   `src/game/`: Game logic (GameState, MazeGenerator).
-   `src/display/`: Display manager.
-   `src/input/`: UART input parser.
-   `lib/`: RP2040Matrix library.