#ifndef M64_PLAYER_CONTROLLER_H
#define M64_PLAYER_CONTROLLER_H

#include "m64_media_file.hpp"
#include "m64_mp3_player.hpp"
#include "m64_t4_gfx.hpp"
#include <ILI9341_t3n.h>

#define FILES_MAX 250

#define FILELIST_LINE_HEIGHT 14
#define FILELIST_DISP_MAX 20
#define FILELIST_DISP_LEN_MAX 33
#define FILELIST_TOP 33

#define PIN_BTN_PREV 16       // ORNG
#define PIN_BTN_NEXT 18       // WHITE
#define PIN_BTN_VOL_UP 17     // BLU
#define PIN_BTN_VOL_DWN 5     // GN
#define PIN_BTN_PLAY_PAUSE 4  // BLK

#define BTN_PREV 1
#define BTN_NEXT 2
#define BTN_VOL_UP 4
#define BTN_VOL_DWN 8
#define BTN_PLAY_PAUSE 16

#define CONTROLLER_STATE_IDLE 0
#define CONTROLLER_STATE_PLAYING 1
#define CONTROLLER_STATE_PAUSED 2
#define CONTROLLER_STATE_STOPPED 4
#define CONTROLLER_STATE_FILESELECTION 8
#define CONTROLLER_STATE_LOADFILES 16

class m64_player_controller {
public:
  void init(m64gfx *g, m64_mp3_player *mp3pl, AudioPlaySdMp3 *m,
            AudioMixer4 *mxL, AudioMixer4 *mxR, double v);

  void load_files(char *path);
  void enter_directory(char *path, int set_to);

  unsigned int read_btns();
  void btn_pressed_next();
  void btn_pressed_prev();
  void btn_pressed_vol_up();
  void btn_pressed_vol_dwn();
  void btn_pressed_pp();

  void display_filelist(bool only_update = true);
  void display_track_time();
  void display_fft_256();

  void update_display();

  void play();
  void pause();
  void stop();

  void tick();

  m64_mp3_player *mp3_player;
  m64_media_filelist filelist;

  int current_filenum;
  int prev_filenum;
  int above_filenum;
  char current_path[ENTRY_PATH_MAX_LEN];

  unsigned int fft_data_256[256];
  unsigned int peak = 0;

  int FILELIST_START_PAGE = 0;

  unsigned int BTN_STATE = 0;
  unsigned int STATE = 0;

  unsigned int TICK_CNT_STATS = 0;
  unsigned int TICK_CNT_STATS_MAX = 10;

  unsigned int TICK_CNT_FFT = 0;
  unsigned int TICK_CNT_FFT_MAX = 10;

  unsigned int TICK_CNT_DISPUPDATE = 0;
  unsigned int TICK_CNT_DISPUPDATE_MAX = 50; // unused w/o fb

  m64gfx *gfx = 0;
};

#endif
