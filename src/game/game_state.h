// game/game_state.h
// Game state management, player movement, and rendering logic
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <Arduino.h>
#include "../display/display_manager.h"
#include "maze_generator.h"

enum GameMode {
    STATE_START,
    STATE_PLAYING,
    STATE_WIN
};

// Player data structure
struct Player {
    uint8_t x, y;              // Position
    uint16_t color;            // Player color
    uint8_t current_cell_dirs; // NEW: Per-player maze state (bitfield)
    uint16_t moves;            // NEW: Per-player move counter
};

class GameState {
private:
    GameMode state;
    Player players[2];         // NEW: Array of two players
    MazeGenerator maze;
    
    uint8_t active_player;     // NEW: 0 or 1 (whose turn)
    uint8_t winner;            // NEW: 0, 1, or 255 (no winner)
    bool two_player_mode;      // NEW: Mode toggle flag

    void resetGame();
    
    // Internal helpers
    void handleSinglePlayerMove(Direction dir);
    void handleTwoPlayerMove(Direction dir);
    bool isValidMove(const Player& p, Direction dir);
    void movePlayer(Player& p, Direction dir);
    void renderSinglePlayer(DisplayManager* display);
    void renderTwoPlayer(DisplayManager* display);
    void renderPlayerFog(DisplayManager* display, const Player& p, uint16_t fog_color);
    void renderGoal(DisplayManager* display);
    void renderStartScreen(DisplayManager* display);
    void renderWinScreen(DisplayManager* display);
    bool isAdjacent(const Player& p, uint8_t gx, uint8_t gy);
    bool canReachGoal(const Player& p, uint8_t gx, uint8_t gy);

public:
    GameState(); // Constructor
    void init();
    void handleInput(Direction dir);
    void update();
    void render(DisplayManager* display);
    // bool checkWin(); // Removed in favor of state transition logic inside handlers
    
    void setTwoPlayerMode(bool enable);
    uint8_t getActivePlayer();
    uint16_t getPlayerMoves(uint8_t player_id);
    bool isStartScreen() { return state == STATE_START; }
};

#endif // GAME_STATE_H
