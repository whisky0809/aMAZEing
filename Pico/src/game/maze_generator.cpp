// game/maze_generator.cpp
#include "maze_generator.h"
#include "../config.h"

void MazeGenerator::init() {
    // Default start state
    current_cell_dirs = 0;
    
    // Default goal (will be overwritten by GameState)
    goal_x = MAZE_WIDTH - 1;
    goal_y = MAZE_HEIGHT - 1;

    // Pick a random seed for this maze instance
    // This ensures the maze is consistent (deterministic) for all players
    // during a single game session, but different between resets.
    maze_seed = random(1000000);
}

uint8_t MazeGenerator::getCurrentDirections() {
    return current_cell_dirs;
}

bool MazeGenerator::isDirectionValid(uint8_t x, uint8_t y, Direction dir) {
    // Check if direction is out of bounds (physically impossible)
    if (isOutOfBounds(x, y, dir)) {
        return false;
    }

    // Check if the current cell allows movement in this direction
    return (current_cell_dirs & (1 << dir)) != 0;
}

void MazeGenerator::generateNewDirections(uint8_t x, uint8_t y) {
    // Reverted to non-deterministic generation per user request.
    // This prevents players from getting permanently stuck in loops
    // by allowing the maze structure to change when a cell is re-visited.
    // Note: Players may see different walls at the same location.
    // randomSeed(maze_seed + (long)x * 37 + (long)y * 73); 

    uint8_t dirs = 0;
    // Randomly choose to have 1, 2, or 3 exits from this cell
    uint8_t num_exits = 1 + (random() % 3);

    // Try to add random directions
    int attempts = 0;
    int added = 0;
    
    // Attempt 10 times to pick unique, valid directions.
    while (added < num_exits && attempts < 10) {
        Direction dir = (Direction)(random() % 4); // Pick 0-3

        // Check 1: Is it physically possible? (Not off edge of map)
        // Check 2: Have we already picked this direction? (!(dirs & (1 << dir)))
        if (!isOutOfBounds(x, y, dir) && !(dirs & (1 << dir))) {
            // Add direction to the bitmask using OR operator
            dirs |= (1 << dir);
            added++;
        }
        attempts++;
    }

    // Failsafe: Ensure at least one direction is available if the loop failed
    if (dirs == 0) {
        for (int d = 0; d < 4; d++) {
            if (!isOutOfBounds(x, y, (Direction)d)) {
                dirs |= (1 << d);
                break; // Found one valid exit, stop.
            }
        }
    }

    // Store the generated exits for the current cell
    current_cell_dirs = dirs;
}

void MazeGenerator::setGoal(uint8_t gx, uint8_t gy) {
    goal_x = gx;
    goal_y = gy;
}

bool MazeGenerator::isGoal(uint8_t x, uint8_t y) {
    return (x == goal_x && y == goal_y);
}

bool MazeGenerator::isOutOfBounds(uint8_t x, uint8_t y, Direction dir) {
    switch (dir) {
        case NORTH: return (y == 0);
        case SOUTH: return (y >= MAZE_HEIGHT - 1);
        case EAST:  return (x >= MAZE_WIDTH - 1);
        case WEST:  return (x == 0);
        default:    return true;
    }
}
