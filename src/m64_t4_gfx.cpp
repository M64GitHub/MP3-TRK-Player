#include "m64_t4_gfx.hpp"

// -- m64gfx
void m64gfx::init(ILI9341_t3n *t3n, uint16_t *framebuffer, int w, int h,
                  int r) {
  tft = t3n;
  width = w;
  height = h;
  fb = framebuffer;
  rotation = r;
}

// -- m64gfx_item

void m64gfx_item::init(m64gfx *g) { gfx = g; }

// -- m64gfx_fadeText

m64gfx_fadeText::m64gfx_fadeText(ILI9341_t3n *t3n, int n, uint16_t *c, char *s,
                                 int x, int y, int t, bool appendreverse) {
  tft = t3n;
  set_colors_RGB(n, c);
  set_text(s);
  set_xy(x, y);
  set_timing(t);
}

void m64gfx_fadeText::set_colors_RGB(int n, uint16_t *c) {
  int max = n;
  if (max > MAX_COLORS)
    max = MAX_COLORS;

  for (int i = 0; i < max; i++) {
    int R = c[i] & 0x00ff0000;
    R = R >> 16;
    int G = c[i] & 0x0000ff00;
    G = G >> 8;
    int B = c[i] & 0x000000ff;
    colors[i] = ConvertRGB(R, G, B);
  }
  num_colors = max;
}

void m64gfx_fadeText::set_timing(int t) { ticks_speed = t; }

void m64gfx_fadeText::set_xy(int x, int y) {
  xx = x;
  yy = y;
}

void m64gfx_fadeText::set_text(char *s) { strncpy(text, s, MAX_CHARS - 1); }

void m64gfx_fadeText::draw() {
  if (!tft)
    return;

  tft->setTextColor(colors[current_color_idx]);
  tft->drawString(text, strlen(text), xx, yy);
}

void m64gfx_fadeText::tick() {
  tick_ctr_speed++;
  if (tick_ctr_speed >= ticks_speed) {
    tick_ctr_speed = 0;
    next_color();
  }
}

void m64gfx_fadeText::next_color() {
  current_color_idx++;
  if (current_color_idx >= num_colors)
    current_color_idx = 0;
}

// --
//

unsigned int ConvertRGB(int R, int G, int B) {
  return (((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3));
}

void Color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (color >> 8) & 0x00F8;
  g = (color >> 3) & 0x00FC;
  b = (color << 3) & 0x00F8;
}

uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// void dimWriteScreen_part(uint16_t *pcolors,
//                          int dimamount,
//                          int xoffset, int yoffset,
//                          int height, int width) {
//   // memcpy(pcolors, &screen, sizeof(screen));
//   int p;
//   uint8_t r, g, b;
//
//   for(int y=0; y<height; y++)
//   for(int x=0; x<width; x++) {
// 	  p = pcolors[y * width + x];
//       Color565toRGB(p, r,g,b);
// 	  if(((y+yoffset) > 0)&&((y+yoffset) < ILI9341_TFTHEIGHT))
// 	   t3n->drawPixel(x,y+yoffset, Color565(r>>(dimamount+1),
// g>>(dimamount+1), b>>dimamount));
//   }
// }
