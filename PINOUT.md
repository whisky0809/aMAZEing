# Wiring Guide - aMAZEing

Complete pinout for the aMAZEing maze game system.

## System Overview

```
[Joystick + Buttons] --> [Arduino R4 WiFi] <--UART--> [Pico W] --> [64x64 LED Matrix]
                              |
                              v
                         [iPad via USB HID]
```

---

## Arduino UNO R4 WiFi (Controller)

### Joystick
| Signal | R4 Pin | Notes |
|--------|--------|-------|
| **Jx** | A0 | X-axis analog |
| **Jy** | A1 | Y-axis analog |
| **Jsw** | D6 | Joystick switch, left click in mouse mode |
| **VCC** | 5V | |
| **GND** | GND | |

### Buttons
| Button | R4 Pin | Function |
|--------|--------|----------|
| **A** | D2 | Send 'A' to ProtoPie |
| **B** | D3 | Send 'B' to ProtoPie |
| **C** | D4 | Send 'C' to ProtoPie |
| **D** | D5 | Send 'D' to ProtoPie |
| **Enter** | D7 | Start game + ENTER key |
| **Power Switch** | D8 | Win on OFF, Reset on ON |
| **Toggle** | D9 | Hold for maze control |

### UART (to Pico)
| R4 Pin | Pico Pin | Notes |
|--------|----------|-------|
| **D0 (RX)** | GP16 (TX) | Pico sends V/I/G responses to R4 |
| **D1 (TX)** | GP17 (RX) | R4 sends commands to Pico |
| **GND** | GND | Common ground (critical!) |

---

## Raspberry Pi Pico W (Maze Engine)

### HUB75 Interface (64x64 LED Matrix)
| Signal | Function | Pico Pin |
|--------|----------|----------|
| **R0** | Red Data (Upper) | GP0 |
| **G0** | Green Data (Upper) | GP1 |
| **B0** | Blue Data (Upper) | GP2 |
| **R1** | Red Data (Lower) | GP3 |
| **G1** | Green Data (Lower) | GP4 |
| **B1** | Blue Data (Lower) | GP5 |
| **A** | Row Select A | GP6 |
| **B** | Row Select B | GP7 |
| **C** | Row Select C | GP8 |
| **D** | Row Select D | GP9 |
| **E** | Row Select E | GP10 |
| **LAT** | Latch | GP11 |
| **OE** | Output Enable | GP12 |
| **CLK** | Clock | GP13 |
| **GND** | Ground | GND |

### UART (to R4)
| Signal | Pico Pin | R4 Pin | Notes |
|--------|----------|--------|-------|
| **TX** | GP16 | 0 (RX) | Pico sends V/I/G responses |
| **RX** | GP17 | 1 (TX) | Pico receives commands |
| **GND** | GND | GND | Common ground (critical!) |

---

## Power Wiring

| Component | Power Source | Notes |
|-----------|--------------|-------|
| **R4** | USB (from iPad) | Powers R4 and provides HID connection |
| **Pico** | VSYS from 5V PSU or USB | Independent power recommended |
| **LED Matrix** | 5V PSU directly | 3-5A recommended. **DO NOT** power through Pico! |

**Important:** All GND connections must be tied together (R4, Pico, Matrix, PSU).
