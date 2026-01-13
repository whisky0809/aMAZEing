#include <Keyboard.h>
#include <Mouse.h>

/*
  Pico expects 8-byte binary packets:
    AA 55 Xlo Xhi Ylo Yhi mask checksum(XOR 0..6)

  Turn-based behavior (NEW):
  - MINIGAME_MODE: Mouse control active, maze frozen. Waiting for BTN_A.
  - MAZE_ARMED: Mouse frozen. Waiting for Joystick move.
  - MAZE_PULSING: Sending ONE move to Pico.
  - WAITING_ACK: Waiting for V/I/W from Pico.
*/

/// ---------- Pins ----------
static const int PIN_VRX = A0;
static const int PIN_VRY = A1;
static const int PIN_SW  = 9;

static const int BTN_A = 2;
static const int BTN_B = 3;
static const int BTN_C = 4;
static const int BTN_D = 5;

static const int BTN_ENTER = 7;
static const int SW_POWER  = 8;

// NEW: Reset button (Plan suggests 13)
static const int BTN_RESET = 13;

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
static const uint16_t SW_DEBOUNCE_MS = 30;
static const uint16_t BTN_DEBOUNCE_MS = 140;

/// ---------- Protocol ----------
static const uint8_t H0 = 0xAA;
static const uint8_t H1 = 0x55;

/// ---------- Mode State ----------
enum Player { PLAYER_1, PLAYER_2 };
static Player currentPlayer = PLAYER_1;

enum ControlState {
  MINIGAME_MODE,     // ProtoPie mouse control, maze frozen, waiting for BTN_A
  MAZE_ARMED,        // BTN_A pressed, waiting for joystick direction
  MAZE_PULSING,      // Sending move pulse to Pico
  WAITING_ACK        // Waiting for Pico's V/I/W response
};
static ControlState state = MINIGAME_MODE;

/// ---------- Timing ----------
static uint32_t lastSendMs = 0;
static uint32_t lastMouseMs = 0;
static uint32_t lastA=0, lastB=0, lastC=0, lastD=0;
static uint32_t lastEnter = 0;
static uint32_t lastReset = 0;

/// ---------- Joystick-switch debounce ----------
static bool swStable = false;
static bool swLastReading = false;
static uint32_t swLastChange = 0;

/// ---------- Power switch ----------
static const int POWER_OFF_LEVEL = HIGH;
static bool powerWasOff = false;

/// ---------- One-move pulse ----------
static const uint16_t MOVE_PULSE_MS   = 70;
static const uint16_t NEUTRAL_HOLD_MS = 40; // Not strictly used in new ACK mode but kept for safety

enum Dir { DIR_NEUTRAL, DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };

static bool pulsingMove  = false;
static uint32_t pulseStartMs = 0;
static Dir lastDir = DIR_NEUTRAL;
static Dir pulseDir = DIR_NEUTRAL;

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

