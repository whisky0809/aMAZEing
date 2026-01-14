// uart_input.cpp - Binary packet input handler for R4 joystick controller
#include "uart_input.h"

// Debounce time between direction changes (ms)
// 150ms can feel a bit slow; 110ms is snappier but still stable
static const unsigned long DIRECTION_DEBOUNCE_MS = 110;

// Debug levels: 0 = off, 1 = important events only, 2 = verbose (every packet)
#define UART_DEBUG_LEVEL 1

UARTInput::UARTInput()
    : packet_idx(0)
    , reset_requested(false)
    , win_requested(false)
    , last_direction(DIR_NONE)
    , last_direction_time(0)
{
}

void UARTInput::init() {
    // You are using GP16 (TX) and GP17 (RX) on Pico W.
    // Those pins are UART0 alternate pins -> use Serial1 (UART0).
    Serial1.setTX(UART_TX_PIN);   // GP16
    Serial1.setRX(UART_RX_PIN);   // GP17
    Serial1.begin(UART_BAUD);

#if UART_DEBUG_LEVEL >= 1
    Serial.println("[UART] Init: GP16/GP17, 115200 baud");
#endif
}

Direction UARTInput::rawToDirection(uint16_t x, uint16_t y) {
    // Raw values from R4 are 0..1023
    // Center around ~512 then apply deadzone
    int dx = (int)x - JOY_CENTER;
    int dy = (int)y - JOY_CENTER;

    bool x_active = (abs(dx) > JOY_DEADZONE);
    bool y_active = (abs(dy) > JOY_DEADZONE);

    if (!x_active && !y_active) {
        return DIR_NONE;
    }

    // Choose dominant axis
    if (abs(dy) > abs(dx)) {
        // Y axis dominant
        // IMPORTANT:
        // With typical joystick wiring, pushing UP usually makes ADC value go DOWN (dy < 0).
        if (dy < 0) return NORTH;
        else        return SOUTH;
    } else {
        // X axis dominant
        if (dx < 0) return WEST;
        else        return EAST;
    }
}

bool UARTInput::parsePacket() {
    // Validate header
    if (packet_buf[0] != UART_HEADER_0 || packet_buf[1] != UART_HEADER_1) {
        return false;
    }

    // Validate checksum (XOR of bytes 0-6)
    uint8_t checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum ^= packet_buf[i];
    }
    return (checksum == packet_buf[7]);
}

Direction UARTInput::getCommand() {
    Direction result = DIR_NONE;
    unsigned long now = millis();

    // READ FROM Serial1 (UART0) because GP16/GP17 are UART0 pins.
    while (Serial1.available()) {
        uint8_t byte = (uint8_t)Serial1.read();

        // Handle single-char events from R4 (only when we're not mid-packet)
        if (packet_idx == 0) {
            if (byte == 'R') {
#if UART_DEBUG_LEVEL >= 1
                Serial.println("[UART] Reset command");
#endif
                reset_requested = true;
                continue;
            }
            if (byte == 'o') {
#if UART_DEBUG_LEVEL >= 1
                Serial.println("[UART] Win command");
#endif
                win_requested = true;
                continue;
            }
        }

        // Packet parsing state machine
        if (packet_idx == 0) {
            // Looking for first header byte
            if (byte == UART_HEADER_0) {
                packet_buf[packet_idx++] = byte;
            }
        } else if (packet_idx == 1) {
            // Looking for second header byte
            if (byte == UART_HEADER_1) {
                packet_buf[packet_idx++] = byte;
            } else if (byte == UART_HEADER_0) {
                // Could be start of new packet, stay at idx 1
                packet_buf[0] = byte;
            } else {
                packet_idx = 0;
            }
        } else {
            // Collect payload bytes
            packet_buf[packet_idx++] = byte;

            if (packet_idx >= UART_PACKET_LEN) {
                // Full packet received
                if (parsePacket()) {
                    // Extract joystick values (little-endian)
                    uint16_t rawX = (uint16_t)packet_buf[2] | ((uint16_t)packet_buf[3] << 8);
                    uint16_t rawY = (uint16_t)packet_buf[4] | ((uint16_t)packet_buf[5] << 8);

                    Direction dir = rawToDirection(rawX, rawY);

                    // Debouncing: only output a direction if it's new
                    // OR enough time has passed holding that direction
                    if (dir != DIR_NONE) {
                        if (dir != last_direction || (now - last_direction_time >= DIRECTION_DEBOUNCE_MS)) {
                            result = dir;
                            last_direction = dir;
                            last_direction_time = now;
#if UART_DEBUG_LEVEL >= 1
                            // Only log when direction is accepted
                            const char* dirName = (dir == NORTH) ? "N" :
                                                  (dir == SOUTH) ? "S" :
                                                  (dir == EAST) ? "E" : "W";
                            Serial.print("[UART] Dir: ");
                            Serial.println(dirName);
#endif
                        }
                    } else {
                        // Returned to center: allow immediate next direction
                        last_direction = DIR_NONE;
                    }
                }
#if UART_DEBUG_LEVEL >= 2
                else {
                    Serial.println("[UART] Bad checksum");
                }
#endif

                // Reset packet state
                packet_idx = 0;
            }
        }
    }

    return result;
}

bool UARTInput::isResetRequested() {
    if (reset_requested) {
        reset_requested = false;
        return true;
    }
    return false;
}

bool UARTInput::isWinRequested() {
    if (win_requested) {
        win_requested = false;
        return true;
    }
    return false;
}
