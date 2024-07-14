#include "m64_mp3_player.hpp"
#include <Arduino.h>

void m64_mp3_player::init(AudioPlaySdMp3 *m, AudioMixer4 *mxL, AudioMixer4 *mxR,
                          double v) {
  audio_play_sd_mp3 = m;
  mixerL = mxL;
  mixerR = mxR;
  volume = v;

  mixerL->gain(0, v); // MP3 L
  mixerL->gain(1, v); // MEM L

  mixerR->gain(0, v); // MP3 R
  mixerR->gain(1, v); // MEM R
}

void m64_mp3_player::tick() {}
