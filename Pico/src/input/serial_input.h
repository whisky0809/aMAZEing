// input/serial_input.h
// Serial command parser for WASD controls
#ifndef SERIAL_INPUT_H
#define SERIAL_INPUT_H

#include <Arduino.h>
#include "../game/maze_generator.h"

class SerialInput {
private:
    bool reset_requested;
    // bool mode_toggle_requested;  // Disabled - 2-player only

public:
    SerialInput();
    Direction getCommand();
    bool isResetRequested();
    // bool isToggleModeRequested();  // Disabled - 2-player only
};

#endif // SERIAL_INPUT_H