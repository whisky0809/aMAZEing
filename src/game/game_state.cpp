// game/game_state.cpp
#include "game_state.h"
#include "../config.h"
#include "../sprites/player_sprites.h"

GameState::GameState() {
    two_player_mode = false; // Default
    active_player = 0;
    winner = 255;
    state = STATE_START;
}

void GameState::init() {
    state = STATE_START;
    // two_player_mode preserved from constructor or previous setTwoPlayerMode call
    Serial.println("Game initialized. State: START");
}

void GameState::setTwoPlayerMode(bool enable) {
    two_player_mode = enable;
}

uint8_t GameState::getActivePlayer() {
    return active_player;
}

uint16_t GameState::getPlayerMoves(uint8_t player_id) {
    if (player_id < 2) return players[player_id].moves;
    return 0;
}

void GameState::resetGame() {
    #ifdef DEBUG_MODE
    Serial.println("DEBUG: resetGame() started");
    #endif

    // Initialize maze (clears grid)
    maze.init();

    #ifdef DEBUG_MODE
    Serial.println("DEBUG: Picking player start position...");
    #endif

    // Randomize Start Position
    uint8_t sx = random(0, MAZE_SIZE);
    uint8_t sy = random(0, MAZE_SIZE);
    #ifdef DEBUG_MODE
    Serial.print("DEBUG: P1 start = (");
    Serial.print(sx);
    Serial.print(",");
    Serial.print(sy);
    Serial.println(")");
    #endif

    // Initialize Player 1
    players[0].x = sx;
    players[0].y = sy;
    players[0].color = PLAYER1_COLOR;
    players[0].moves = 0;
    SpriteRenderer::initInstance(&players[0].sprite, &PLAYER1_SPRITE);

    // Generate initial directions at start
    maze.generateNewDirections(sx, sy);
    players[0].current_cell_dirs = maze.getCurrentDirections();
    #ifdef DEBUG_MODE
    Serial.println("DEBUG: P1 initialized");
    #endif

    if (two_player_mode) {
        #ifdef DEBUG_MODE
        Serial.println("DEBUG: Setting up Player 2...");
        #endif
        // Initialize Player 2 at DIFFERENT position
        uint8_t sx2, sy2;
        int dist_sq_p2;
        int p2_attempts = 0;
        do {
            sx2 = random(0, MAZE_SIZE);
            sy2 = random(0, MAZE_SIZE);

            // Ensure some distance from Player 1
            int dx = (int)sx - (int)sx2;
            int dy = (int)sy - (int)sy2;
            dist_sq_p2 = dx*dx + dy*dy;
            p2_attempts++;
            if (p2_attempts > 1000) {
                #ifdef DEBUG_MODE
                Serial.println("DEBUG: P2 placement timeout!");
                #endif
                break;
            }
        } while (dist_sq_p2 < 25); // Min distance 5 units

        players[1].x = sx2;
        players[1].y = sy2;
        players[1].color = PLAYER2_COLOR;
        players[1].moves = 0;
        SpriteRenderer::initInstance(&players[1].sprite, &PLAYER2_SPRITE);

        // Generate initial directions for Player 2
        maze.generateNewDirections(sx2, sy2);
        players[1].current_cell_dirs = maze.getCurrentDirections();
        #ifdef DEBUG_MODE
        Serial.println("DEBUG: P2 initialized");
        #endif
    }

    #ifdef DEBUG_MODE
    Serial.println("DEBUG: Picking goal position...");
    #endif

    // Pick goal position (ensure minimum distance from BOTH players)
    uint8_t gx, gy;
    int dist_sq1, dist_sq2;
    int goal_attempts = 0;
    do {
        gx = random(0, MAZE_SIZE);
        gy = random(0, MAZE_SIZE);

        // Check dist from P1
        int dx1 = (int)sx - (int)gx;
        int dy1 = (int)sy - (int)gy;
        dist_sq1 = dx1*dx1 + dy1*dy1;

        // Check dist from P2 (if relevant)
        if (two_player_mode) {
             int dx2 = (int)players[1].x - (int)gx;
             int dy2 = (int)players[1].y - (int)gy;
             dist_sq2 = dx2*dx2 + dy2*dy2;
        } else {
            dist_sq2 = 999; // Always pass
        }

        goal_attempts++;
        if (goal_attempts > 1000) {
            #ifdef DEBUG_MODE
            Serial.println("DEBUG: Goal placement timeout!");
            #endif
            break;
        }
    } while (dist_sq1 < 100 || dist_sq2 < 100); // Min distance ~10 units from both

    #ifdef DEBUG_MODE
    Serial.print("DEBUG: Goal = (");
    Serial.print(gx);
    Serial.print(",");
    Serial.print(gy);
    Serial.println(")");
    #endif

    maze.setGoal(gx, gy);

    active_player = 0;  // Player 1 starts
    winner = 255;       // No winner yet
    state = STATE_PLAYING;

    Serial.println("Game Reset. State: PLAYING");
    if (two_player_mode) Serial.println("Mode: Two Player");
}

