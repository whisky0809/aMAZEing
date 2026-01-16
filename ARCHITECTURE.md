# aMAZEing - System Architecture

## Overview

**aMAZEing** is an interactive two-player turn-based maze game that combines physical hardware with digital displays. Players navigate through a procedurally-generated maze shown on an LED matrix while taking turns playing minigames on an iPad.

The system consists of three main components working together:
1. **Physical Controller** - Arduino that reads player input (joystick & buttons)
2. **Maze Engine** - Raspberry Pi Pico that generates and displays the maze
3. **iPad App** - ProtoPie application for minigames and visual feedback

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         aMAZEing SYSTEM                                 │
│                                                                         │
│   ┌─────────────┐      ┌─────────────┐      ┌─────────────────────┐    │
│   │   PLAYER    │      │   PLAYER    │      │     PHYSICAL        │    │
│   │   INPUT     │ ───► │  CONTROLLER │ ───► │     DISPLAY         │    │
│   │  (Joystick  │      │  (Arduino   │      │   (LED Matrix       │    │
│   │  & Buttons) │      │   UNO R4)   │      │    + iPad)          │    │
│   └─────────────┘      └─────────────┘      └─────────────────────┘    │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## System Architecture Diagram

```
                              ┌─────────────────────┐
                              │    PLAYER INPUT     │
                              │  ┌───────────────┐  │
                              │  │   Joystick    │  │
                              │  │  (2-axis +    │  │
                              │  │   button)     │  │
                              │  └───────────────┘  │
                              │  ┌───────────────┐  │
                              │  │   6 Buttons   │  │
                              │  │ (A,B,C,D,     │  │
                              │  │  ENTER,Power) │  │
                              │  └───────────────┘  │
                              │  ┌───────────────┐  │
                              │  │ Toggle Button │  │
                              │  │               │  │
                              │  └───────────────┘  │
                              └──────────┬──────────┘
                                         │
                                         │ GPIO/ADC
                                         ▼
┌────────────────────────────────────────────────────────────────────────────┐
│                        ARDUINO UNO R4 WiFi                                 │
│                     (Central Controller Hub)                               │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │  • Reads joystick position and button states                         │  │
│  │  • Manages game state (whose turn, maze vs minigame mode)            │  │
│  │  • Routes input to appropriate destination                           │  │
│  │  • Acts as "traffic controller" for the entire system                │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
└───────────────────┬────────────────────────────────┬───────────────────────┘
                    │                                │
          UART Wire │                                │ USB Cable
          (Serial)  │                                │ (Keyboard/Mouse)
                    ▼                                ▼
┌───────────────────────────────────┐    ┌───────────────────────────────────┐
│     RASPBERRY PI PICO W           │    │            iPAD                   │
│       (Maze Engine)               │    │     (ProtoPie Minigames)          │
│  ┌─────────────────────────────┐  │    │  ┌─────────────────────────────┐  │
│  │ • Generates the maze        │  │    │  │ • Displays minigames        │  │
│  │ • Validates player moves    │  │    │  │ • Receives button presses   │  │
│  │ • Tracks player positions   │  │    │  │   as keyboard input         │  │
│  │ • Renders graphics          │  │    │  │ • Receives joystick as      │  │
│  │ • Detects goal reached      │  │    │  │   mouse movement            │  │
│  └─────────────────────────────┘  │    │  │ • Controls game flow        │  │
│               │                   │    │  └─────────────────────────────┘  │
│               │ HUB75             │    └───────────────────────────────────┘
│               │ (LED Protocol)    │
│               ▼                   │
│  ┌─────────────────────────────┐  │
│  │    64x64 RGB LED MATRIX     │  │
│  │  ┌───────────────────────┐  │  │
│  │  │ ░░░ P1 GO! ░░░░░░░░░░ │  │  │  ◄── Status bar (8 pixels)
│  │  │ ┌─┬─┬─┬─┬─┬─┬─┬─┐     │  │  │
│  │  │ │▓│█│ │ │ │ │ │☆│     │  │  │  ◄── Maze with walls,
│  │  │ │ │ │ │ │ │ │ │ │     │  │  │      players, and goal
│  │  │ │█│ │ │ │ │ │ │ │     │  │  │
│  │  │ │●│█│ │ │ │ │ │ │     │  │  │      ● = Player 1
│  │  │ │ │ │ │ │ │ │ │ │     │  │  │      ▓ = Player 2
│  │  │ │ │ │ │ │ │ │ │ │     │  │  │      ☆ = Goal
│  │  │ └─┴─┴─┴─┴─┴─┴─┴─┘     │  │  │      █ = Walls
│  │  └───────────────────────┘  │  │
│  └─────────────────────────────┘  │
└───────────────────────────────────┘
```

