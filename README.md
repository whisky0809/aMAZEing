# aMAZEing - Digital Alertness Project

**aMAZEing** is an interactive two-player game that combines a physical maze on a 64x64 LED matrix with digital minigames on an iPad. Players alternate between solving the maze using a joystick controller and playing minigames on the tablet.

## System Architecture

The system consists of three main components communicating in a chain:

```mermaid
graph LR
    R4[Arduino UNO R4 WiFi] -- UART (Bidirectional) --> Pico[Raspberry Pi Pico W];
    R4 -- USB HID (Keyboard/Mouse) --> iPad[iPad (ProtoPie)];
    Pico -- HUB75 --> Matrix[64x64 LED Matrix];
    Joystick -- Analog --> R4;
    Buttons -- GPIO --> R4;
```

1.  **Controller (Arduino UNO R4 WiFi)**:
    -   Reads Joystick and Buttons.
    -   Acts as a USB Keyboard/Mouse for the iPad (ProtoPie).
    -   Controls the game state (Turn-based logic).
    -   Sends move commands to the Pico.

2.  **Maze Engine (Raspberry Pi Pico W)**:
    -   Renders the Maze and Players on the 64x64 LED Matrix.
    -   Validates moves (Wall collision detection).
    -   Sends validation results (Valid/Invalid/Win) back to the R4.

3.  **Display/Minigames (iPad + ProtoPie)**:
    -   Runs the visual interface and minigames.
    -   Receives input from R4 (Mouse for minigames, Keys 'v'/'w' for game flow).

## Hardware Requirements

-   **Arduino UNO R4 WiFi** (Main Controller)
-   **Raspberry Pi Pico W** (Maze Graphics & Logic)
-   **64x64 RGB LED Matrix** (HUB75 Interface)
-   **Analog Joystick** (2-axis + switch)
-   **Push Buttons** (4x for A/B/C/D, 1x Enter, 1x Reset)
-   **Power Supply** (5V High Current for Matrix)
-   **iPad** running ProtoPie Player

## Quick Start Guide

1.  **Hardware Setup**: Connect components as per `PINOUT.md`.
2.  **Flash Pico**:
    -   Open `Pico/` in PlatformIO.
    -   Upload firmware to Raspberry Pi Pico W.
3.  **Flash R4**:
    -   Open `R4/R4_aMAZEing/R4_aMAZEing.ino` in Arduino IDE.
    -   Select "Arduino UNO R4 WiFi".
    -   Upload firmware.
4.  **ProtoPie**:
    -   Load `aMAZEing_PrototypeMVP.pie` on the iPad.
5.  **Play**:
    -   Connect R4 to iPad via USB (adapter required).
    -   Power up the Matrix/Pico.
    -   Press **D7 (ENTER)** to start the game.
    -   Hold **D9** to control the maze, tilt joystick to move.
    -   Release **D9** to return to mouse mode for minigames.

## Folder Structure

-   `Pico/`: Source code for Raspberry Pi Pico (Maze Engine).
-   `R4/`: Arduino sketches for UNO R4 (Controller).
-   `aMAZEing_PrototypeMVP.pie`: ProtoPie project file.

For detailed documentation, see:
-   [Pico Documentation](Pico/README.md)
-   [R4 Documentation](R4/README.md)
