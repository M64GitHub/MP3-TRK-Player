#ifndef M64_GFX_H
#define M64_GFX_H

#include <ILI9341_t3n.h>

#define LX 240
#define LY 320

#define CENTER ILI9341_t3n::CENTER

class m64gfx {
public:
  void init(ILI9341_t3n *t3n, uint16_t *framebuffer, int w, int h, int r);

  ILI9341_t3n *tft;

  int width = 0;
  int height = 0;
  int rotation = 0;

  uint16_t *fb;
};

class m64gfx_item {
public:
  void init(m64gfx *g);

  virtual void draw();
  virtual void tick();

  m64gfx *gfx;
};

// --
// void put_sprite(

// -- faders etc classes

#define MAX_COLORS 16
#define MAX_CHARS 16

class m64gfx_fadeText : m64gfx_item {
public:
  m64gfx_fadeText(ILI9341_t3n *t3n, int n, uint16_t *c, char *s, int x, int y,
                  int t, bool appendreverse = true);

  void set_colors_RGB(int n, uint16_t *c);
  void set_timing(int t);
  void set_xy(int x, int y);
  void set_text(char *s);

  void draw();

  void tick();

  void next_color();

  uint16_t colors[MAX_COLORS];
  int num_colors = 0;
  char text[MAX_CHARS];
  int ticks_speed = 0;    // max
  int tick_ctr_speed = 0; // current
  int xx = 0;
  int yy = 0;
  ILI9341_t3n *tft = 0;

  int current_color_idx = 0;
};

unsigned int ConvertRGB(int R, int G, int B);
void Color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b);
uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);

void dimWriteScreen_part(uint16_t *pcolors, int dimamount, int xoffset,
                         int yoffset, int height, int width);

#endif
