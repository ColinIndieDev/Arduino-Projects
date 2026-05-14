#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

//
// Drawing
//

#define SECONDS(x) ((x) * 1000)

typedef struct {
  u16 x, y;
} vec2;
#define VEC2(x, y) (vec2){x, y}
typedef struct {
  u8 r, g, b;
} rgb;
#define RGB(r, g, b) (rgb){r, g, b}
#define BLACK   RGB(0, 0, 0)
#define BLUE    RGB(0, 0, 255)
#define RED     RGB(255, 0, 0)
#define GREEN   RGB(0, 255, 0)
#define WHITE   RGB(255, 255, 255)
u16 rgb_to_rgb565(rgb color) {
  return ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
}
MCUFRIEND_kbv tft;
u16 screen_width = 320;
u16 screen_height = 240;
rgb bg_color;

void init_display() {
  u16 id = tft.readID();
  tft.begin(id);
  tft.setRotation(1);
}

void clear_background(rgb color) {
  bg_color = color;
  tft.fillScreen(rgb_to_rgb565(color));
}

void draw_rect(vec2 pos, vec2 size, rgb color) {
  tft.fillRect(pos.x, pos.y, size.x, size.y, rgb_to_rgb565(color));
}

void draw_circle(vec2 pos, u16 radius, rgb color) {
  tft.fillCircle(pos.x, pos.y, radius, rgb_to_rgb565(color));
}

void draw_text(vec2 pos, u16 scale, char *txt, rgb color) {
  tft.setCursor(pos.x, pos.y);
  tft.setTextColor(rgb_to_rgb565(color), rgb_to_rgb565(bg_color));
  tft.setTextSize(scale);
  tft.println(txt);
}

//
// Touchscreen
//

#define YP A3  
#define XM A2  
#define YM 9   
#define XP 8   

#define MIN_PRESSURE 10
#define MAX_PRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

bool touched_display() {
  TSPoint p = ts.getPoint();
    
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);

  if (p.z > MIN_PRESSURE && p.z < MAX_PRESSURE) {
    return true;
  }
  return false;
}

//
// Project
//

float jmp_timer = 0;
float air_time = SECONDS(1);
bool air = false;
vec2 cube_pos = VEC2(50, screen_height / 2);

void setup() {
  init_display();
  clear_background(BLACK);
}

void loop() {
  draw_rect(cube_pos, VEC2(50, 50), GREEN);
  if (jmp_timer + air_time <= millis() && air) {
    draw_rect(cube_pos, VEC2(50, 50), BLACK);
    cube_pos.y = screen_height / 2;
    air = false;
  }
  if (touched_display() && !air) {
    draw_rect(cube_pos, VEC2(50, 50), BLACK);
    cube_pos.y = screen_height / 2 - 100;
    air = true;
    jmp_timer = millis();
  }
}