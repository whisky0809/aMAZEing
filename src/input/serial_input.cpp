// input/serial_input.cpp
#include "serial_input.h"

SerialInput::SerialInput() {
    reset_requested = false;
    mode_toggle_requested = false;  // NEW
}

Direction SerialInput::getCommand() {
    if (!Serial.available()) {
        return DIR_NONE;
    }

    char cmd = toupper(Serial.read());

    // Flush remaining buffer to avoid queuing commands
    while (Serial.available()) {
        Serial.read();
    }

    // Parse command
    switch (cmd) {
        case 'W':
            return NORTH;
        case 'A':
            return WEST;
        case 'S':
            return SOUTH;
        case 'D':
            return EAST;
        case 'R':
            reset_requested = true;
            return DIR_NONE;
        case 'T':  // NEW: Toggle two-player mode
            mode_toggle_requested = true;
            return DIR_NONE;
        default:
            return DIR_NONE;
    }
}

bool SerialInput::isResetRequested() {
    if (reset_requested) {
        reset_requested = false;  // Clear flag
        return true;
    }
    return false;
}

bool SerialInput::isToggleModeRequested() {
    if (mode_toggle_requested) {
        mode_toggle_requested = false;
        return true;
    }
    return false;
}