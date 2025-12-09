# Wiring Guide - 64x64 LED Matrix Maze Game

## Hardware Requirements
- **Microcontroller:** Raspberry Pi Pico / Pico W (RP2040)
- **Display:** 64x64 RGB LED Matrix Panel (HUB75E interface)
- **Power:** 5V Power Supply (External, 3-5A recommended)
- **Logic Levels:** 3.3V (RP2040) to 5V (Display). *Note: Most HUB75 panels work fine with 3.3V logic, but a level shifter (74HCT245) is recommended for reliability.*

## Pin Connections (PCB_LAYOUT_V1)

### HUB75 Interface
Connect the HUB75 ribbon cable to the Pico pins as follows.

| Signal | Function          | Pico Pin | Description                  |
|--------|-------------------|----------|------------------------------|
| **R0** | Red Data (Upper)  | **GP0**  | Top half red pixel data      |
| **G0** | Green Data (Upper)| **GP1**  | Top half green pixel data    |
| **B0** | Blue Data (Upper) | **GP2**  | Top half blue pixel data     |
| **R1** | Red Data (Lower)  | **GP3**  | Bottom half red pixel data   |
| **G1** | Green Data (Lower)| **GP4**  | Bottom half green pixel data |
| **B1** | Blue Data (Lower) | **GP5**  | Bottom half blue pixel data  |
| **A**  | Row Select A      | **GP6**  | Row Address Bit 0            |
| **B**  | Row Select B      | **GP7**  | Row Address Bit 1            |
| **C**  | Row Select C      | **GP8**  | Row Address Bit 2            |
| **D**  | Row Select D      | **GP9**  | Row Address Bit 3            |
| **E**  | Row Select E      | **GP10** | Row Address Bit 4 (1/32 scan)|
| **LAT**| Latch             | **GP11** | Data Latch                   |
| **OE** | Output Enable     | **GP12** | Display ON/OFF (Active Low)  |
| **CLK**| Clock             | **GP13** | Data Shift Clock             |
| **GND**| Ground            | **GND**  | Connect to Display GND       |

### Power Wiring
*   **VSYS (Pico):** Connect to 5V PSU (or USB).
*   **VCC (Display):** Connect directly to 5V PSU. **DO NOT power the display through the Pico.**
*   **GND:** Common Ground between Pico, Display, and PSU is **mandatory**.

### Input Controls (Future Upgrade)
These pins are reserved in the code for joystick support.

| Signal | Component | Pico Pin | Type |
|--------|-----------|----------|------|
| **VRx**| Joystick X| **GP26** | ADC0 |
| **VRy**| Joystick Y| **GP27** | ADC1 |
| **SW** | Switch    | **GP22** | Digital Input (Pull-up) |

## Troubleshooting
*   **Ghosting:** If you see vertical smearing, try lowering brightness in code or checking your Ground connections.
*   **Flickering:** Ensure your power supply can provide enough current (Amps). A full white screen can draw 4A+.
*   **No Red/Blue/Green:** Check the R0/G0/B0 connection order. Some panels label them differently.
