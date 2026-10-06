#pragma once
#include <Arduino.h>

// -------------------- OLED --------------------
#define SCREEN_W 128
#define SCREEN_H 64
#define HUD_H 8
#define VIEW_H (SCREEN_H - HUD_H)

#define OLED_SDA D2   // GPIO4
#define OLED_SCL D1   // GPIO5
#define OLED_ADDR 0x3C

// -------------------- Physical buttons --------------------
// LEFT / RIGHT / FORWARD / BACK use INPUT_PULLUP and go to GND.
// FIRE uses GPIO16 INPUT_PULLDOWN_16 and goes to 3.3V.
#define PIN_LEFT     D5   // GPIO14 -> button -> GND
#define PIN_RIGHT    D6   // GPIO12 -> button -> GND
#define PIN_FORWARD  D7   // GPIO13 -> button -> GND
#define PIN_BACK     D3   // GPIO0  -> button -> GND (do NOT hold while booting/resetting)
#define PIN_FIRE     D0   // GPIO16 -> button -> 3.3V

// -------------------- Wi-Fi AP --------------------
static const char AP_SSID[] = "DOOM-ESP8266";
static const char AP_PASS[] = "doom1234";

// -------------------- WebSocket --------------------
#define HTTP_PORT 80
#define WS_PORT 81
#define WEB_INPUT_TIMEOUT_MS 350

// -------------------- Game --------------------
#define LOGIC_INTERVAL_MS 16
#define RENDER_INTERVAL_MS 33

#define MAP_W 12
#define MAP_H 12

#define FOV_RAD 1.0471975512f   // 60 degrees
#define MAX_RAY_DIST 10.0f
#define RAY_STEP 0.05f
#define RAY_COUNT 64

#define PLAYER_MOVE_SPEED 1.45f
#define PLAYER_ROT_SPEED 1.90f

enum InputBits : uint8_t {
  IN_LEFT    = 1 << 0,
  IN_RIGHT   = 1 << 1,
  IN_FORWARD = 1 << 2,
  IN_BACK    = 1 << 3,
  IN_FIRE    = 1 << 4
};
