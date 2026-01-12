// uart_input.h - Binary packet input handler for R4 joystick controller
#ifndef UART_INPUT_H
#define UART_INPUT_H

#include <Arduino.h>
#include "../game/maze_generator.h"
#include "../config.h"

class UARTInput {
private:
    // Packet buffer and parsing state
    uint8_t packet_buf[UART_PACKET_LEN];
    uint8_t packet_idx;

    // One-shot command flags
    bool reset_requested;
    bool mode_toggle_requested;

    // Direction debouncing
    Direction last_direction;
    unsigned long last_direction_time;

    // Main button state tracking (for short/long press via 'R'/'T' chars)
    // Note: The R4 sends 'R' or 'T' separately for button events

    // Internal helpers
    bool parsePacket();
    Direction rawToDirection(uint16_t x, uint16_t y);

public:
    UARTInput();
    void init();
    Direction getCommand();
    bool isResetRequested();
    bool isToggleModeRequested();
};

#endif // UART_INPUT_H
