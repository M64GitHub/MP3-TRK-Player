#ifndef MAIN_H
#define MAIN_H

void playFile(const char *fn);

void init_all();
void init_hw();
void init_tft();
void init_gfx();
void init_audio();
void init_sd();
void init_timing();
void init_controller();

void tick_controller(); // tick callback
void tick_sids();       // sid timer callback

void wait4UserInput();
#endif