static inline uint8_t makeBtnMask(bool jswHeld, bool aHeld) {
  uint8_t m = 0;
  if (jswHeld) m |= (1 << 0);
  if (aHeld)   m |= (1 << 1);
  return m;
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

static bool swPressedEvent(uint32_t now) {
  bool reading = (digitalRead(PIN_SW) == LOW);
  if (reading != swLastReading) {
    swLastChange = now;
    swLastReading = reading;
  }
  if ((now - swLastChange) > SW_DEBOUNCE_MS) {
    if (reading != swStable) {
      swStable = reading;
      if (swStable) return true;
    }
  }
  return false;
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
  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(SW_POWER, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP); // R4 WiFi USER button check needed

  Serial.begin(115200);
  delay(200);

  Keyboard.begin();
  delay(250);
  Mouse.begin();
  delay(250);

  Serial1.begin(BAUD_PICO);

  powerWasOff = (digitalRead(SW_POWER) == POWER_OFF_LEVEL);

  Serial.println("UNO R4 started. Mode: MINIGAME_MODE");
  Serial.println("P1 Turn. Waiting for BTN_A to arm maze.");
}

void loop() {
  uint32_t now = millis();

  // 1. Power Switch Logic
  bool powerIsOff = (digitalRead(SW_POWER) == POWER_OFF_LEVEL);
  if (powerIsOff && !powerWasOff) {
    Keyboard.write('o');
    Serial.println("[POWER] OFF -> sent 'o'");
  }
  powerWasOff = powerIsOff;

  // 2. Reset Button Logic
  if (digitalRead(BTN_RESET) == LOW && (now - lastReset >= BTN_DEBOUNCE_MS)) {
    Serial1.write('R'); // Send to Pico
    Keyboard.write('r'); // Send to ProtoPie
    currentPlayer = PLAYER_1;
    state = MINIGAME_MODE;
    pulsingMove = false;
    lastReset = now;
    Serial.println("[RESET] System Reset -> P1 / MINIGAME");
  }

  // 3. Read Inputs
  int rawX_i = analogRead(PIN_VRX);
  int rawY_i = analogRead(PIN_VRY);
  int yCentered = (rawY_i - CENTER) * Y_POLARITY;
  int rawY_fixed = constrain(yCentered + CENTER, 0, 1023);
  uint16_t rawX = (uint16_t)constrain(rawX_i, 0, 1023);
  uint16_t rawY = (uint16_t)rawY_fixed;
  
  int cx = (int)rawX - CENTER;
  int cy = (int)rawY - CENTER;
  int x = applyDeadzone(cx);
  int y = applyDeadzone(cy);

  // bool swPress = swPressedEvent(now); // Unused in new plan
  bool swHeld  = (digitalRead(PIN_SW) == LOW);
  bool aHeld   = (digitalRead(BTN_A) == LOW);
  uint8_t mask = makeBtnMask(swHeld, aHeld); // Pass button state to Pico if needed

  // 4. Global Button Handlers (B, C, D, Enter)
  if (digitalRead(BTN_ENTER) == LOW && (now - lastEnter >= BTN_DEBOUNCE_MS)) {
    Keyboard.write(KEY_RETURN);
    lastEnter = now;
    Serial.println("[KEY] ENTER");
  }
  if (digitalRead(BTN_B) == LOW && (now - lastB >= BTN_DEBOUNCE_MS)) { tapABCD('B'); lastB = now; }
  if (digitalRead(BTN_C) == LOW && (now - lastC >= BTN_DEBOUNCE_MS)) { tapABCD('C'); lastC = now; }
  if (digitalRead(BTN_D) == LOW && (now - lastD >= BTN_DEBOUNCE_MS)) { tapABCD('D'); lastD = now; }

  // 5. BTN_A Handler (Arms Maze)
  if (digitalRead(BTN_A) == LOW && (now - lastA >= BTN_DEBOUNCE_MS)) {
    if (state == MINIGAME_MODE) {
      state = MAZE_ARMED;
      Serial.print("[ARMED] Player ");
      Serial.println(currentPlayer == PLAYER_1 ? "1" : "2");
    }
    lastA = now;
  }

  // 6. State Machine
  switch (state) {
    case MINIGAME_MODE:
      // Mouse Control Active
      if (now - lastMouseMs >= MOUSE_UPDATE_MS) {
        int dx = joyToSpeed(x);
        int dy = joyToSpeed(y);
        if (dx != 0 || dy != 0) Mouse.move(dx, dy, 0);
        lastMouseMs = now;
      }
      // Keep Pico Maze Frozen
      if (now - lastSendMs >= SEND_PERIOD_MS) {
        sendPacket(CENTER, CENTER, mask);
        lastSendMs = now;
      }
      break;

    case MAZE_ARMED:
      // Mouse Frozen. Waiting for Joystick Direction.
      // Send freeze packet to Pico
      if (now - lastSendMs >= SEND_PERIOD_MS) {
        sendPacket(CENTER, CENTER, mask);
        lastSendMs = now;
      }
      
      { 
        Dir dirNow = getDirFromXY(x, y);
        // Trigger on Neutral -> Direction
        if (lastDir == DIR_NEUTRAL && dirNow != DIR_NEUTRAL) {
          state = MAZE_PULSING;
          pulseDir = dirNow;
          pulseStartMs = now;
          pulsingMove = true;
          Serial.print("[MAZE] Move triggered: ");
          Serial.println((int)pulseDir);
        }
        lastDir = dirNow;
      }
      break;

    case MAZE_PULSING:
      // Send Direction Pulse for MOVE_PULSE_MS
      if (now - lastSendMs >= SEND_PERIOD_MS) {
        uint16_t px, py;
        dirToRaw(pulseDir, px, py);
        sendPacket(px, py, mask);
        lastSendMs = now;
      }
      // Timeout -> Go to WAITING_ACK
      if (now - pulseStartMs >= MOVE_PULSE_MS) {
        state = WAITING_ACK;
        // Send neutral once to stop drift
        sendPacket(CENTER, CENTER, mask);
        Serial.println("[MAZE] Pulse done, waiting ACK...");
      }
      break;

    case WAITING_ACK:
      // Send Neutral to Pico while waiting
      if (now - lastSendMs >= SEND_PERIOD_MS) {
        sendPacket(CENTER, CENTER, mask);
        lastSendMs = now;
      }

      // Check for Response
      while (Serial1.available()) {
        char c = Serial1.read();
        if (c == 'V') {
           // Valid Move
           currentPlayer = (currentPlayer == PLAYER_1) ? PLAYER_2 : PLAYER_1;
           Keyboard.write('v'); // Tell ProtoPie
           state = MINIGAME_MODE;
           Serial.print("[VALID] Swap to P");
           Serial.println(currentPlayer == PLAYER_1 ? "1" : "2");
        } 
        else if (c == 'I') {
           // Invalid Move
           state = MAZE_ARMED; // Let them try again
           Serial.println("[INVALID] Try again");
        } 
        else if (c == 'W') {
           // Win
           Keyboard.write('w'); // Tell ProtoPie
           state = MINIGAME_MODE;
           Serial.println("[WIN] Game Over");
        }
      }
      break;
  }
}