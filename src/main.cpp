// main.cpp
// Maze Game - Main entry point and game loop
#include <Arduino.h>
#include "config.h"
#include "display/display_manager.h"
#include "game/game_state.h"
#include "input/serial_input.h"
#include "input/joystick_input.h"

// Game objects
DisplayManager display;
GameState game;
SerialInput serial_input;
JoystickInput joystick_input;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);  // Wait for serial connection

    Serial.println("========================================");
    Serial.println("  Maze Game - Raspberry Pi Pico W");
    Serial.println("  64x64 LED Matrix Edition");
    Serial.println("========================================");
    Serial.println();

    // Initialize display
    Serial.println("Initializing display...");
    if (!display.init()) {
        Serial.println("ERROR: Display initialization failed!");
        Serial.println("Check library installation and wiring.");
        while (1) {
            delay(1000);  // Halt execution
        }
    }
    Serial.println("Display initialized successfully!");

    // Initialize random seed using ADC noise from temp sensor
    randomSeed(analogRead(28));
    Serial.println("Random seed initialized");

    // Initialize joystick input
    Serial.println("Initializing joystick...");
    joystick_input.init();
    Serial.println("Joystick initialized (GP26=X, GP27=Y, GP22=BTN)");

    // Initialize game
    Serial.println("Initializing game state...");
    game.init();

    Serial.println();
    Serial.println("========================================");
    Serial.println("         GAME STARTED!");
    Serial.println("========================================");
    Serial.println("Controls:");
    Serial.println("  Joystick: Move in any direction");
    Serial.println("  Button short press: Reset Game");
    Serial.println("  Button long press: Toggle Mode");
    Serial.println("  --- Serial Backup ---");
    Serial.println("  W/A/S/D = Move, R = Reset, T = Mode");
    Serial.println("========================================");
    Serial.println();
}

void loop() {
    // Poll BOTH input sources (joystick has priority if both active)
    Direction serial_dir = serial_input.getCommand();
    Direction joy_dir = joystick_input.getCommand();

    // Use joystick input if available, otherwise serial
    Direction dir = (joy_dir != DIR_NONE) ? joy_dir : serial_dir;

    if (dir != DIR_NONE) {
        game.handleInput(dir);
    }

    // Check for reset request from either source
    if (serial_input.isResetRequested() || joystick_input.isResetRequested()) {
        Serial.println();
        Serial.println("========================================");
        Serial.println("         GAME RESET");
        Serial.println("========================================");
        game.init();
        Serial.println();
    }

    // Handle mode toggle from either source
    bool toggle_requested = serial_input.isToggleModeRequested() ||
                            joystick_input.isToggleModeRequested();
    if (toggle_requested && game.isStartScreen()) {
        static bool two_player = false;
        two_player = !two_player;
        game.setTwoPlayerMode(two_player);
        Serial.println("========================================");
        Serial.print("  Two-player mode: ");
        Serial.println(two_player ? "ON" : "OFF");
        Serial.println("========================================");
        game.init();  // Restart with new mode
    }

    // Update game logic
    game.update();

    // Render to display
    game.render(&display);
    
    // Game loop timing (~60 FPS)
    delay(16);
}