---

## How the Components Communicate

The three main components communicate using different methods depending on what information needs to be sent:

```
┌──────────────┐                    ┌──────────────┐                    ┌──────────────┐
│              │    UART SERIAL     │              │     USB HID        │              │
│   Arduino    │ ◄────────────────► │    Pico      │                    │    iPad      │
│    UNO R4    │  (Two-way wire)    │   (Maze)     │                    │  (ProtoPie)  │
│              │ ──────────────────►│              │                    │              │
│              │                    └──────────────┘                    │              │
│              │ ───────────────────────────────────────────────────────►              │
│              │            USB (Keyboard & Mouse)                      │              │
└──────────────┘                                                        └──────────────┘

Communication Types:
━━━━━━━━━━━━━━━━━━━━

┌─────────────────────────────────────────────────────────────────────────────────────┐
│ ARDUINO → PICO (via UART wire)                                                      │
│ ─────────────────────────────                                                       │
│ • Movement commands (which direction the player wants to move)                      │
│ • Game control commands (start game, reset, player switched)                        │
│ • Mode changes (player holding D9 button or not)                                    │
└─────────────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────────────┐
│ PICO → ARDUINO (via UART wire)                                                      │
│ ─────────────────────────────                                                       │
│ • Move validation ("Yes, you can move there" or "No, there's a wall")               │
│ • Goal reached notification ("Player found the goal!")                              │
└─────────────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────────────┐
│ ARDUINO → iPAD (via USB cable)                                                      │
│ ─────────────────────────────                                                       │
│ • Button presses appear as keyboard keys (A, B, C, D)                               │
│ • Joystick movement appears as mouse cursor movement                                │
│ • Special keys for game events (turn complete, win, reset)                          │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

---

## Game Flow - How a Turn Works

The game follows a turn-based pattern where players alternate between maze navigation and minigames:

```
                                    ┌─────────────────┐
                                    │   GAME START    │
                                    │ (Press ENTER)   │
                                    └────────┬────────┘
                                             │
                                             ▼
                              ┌──────────────────────────────┐
                              │     PLAYER 1's TURN          │
                              │  (Status bar: "P1 GO!")      │
                              └──────────────┬───────────────┘
                                             │
                     ┌───────────────────────┴───────────────────────┐
                     │                                               │
                     ▼                                               ▼
         ┌─────────────────────┐                        ┌─────────────────────┐
         │  HOLD D9 BUTTON     │                        │  RELEASE D9 BUTTON  │
         │  (Maze Mode Active) │                        │  (Minigame Mode)    │
         └──────────┬──────────┘                        └──────────┬──────────┘
                    │                                              │
                    ▼                                              ▼
         ┌─────────────────────┐                        ┌─────────────────────┐
         │  Move joystick to   │                        │  Status bar shows   │
         │  navigate maze      │                        │  "MiniWait"         │
         └──────────┬──────────┘                        │                     │
                    │                                   │  iPad active for    │
                    ▼                                   │  minigame play      │
         ┌─────────────────────┐                        └─────────────────────┘
         │  Pico checks move:  │
         │  Wall? Boundary?    │
  ┌────► └──────────┬──────────┘
  │                 │
  │      ┌──────────┴──────────┐
  │      │                     │
  │      ▼                     ▼
┌─────────────────┐   ┌─────────────────┐
│  MOVE INVALID   │   │  MOVE VALID     │
│  (Hit a wall)   │   │  (Path clear)   │
│                 │   │                 │
│  Player stays   │   │  Player moves   │
│  in position    │   │  Switch to P2   │
│  Can try again  │   │  Send to iPad:  │
│                 │   │ "Start minigame"│
└─────────────────┘   └────────┬────────┘
                               │
                               ▼
                     ┌─────────────────────┐
                     │   PLAYER 2's TURN   │
                     │ (Status: "P2 GO!")  │
                     └──────────┬──────────┘
                                │
                                │ (Same flow repeats)
                                ▼
                    ┌─────────────────────┐
                    │  GOAL REACHED?      │
                    │                     │
                    │  Yes: Goal moves    │
                    │  to new location    │
                    │                     │
                    │  Power switch OFF:  │
                    │  WIN SCREEN!        │
                    └─────────────────────┘
