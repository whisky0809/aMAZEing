# Maze Game - 64x64 LED Matrix (Pico Engine)

Procedurally-generated maze navigation game for Raspberry Pi Pico W, designed to be controlled by an external Arduino UNO R4 WiFi via UART.

## Features
- **Two-Player Turn-Based**: Players alternate turns, one move per turn.
- **Hold-to-Control**: Maze only responds when D9 is held on the R4 controller.
- **Status Bar**: Shows "MiniWait" (D9 released) or "P1 GO!"/"P2 GO!" (D9 held).
- **Bidirectional UART**: Sends move validation back to controller.
- **64x64 HUB75 Matrix**: High-refresh rate display.

## Display Layout

The 64x64 matrix is divided into two regions:

```
Row 0-7:   STATUS BAR (Text: "MiniWait", "P1 GO!", "P2 GO!")
Row 8-63:  MAZE AREA (7x8 Grid, 8x8 pixels per cell)
```

## Communication Protocol (UART0)

**Pins**:
- **TX**: GP16 (Connects to R4 RX)
- **RX**: GP17 (Connects to R4 TX)
- **Baud**: 115200

**R4 -> Pico Commands (Single Byte)**:
- `s`: Start Game (from start screen)
- `R`: Reset Game (restart to start screen)
- `o`: Win Trigger (power switch flipped)
- `H`: D9 Held (maze control active)
- `U`: D9 Unheld (maze control released)

**R4 -> Pico Commands (Binary Packet)**:
- 8-byte packet: `[0xAA][0x55][Xlo][Xhi][Ylo][Yhi][mask][checksum]`

**Pico -> R4 Responses (Single Byte)**:
- `V`: Move Valid (player moved successfully)
- `I`: Move Invalid (hit wall/boundary)
- `G`: Goal Reached (goal relocates, game continues)

## Hardware Setup

See `../PINOUT.md` for detailed wiring.

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