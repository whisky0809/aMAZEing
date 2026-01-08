// joystick_input.cpp - HW-504 analog joystick input handler
#include "joystick_input.h"
#include <Arduino.h>

JoystickInput::JoystickInput()
    : reset_requested(false)
    , mode_toggle_requested(false)
    , last_direction(DIR_NONE)
    , last_direction_time(0)
    , button_was_pressed(false)
    , button_press_start(0)
{
}

void JoystickInput::init() {
    // Configure button pin with internal pull-up (HW-504 button is active LOW)
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);

    // ADC pins don't need explicit setup on Pico - analogRead() handles it
    // But we can do an initial read to "warm up" the ADC
    analogRead(JOY_X_PIN);
    analogRead(JOY_Y_PIN);
}

Direction JoystickInput::readJoystickDirection() {
    // Read 12-bit ADC values (0-4095 on Pico)
    int x_val = analogRead(JOY_X_PIN);
    int y_val = analogRead(JOY_Y_PIN);

#ifdef DEBUG_MODE
    // Print raw ADC values periodically for calibration
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 500) {
        Serial.print("JOY ADC: X=");
        Serial.print(x_val);
        Serial.print(" Y=");
        Serial.println(y_val);
        last_debug = millis();
    }
#endif

    // Check if in dead zone (center position)
    bool x_centered = (x_val >= JOY_CENTER_MIN && x_val <= JOY_CENTER_MAX);
    bool y_centered = (y_val >= JOY_CENTER_MIN && y_val <= JOY_CENTER_MAX);

    if (x_centered && y_centered) {
        return DIR_NONE;
    }

    // Determine which axis has stronger deflection
    int x_deflection = abs(x_val - JOY_CENTER);  // Distance from center
    int y_deflection = abs(y_val - JOY_CENTER);

    if (x_deflection > y_deflection) {
        // X-axis dominant: WEST or EAST
        if (x_val < JOY_CENTER_MIN) {
            return WEST;
        } else if (x_val > JOY_CENTER_MAX) {
            return EAST;
        }
    } else {
        // Y-axis dominant: NORTH or SOUTH
        if (y_val < JOY_CENTER_MIN) {
            return NORTH;
        } else if (y_val > JOY_CENTER_MAX) {
            return SOUTH;
        }
    }

    return DIR_NONE;
}

bool JoystickInput::isButtonPressed() {
    // HW-504 button is active LOW (pressed = LOW)
    return digitalRead(JOY_BTN_PIN) == LOW;
}

Direction JoystickInput::getCommand() {
    unsigned long now = millis();

    // --- Handle button state machine ---
    bool button_pressed = isButtonPressed();

    if (button_pressed && !button_was_pressed) {
        // Button just pressed - record start time
        button_press_start = now;
        button_was_pressed = true;
    }
    else if (!button_pressed && button_was_pressed) {
        // Button just released - determine press type
        unsigned long press_duration = now - button_press_start;
        button_was_pressed = false;

        if (press_duration >= JOY_LONG_PRESS) {
            // Long press: toggle mode
            mode_toggle_requested = true;
        } else if (press_duration >= JOY_BTN_DEBOUNCE) {
            // Short press: reset
            reset_requested = true;
        }
        // Ignore very short presses (< debounce) as noise
    }

    // --- Handle joystick direction ---
    Direction current_dir = readJoystickDirection();

    // If joystick returned to center, reset tracking
    if (current_dir == DIR_NONE) {
        last_direction = DIR_NONE;
        return DIR_NONE;
    }

    // Debounce: only register if direction changed or enough time passed
    if (current_dir != last_direction) {
        // New direction - check debounce time
        if (now - last_direction_time >= JOY_DEBOUNCE_MS) {
            last_direction = current_dir;
            last_direction_time = now;
            return current_dir;
        }
    }

    // Same direction held, or debouncing - don't send repeat
    return DIR_NONE;
}

bool JoystickInput::isResetRequested() {
    if (reset_requested) {
        reset_requested = false;  // Auto-clear (one-shot)
        return true;
    }
    return false;
}

bool JoystickInput::isToggleModeRequested() {
    if (mode_toggle_requested) {
        mode_toggle_requested = false;  // Auto-clear (one-shot)
        return true;
    }
    return false;
}
