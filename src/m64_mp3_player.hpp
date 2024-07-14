#ifndef M64_MP3_PLAYER_H
#define M64_MP3_PLAYER_H

#include <Audio.h>
#include <SD.h>
#include <play_sd_mp3.h>

#include "m64_media_file.hpp"

#define DISP_DETAILS_H 60

// typedef struct _MP3FrameInfo {
// 	int bitrate;
// 	int nChans;
// 	int samprate;
// 	int bitsPerSample;
// 	int outputSamps;
// 	int layer;
// 	int version;
// } MP3FrameInfo;

// --

#define PLAYER_STATE_IDLE 0
#define PLAYER_STATE_PLAYING 1
#define PLAYER_STATE_PAUSED 2
#define PLAYER_STATE_STOPPED 4

class m64_mp3_player {
public:
  void init(AudioPlaySdMp3 *m, AudioMixer4 *mxL, AudioMixer4 *mxR, double v);

  void set_file(m64_media_file *mfile);

  void play();
  void pause();
  void stop();

  void tick();

  void set_fft_data_256(unsigned int arr[]);

  m64_media_file *f = 0;

  AudioPlaySdMp3 *audio_play_sd_mp3 = 0;
  AudioMixer4 *mixerL = 0;
  AudioMixer4 *mixerR = 0;
  double volume = 0.0;

  unsigned int STATE = 0;
};

#endif