void GameState::handleInput(Direction dir) {
    if (dir == DIR_NONE) return;

    switch (state) {
        case STATE_START:
            resetGame();
            break;

        case STATE_PLAYING:
            if (two_player_mode) {
                handleTwoPlayerMove(dir);
            } else {
                handleSinglePlayerMove(dir);
            }
            break;

        case STATE_WIN:
            state = STATE_START;
            break;
    }
}

void GameState::handleSinglePlayerMove(Direction dir) {
    Player& p = players[0];

    #ifdef DEBUG_MODE
    // Debug: Show what directions are available
    Serial.print("Input: ");
    Serial.print(dir == NORTH ? "N" : dir == SOUTH ? "S" : dir == EAST ? "E" : dir == WEST ? "W" : "?");
    Serial.print(" | Pos: (");
    Serial.print(p.x);
    Serial.print(",");
    Serial.print(p.y);
    Serial.print(") | Available: ");
    if (p.current_cell_dirs & (1 << NORTH)) Serial.print("N");
    if (p.current_cell_dirs & (1 << SOUTH)) Serial.print("S");
    if (p.current_cell_dirs & (1 << EAST)) Serial.print("E");
    if (p.current_cell_dirs & (1 << WEST)) Serial.print("W");
    #endif

    if (isValidMove(p, dir)) {
        movePlayer(p, dir);
        p.moves++;
        maze.generateNewDirections(p.x, p.y);
        p.current_cell_dirs = maze.getCurrentDirections();
        #ifdef DEBUG_MODE
        Serial.println(" -> MOVED");
        #endif

        if (maze.isGoal(p.x, p.y)) {
            state = STATE_WIN;
            winner = 0;
        }
    } else {
        #ifdef DEBUG_MODE
        Serial.println(" -> BLOCKED");
        #endif
    }
}

void GameState::handleTwoPlayerMove(Direction dir) {
    Player& p = players[active_player];

    if (isValidMove(p, dir)) {
        // Valid move: execute and switch turns
        movePlayer(p, dir);
        p.moves++;
        
        // Generate directions for THIS player's new location
        maze.generateNewDirections(p.x, p.y);
        p.current_cell_dirs = maze.getCurrentDirections();

        if (maze.isGoal(p.x, p.y)) {
            state = STATE_WIN;
            winner = active_player;
            return;
        }

        // Switch turns (toggle between 0 and 1)
        active_player = 1 - active_player;
    }
    // Invalid moves are ignored - no turn switch
}

bool GameState::isValidMove(const Player& p, Direction dir) {
    // Check grid bounds
    switch (dir) {
        case NORTH: if (p.y == 0) return false; break;
        case SOUTH: if (p.y >= MAZE_SIZE - 1) return false; break;
        case EAST:  if (p.x >= MAZE_SIZE - 1) return false; break;
        case WEST:  if (p.x == 0) return false; break;
        default: return false;
    }

    // Check player's current cell allows this direction
    return (p.current_cell_dirs & (1 << dir)) != 0;
}

void GameState::movePlayer(Player& p, Direction dir) {
    switch (dir) {
        case NORTH: p.y--; break;
        case SOUTH: p.y++; break;
        case EAST:  p.x++; break;
        case WEST:  p.x--; break;
    }
}

void GameState::update() {
    // Update sprite animations
    SpriteRenderer::updateAnimation(&players[0].sprite);
    if (two_player_mode) {
        SpriteRenderer::updateAnimation(&players[1].sprite);
    }
}

