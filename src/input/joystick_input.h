// joystick_input.h - HW-504 analog joystick input handler
#ifndef JOYSTICK_INPUT_H
#define JOYSTICK_INPUT_H

#include "../game/maze_generator.h"  // For Direction enum
#include "../config.h"

class JoystickInput {
private:
    // One-shot command flags
    bool reset_requested;
    bool mode_toggle_requested;

    // Direction debouncing state
    Direction last_direction;
    unsigned long last_direction_time;

    // Button state tracking
    bool button_was_pressed;
    unsigned long button_press_start;

    // Internal helpers
    Direction readJoystickDirection();
    bool isButtonPressed();

public:
    JoystickInput();

    // Initialize ADC and GPIO pins
    void init();

    // Poll for direction input (returns DIR_NONE if no input or debouncing)
    Direction getCommand();

    // One-shot flag checks (auto-clear after reading)
    bool isResetRequested();
    bool isToggleModeRequested();
};

#endif // JOYSTICK_INPUT_H
