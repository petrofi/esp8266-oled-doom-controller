#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "config.h"
#include "web_ui.h"

// ============================================================
// ESP8266 + 1.3" SH1106 OLED "DOOM-like" raycasting demo
//
// Controls:
//   Phone: connect to DOOM-ESP8266 / doom1234 and open 192.168.4.1
//   Physical buttons: optional, can be used at the same time.
//
// This project does NOT contain DOOM WAD files or copyrighted DOOM assets.
// ============================================================

Adafruit_SH1106G display(SCREEN_W, SCREEN_H, &Wire, -1);
ESP8266WebServer httpServer(HTTP_PORT);
WebSocketsServer webSocket(WS_PORT);

volatile uint8_t webMask = 0;
volatile unsigned long lastWebInputMs = 0;

// -------------------- Map --------------------
// # = wall, . = empty
const char WORLD[MAP_H][MAP_W + 1] PROGMEM = {
  "############",
  "#..........#",
  "#..##......#",
  "#..#.......#",
  "#..#..###..#",
  "#..........#",
  "#.###......#",
  "#......##..#",
  "#..........#",
  "#..##......#",
  "#..........#",
  "############"
};

struct Enemy {
  float x;
  float y;
  int8_t hp;
  bool alive;
  unsigned long nextAttackMs;
};

Enemy enemies[4];

float playerX = 2.5f;
float playerY = 2.5f;
float playerA = 0.0f;
int playerHP = 100;
uint8_t kills = 0;
bool gameOver = false;
bool gameWon = false;

unsigned long lastLogicMs = 0;
unsigned long lastRenderMs = 0;
unsigned long lastFireMs = 0;
unsigned long muzzleUntilMs = 0;

float depthBuffer[RAY_COUNT];

// -------------------- Utility --------------------
static float wrapAngle(float a) {
  while (a > PI) a -= TWO_PI;
  while (a < -PI) a += TWO_PI;
  return a;
}

static bool isWallCell(int x, int y) {
  if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return true;
  return pgm_read_byte(&(WORLD[y][x])) == '#';
}

static bool isWall(float x, float y) {
  return isWallCell((int)x, (int)y);
}

static bool canStand(float x, float y) {
  const float r = 0.18f;
  return !isWall(x-r, y-r) &&
         !isWall(x+r, y-r) &&
         !isWall(x-r, y+r) &&
         !isWall(x+r, y+r);
}

static bool lineOfSight(float x0, float y0, float x1, float y1) {
  const float dx = x1 - x0;
  const float dy = y1 - y0;
  const float dist = sqrtf(dx*dx + dy*dy);
  if (dist < 0.001f) return true;

  const float sx = dx / dist * 0.05f;
  const float sy = dy / dist * 0.05f;
  float x = x0;
  float y = y0;

  const int steps = (int)(dist / 0.05f);
  for (int i = 0; i < steps - 2; ++i) {
    x += sx;
    y += sy;
    if (isWall(x, y)) return false;
  }
  return true;
}

// -------------------- Physical buttons --------------------
static uint8_t readPhysicalMask() {
  uint8_t m = 0;

  if (digitalRead(PIN_LEFT) == LOW)    m |= IN_LEFT;
  if (digitalRead(PIN_RIGHT) == LOW)   m |= IN_RIGHT;
  if (digitalRead(PIN_FORWARD) == LOW) m |= IN_FORWARD;
  if (digitalRead(PIN_BACK) == LOW)    m |= IN_BACK;

  // GPIO16 uses INPUT_PULLDOWN_16, so pressed = HIGH.
  if (digitalRead(PIN_FIRE) == HIGH)   m |= IN_FIRE;

  return m;
}

static uint8_t getInputMask() {
  uint8_t wm = 0;
  const unsigned long now = millis();

  noInterrupts();
  const unsigned long last = lastWebInputMs;
  const uint8_t current = webMask;
  interrupts();

  if (now - last <= WEB_INPUT_TIMEOUT_MS) {
    wm = current;
  }

  return readPhysicalMask() | wm;
}

