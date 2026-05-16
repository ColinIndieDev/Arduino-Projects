#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <EEPROM.h>

typedef bool b8;
typedef float f32;
typedef int i32;

//
// Drawing
//

#define SECONDS(x) ((x) * 1000)

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
  tft.setRotation(1);
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

b8 touched_display() {
  TSPoint p = ts.getPoint();
  
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(XP, OUTPUT); 

  if (p.z > MIN_PRESSURE && p.z < MAX_PRESSURE) {
    return true;
  }
  return false;
}

//
// Time
//

f32 old_time = 0;

f32 calculate_dt() {
  f32 cur = millis();
  f32 dt = cur - old_time;
  old_time = cur;
  return dt;
}

//
// Project
//

f32 start = 0;
u32 score = 0;
u32 last_score = 67;
i32 highscore = 0;
const i32 EEPROM_ADDR = 0;

void draw_score() {
  if (last_score == score) {
    return;
  }
  char str[6];
  snprintf(str, sizeof(str), "%05d", score);
  draw_text(VEC2I(screen_width - 110, 10), 3, str, WHITE);
  last_score = score;
}

void draw_highscore() {
  char str[6];
  snprintf(str, sizeof(str), "%05d", highscore);
  draw_text(VEC2I(10, 10), 3, str, WHITE);
}

void update_highscore() {
  if (score > highscore) {
    highscore = score;
    EEPROM.put(EEPROM_ADDR, highscore);
  }
}

const u16 ground_y = screen_height * 0.75f;

struct player {
  vec2i size = VEC2I(19, 20);
  vec2i pos = VEC2I(50, ground_y - size.y);

  u8 sprite[19 * 20] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  };

  f32 jmp_timer = 0;
  f32 air_time = SECONDS(0.6f);
  b8 air = false;
};

void draw_player(struct player *p, color c) {
  for (u32 i = 0; i < 19 * 20; i++) {
    if (p->sprite[i] == 1) {
      draw_pixel(VEC2I((u16)p->pos.x + i % p->size.x, (u16)p->pos.y + i / p->size.x), c);
    }
  }
}

void update_player(struct player *p) {
  draw_player(p, WHITE);
  if (p->jmp_timer + p->air_time <= millis() && p->air) {
    for (u32 i = 0; i < 3; i++) {
      draw_player(p, BLACK);
      p->pos.y += p->size.y * 0.4f;
      draw_player(p, WHITE);
    }
    p->air = false;
  }
  if (touched_display() && !p->air) {
    for (u32 i = 0; i < 3; i++) {
      draw_player(p, BLACK);
      p->pos.y -= p->size.y * 0.4f;
      draw_player(p, WHITE);
    }
    p->air = true;
    p->jmp_timer = millis();
  }
}

struct obstacle {
  vec2i size = VEC2I(16, 16);

  u8 sprite[16 * 16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0,
    1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
  };

  vec2f pos = VEC2F(screen_width, ground_y - size.y);
  f32 speed = 0.1f;
  b8 scored = false;
};

void draw_obstacle(struct obstacle *o, color c) {
  for (u32 i = 0; i < 16 * 16; i++) {
    if (o->sprite[i] == 1) {
      draw_pixel(VEC2I((u16)o->pos.x + i % o->size.x, (u16)o->pos.y + i / o->size.x), c);
    }
  }
}

void update_obstacle(struct obstacle *o, f32 dt) {
  draw_obstacle(o, BLACK);
  o->pos.x -= o->speed * dt;
  draw_obstacle(o, WHITE);
}

bool check_collision_player_obstacle(struct player *p, struct obstacle *o) {
  return (p->pos.x < o->pos.x + o->size.x &&
          p->pos.x + p->size.x > o->pos.x &&
          p->pos.y < o->pos.y + o->size.y &&
          p->pos.y + p->size.y > o->pos.y);
}

#define MAX_OBSTACLES 3
struct obstacles {
  obstacle data[MAX_OBSTACLES];
  u16 size = 0;
  f32 timer = 0;
  f32 dt = SECONDS(3);
};

