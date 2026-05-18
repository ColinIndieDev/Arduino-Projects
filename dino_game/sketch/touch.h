#pragma once

#include <TouchScreen.h>

#define YP A3  
#define XM A2  
#define YM 9   
#define XP 8   

#define MIN_PRESSURE 10
#define MAX_PRESSURE 1000

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
b8 was_pressed = false;

b8 display_pressed(vec2i *v) {
  TSPoint p = ts.getPoint();
  
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YM, OUTPUT);
  pinMode(XP, OUTPUT); 

  b8 cur_pressed = (p.z > MIN_PRESSURE && p.z < MAX_PRESSURE);
  b8 register_click = false;

  if (cur_pressed && !was_pressed) {
    u32 pixel_x = map(p.y, TS_MINY, TS_MAXY, screen_width, 0);
    
    u32 pixel_y = map(p.x, TS_MINX, TS_MAXX, screen_height, 0);
    
    *v = VEC2I(pixel_x, pixel_y);
    register_click = true;
  }

  was_pressed = cur_pressed;

  return register_click;
}