// main.cpp
// Maze Game - Main entry point and game loop
#include <Arduino.h>
#include "config.h"
#include "display/display_manager.h"
#include "game/game_state.h"
#include "input/serial_input.h"
#include "input/uart_input.h"

// Game objects
DisplayManager display;
GameState game;
SerialInput serial_input;
UARTInput uart_input;

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

    // Initialize UART input from R4 controller
    Serial.println("Initializing UART input...");
    uart_input.init();
    Serial.println("UART initialized (GP17=RX, 115200 baud, binary protocol)");

    // Initialize game
    Serial.println("Initializing game state...");
    game.init();

    Serial.println();
    Serial.println("========================================");
    Serial.println("         GAME STARTED!");
    Serial.println("========================================");
    Serial.println("Controls (R4 Joystick via UART):");
    Serial.println("  Joystick: Move in any direction");
    Serial.println("  Button short press: Reset Game");
    Serial.println("  --- USB Serial Backup ---");
    Serial.println("  U/H/J/K = Move, R = Reset");
    Serial.println("  (U=Up, H=Left, J=Down, K=Right)");
    Serial.println("========================================");
    Serial.println();
}

void loop() {
    // Poll BOTH input sources (UART has priority if both active)
    Direction serial_dir = serial_input.getCommand();
    Direction uart_dir = uart_input.getCommand();

    // Use UART input if available, otherwise USB serial
    Direction dir = (uart_dir != DIR_NONE) ? uart_dir : serial_dir;

    if (dir != DIR_NONE) {
        // Debug: Print received direction
        static const char* dirNames[] = {"N", "E", "S", "W"};
        Serial.print("[INPUT] Direction: ");
        Serial.println(dirNames[dir]);

        game.handleInput(dir);

        // Send response to R4 controller
        MoveResult res = game.getLastMoveResult();
        if (res == MOVE_VALID) {
            Serial1.write('V');
            Serial.println("[RESPONSE] V (valid move)");
        } else if (res == MOVE_INVALID) {
            Serial1.write('I');
            Serial.println("[RESPONSE] I (blocked)");
        } else if (res == MOVE_GOAL) {
            Serial1.write('G');
            Serial.println("[RESPONSE] G (goal reached!)");
        }
    }

    // Check for reset request from either source
    if (serial_input.isResetRequested() || uart_input.isResetRequested()) {
        Serial.println("[GAME] Reset");
        game.init();
    }

    // Check for win trigger (power switch flipped)
    if (uart_input.isWinRequested()) {
        game.triggerWin();
    }

    // Update game logic
    game.update();

    // Render to display
    game.render(&display);
    
    // Game loop timing (~60 FPS)
    delay(16);
}
