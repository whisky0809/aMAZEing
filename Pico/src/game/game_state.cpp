// game/game_state.cpp
#include "game_state.h"
#include "../config.h"
#include "../sprites/player_sprites.h"
#include "../sprites/goal_sprites.h"

GameState::GameState() {
    active_player = 0;
    winner = 0;
    state = STATE_START;
    lastMoveResult = MOVE_NONE;
    goalMessageStart = 0;
}

void GameState::init() {
    state = STATE_START;
    lastMoveResult = MOVE_NONE;  // Clear any stale move result
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
    uint8_t sx = random(0, MAZE_WIDTH);
    uint8_t sy = random(0, MAZE_HEIGHT);
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

    // Initialize Player 2 at a different position
    #ifdef DEBUG_MODE
    Serial.println("DEBUG: Setting up Player 2...");
    #endif
    uint8_t sx2, sy2;
        int dist_sq_p2;
        int p2_attempts = 0;
        do {
            sx2 = random(0, MAZE_WIDTH);
            sy2 = random(0, MAZE_HEIGHT);

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

    #ifdef DEBUG_MODE
    Serial.println("DEBUG: Picking goal position...");
    #endif

    // Pick goal position (ensure minimum distance from BOTH players)
    uint8_t gx, gy;
    int dist_sq1, dist_sq2;
    int goal_attempts = 0;
    do {
        gx = random(0, MAZE_WIDTH);
        gy = random(0, MAZE_HEIGHT);

        // Check dist from P1
        int dx1 = (int)sx - (int)gx;
        int dy1 = (int)sy - (int)gy;
        dist_sq1 = dx1*dx1 + dy1*dy1;

        // Check dist from P2 (always 2-player)
        int dx2 = (int)players[1].x - (int)gx;
        int dy2 = (int)players[1].y - (int)gy;
        dist_sq2 = dx2*dx2 + dy2*dy2;

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

    // Initialize goal sprite
    SpriteRenderer::initInstance(&goal_sprite, &GOAL_SPRITE);

    active_player = 0;  // Player 1 starts
    state = STATE_PLAYING;
}

void GameState::relocateGoal() {
    // Pick new goal position away from both players
    uint8_t gx, gy;
    int dist_sq1, dist_sq2;
    int goal_attempts = 0;

    do {
        gx = random(0, MAZE_WIDTH);
        gy = random(0, MAZE_HEIGHT);

        // Check dist from P1
        int dx1 = (int)players[0].x - (int)gx;
        int dy1 = (int)players[0].y - (int)gy;
        dist_sq1 = dx1*dx1 + dy1*dy1;

        // Check dist from P2
        int dx2 = (int)players[1].x - (int)gx;
        int dy2 = (int)players[1].y - (int)gy;
        dist_sq2 = dx2*dx2 + dy2*dy2;

        goal_attempts++;
        if (goal_attempts > 1000) break;
    } while (dist_sq1 < 49 || dist_sq2 < 49);  // Min distance ~7 units from both

    maze.setGoal(gx, gy);

    #ifdef DEBUG_MODE
    Serial.print("Goal relocated to (");
    Serial.print(gx);
    Serial.print(",");
    Serial.print(gy);
    Serial.println(")");
    #endif
}

void GameState::handleInput(Direction dir) {
    if (dir == DIR_NONE) return;

    switch (state) {
        case STATE_START:
            resetGame();
            lastMoveResult = MOVE_VALID;  // Signal R4 that game started successfully
            break;

        case STATE_PLAYING:
            handleTwoPlayerMove(dir);
            break;

        case STATE_GOAL_MESSAGE:
            // Ignore input during goal message display
            break;
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
            // Goal reached! Show message, relocate goal, continue game
            state = STATE_GOAL_MESSAGE;
            goalMessageStart = millis();
            lastMoveResult = MOVE_GOAL;
            relocateGoal();  // Move goal to new position
            // Switch turns after goal reached
            active_player = 1 - active_player;
            return;
        }

        // Switch turns (toggle between 0 and 1)
        active_player = 1 - active_player;
        lastMoveResult = MOVE_VALID;
    } else {
        lastMoveResult = MOVE_INVALID;
    }
}

MoveResult GameState::getLastMoveResult() {
    MoveResult result = lastMoveResult;
    lastMoveResult = MOVE_NONE;  // Clear after read
    return result;
}

void GameState::triggerWin() {
    if (state == STATE_PLAYING || state == STATE_GOAL_MESSAGE) {
        winner = active_player;  // Current player wins (escaped)
        state = STATE_WIN;
        Serial.print("[GAME] Player ");
        Serial.print(winner + 1);
        Serial.println(" wins!");
    }
}

bool GameState::isValidMove(const Player& p, Direction dir) {
    // Check grid bounds
    switch (dir) {
        case NORTH: if (p.y == 0) return false; break;
        case SOUTH: if (p.y >= MAZE_HEIGHT - 1) return false; break;
        case EAST:  if (p.x >= MAZE_WIDTH - 1) return false; break;
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
    // Update sprite animations (always 2-player)
    SpriteRenderer::updateAnimation(&players[0].sprite);
    SpriteRenderer::updateAnimation(&players[1].sprite);
    SpriteRenderer::updateAnimation(&goal_sprite);

    // Check if goal message display time has elapsed (2 seconds)
    if (state == STATE_GOAL_MESSAGE && (millis() - goalMessageStart >= 2000)) {
        state = STATE_PLAYING;
    }
}

void GameState::render(DisplayManager* display) {
    display->clear();

    if (state == STATE_START) {
        renderStartScreen(display);
    } else if (state == STATE_GOAL_MESSAGE) {
        renderGoalMessage(display);
    } else if (state == STATE_WIN) {
        renderWinScreen(display);
    } else {
        // STATE_PLAYING
        renderStatusBar(display);
        renderTwoPlayer(display);
    }

    display->update();
}

void GameState::renderStatusBar(DisplayManager* display) {
    display->fillRect(0, 0, 64, STATUS_BAR_HEIGHT, 0x0000);  // Clear bar
    display->setTextSize(1);
    
    // Simple text for now: "P1" or "P2"
    display->setCursor(2, 0);
    display->setTextColor(players[active_player].color);
    display->print("P");
    display->print(active_player + 1);
    display->print(" TURN");
}

void GameState::renderTwoPlayer(DisplayManager* display) {
    // 1. Render Player 1's fog-of-war (blue)
    renderPlayerFog(display, players[0], P1_FOG_COLOR);
    // 2. Render Player 1's blocked indicators
    renderBlockedDirections(display, players[0]);

    // 3. Render Player 2's fog-of-war (purple)
    renderPlayerFog(display, players[1], P2_FOG_COLOR);
    // 4. Render Player 2's blocked indicators
    renderBlockedDirections(display, players[1]);

    // 5. Draw goal (always visible, shared)
    renderGoal(display);

    // 6. Draw both players (always visible)
    SpriteRenderer::draw(display, &players[0].sprite,
                         players[0].x * CELL_SIZE, players[0].y * CELL_SIZE + MAZE_OFFSET_Y);
    SpriteRenderer::draw(display, &players[1].sprite,
                         players[1].x * CELL_SIZE, players[1].y * CELL_SIZE + MAZE_OFFSET_Y);

    // 7. Draw turn indicator (corner square)
    display->drawRect(TURN_INDICATOR_X - 1, TURN_INDICATOR_Y - 1, 6, 6, ACTIVE_HIGHLIGHT);
    display->fillRect(TURN_INDICATOR_X, TURN_INDICATOR_Y, 4, 4,
                      players[active_player].color);
}

void GameState::renderPlayerFog(DisplayManager* display, const Player& p, uint16_t fog_color) {
    uint8_t dirs = p.current_cell_dirs;
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();

    // Helper lambda for drawing thick bordered cells
    auto drawThickBorder = [&](int16_t x, int16_t y) {
        // Draw 2-pixel thick border (outer + inner rectangle)
        display->drawRect(x, y, CELL_SIZE, CELL_SIZE, fog_color);
        display->drawRect(x+1, y+1, CELL_SIZE-2, CELL_SIZE-2, fog_color);
    };

    // Draw border for each valid adjacent cell (don't overdraw goal)
    if ((dirs & (1 << NORTH)) && p.y > 0) {
        if (!(p.x == gx && p.y - 1 == gy))
            drawThickBorder(p.x * CELL_SIZE, (p.y - 1) * CELL_SIZE + MAZE_OFFSET_Y);
    }
    if ((dirs & (1 << SOUTH)) && p.y < MAZE_HEIGHT - 1) {
        if (!(p.x == gx && p.y + 1 == gy))
            drawThickBorder(p.x * CELL_SIZE, (p.y + 1) * CELL_SIZE + MAZE_OFFSET_Y);
    }
    if ((dirs & (1 << EAST)) && p.x < MAZE_WIDTH - 1) {
        if (!(p.x + 1 == gx && p.y == gy))
            drawThickBorder((p.x + 1) * CELL_SIZE, p.y * CELL_SIZE + MAZE_OFFSET_Y);
    }
    if ((dirs & (1 << WEST)) && p.x > 0) {
        if (!(p.x - 1 == gx && p.y == gy))
            drawThickBorder((p.x - 1) * CELL_SIZE, p.y * CELL_SIZE + MAZE_OFFSET_Y);
    }
}

void GameState::renderBlockedDirections(DisplayManager* display, const Player& p) {
    uint8_t dirs = p.current_cell_dirs;
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();

    // For each direction: if cell exists but direction blocked, draw red line
    // on the NEIGHBOR cell's edge facing the player

    // NORTH neighbor - draw red line on ITS bottom edge (facing player)
    if (p.y > 0 && !(dirs & (1 << NORTH))) {
        int16_t nx = p.x * CELL_SIZE;
        int16_t ny = (p.y - 1) * CELL_SIZE + MAZE_OFFSET_Y;
        // Skip if this is the goal cell (handled separately)
        if (!(p.x == gx && p.y - 1 == gy)) {
            display->fillRect(nx, ny + CELL_SIZE - 2, CELL_SIZE, 2, BLOCKED_COLOR);
        }
    }

    // SOUTH neighbor - draw red line on ITS top edge (facing player)
    if (p.y < MAZE_HEIGHT - 1 && !(dirs & (1 << SOUTH))) {
        int16_t nx = p.x * CELL_SIZE;
        int16_t ny = (p.y + 1) * CELL_SIZE + MAZE_OFFSET_Y;
        if (!(p.x == gx && p.y + 1 == gy)) {
            display->fillRect(nx, ny, CELL_SIZE, 2, BLOCKED_COLOR);
        }
    }

    // EAST neighbor - draw red line on ITS left edge (facing player)
    if (p.x < MAZE_WIDTH - 1 && !(dirs & (1 << EAST))) {
        int16_t nx = (p.x + 1) * CELL_SIZE;
        int16_t ny = p.y * CELL_SIZE + MAZE_OFFSET_Y;
        if (!(p.x + 1 == gx && p.y == gy)) {
            display->fillRect(nx, ny, 2, CELL_SIZE, BLOCKED_COLOR);
        }
    }

    // WEST neighbor - draw red line on ITS right edge (facing player)
    if (p.x > 0 && !(dirs & (1 << WEST))) {
        int16_t nx = (p.x - 1) * CELL_SIZE;
        int16_t ny = p.y * CELL_SIZE + MAZE_OFFSET_Y;
        if (!(p.x - 1 == gx && p.y == gy)) {
            display->fillRect(nx + CELL_SIZE - 2, ny, 2, CELL_SIZE, BLOCKED_COLOR);
        }
    }
}

void GameState::drawGoalBarrier(DisplayManager* display, uint8_t gx, uint8_t gy, const Player& p) {
    int16_t gpx = gx * CELL_SIZE;
    int16_t gpy = gy * CELL_SIZE + MAZE_OFFSET_Y;

    // Draw red barrier on the goal cell edge facing the player
    if (p.x < gx) {  // Player is WEST of goal -> barrier on goal's LEFT edge
        display->fillRect(gpx, gpy, 2, CELL_SIZE, BLOCKED_COLOR);
    } else if (p.x > gx) {  // Player is EAST of goal -> barrier on goal's RIGHT edge
        display->fillRect(gpx + CELL_SIZE - 2, gpy, 2, CELL_SIZE, BLOCKED_COLOR);
    } else if (p.y < gy) {  // Player is NORTH of goal -> barrier on goal's TOP edge
        display->fillRect(gpx, gpy, CELL_SIZE, 2, BLOCKED_COLOR);
    } else if (p.y > gy) {  // Player is SOUTH of goal -> barrier on goal's BOTTOM edge
        display->fillRect(gpx, gpy + CELL_SIZE - 2, CELL_SIZE, 2, BLOCKED_COLOR);
    }
}

void GameState::renderGoal(DisplayManager* display) {
    uint8_t gx = maze.getGoalX();
    uint8_t gy = maze.getGoalY();
    int16_t gpx = gx * CELL_SIZE;
    int16_t gpy = gy * CELL_SIZE + MAZE_OFFSET_Y;

    // Calculate distance for color (always 2-player)
    uint8_t dist1 = abs((int)players[0].x - (int)gx) + abs((int)players[0].y - (int)gy);
    uint8_t dist2 = abs((int)players[1].x - (int)gx) + abs((int)players[1].y - (int)gy);
    uint8_t min_dist = (dist2 < dist1) ? dist2 : dist1;

    // Check adjacency and reachability for each player (always 2-player)
    bool p1_adjacent = isAdjacent(players[0], gx, gy);
    bool p2_adjacent = isAdjacent(players[1], gx, gy);
    bool p1_can_reach = p1_adjacent && canReachGoal(players[0], gx, gy);
    bool p2_can_reach = p2_adjacent && canReachGoal(players[1], gx, gy);

    if (p1_can_reach || p2_can_reach) {
        // ACCESSIBLE: Blue door with center 4x4 black ("open" look)
        SpriteRenderer::drawWithColorTint(display, &goal_sprite, gpx, gpy, GOAL_ACCESSIBLE);
        // Clear center 4x4 to black (makes door look "open")
        display->fillRect(gpx + 2, gpy + 2, 4, 4, 0x0000);
    } else if (p1_adjacent || p2_adjacent) {
        // ADJACENT BUT BLOCKED: Green door + red barrier line
        SpriteRenderer::drawWithColorTint(display, &goal_sprite, gpx, gpy, GOAL_COLOR);
        // Draw barrier for whichever player is adjacent but blocked
        if (p1_adjacent && !p1_can_reach) {
            drawGoalBarrier(display, gx, gy, players[0]);
        }
        if (p2_adjacent && !p2_can_reach) {
            drawGoalBarrier(display, gx, gy, players[1]);
        }
    } else {
        // NOT ADJACENT: Normal distance-based coloring
        uint16_t goal_color = getGoalColorForDistance(min_dist);
        SpriteRenderer::drawWithColorTint(display, &goal_sprite, gpx, gpy, goal_color);
    }
}

uint16_t GameState::getGoalColorForDistance(uint8_t min_distance) {
    if (min_distance == 0) return GOAL_COLOR;           // On goal (won) - green
    if (min_distance == 1) return GOAL_DIST_ADJACENT;   // Adjacent - green
    if (min_distance <= 4) return GOAL_DIST_CLOSE;      // Close - yellow
    if (min_distance <= 9) return GOAL_DIST_MEDIUM;     // Medium - orange
    return GOAL_DIST_FAR;                                // Far (≥10) - red
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

    // Footer
    display->setTextColor(GOAL_COLOR);
    display->setCursor(4, 55);
    display->print(TEXT_PRESS_KEY);
}

void GameState::renderGoalMessage(DisplayManager* display) {
    // Display "A little bit more" with rainbow colors
    display->setTextSize(1);

    // Calculate rainbow hue based on time for animation
    uint32_t elapsed = millis() - goalMessageStart;
    uint8_t hue_offset = (elapsed / 50) % 256;  // Shift hue over time

    // Rainbow colors array (6 colors cycling)
    uint16_t rainbow_colors[] = {
        0xF800,  // Red
        0xFC00,  // Orange
        0xFFE0,  // Yellow
        0x07E0,  // Green
        0x001F,  // Blue
        0xF81F   // Magenta
    };

    // Helper buffer for single character printing
    char buf[2] = {0, 0};

    // Line 1 (centered)
    const char* line1 = TEXT_GOAL_L1;
    int16_t x1 = (64 - 8 * 6) / 2;  // 8 chars * 6px = 48px, center
    display->setCursor(x1, 18);
    for (int i = 0; line1[i] != '\0'; i++) {
        uint8_t color_idx = (i + hue_offset / 43) % 6;
        display->setTextColor(rainbow_colors[color_idx]);
        buf[0] = line1[i];
        display->print(buf);
    }

    // Line 2 (centered)
    const char* line2 = TEXT_GOAL_L2;
    int16_t x2 = (64 - 8 * 6) / 2;  // 8 chars * 6px = 48px, center
    display->setCursor(x2, 32);
    for (int i = 0; line2[i] != '\0'; i++) {
        uint8_t color_idx = (i + 4 + hue_offset / 43) % 6;  // Offset by 4 for continuity
        display->setTextColor(rainbow_colors[color_idx]);
        buf[0] = line2[i];
        display->print(buf);
    }
}

void GameState::renderWinScreen(DisplayManager* display) {
    display->setTextSize(1);

    // Line 1: "P1 ESCAPED!" or "P2 ESCAPED!" (winner in their color)
    display->setCursor(4, 4);
    display->setTextColor(players[winner].color);
    display->print("P");
    display->print(winner + 1);
    display->print(TEXT_WIN_ESCAPED);

    // Line 2: "P2 wants" or "P1 wants" (other player)
    uint8_t other = 1 - winner;
    display->setCursor(10, 18);
    display->setTextColor(players[other].color);
    display->print("P");
    display->print(other + 1);
    display->print(TEXT_WIN_WANTS);

    // Lines 3-5: "one more / rabbit / hole..." (white)
    display->setTextColor(0xFFFF);
    display->setCursor(8, 30);
    display->print(TEXT_WIN_L3);
    display->setCursor(14, 42);
    display->print(TEXT_WIN_L4);
    display->setCursor(10, 54);
    display->print(TEXT_WIN_L5);
}