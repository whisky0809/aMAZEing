# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build project
pio run

# Upload to Pico W (hold BOOTSEL button while plugging in USB)
pio run --target upload

# Monitor serial output (115200 baud)
pio device monitor

# Clean build
pio run --target clean
```

## Architecture Overview

This is a PlatformIO-based Arduino project for Raspberry Pi Pico targeting a 64x64 HUB75E RGB LED matrix display. The game is a procedurally-generated maze with fog-of-war mechanics.

### Core Components

- **src/main.cpp** - Game loop entry point, initializes display/game/input and runs at ~60 FPS
- **src/config.h** - Pin definitions, RGB565 color constants, and game settings
- **src/display/display_manager** - Abstraction layer over GFXMatrix library, wraps Adafruit_GFX
- **src/game/game_state** - State machine (START/PLAYING/WIN), handles 1P/2P modes, player movement, and rendering
- **src/game/maze_generator** - Procedural maze using seeded random directions (memoryless design - no grid storage)
- **src/input/serial_input** - WASD command parser over USB serial

### Display Library

- **lib/RP2040Matrix/** - Custom HUB75 driver using PIO for flicker-free BCM (Binary Code Modulation)
- `GFXMatrix` extends `Adafruit_GFX` for standard drawing primitives
- Hardcoded pin mapping: GP0-5 (RGB data), GP6-10 (row select A-E), GP11-13 (LAT/OE/CLK)

### Key Build Flags (platformio.ini)

- `MAZE_SIZE=64` - Maze grid dimensions
- `PCB_LAYOUT_V1` - Pin configuration variant
- `HUB75_SIZE=4040` - Display size config
- `HUB75_BCM` - Enable Binary Code Modulation

### Color Format

All colors use RGB565 format (16-bit). Key colors defined in config.h:
- PLAYER_COLOR (0xF800 red), GOAL_COLOR (0x07E0 green), PATH_COLOR (0x001F blue)

## Game Controls

Serial input (115200 baud): W/A/S/D movement, R reset, T toggle 1P/2P mode (on start screen)