// -------------------- WebSocket --------------------
static uint8_t parseMask(const uint8_t* payload, size_t length) {
  unsigned int value = 0;
  for (size_t i = 0; i < length; ++i) {
    const char c = (char)payload[i];
    if (c >= '0' && c <= '9') {
      value = value * 10 + (unsigned int)(c - '0');
      if (value > 255) break;
    }
  }
  return (uint8_t)(value & 0x1F);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  (void)num;

  if (type == WStype_TEXT) {
    const uint8_t m = parseMask(payload, length);

    noInterrupts();
    webMask = m;
    lastWebInputMs = millis();
    interrupts();
  }
  else if (type == WStype_DISCONNECTED) {
    noInterrupts();
    webMask = 0;
    lastWebInputMs = 0;
    interrupts();
  }
}

// -------------------- Game --------------------
void resetGame() {
  playerX = 2.5f;
  playerY = 2.5f;
  playerA = 0.0f;
  playerHP = 100;
  kills = 0;
  gameOver = false;
  gameWon = false;
  lastFireMs = 0;
  muzzleUntilMs = 0;

  enemies[0] = {8.5f, 2.5f, 2, true, 0};
  enemies[1] = {9.0f, 8.5f, 2, true, 0};
  enemies[2] = {4.5f, 9.0f, 2, true, 0};
  enemies[3] = {7.5f, 5.5f, 2, true, 0};
}

void fireWeapon() {
  const unsigned long now = millis();
  if (now - lastFireMs < 230) return;

  lastFireMs = now;
  muzzleUntilMs = now + 90;

  int best = -1;
  float bestDist = 999.0f;

  for (int i = 0; i < 4; ++i) {
    if (!enemies[i].alive) continue;

    const float dx = enemies[i].x - playerX;
    const float dy = enemies[i].y - playerY;
    const float dist = sqrtf(dx*dx + dy*dy);
    const float a = atan2f(dy, dx);
    const float diff = fabsf(wrapAngle(a - playerA));

    // Small aim cone around center of screen.
    if (diff < 0.13f && dist < bestDist && lineOfSight(playerX, playerY, enemies[i].x, enemies[i].y)) {
      best = i;
      bestDist = dist;
    }
  }

  if (best >= 0) {
    enemies[best].hp--;
    if (enemies[best].hp <= 0) {
      enemies[best].alive = false;
      kills++;
      if (kills >= 4) {
        gameWon = true;
      }
    }
  }
}

void updateEnemies(float dt) {
  const unsigned long now = millis();

  for (int i = 0; i < 4; ++i) {
    Enemy &e = enemies[i];
    if (!e.alive) continue;

    const float dx = playerX - e.x;
    const float dy = playerY - e.y;
    const float dist = sqrtf(dx*dx + dy*dy);

    if (dist > 6.5f) continue;
    if (!lineOfSight(e.x, e.y, playerX, playerY)) continue;

    if (dist > 0.85f) {
      const float speed = 0.34f;
      const float nx = e.x + (dx / dist) * speed * dt;
      const float ny = e.y + (dy / dist) * speed * dt;

      if (canStand(nx, e.y)) e.x = nx;
      if (canStand(e.x, ny)) e.y = ny;
    } else if (now >= e.nextAttackMs) {
      e.nextAttackMs = now + 850;
      playerHP -= 7;
      if (playerHP <= 0) {
        playerHP = 0;
        gameOver = true;
      }
    }
  }
}

void updateGame(float dt) {
  const uint8_t in = getInputMask();

  if (gameOver || gameWon) {
    // FIRE restarts the game after death / win.
    if (in & IN_FIRE) {
      if (millis() - lastFireMs > 400) {
        resetGame();
        lastFireMs = millis();
      }
    }
    return;
  }

  if (in & IN_LEFT)  playerA -= PLAYER_ROT_SPEED * dt;
  if (in & IN_RIGHT) playerA += PLAYER_ROT_SPEED * dt;
  playerA = wrapAngle(playerA);

  float move = 0.0f;
  if (in & IN_FORWARD) move += PLAYER_MOVE_SPEED * dt;
  if (in & IN_BACK)    move -= PLAYER_MOVE_SPEED * dt;

  if (fabsf(move) > 0.0001f) {
    const float nx = playerX + cosf(playerA) * move;
    const float ny = playerY + sinf(playerA) * move;

    if (canStand(nx, playerY)) playerX = nx;
    if (canStand(playerX, ny)) playerY = ny;
  }

  if (in & IN_FIRE) fireWeapon();

  updateEnemies(dt);
}