```

---

## Control Modes Explained

The system has two distinct control modes, switched by holding or releasing the D9 button:

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                                                                                     │
│   MODE 1: MINIGAME MODE (D9 released - default)                                     │
│   ═══════════════════════════════════════════                                       │
│                                                                                     │
│   ┌─────────────┐         ┌─────────────┐         ┌─────────────┐                   │
│   │  Joystick   │ ──────► │   Arduino   │ ──────► │    iPad     │                   │
│   │  movement   │  Mouse  │             │   USB   │   cursor    │                   │
│   └─────────────┘  data   └─────────────┘         └─────────────┘                   │
│                                                                                     │
│   ┌─────────────┐         ┌─────────────┐         ┌─────────────┐                   │
│   │  Buttons    │ ──────► │   Arduino   │ ──────► │    iPad     │                   │
│   │  A,B,C,D    │  Keys   │             │   USB   │  keyboard   │                   │
│   └─────────────┘         └─────────────┘         └─────────────┘                   │
│                                                                                     │
│   LED Matrix shows: "MiniWait" in status bar                                        │
│                                                                                     │
└─────────────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────────────┐
│                                                                                     │
│   MODE 2: MAZE MODE (D9 held down)                                                  │
│   ══════════════════════════════                                                    │
│                                                                                     │
│   ┌─────────────┐         ┌─────────────┐         ┌─────────────┐                   │
│   │  Joystick   │ ──────► │   Arduino   │ ──────► │    Pico     │                   │
│   │  direction  │  Move   │             │  UART   │   (Maze)    │                   │
│   └─────────────┘  cmd    └─────────────┘         └─────────────┘                   │
│                                                          │                          │
│                                                          │ Render                   │
│                                                          ▼                          │
│                                                   ┌─────────────┐                   │
│                                                   │ LED Matrix  │                   │
│                                                   │  (64x64)    │                   │
│                                                   └─────────────┘                   │
│                                                                                     │
│   LED Matrix shows: "P1 GO!" or "P2 GO!" in status bar                              │
│                                                                                     │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

---

## Hardware Components

### Component Specifications

| Component | Description | Role |
|-----------|-------------|------|
| **Arduino UNO R4 WiFi** | 32-bit microcontroller with USB HID support | Central controller, input processing |
| **Raspberry Pi Pico W** | Dual-core microcontroller | Maze generation, graphics rendering |
| **64x64 RGB LED Matrix** | 4096 individually addressable LEDs | Physical maze display |
| **2-Axis Joystick** | Analog joystick with push button | Player navigation input |
| **6 Push Buttons** | Tactile switches (A, B, C, D, ENTER, D9) | Game controls and mode switching |
| **Power Switch** | Toggle switch | Win condition trigger |
| **iPad** | Apple tablet running ProtoPie | Minigame display and interaction |

---

## Software Architecture

### Code Organization

```
aMAZEing/
│
├── ARCHITECTURE.md          ◄─── This document
├── README.md                ◄─── Project overview
├── PINOUT.md                ◄─── Wiring diagram
│
├── Pico/                    ◄─── MAZE ENGINE
│   ├── src/
│   │   ├── main.cpp              Entry point, main game loop
│   │   ├── config.h              Pin definitions, colors, settings
│   │   │
│   │   ├── game/
│   │   │   ├── game_state.cpp    Two-player game logic, turn management
│   │   │   └── maze_generator.cpp Procedural maze creation, move validation
│   │   │
│   │   ├── display/
│   │   │   └── display_manager.cpp LED matrix control abstraction
│   │   │
│   │   ├── input/
│   │   │   └── uart_input.cpp    Communication with Arduino
│   │   │
│   │   └── sprites/
│   │       ├── player_sprites.h  Animated player graphics
│   │       └── goal_sprites.h    Animated goal graphics
│   │
│   └── lib/
│       └── RP2040Matrix/         Custom LED matrix driver
│
├── R4/                      ◄─── CONTROLLER
│   └── R4_aMAZEing/
│       └── R4_aMAZEing.ino       Main controller firmware
│
└── aMAZEing_PrototypeMVP.pie ◄── iPAD APP (ProtoPie project file)
```

### Data Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              DATA FLOW                                              │
└─────────────────────────────────────────────────────────────────────────────────────┘

                    Physical                   Processing                    Output
                    ─────────                  ──────────                    ──────

    ┌─────────┐     Analog      ┌───────────┐  Digital     ┌───────────┐
    │Joystick │ ─────────────►  │           │ ───────────► │           │
    │ X/Y     │   0-1023        │           │              │           │
    └─────────┘                 │           │              │           │
                                │  Arduino  │              │           │
    ┌─────────┐     Digital     │   UNO R4  │              │           │
    │ Buttons │ ─────────────►  │           │              │   Pico    │   ┌─────────┐
    │ A,B,C,D │   HIGH/LOW      │           │   UART       │   Maze    │ ──► LED     │
    │ ENTER   │                 │           │ ───────────► │  Engine   │   │ Matrix  │
    │ D9      │                 │           │  Direction   │           │   └─────────┘
    └─────────┘                 │           │  Commands    │           │
                                │           │              │           │
    ┌─────────┐     Digital     │           │              │           │
    │ Power   │ ─────────────►  │           │  ◄───────────│           │
    │ Switch  │   ON/OFF        │           │   V/I/G      │           │
    └─────────┘                 │           │  Responses   └───────────┘
                                │           │
                                │           │   USB HID    ┌───────────┐
                                │           │ ───────────► │   iPad    │
                                │           │  Keys/Mouse  │ ProtoPie  │
                                └───────────┘              └───────────┘

    Legend:
    V = Valid move      (player successfully moved)
    I = Invalid move    (blocked by wall)
    G = Goal reached    (player found the goal)
```

