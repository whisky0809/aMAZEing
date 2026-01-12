// uart_input.cpp - Binary packet input handler for R4 joystick controller
#include "uart_input.h"

// Debounce time between direction changes (ms)
static const unsigned long DIRECTION_DEBOUNCE_MS = 150;

UARTInput::UARTInput()
    : packet_idx(0)
    , reset_requested(false)
    , mode_toggle_requested(false)
    , last_direction(DIR_NONE)
    , last_direction_time(0)
{
}

void UARTInput::init() {
    Serial2.setRX(UART_RX_PIN);
    Serial2.begin(UART_BAUD);
}

Direction UARTInput::rawToDirection(uint16_t x, uint16_t y) {
    // Convert raw ADC values (0-1023) to direction
    // Center is ~512, apply deadzone
    int dx = (int)x - JOY_CENTER;
    int dy = (int)y - JOY_CENTER;

    // Check if outside deadzone
    bool x_active = (abs(dx) > JOY_DEADZONE);
    bool y_active = (abs(dy) > JOY_DEADZONE);

    if (!x_active && !y_active) {
        return DIR_NONE;
    }

    // Determine dominant axis
    if (abs(dy) > abs(dx)) {
        // Y axis dominant
        // Note: R4 inverts Y so pushing up sends negative values
        if (dy < 0) return NORTH;  // Up
        else return SOUTH;         // Down
    } else {
        // X axis dominant
        if (dx < 0) return WEST;   // Left
        else return EAST;          // Right
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
    if (checksum != packet_buf[7]) {
        return false;
    }

    return true;
}

Direction UARTInput::getCommand() {
    Direction result = DIR_NONE;
    unsigned long now = millis();

    while (Serial2.available()) {
        uint8_t byte = Serial2.read();

        // Handle single-char button events (R/T) sent by R4
        if (packet_idx == 0) {
            if (byte == 'R') {
                reset_requested = true;
                continue;
            } else if (byte == 'T') {
                mode_toggle_requested = true;
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
                // Invalid, reset
                packet_idx = 0;
            }
        } else {
            // Collecting payload bytes
            packet_buf[packet_idx++] = byte;

            if (packet_idx >= UART_PACKET_LEN) {
                // Full packet received
                if (parsePacket()) {
                    // Extract joystick values (little-endian)
                    uint16_t rawX = packet_buf[2] | (packet_buf[3] << 8);
                    uint16_t rawY = packet_buf[4] | (packet_buf[5] << 8);
                    // uint8_t btnMask = packet_buf[6];  // Available if needed

                    Direction dir = rawToDirection(rawX, rawY);

                    // Apply debouncing
                    if (dir != DIR_NONE) {
                        if (dir != last_direction || (now - last_direction_time >= DIRECTION_DEBOUNCE_MS)) {
                            result = dir;
                            last_direction = dir;
                            last_direction_time = now;
                        }
                    } else {
                        // Joystick returned to center, allow immediate next input
                        last_direction = DIR_NONE;
                    }
                }
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

bool UARTInput::isToggleModeRequested() {
    if (mode_toggle_requested) {
        mode_toggle_requested = false;
        return true;
    }
    return false;
}