void GameState::render(DisplayManager* display) {
    display->clear();

    if (state == STATE_START) {
        renderStartScreen(display);
    } else if (state == STATE_WIN) {
        renderWinScreen(display);
    } else {
        // STATE_PLAYING
        if (two_player_mode) {
            renderTwoPlayer(display);
        } else {
            renderSinglePlayer(display);
        }
    }

    display->update();
}

void GameState::renderSinglePlayer(DisplayManager* display) {
    // Re-use standard fog color (Blue/PATH_COLOR)
    renderPlayerFog(display, players[0], PATH_COLOR);
    renderGoal(display);
    // Render player sprite
    SpriteRenderer::draw(display, &players[0].sprite,
                         players[0].x * CELL_SIZE, players[0].y * CELL_SIZE);
}

void GameState::renderTwoPlayer(DisplayManager* display) {
    // 1. Render Player 1's fog-of-war (blue)
    renderPlayerFog(display, players[0], P1_FOG_COLOR);

    // 2. Render Player 2's fog-of-war (purple)
    renderPlayerFog(display, players[1], P2_FOG_COLOR);

    // 3. Draw goal (always visible, shared)
    renderGoal(display);

    // 4. Draw both players (always visible)
    SpriteRenderer::draw(display, &players[0].sprite,
                         players[0].x * CELL_SIZE, players[0].y * CELL_SIZE);
    SpriteRenderer::draw(display, &players[1].sprite,
                         players[1].x * CELL_SIZE, players[1].y * CELL_SIZE);
    
    // 5. Draw turn indicator (corner square)
    // Draw border
    display->drawRect(TURN_INDICATOR_X - 1, TURN_INDICATOR_Y - 1, 6, 6, ACTIVE_HIGHLIGHT);
    // Draw inner color
    display->fillRect(TURN_INDICATOR_X, TURN_INDICATOR_Y, 4, 4,
                      players[active_player].color);
}

void GameState::renderPlayerFog(DisplayManager* display, const Player& p, uint16_t fog_color) {
    uint8_t dirs = p.current_cell_dirs;
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();

    // Draw each valid adjacent cell (don't overdraw goal)
    if ((dirs & (1 << NORTH)) && p.y > 0) {
        if (!(p.x == gx && p.y - 1 == gy))
            display->fillRect(p.x * CELL_SIZE, (p.y - 1) * CELL_SIZE, CELL_SIZE, CELL_SIZE, fog_color);
    }
    if ((dirs & (1 << SOUTH)) && p.y < MAZE_SIZE - 1) {
        if (!(p.x == gx && p.y + 1 == gy))
            display->fillRect(p.x * CELL_SIZE, (p.y + 1) * CELL_SIZE, CELL_SIZE, CELL_SIZE, fog_color);
    }
    if ((dirs & (1 << EAST)) && p.x < MAZE_SIZE - 1) {
        if (!(p.x + 1 == gx && p.y == gy))
            display->fillRect((p.x + 1) * CELL_SIZE, p.y * CELL_SIZE, CELL_SIZE, CELL_SIZE, fog_color);
    }
    if ((dirs & (1 << WEST)) && p.x > 0) {
        if (!(p.x - 1 == gx && p.y == gy))
            display->fillRect((p.x - 1) * CELL_SIZE, p.y * CELL_SIZE, CELL_SIZE, CELL_SIZE, fog_color);
    }
}

void GameState::renderGoal(DisplayManager* display) {
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();

    bool p1_can_reach = false;
    bool p2_can_reach = false;

    // Check if Player 1 can reach goal
    if (isAdjacent(players[0], gx, gy)) {
        p1_can_reach = canReachGoal(players[0], gx, gy);
    }

    // Check if Player 2 can reach goal (if in two-player mode)
    if (two_player_mode && isAdjacent(players[1], gx, gy)) {
        p2_can_reach = canReachGoal(players[1], gx, gy);
    }

    // Screen position of this cell
    int16_t sx = gx * CELL_SIZE;
    int16_t sy = gy * CELL_SIZE;
    int16_t half = CELL_SIZE / 2;

    if (p1_can_reach || p2_can_reach) {
        // Checkerboard pattern (reachable) - 2x2 grid of half-sized squares
        display->fillRect(sx,        sy,        half, half, GOAL_COLOR);  // Top-left
        display->fillRect(sx + half, sy,        half, half, PATH_COLOR);  // Top-right
        display->fillRect(sx,        sy + half, half, half, PATH_COLOR);  // Bottom-left
        display->fillRect(sx + half, sy + half, half, half, GOAL_COLOR);  // Bottom-right
    } else {
        // Solid green (visible but not adjacent)
        display->fillRect(sx, sy, CELL_SIZE, CELL_SIZE, GOAL_COLOR);
    }
}

