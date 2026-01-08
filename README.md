# Maze Game - 64x64 LED Matrix

Procedurally-generated maze navigation game for Raspberry Pi Pico W with 64x64 HUB75E RGB LED Matrix.

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Setup (New Machine)](#software-setup-new-machine)
- [Building & Uploading](#building--uploading)
- [Game Controls](#game-controls)
- [Pin Connections](#pin-connections-pcb_layout_v1)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)
- [Credits](#credits)

---

## Hardware Requirements

- Raspberry Pi Pico (or Pico W)
- 64x64 HUB75E RGB LED Matrix Panel (P2.5-P3 pitch)
- 5V Power Supply (3-5A minimum for the LED panel)
- USB cable for programming

---

## Software Setup (New Machine)

Follow these steps **in order** when setting up on a new computer.

### Step 1: Install VS Code and PlatformIO

1. Download and install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code → Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE" and install it
4. Restart VS Code after installation
5. Wait for PlatformIO to finish initial setup (can take several minutes)

### Step 2: Install Zadig USB Driver (Windows Only)

The Pico in BOOTSEL mode requires a WinUSB driver to work with `picotool`.

1. Download [Zadig](https://zadig.akeo.ie/) (latest version)
2. **Put your Pico in BOOTSEL mode:**
   - Unplug the Pico
   - Hold the **BOOTSEL** button on the Pico
   - While holding, plug in the USB cable
   - Release the button after plugging in
   - The Pico should appear as a USB drive named "RPI-RP2"
3. Run Zadig as Administrator
4. Go to **Options → List All Devices**
5. Select **"RP2 BOOT"** from the dropdown
6. Ensure the target driver shows **WinUSB** (use arrows to select if needed)
7. Click **"Install Driver"** or **"Replace Driver"**
8. Wait for completion (may take a minute)

> ⚠️ **Important**: If you don't see "RP2040 BOOT", make sure the Pico is in BOOTSEL mode (step 2).

### Step 3: Open the Project

1. Open VS Code
2. Click **File → Open Folder**
3. Navigate to and select the `Maze_game(new)` folder
4. Wait for PlatformIO to detect the project and install dependencies (check the bottom status bar)

### Step 4: Verify platformio.ini Configuration

The `platformio.ini` file must use the community platform for the Earle Philhower Arduino core:

```ini
[env:pico]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
framework = arduino
board_build.core = earlephilhower
```

> ⚠️ **Common Issue**: If `platform = raspberrypi` (without the GitHub URL), the build will fail with `Arduino.h: No such file or directory`. This is because the official platform uses the incompatible Arduino Mbed core instead of the Earle Philhower core.

---

## Building & Uploading

### Option A: Using VS Code / PlatformIO GUI

1. Click the **PlatformIO icon** (alien head) in the left sidebar
2. Under **PROJECT TASKS → pico**:
   - Click **Build** to compile
   - Click **Upload** to flash to Pico
   - Click **Monitor** to view serial output

### Option B: Using Command Line

```bash
# Build the project
pio run

# Upload to Pico (put Pico in BOOTSEL mode first!)
pio run --target upload

# Monitor serial output (115200 baud)
pio device monitor

# Build, upload, and monitor in one command
pio run --target upload --target monitor

# Clean build artifacts
pio run --target clean
```

### Upload Process

1. **Put Pico in BOOTSEL mode** before uploading:
   - Unplug USB
   - Hold BOOTSEL button
   - Plug in USB while holding button
   - Release button
2. Run the upload command
3. The Pico will automatically reboot after flashing

---

## Game Controls

### Joystick (HW-504 Module)

| Input | Action |
|-------|--------|
| **Stick Up** | Move Up |
| **Stick Left** | Move Left |
| **Stick Down** | Move Down |
| **Stick Right** | Move Right |
| **Short Press** | Reset Game |
| **Long Press** | Toggle 1P/2P mode (on start screen) |

### Serial (Debug)

Connect via serial monitor (115200 baud) and use:

| Key | Action |
|-----|--------|
| **W** | Move Up |
| **A** | Move Left |
| **S** | Move Down |
| **D** | Move Right |
| **R** | Reset Game |
| **T** | Toggle 1P/2P mode (on start screen) |

---

## Pin Connections (PCB_LAYOUT_V1)

### HUB75 Interface Wiring

| Function | Pico Pin | HUB75 Pin |
|----------|----------|-----------|
| R0 (Red Upper) | GP0 | R0 |
| G0 (Green Upper) | GP1 | G0 |
| B0 (Blue Upper) | GP2 | B0 |
| R1 (Red Lower) | GP3 | R1 |
| G1 (Green Lower) | GP4 | G1 |
| B1 (Blue Lower) | GP5 | B1 |
| A (Row Select) | GP6 | A |
| B (Row Select) | GP7 | B |
| C (Row Select) | GP8 | C |
| D (Row Select) | GP9 | D |
| E (Row Select) | GP10 | E |
| LATCH | GP11 | LAT |
| OE (Output Enable) | GP12 | OE |
| CLK (Clock) | GP13 | CLK |
| GND | GND | GND |

> ⚠️ **IMPORTANT:** Power the LED panel externally with a 5V power supply (3-5A). DO NOT power it from the Pico's VBUS/VSYS pins.

---

## Game Features

- **Procedural Generation**: Random maze on each playthrough
- **Fog of War**: Only see player and adjacent cells
- **1-Player / 2-Player modes**: Toggle with 'T' on start screen
- **Win Detection**: Reach the green goal pixel
- **Move Counter**: Track efficiency via serial output

### Display Colors & Visual Feedback

| Color | RGB565 | Meaning |
|-------|--------|---------|
| Red | 0xF800 | Player 1 / Blocked indicator / Goal far away |
| Cyan | 0x07FF | Player 2 |
| Blue | 0x001F | Available paths / Goal accessible (open door) |
| Green | 0x07E0 | Goal close / Goal blocked (with red barrier) |
| Orange | 0xFD20 | Goal medium distance |
| Yellow | 0xFFE0 | Goal close distance |
| Black | 0x0000 | Walls / Open door center |

### Goal Distance Coloring (Heat Map)

The goal changes color based on distance from the nearest player:

| Distance | Color | Meaning |
|----------|-------|---------|
| ≥10 cells | Red | Far away |
| 5-9 cells | Orange | Medium distance |
| 2-4 cells | Yellow | Getting close |
| 1 cell (adjacent) | Green | Almost there! |
| Adjacent + can enter | Blue (open) | Door is open - enter now! |
| Adjacent + blocked | Green + red line | Can't enter from this side |

---

## Troubleshooting

### "Arduino.h: No such file or directory"

**Cause**: Using the wrong Arduino core (Mbed instead of Earle Philhower).

**Fix**: Update `platformio.ini`:
```ini
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board_build.core = earlephilhower
```
Then clean and rebuild:
```bash
pio run --target clean && pio run
```

### "No accessible RP2040 devices in BOOTSEL mode" / Zadig driver needed

**Cause**: Windows doesn't have the correct USB driver for the Pico in BOOTSEL mode.

**Fix**: Install WinUSB driver via Zadig (see [Step 2](#step-2-install-zadig-usb-driver-windows-only) above).

### Upload times out waiting for device

**Cause**: Pico wasn't in BOOTSEL mode when upload started.

**Fix**:
1. Put Pico in BOOTSEL mode (hold button → plug in → release)
2. Run upload command within 10 seconds

### Display not working

1. Check all wiring connections match the pin table above
2. Verify external 5V power supply is connected and adequate (3-5A)
3. Confirm GND is shared between Pico and LED panel
4. Check that the LED panel is a HUB75**E** (with E pin) for 64x64 displays

### Flickering display

- Ensure 5V power supply provides at least 3A
- Verify `HUB75_BCM` flag is set in platformio.ini
- Check CLK, LATCH, and OE pin connections

### Serial monitor shows garbage

- Ensure baud rate is set to **115200**
- In VS Code: check `monitor_speed = 115200` in platformio.ini

---

## Project Structure

```
Maze_game(new)/
├── platformio.ini          # Build configuration
├── CLAUDE.md               # AI assistant instructions
├── README.md               # This file
├── src/
│   ├── main.cpp            # Game loop entry point
│   ├── config.h            # Pin definitions & color constants
│   ├── display/
│   │   ├── display_manager.h
│   │   └── display_manager.cpp
│   ├── game/
│   │   ├── maze_generator.h
│   │   ├── maze_generator.cpp
│   │   ├── game_state.h
│   │   └── game_state.cpp
│   ├── input/
│   │   ├── serial_input.h
│   │   ├── serial_input.cpp
│   │   ├── joystick_input.h
│   │   └── joystick_input.cpp
│   └── sprites/
│       ├── sprite.h
│       ├── sprite.cpp
│       ├── player_sprites.h
│       └── goal_sprites.h
└── lib/
    └── RP2040Matrix/       # HUB75 display driver (PIO-based)
        ├── GFXMatrix.cpp/h
        ├── hub75.h
        ├── hub75_BCM.c
        └── ps_hub75_64_BCM.pio.h
```

### Build Flags (platformio.ini)

| Flag | Purpose |
|------|---------|
| `PCB_LAYOUT_V1` | Use V1 pin configuration |
| `HUB75_SIZE=4040` | Configure for 64x64 display |
| `HUB75_BCM` | Enable Binary Code Modulation (flicker-free) |

---

## Development Phases

- [x] Phase 1: Project Setup & Library Integration
- [x] Phase 2: Display Verification
- [x] Phase 3: Input System Testing
- [x] Phase 4: Player Movement
- [x] Phase 5: Maze Generation
- [x] Phase 6: Fog of War Rendering
- [x] Phase 7: Win Condition & Polish
- [x] Phase 8: Joystick Support (HW-504)
- [x] Phase 9: Visual Feedback (distance coloring, blocked indicators)

---

## Credits

- Display Driver: Based on [RP2040Matrix](https://github.com/ClemensElflein/RP2040Matrix) by Clemens Elflein
- Original HUB75 Driver: [RP2040matrix-v2](https://github.com/pitschu/RP2040matrix-v2) by Peter Schulten
- Graphics Library: [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
- PlatformIO Platform: [maxgerhardt/platform-raspberrypi](https://github.com/maxgerhardt/platform-raspberrypi)

## License

This project is for educational purposes. See individual library licenses for their respective components.
