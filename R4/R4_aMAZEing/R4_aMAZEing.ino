#include <Keyboard.h>
#include <Mouse.h>

/*
  R4 Controller for aMAZEing maze game.

  UART Protocol: 8-byte binary packets to Pico
    [0xAA][0x55][Xlo][Xhi][Ylo][Yhi][mask][checksum]

  Control Modes:
  - PROTOPIE_MODE: Joystick controls mouse cursor, D6 = left click
  - MAZE_HELD: D9 held - joystick controls maze (one move per hold)

  State Flow:
    - D7 (ENTER) starts the game (sends 's' to Pico)
    - D9 held: switches to MAZE_HELD, sends 'H' to Pico
    - Make one move with joystick (direction packet sent)
    - D9 released: sends 'U' to Pico, 't' to ProtoPie if move was made

  Buttons: A/B/C/D send keyboard letters, ENTER starts game + sends return key
*/

/// ---------- Pins ----------
static const int PIN_VRX = A0;
static const int PIN_VRY = A1;
static const int PIN_SW  = 6;       // Joystick switch on D6 (left click)
static const int BTN_TOGGLE = 9;    // Mode toggle button on D9

static const int BTN_A = 2;
static const int BTN_B = 3;
static const int BTN_C = 4;
static const int BTN_D = 5;

static const int BTN_ENTER = 7;
static const int SW_POWER  = 8;


/// ---------- UART ----------
static const uint32_t BAUD_PICO = 115200;
static const uint16_t SEND_PERIOD_MS = 20;

/// ---------- Joystick ----------
static const int CENTER = 512;
static const int DEADZONE = 120;
static const int Y_POLARITY = -1;

/// ---------- Mouse ----------
static const int MAX_SPEED = 10;
static const uint16_t MOUSE_UPDATE_MS = 10;

/// ---------- Debounce ----------
static const uint16_t BTN_DEBOUNCE_MS = 140;

/// ---------- Protocol ----------
static const uint8_t H0 = 0xAA;
static const uint8_t H1 = 0x55;

/// ---------- Mode State ----------
enum ControlState {
  PROTOPIE_MODE,     // Joystick = mouse, D6 = left click, maze frozen
  MAZE_HELD          // D9 held - joystick controls maze (one move per hold)
};
static ControlState state = PROTOPIE_MODE;

/// ---------- Timing ----------
static uint32_t lastSendMs = 0;
static uint32_t lastMouseMs = 0;
static uint32_t lastA=0, lastB=0, lastC=0, lastD=0;
static uint32_t lastEnter = 0;
static uint32_t lastClick = 0;   // D6 left click debounce


/// ---------- Power switch ----------
static const int POWER_OFF_LEVEL = HIGH;
static bool powerWasOff = false;

/// ---------- Game start ----------
static bool gameStarted = false;  // Has the maze game been started?

/// ---------- D9 Hold Detection ----------
enum Dir { DIR_NEUTRAL, DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };

static bool d9_currently_held = false;      // Real-time D9 button state
static bool move_made_this_hold = false;    // Has a valid move been made this hold cycle?
static uint32_t lastD9Check = 0;            // For D9 polling interval
static Dir lastDir = DIR_NEUTRAL;

/// ---------- Helpers ----------
static inline int applyDeadzone(int v) {
  return (abs(v) < DEADZONE) ? 0 : v;
}

static inline int joyToSpeed(int v) {
  long s = (long)v * MAX_SPEED / 512;
  if (s >  MAX_SPEED) s =  MAX_SPEED;
  if (s < -MAX_SPEED) s = -MAX_SPEED;
  return (int)s;
}

static inline void tapKeyLower(char c) { Keyboard.write(c); }

static const bool SEND_UPPERCASE_ABCD = false;

static inline void tapABCD(char letterUpper) {
  if (SEND_UPPERCASE_ABCD) {
    char lower = (char)(letterUpper + 32);
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.write(lower);
    Keyboard.release(KEY_LEFT_SHIFT);
  } else {
    tapKeyLower((char)(letterUpper + 32)); // a/b/c/d
  }
}