bool GameState::isAdjacent(const Player& p, uint8_t gx, uint8_t gy) {
    int dx = abs((int)p.x - (int)gx);
    int dy = abs((int)p.y - (int)gy);
    return (dx + dy == 1);
}

bool GameState::canReachGoal(const Player& p, uint8_t gx, uint8_t gy) {
    uint8_t dirs = p.current_cell_dirs;
    if (gx > p.x && (dirs & (1 << EAST))) return true;
    if (gx < p.x && (dirs & (1 << WEST))) return true;
    if (gy > p.y && (dirs & (1 << SOUTH))) return true;
    if (gy < p.y && (dirs & (1 << NORTH))) return true;
    return false;
}

void GameState::renderStartScreen(DisplayManager* display) {
    display->setTextSize(1);
    
    // Line 1: "It's" (Centered)
    // "It's" is 4 chars -> 24px wide. Center is (64-24)/2 = 20
    display->setTextColor(0xFFFF); // White
    display->setCursor(20, 10);
    display->print(TEXT_TITLE_L1);

    // Line 2: "aMAZEing"
    // "aMAZEing" is 8 chars -> 48px wide. Center is (64-48)/2 = 8
    // Start X = 8
    int cursorX = 8;
    int cursorY = 22; // Line 2 Y position

    // "a" - White
    display->setTextColor(0xFFFF);
    display->setCursor(cursorX, cursorY);
    display->print(TEXT_TITLE_L2_PRE);
    cursorX += 6; // Advance 1 char

    // "MAZE" - Red
    display->setTextColor(PLAYER_COLOR); // Red
    display->setCursor(cursorX, cursorY);
    display->print(TEXT_TITLE_L2_MAIN);
    cursorX += 24; // Advance 4 chars

    // "ing" - White
    display->setTextColor(0xFFFF); // White
    display->setCursor(cursorX, cursorY);
    display->print(TEXT_TITLE_L2_SUF);

    // Mode Indicator
    if (two_player_mode) {
        display->setTextColor(PLAYER2_COLOR);
        display->setCursor(8, 35);
        display->print(TEXT_MODE_2P);
    } else {
        display->setTextColor(PATH_COLOR);
        display->setCursor(8, 35);
        display->print(TEXT_MODE_1P);
    }

    // Toggle Instruction
    display->setTextColor(0xAAAA); // Light Gray
    display->setCursor(10, 45);
    display->print(TEXT_TOGGLE_BTN);

    // Footer
    display->setTextColor(GOAL_COLOR);
    display->setCursor(4, 55);
    display->print(TEXT_PRESS_KEY);
}

void GameState::renderWinScreen(DisplayManager* display) {
    if (!two_player_mode) {
        // Existing single-player win screen
        display->setTextColor(GOAL_COLOR);
        display->setCursor(10, 10);
        display->print(TEXT_YOU_WIN);
        display->setTextColor(0xFFFF);
        display->setCursor(10, 25);
        display->print(TEXT_MOVES_LABEL);
        display->print(players[0].moves);
    } else {
        // Two-player win screen
        display->setTextColor(GOAL_COLOR);
        display->setCursor(4, 5);
        display->print(TEXT_PLAYER_PREFIX);
        display->print(winner + 1);
        display->setCursor(10, 18);
        display->print(TEXT_WINS_SUFFIX);

        // Show both move counts
        display->setTextColor(PLAYER1_COLOR);
        display->setCursor(4, 32);
        display->print(TEXT_P1_LABEL);
        display->print(players[0].moves);

        
        display->setTextColor(PLAYER2_COLOR);
        display->setCursor(4, 42);
        display->print(TEXT_P2_LABEL);
        display->print(players[1].moves);
    }

    display->setTextColor(PLAYER_COLOR); // Red
    display->setCursor(4, 54);
    display->print(TEXT_PRESS_KEY);
}