---

## Key Technical Decisions

### Why Two Microcontrollers?

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  ARDUINO UNO R4 WiFi                      │  RASPBERRY PI PICO W                    │
│  ════════════════════                     │  ════════════════════                   │
│                                           │                                         │
│  ✓ Native USB HID support                 │  ✓ Dual-core processor                  │
│    (can act as keyboard/mouse)            │    (one core for graphics,              │
│                                           │     one for game logic)                 │
│  ✓ Familiar Arduino ecosystem             │                                         │
│    (easy to program)                      │  ✓ Programmable IO (PIO)                │
│                                           │    (precise timing for LED matrix)      │
│  ✓ Built-in USB-C port for iPad           │                                         │
│                                           │  ✓ High-speed rendering                 │
│  ✗ Not powerful enough for                │    (~60 FPS possible)                   │
│    real-time graphics                     │                                         │
│                                           │  ✗ No native USB HID                    │
│                                           │    (can't easily act as keyboard)       │
└─────────────────────────────────────────────────────────────────────────────────────┘

RESULT: Each microcontroller handles what it does best!
```

### Why Hold-to-Play Control Scheme?

The D9 button creates a clear separation between two very different control modes:

```
Problem Solved:
───────────────
Without hold-to-play, joystick movements during minigames
would unintentionally move the maze player.

```

---

## Summary

aMAZEing demonstrates how physical computing can create engaging interactive experiences by combining:

- **Hardware Integration**: Multiple microcontrollers working in harmony
- **Real-time Graphics**: Smooth animations on a physical LED matrix
- **Intuitive Controls**: Clear mode switching with hold-to-play
- **Two-Player Gameplay**: Turn-based competition with visual feedback
- **Multi-Display Experience**: LED matrix for maze + iPad for minigames

The architecture prioritizes:
1. **Responsiveness** - Immediate visual feedback to player actions
2. **Reliability** - Clear communication protocols with validation
3. **Separation of Concerns** - Each component has a focused responsibility
4. **User Experience** - Physical controls with tactile feedback

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                                                                                     │
│     PHYSICAL INPUT  ───►  SMART PROCESSING  ───►  VISUAL OUTPUT                     │
│                                                                                     │
│     (Joystick &          (Arduino routes,        (LED Matrix shows maze,            │
│      Buttons)             Pico validates)         iPad shows minigames)             │
│                                                                                     │
│                    A seamless interactive experience                                │
│                                                                                     │
└─────────────────────────────────────────────────────────────────────────────────────┘
```
