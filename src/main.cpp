// main.cpp
// Maze Game - Main entry point and game loop
#include <Arduino.h>
#include "config.h"
#include "display/display_manager.h"
#include "game/game_state.h"
#include "input/serial_input.h"

// Game objects
DisplayManager display;
GameState game;
SerialInput input;

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

    // Initialize game
    Serial.println("Initializing game state...");
    game.init();

    Serial.println();
    Serial.println("========================================");
    Serial.println("         GAME STARTED!");
    Serial.println("========================================");
    Serial.println("Controls:");
    Serial.println("  W = Move Up");
    Serial.println("  A = Move Left");
    Serial.println("  S = Move Down");
    Serial.println("  D = Move Right");
    Serial.println("  R = Reset Game");
    Serial.println("========================================");
    Serial.println();
}

void loop() {
    // Handle input
    Direction dir = input.getCommand();

    if (dir != DIR_NONE) {
        game.handleInput(dir);
    }
    
    // Check for reset request
    if (input.isResetRequested()) {
        Serial.println();
        Serial.println("========================================");
        Serial.println("         GAME RESET");
        Serial.println("========================================");
        game.init();
        Serial.println();
    }

    // Handle mode toggle
    if (input.isToggleModeRequested() && game.isStartScreen()) {
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
