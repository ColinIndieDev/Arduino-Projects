#pragma once

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#define VEC2I(x, y) (vec2i){x, y}
typedef struct {
  u16 x, y;
} vec2i;

#define VEC2F(x, y) (vec2f){x, y}
typedef struct {
  f32 x, y;
} vec2f;

#define VEC2F_TO_VEC2I(v) (vec2i){(v).x, (v).y}

#define RGB(r, g, b) (color){r, g, b}
typedef struct {
  u8 r, g, b;
} color;

#define BLACK   RGB(0, 0, 0)
#define BLUE    RGB(0, 0, 255)
#define RED     RGB(255, 0, 0)
#define GREEN   RGB(0, 255, 0)
#define WHITE   RGB(255, 255, 255)
#define YELLOW  RGB(255, 255, 0)
#define CYAN    RGB(0, 255, 255)

u16 color_to_rgb565(color c) {
  return ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);
}

MCUFRIEND_kbv tft;
const u16 screen_width = 320;
const u16 screen_height = 240;
color bg_color;

void init_display() {
  u16 id = tft.readID();
  tft.begin(id);
  tft.setRotation(3);
}

void clear_background(color c) {
  bg_color = c;
  tft.fillScreen(color_to_rgb565(c));
}

void draw_pixel(vec2i pos, color c) {
  tft.drawPixel(pos.x, pos.y, color_to_rgb565(c));
}

void draw_rect(vec2i pos, vec2i size, color c) {
  tft.fillRect(pos.x, pos.y, size.x, size.y, color_to_rgb565(c));
}

void draw_circle(vec2i pos, u16 radius, color c) {
  tft.fillCircle(pos.x, pos.y, radius, color_to_rgb565(c));
}

void draw_text(vec2i pos, u16 scale, char *txt, color c) {
  tft.setCursor(pos.x, pos.y);
  tft.setTextColor(color_to_rgb565(c), color_to_rgb565(bg_color));
  tft.setTextSize(scale);
  tft.println(txt);
}

void draw_line_horizontal(vec2i start, u8 len, color c) {
  tft.drawFastHLine(start.x, start.y, len, color_to_rgb565(c));
}