static void sendPacket(uint16_t rawX, uint16_t rawY, uint8_t btnMask) {
  uint8_t p[8];
  p[0] = H0;
  p[1] = H1;
  p[2] = (uint8_t)(rawX & 0xFF);
  p[3] = (uint8_t)(rawX >> 8);
  p[4] = (uint8_t)(rawY & 0xFF);
  p[5] = (uint8_t)(rawY >> 8);
  p[6] = btnMask;

  uint8_t cs = 0;
  for (int i = 0; i <= 6; i++) cs ^= p[i];
  p[7] = cs;

  Serial1.write(p, sizeof(p));
}


static inline Dir getDirFromXY(int x, int y) {
  if (x == 0 && y == 0) return DIR_NEUTRAL;
  if (abs(x) >= abs(y)) {
    return (x > 0) ? DIR_RIGHT : DIR_LEFT;
  } else {
    return (y > 0) ? DIR_DOWN : DIR_UP;
  }
}

static inline void dirToRaw(Dir d, uint16_t &outX, uint16_t &outY) {
  switch (d) {
    case DIR_LEFT:  outX = 0;    outY = CENTER; break;
    case DIR_RIGHT: outX = 1023; outY = CENTER; break;
    case DIR_UP:    outX = CENTER; outY = 0;    break;
    case DIR_DOWN:  outX = CENTER; outY = 1023; break;
    default:        outX = CENTER; outY = CENTER; break;
  }
}

void setup() {
  pinMode(PIN_SW, INPUT_PULLUP);     // D6 - joystick switch (left click)
  pinMode(BTN_TOGGLE, INPUT_PULLUP); // D9 - mode toggle
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(SW_POWER, INPUT_PULLUP);

  Serial.begin(115200);
  delay(200);

  Keyboard.begin();
  delay(250);
  Mouse.begin();
  delay(250);

  Serial1.begin(BAUD_PICO);

  powerWasOff = (digitalRead(SW_POWER) == POWER_OFF_LEVEL);

  Serial.println("UNO R4 started. Mode: PROTOPIE_MODE");
  Serial.println("D6=click, D9=toggle mode, ABCD=keys");
}