void update_obstacles(struct player *p, struct obstacles *os, f32 dt) {
  for (u16 i = 0; i < os->size; i++) {
    obstacle *o = &os->data[i];
    update_obstacle(o, dt);

    if (o->pos.x + o->size.x < p->pos.x && !o->scored) {
      o->scored = true;
      score += 100;
    }

    if (check_collision_player_obstacle(p, o)) {
      draw_text(VEC2I(screen_width / 6.7, screen_height / 2.5), 4, "GAME OVER", WHITE);
      draw_score();
      update_highscore();
      delay(2000);
      
      score = 0;
      last_score = 67;
      start = millis();
      os->size = 0;
      draw_bg();
      draw_highscore();
      return;
    }
  }

  if (os->size > 0 && (os->data[0].pos.x + os->data[0].size.x < 10)) {
    draw_obstacle(&os->data[0], BLACK); 
    
    for (u16 i = 0; i < os->size - 1; i++) {
      os->data[i] = os->data[i + 1];
    }
    os->size--;
  }
  
  if (os->timer + os->dt <= millis()) {
    os->timer = millis();
    os->dt = SECONDS(random(1, 5));
    if (os->size < MAX_OBSTACLES) {                                     
      os->data[os->size++] = (obstacle){};                                          
    }                                                                                                                                                                     
  }
}

struct cloud {
  vec2f pos = VEC2F(screen_width, 0);
  f32 speed = 0.05f;
  vec2i size = VEC2I(24, 8);

  u8 sprite[24 * 8] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  };
};

void draw_cloud(struct cloud *cl, color c) {
  for (u32 i = 0; i < 24 * 8; i++) {
    if (cl->sprite[i] == 1) {
      draw_pixel(VEC2I((u16)cl->pos.x + i % cl->size.x, (u16)cl->pos.y + i / cl->size.x), c);
    }
  }
}

void update_cloud(struct cloud *c, f32 dt) {
  draw_cloud(c, BLACK);
  c->pos.x -= c->speed * dt;
  draw_cloud(c, WHITE);
}

#define MAX_CLOUDS 5
struct sky {
  cloud clouds[MAX_CLOUDS];
  u16 clouds_size = 0;
  f32 clouds_timer = 0;
  f32 clouds_dt = SECONDS(2);
};

void update_sky(struct sky *s, f32 dt) {
  for (u16 i = 0; i < s->clouds_size; i++) {
    cloud *c = &s->clouds[i];
    update_cloud(c, dt);
  }

  if (s->clouds_size > 0 && (s->clouds[0].pos.x + s->clouds[0].size.x < 10)) {
    draw_cloud(&s->clouds[0], BLACK); 
    
    for (u16 i = 0; i < s->clouds_size - 1; i++) {
      s->clouds[i] = s->clouds[i + 1];
    }
    s->clouds_size--;
  }
  
  if (s->clouds_timer + s->clouds_dt <= millis()) {
    s->clouds_timer = millis();
    s->clouds_dt = SECONDS(random(1, 5));
    if (s->clouds_size < MAX_CLOUDS) {                                     
      s->clouds[s->clouds_size] = (cloud){};    
      s->clouds[s->clouds_size].pos.y = random(50, 121);
      s->clouds_size++;                                   
    }                                                                                                                                                                     
  }
}

void draw_bg() {
  clear_background(BLACK);
  draw_line_horizontal(VEC2I(0, ground_y), screen_width * 0.5f, WHITE);
  draw_line_horizontal(VEC2I(screen_width * 0.5f, ground_y), screen_width * 0.5f, WHITE);
}

player p;
obstacles os;
sky s;

void setup() {
  start = millis();
  randomSeed(analogRead(0));
  EEPROM.get(EEPROM_ADDR, highscore);
  if (highscore < 0) {
    highscore = 0;
  }
  init_display();

  draw_bg();
  draw_highscore();
}

void loop() { 
  float dt = calculate_dt();

  update_sky(&s, dt);

  update_obstacles(&p, &os, dt);

  update_player(&p);

  score = (millis() - start) * 0.01f;
}