// -------------------- Rendering --------------------
void castWalls() {
  for (int r = 0; r < RAY_COUNT; ++r) {
    const float t = (float)r / (float)(RAY_COUNT - 1);
    const float rayA = playerA - FOV_RAD * 0.5f + t * FOV_RAD;

    const float dx = cosf(rayA) * RAY_STEP;
    const float dy = sinf(rayA) * RAY_STEP;

    float rx = playerX;
    float ry = playerY;
    float dist = 0.0f;

    while (dist < MAX_RAY_DIST) {
      rx += dx;
      ry += dy;
      dist += RAY_STEP;
      if (isWall(rx, ry)) break;
    }

    // Correct fish-eye distortion.
    float corrected = dist * cosf(rayA - playerA);
    if (corrected < 0.10f) corrected = 0.10f;
    depthBuffer[r] = corrected;

    int wallH = (int)(44.0f / corrected);
    if (wallH < 2) wallH = 2;
    if (wallH > VIEW_H) wallH = VIEW_H;

    const int y0 = (VIEW_H - wallH) / 2;
    const int x = r * 2;

    // Simple distance shading for monochrome OLED.
    if (corrected < 3.2f) {
      display.drawFastVLine(x, y0, wallH, SH110X_WHITE);
      display.drawFastVLine(x + 1, y0, wallH, SH110X_WHITE);
    } else if (corrected < 5.7f) {
      display.drawFastVLine(x, y0, wallH, SH110X_WHITE);
    } else {
      for (int y = y0; y < y0 + wallH; y += 2) {
        display.drawPixel(x, y, SH110X_WHITE);
      }
    }
  }
}

void drawEnemySprite(const Enemy &e) {
  if (!e.alive) return;

  const float dx = e.x - playerX;
  const float dy = e.y - playerY;
  const float dist = sqrtf(dx*dx + dy*dy);
  if (dist < 0.15f) return;

  const float enemyA = atan2f(dy, dx);
  const float rel = wrapAngle(enemyA - playerA);

  if (fabsf(rel) > FOV_RAD * 0.65f) return;

  const float norm = (rel + FOV_RAD * 0.5f) / FOV_RAD;
  const int sx = (int)(norm * SCREEN_W);

  int size = (int)(28.0f / dist);
  if (size < 3) size = 3;
  if (size > 34) size = 34;

  const int centerRay = constrain(sx / 2, 0, RAY_COUNT - 1);
  if (dist > depthBuffer[centerRay] + 0.20f) return;

  const int top = VIEW_H / 2 - size / 2;
  const int left = sx - size / 2;

  // Head
  display.drawRect(left + size/4, top, max(2, size/2), max(2, size/3), SH110X_WHITE);
  // Eyes
  if (size >= 8) {
    display.drawPixel(sx - size/6, top + size/7, SH110X_WHITE);
    display.drawPixel(sx + size/6, top + size/7, SH110X_WHITE);
  }
  // Body
  display.drawRect(left, top + size/3, size, max(2, size*2/3), SH110X_WHITE);
  // Cross mark
  display.drawLine(left, top + size/3, left + size - 1, top + size - 1, SH110X_WHITE);
  display.drawLine(left + size - 1, top + size/3, left, top + size - 1, SH110X_WHITE);
}

void drawEnemies() {
  // Far-to-near order is enough for this simple monochrome demo.
  int idx[4] = {0,1,2,3};

  for (int a = 0; a < 3; ++a) {
    for (int b = a + 1; b < 4; ++b) {
      const float dax = enemies[idx[a]].x - playerX;
      const float day = enemies[idx[a]].y - playerY;
      const float dbx = enemies[idx[b]].x - playerX;
      const float dby = enemies[idx[b]].y - playerY;
      const float da = dax*dax + day*day;
      const float db = dbx*dbx + dby*dby;
      if (da < db) {
        const int t = idx[a];
        idx[a] = idx[b];
        idx[b] = t;
      }
    }
  }

  for (int i = 0; i < 4; ++i) {
    drawEnemySprite(enemies[idx[i]]);
  }
}

