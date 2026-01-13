# Wiring Guide - 64x64 LED Matrix Maze Game

## Hardware Requirements
- **Microcontroller:** Raspberry Pi Pico / Pico W (RP2040)
- **Display:** 64x64 RGB LED Matrix Panel (HUB75E interface)
- **Power:** 5V Power Supply (External, 3-5A recommended)

## Pin Connections

### HUB75 Interface (Display)
Connect the HUB75 ribbon cable to the Pico pins as follows.

| Signal | Function          | Pico Pin |
|--------|-------------------|----------|
| **R0** | Red Data (Upper)  | **GP0**  |
| **G0** | Green Data (Upper)| **GP1**  |
| **B0** | Blue Data (Upper) | **GP2**  |
| **R1** | Red Data (Lower)  | **GP3**  |
| **G1** | Green Data (Lower)| **GP4**  |
| **B1** | Blue Data (Lower) | **GP5**  |
| **A**  | Row Select A      | **GP6**  |
| **B**  | Row Select B      | **GP7**  |
| **C**  | Row Select C      | **GP8**  |
| **D**  | Row Select D      | **GP9**  |
| **E**  | Row Select E      | **GP10** |
| **LAT**| Latch             | **GP11** |
| **OE** | Output Enable     | **GP12** |
| **CLK**| Clock             | **GP13** |
| **GND**| Ground            | **GND**  |

### UART Connection (To Arduino R4 WiFi)

This allows the R4 to send joystick commands and receive game status.

| Signal | Pico Pin | R4 Pin | Notes |
|--------|----------|--------|-------|
| **TX** | **GP16** | **RX** | Pico sends V/I/W status |
| **RX** | **GP17** | **TX** | Pico receives Joystick/Reset |
| **GND**| **GND**  | **GND**| Common Ground is CRITICAL |

### Power Wiring
*   **VSYS (Pico):** Connect to 5V PSU (or USB).
*   **VCC (Display):** Connect directly to 5V PSU. **DO NOT power the display through the Pico.**
*   **GND:** Common Ground between Pico, R4, Display, and PSU is **mandatory**.