void loop() {
  uint32_t now = millis();

  // 1. Power Switch Logic
  bool powerIsOff = (digitalRead(SW_POWER) == POWER_OFF_LEVEL);
  if (powerIsOff && !powerWasOff) {
    // Power switched OFF -> Win trigger
    Keyboard.write('o');
    Serial1.write('o');  // Tell Pico to show win screen
    Serial.println("[POWER] OFF -> win trigger");
  } else if (!powerIsOff && powerWasOff) {
    // Power switched ON -> Reset game
    Serial1.write('R');  // Tell Pico to reset
    gameStarted = false;  // Reset start state
    state = PROTOPIE_MODE;
    Serial.println("[POWER] ON -> reset");
  }
  powerWasOff = powerIsOff;

  // 2. Read Inputs
  int rawX_i = analogRead(PIN_VRX);
  int rawY_i = analogRead(PIN_VRY);
  int yCentered = (rawY_i - CENTER) * Y_POLARITY;
  int rawY_fixed = constrain(yCentered + CENTER, 0, 1023);
  uint16_t rawX = (uint16_t)constrain(rawX_i, 0, 1023);
  uint16_t rawY = (uint16_t)rawY_fixed;
  
  int cx = (int)rawX - CENTER;
  int cy = (int)rawY - CENTER;
  // Joystick mounted rotated: swap axes (negation removed to fix inversion)
  int x = applyDeadzone(cy);
  int y = applyDeadzone(cx);

  uint8_t mask = 0;  // Button mask for Pico (not used for mode control)

  // 3. Global Button Handlers (A, B, C, D, Enter)
  if (digitalRead(BTN_ENTER) == LOW && (now - lastEnter >= BTN_DEBOUNCE_MS)) {
    Keyboard.write(KEY_RETURN);
    if (!gameStarted) {
      Serial1.write('s');  // Start game on Pico
      gameStarted = true;
      Serial.println("[ENTER] Game started");
    } else {
      Serial.println("[KEY] ENTER");
    }
    lastEnter = now;
  }
  if (digitalRead(BTN_A) == LOW && (now - lastA >= BTN_DEBOUNCE_MS)) { tapABCD('A'); lastA = now; Serial.println("[KEY] A"); }
  if (digitalRead(BTN_B) == LOW && (now - lastB >= BTN_DEBOUNCE_MS)) { tapABCD('B'); lastB = now; Serial.println("[KEY] B"); }
  if (digitalRead(BTN_C) == LOW && (now - lastC >= BTN_DEBOUNCE_MS)) { tapABCD('C'); lastC = now; Serial.println("[KEY] C"); }
  if (digitalRead(BTN_D) == LOW && (now - lastD >= BTN_DEBOUNCE_MS)) { tapABCD('D'); lastD = now; Serial.println("[KEY] D"); }

  // 4. D9 Hold Detection (poll every ~10ms)
  if (now - lastD9Check >= 10) {
    bool d9_pressed = (digitalRead(BTN_TOGGLE) == LOW);

    if (d9_pressed && !d9_currently_held) {
      // D9 just pressed down
      d9_currently_held = true;
      move_made_this_hold = false;  // Reset for new hold cycle
      lastDir = DIR_NEUTRAL;        // Reset direction tracking
      state = MAZE_HELD;
      Serial1.write('H');  // Tell Pico D9 is held
      Serial.println("[D9] HELD -> MAZE mode");
    }
    else if (!d9_pressed && d9_currently_held) {
      // D9 just released
      d9_currently_held = false;
      state = PROTOPIE_MODE;
      Serial1.write('U');  // Tell Pico D9 released (Unheld)

      // If a valid move was made this hold, notify ProtoPie of turn end
      if (move_made_this_hold) {
        Keyboard.write('t');  // Tell ProtoPie turn complete
        Serial.println("[D9] RELEASED after move -> 't' to ProtoPie");
      } else {
        Serial.println("[D9] RELEASED (no move made)");
      }
    }

    lastD9Check = now;
  }

  // 5. State Machine
  switch (state) {
    case PROTOPIE_MODE:
      // Mouse Control Active
      if (now - lastMouseMs >= MOUSE_UPDATE_MS) {
        int dx = joyToSpeed(x);
        int dy = joyToSpeed(y);
        if (dx != 0 || dy != 0) Mouse.move(dx, dy, 0);
        lastMouseMs = now;
      }
      // D6 Left Click Handler (only in PROTOPIE_MODE)
      if (digitalRead(PIN_SW) == LOW && (now - lastClick >= BTN_DEBOUNCE_MS)) {
        Mouse.click(MOUSE_LEFT);
        lastClick = now;
        Serial.println("[CLICK] Left");
      }
      // Keep Pico Maze Frozen
      if (now - lastSendMs >= SEND_PERIOD_MS) {
        sendPacket(CENTER, CENTER, mask);
        lastSendMs = now;
      }
      break;

    case MAZE_HELD:
      // D9 is held - waiting for joystick input
      // Only allow one move per hold cycle
      if (!move_made_this_hold) {
        Dir dirNow = getDirFromXY(x, y);
        // Trigger on Neutral -> Direction
        if (lastDir == DIR_NEUTRAL && dirNow != DIR_NEUTRAL) {
          // Send direction packet to Pico
          uint16_t px, py;
          dirToRaw(dirNow, px, py);
          sendPacket(px, py, mask);
          move_made_this_hold = true;  // Block further moves until D9 released
          static const char* dirNames[] = {"NEUTRAL", "LEFT", "RIGHT", "UP", "DOWN"};
          Serial.print("[MAZE] Move sent: ");
          Serial.println(dirNames[dirNow]);
        }
        lastDir = dirNow;
      }

      // Keep sending neutral while D9 held (after move or while waiting)
      if (now - lastSendMs >= SEND_PERIOD_MS) {
        if (!move_made_this_hold) {
          // Still waiting for direction - send neutral
          sendPacket(CENTER, CENTER, mask);
        }
        lastSendMs = now;
      }
      break;
  }
}