void drawWeapon() {
  const int cx = SCREEN_W / 2;
  const int baseY = VIEW_H - 1;

  // Tiny pistol silhouette.
  display.fillRect(cx - 4, baseY - 8, 8, 8, SH110X_WHITE);
  display.fillRect(cx - 2, baseY - 13, 4, 6, SH110X_WHITE);

  if (millis() < muzzleUntilMs) {
    display.drawLine(cx, baseY - 14, cx - 5, baseY - 20, SH110X_WHITE);
    display.drawLine(cx, baseY - 14, cx + 5, baseY - 20, SH110X_WHITE);
    display.drawLine(cx, baseY - 14, cx,     baseY - 22, SH110X_WHITE);
  }
}

void drawHUD() {
  display.fillRect(0, VIEW_H, SCREEN_W, HUD_H, SH110X_BLACK);
  display.drawFastHLine(0, VIEW_H, SCREEN_W, SH110X_WHITE);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(1, VIEW_H + 1);
  display.print(F("HP:"));
  display.print(playerHP);

  display.setCursor(52, VIEW_H + 1);
  display.print(F("K:"));
  display.print(kills);
  display.print(F("/4"));

  display.setCursor(92, VIEW_H + 1);
  display.print(WiFi.softAPgetStationNum() > 0 ? F("WEB") : F("---"));
}

void drawEndScreen() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.setCursor(14, 13);
  display.print(gameWon ? F("YOU WIN") : F("GAME OVER"));

  display.setTextSize(1);
  display.setCursor(18, 40);
  display.print(F("FIRE = restart"));
  display.display();
}

void renderGame() {
  if (gameOver || gameWon) {
    drawEndScreen();
    return;
  }

  display.clearDisplay();

  // Horizon
  display.drawFastHLine(0, VIEW_H / 2, SCREEN_W, SH110X_WHITE);

  castWalls();
  drawEnemies();
  drawWeapon();
  drawHUD();

  display.display();
}

// -------------------- Setup --------------------
void showBootScreen() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print(F("ESP8266 OLED DOOM"));

  display.setCursor(0, 14);
  display.print(F("WiFi: "));
  display.print(AP_SSID);

  display.setCursor(0, 26);
  display.print(F("Pass: "));
  display.print(AP_PASS);

  display.setCursor(0, 38);
  display.print(F("Open: 192.168.4.1"));

  display.setCursor(0, 52);
  display.print(F("Physical buttons: OK"));

  display.display();
}

void setup() {
  // Physical buttons
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_FORWARD, INPUT_PULLUP);
  pinMode(PIN_BACK, INPUT_PULLUP);
  pinMode(PIN_FIRE, INPUT_PULLDOWN_16);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);

  if (!display.begin(OLED_ADDR, true)) {
    while (true) {
      yield();
    }
  }

  display.clearDisplay();
  display.display();

  // Wi-Fi AP
  WiFi.mode(WIFI_AP);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.softAP(AP_SSID, AP_PASS);

  // HTTP page
  httpServer.on("/", HTTP_GET, []() {
    httpServer.send_P(200, "text/html; charset=utf-8", WEB_PAGE);
  });

  httpServer.on("/health", HTTP_GET, []() {
    httpServer.send(200, "text/plain", "ok");
  });

  httpServer.onNotFound([]() {
    httpServer.sendHeader("Location", "/", true);
    httpServer.send(302, "text/plain", "");
  });

  httpServer.begin();

  // WebSocket controller
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  showBootScreen();
  delay(1800);

  resetGame();

  lastLogicMs = millis();
  lastRenderMs = millis();
}

// -------------------- Main loop --------------------
void loop() {
  // Handle controls as often as possible.
  webSocket.loop();
  httpServer.handleClient();

  const unsigned long now = millis();

  if (now - lastLogicMs >= LOGIC_INTERVAL_MS) {
    float dt = (now - lastLogicMs) / 1000.0f;
    if (dt > 0.050f) dt = 0.050f;
    lastLogicMs = now;
    updateGame(dt);
  }

  if (now - lastRenderMs >= RENDER_INTERVAL_MS) {
    lastRenderMs = now;
    renderGame();
  }

  yield();
}
