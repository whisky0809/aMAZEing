// game/maze_generator.h
// Procedural maze generation using random direction assignment
// optimized for memoryless/rogue-like gameplay (no grid storage)
#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

#include <Arduino.h>

// Movement directions
enum Direction {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3,
    DIR_NONE = 4
};

class MazeGenerator {
private:
    uint8_t current_cell_dirs; // Bitfield for the active cell only
    uint8_t goal_x, goal_y;
    long maze_seed;            // NEW: Seed for deterministic generation

public:
    void init();
    
    // Returns exits for the CURRENT cell (where player is)
    uint8_t getCurrentDirections();
    
    // Checks if direction is valid from CURRENT cell
    bool isDirectionValid(uint8_t x, uint8_t y, Direction dir);
    
    // Regenerates exits for the given position (updates current_cell_dirs)
    void generateNewDirections(uint8_t x, uint8_t y);
    
    void setGoal(uint8_t gx, uint8_t gy);
    bool isGoal(uint8_t x, uint8_t y);
    uint8_t getGoalX() { return goal_x; }
    uint8_t getGoalY() { return goal_y; }

private:
    bool isOutOfBounds(uint8_t x, uint8_t y, Direction dir);
};

#endif // MAZE_GENERATOR_H
