#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <ili9341_t3n_font_Arial.h>
#include <ili9488_t3_font_ArialBold.h>

#include "m64_player_controller.hpp"

void m64_player_controller::init(m64gfx *g, m64_mp3_player *mp3pl,
                                 AudioPlaySdMp3 *m, AudioMixer4 *mxL,
                                 AudioMixer4 *mxR, double v) {
  gfx = g;
  mp3_player = mp3pl;
  mp3_player->init(m, mxL, mxR, v);

  Serial.println(" * init file list ...");
  filelist.init(FILES_MAX);
  Serial.printf(" * initialized filelist for %d entries\n", FILES_MAX);

  STATE = CONTROLLER_STATE_IDLE;
}

void m64_player_controller::load_files(char *path) {
  STATE = CONTROLLER_STATE_LOADFILES;

  filelist.count = 0;
  unsigned int name_maxlen = ENTRY_NAME_MAX_LEN;

  strncpy(current_path, path, ENTRY_PATH_MAX_LEN);
  File dir = SD.open(current_path);

  // add .. for dirs
  // if(path[1]) // not for "/"
  // if(  media_files[current_filenum].type == FILE_TYPE_DIRECTORY) {
  //     media_files[0].name[0] = '.';
  //     media_files[0].name[1] = '.';
  //     media_files[0].name[2] = 0;
  //     media_files[0].type = FILE_TYPE_DIRECTORY;
  //     num_files++;
  // }

  while (filelist.count < filelist.max) {
    File entry = dir.openNextFile();

    if (!entry)
      break; // no more files

    if (entry.name()[0] == '.')
      continue;

    // skiplist
    if (!strncmp(entry.name(), "System", 6))
      continue;

    // -- append to list
    m64_media_file f;

    strncpy(f.name, entry.name(), ENTRY_NAME_MAX_LEN);

    if (entry.isDirectory()) {
      f.type = FILE_TYPE_DIRECTORY;
      f.size = 0;
    } else {
      f.type = FILE_TYPE_MP3;
      f.size = entry.size();
    }

    filelist.append(&f);

    entry.close();
  }

  STATE = CONTROLLER_STATE_IDLE;
}

void m64_player_controller::enter_directory(char *path, int set_to) {
  load_files(path);
  current_filenum = set_to;
  FILELIST_START_PAGE = current_filenum / 18;
  display_filelist(false);
}

void m64_player_controller::display_filelist(bool only_update) {
  int x = 0;
  int y = 0;
  int max = 0;

  Serial.println(" * m64_player_controller::display_filelist()");
  gfx->tft->setTextColor(Color565(0xe0, 0xe0, 0xe0));
  gfx->tft->setFont(Arial_8);

  max = filelist.count;
  if (max > FILELIST_DISP_MAX)
    max = FILELIST_DISP_MAX;

  for (int i = 0; i < 15; i++) {
    // gfx->tft->fillScreen(Color565(i * 0x10,
    //                               i * 0x10,
    //                               i * 0x18));
    // gfx->tft->fillScreen(ILI9341_BLACK);
    for (int j = 0; j < max; j++) {
      y = j * FILELIST_LINE_HEIGHT;
      if (current_filenum == j) {
        gfx->tft->setFont(Arial_8_Bold);
        gfx->tft->setTextColor(Color565(i * 0x0f, i * 0x0f, i * 0x0f),

                               Color565(i * 0x04, i * 0x04, i * 0x04));
      } else {
        gfx->tft->setFont(Arial_8);
        gfx->tft->setTextColor(Color565(i * 0x08, i * 0x08, i * 0x08));
      }
      gfx->tft->drawString(filelist.files[j].name, FILELIST_DISP_LEN_MAX, x,
                           y + FILELIST_TOP);
    }

    if (!gfx->tft->asyncUpdateActive())
      gfx->tft->updateScreen();
    delay(5);
  }

  Serial.println(" * END ! m64_player_controller::display_filelist()");
}

unsigned int m64_player_controller::read_btns() {
  BTN_STATE = 0;

  if (!digitalReadFast(PIN_BTN_PREV))
    BTN_STATE |= BTN_PREV;
  if (!digitalReadFast(PIN_BTN_NEXT))
    BTN_STATE |= BTN_NEXT;
  if (!digitalReadFast(PIN_BTN_VOL_UP))
    BTN_STATE |= BTN_VOL_UP;
  if (!digitalReadFast(PIN_BTN_VOL_DWN))
    BTN_STATE |= BTN_VOL_DWN;

  if (!digitalReadFast(PIN_BTN_PLAY_PAUSE))
    BTN_STATE |= BTN_PLAY_PAUSE;

  return BTN_STATE;
}

void m64_player_controller::btn_pressed_next() {
  prev_filenum = current_filenum;

  if (current_filenum < (filelist.count - 1)) {
    current_filenum++;
    FILELIST_START_PAGE = current_filenum / 18;

    if (current_filenum % 18)
      display_filelist(true);
    else
      display_filelist(false);
  } else {
    prev_filenum = current_filenum;
    current_filenum = 0;
    FILELIST_START_PAGE = 0;
    display_filelist(false);
  }

  delay(200);
  read_btns();
  while (BTN_STATE & BTN_NEXT) {
    read_btns();
    if (current_filenum < (filelist.count - 1)) {
      prev_filenum = current_filenum;
      current_filenum++;
      FILELIST_START_PAGE = current_filenum / 18;
      if (current_filenum % 18)
        display_filelist(true);
      else
        display_filelist(false);
      delay(10);
    } else {
      prev_filenum = current_filenum;
      current_filenum = 0;
      FILELIST_START_PAGE = 0;
      display_filelist(false);
      delay(10);
    }
  }
  BTN_STATE = 0;

  if (STATE == CONTROLLER_STATE_PLAYING) {
    mp3_player->stop();
    STATE = CONTROLLER_STATE_STOPPED;
    btn_pressed_pp();
  }
}

void m64_player_controller::btn_pressed_prev() {
  prev_filenum = current_filenum;

  if (current_filenum > 0)
    current_filenum--;
  else {
    current_filenum = filelist.count - 1;
    FILELIST_START_PAGE = current_filenum / 18;
    display_filelist(false);
  }

  FILELIST_START_PAGE = current_filenum / 18;

  if (current_filenum % 18 == 17)
    display_filelist(false);
  else
    display_filelist(true);

  delay(200);
  read_btns();
  while (BTN_STATE & BTN_PREV) {
    read_btns();
    if (current_filenum > 0) {
      prev_filenum = current_filenum;
      current_filenum--;
      FILELIST_START_PAGE = current_filenum / 18;
      if (current_filenum % 18 == 17)
        display_filelist(false);
      else
        display_filelist(true);
      delay(10);
    } else {
      prev_filenum = current_filenum;
      current_filenum = filelist.count - 1;
      FILELIST_START_PAGE = current_filenum / 18;
      display_filelist(false);
      delay(10);
    }
  }
  BTN_STATE = 0;

  if (STATE == CONTROLLER_STATE_PLAYING) {
    mp3_player->stop();
    STATE = CONTROLLER_STATE_STOPPED;
    btn_pressed_pp();
  }
}

void m64_player_controller::btn_pressed_vol_dwn() {
  mp3_player->volume = mp3_player->volume / 1.1;

  mp3_player->mixerL->gain(0, mp3_player->volume);
  mp3_player->mixerL->gain(1, mp3_player->volume);
  mp3_player->mixerR->gain(0, mp3_player->volume);
  mp3_player->mixerR->gain(1, mp3_player->volume);

  delay(100);

  BTN_STATE = 0;
}

void m64_player_controller::btn_pressed_vol_up() {
  double volume1 = mp3_player->volume * 1.1;

  if ((volume1 * 1000) > 20)
    return;

  mp3_player->volume = volume1;

  mp3_player->mixerL->gain(0, mp3_player->volume);
  mp3_player->mixerL->gain(1, mp3_player->volume);
  mp3_player->mixerR->gain(0, mp3_player->volume);
  mp3_player->mixerR->gain(1, mp3_player->volume);

  delay(100);
  BTN_STATE = 0;
}

void m64_player_controller::btn_pressed_pp() {
  if (!mp3_player)
    return;

  int ypos = 46;

  if ((STATE == CONTROLLER_STATE_IDLE) || (STATE == CONTROLLER_STATE_STOPPED)) {

    if (filelist.files[current_filenum].type == FILE_TYPE_DIRECTORY) {
      char tmp_name[ENTRY_PATH_MAX_LEN];

      // go UP
      if (filelist.files[current_filenum].name[0] == '.') {
        strcpy(tmp_name, current_path);
        int i = strlen(tmp_name) - 2;
        while (i >= 0) {
          if (tmp_name[i] == '/') {
            tmp_name[i + 1] = 0x0;
            enter_directory(tmp_name, above_filenum);
            return;
          }
          i--;
        }
      }

      strcpy(tmp_name, current_path);
      strcat(tmp_name, filelist.files[current_filenum].name);
      strcat(tmp_name, "/");
      above_filenum = current_filenum;
      enter_directory(tmp_name, 0);

      return;
    }

    play();
    STATE = CONTROLLER_STATE_PLAYING;
    delay(1);
    delay(200 - 1);
    return;
  }

  if (STATE == CONTROLLER_STATE_PLAYING) {
    mp3_player->pause();
    STATE = CONTROLLER_STATE_PAUSED;

    delay(500);
    read_btns();

    if (BTN_STATE & BTN_PLAY_PAUSE) {
      STATE = CONTROLLER_STATE_STOPPED;
      mp3_player->stop();
    }
    return;
  }

  if (STATE == CONTROLLER_STATE_PAUSED) {
    mp3_player->pause();
    STATE = CONTROLLER_STATE_PLAYING;
    delay(200);
    read_btns();
    if (BTN_STATE & BTN_PLAY_PAUSE) {
      STATE = CONTROLLER_STATE_STOPPED;
      mp3_player->stop();
    }
    return;
  }
}
int t_lastsec = -1;
void m64_player_controller::display_track_time() {
  char buf[16];

  TICK_CNT_STATS++;
  if (TICK_CNT_STATS == TICK_CNT_STATS_MAX) {
    TICK_CNT_STATS = 0;
  } else
    return;

  // Serial.println(" -- display_track_time --");

  unsigned int pos_millis = mp3_player->audio_play_sd_mp3->positionMillis();
  int t_min = (pos_millis / 2) / (1000 * 60 * 1000);
  int t_sec = ((pos_millis / 2) / (1000 * 1000)) % 60;
  int t_millis = ((pos_millis / 2) / 1000) % 1000;

  gfx->tft->setTextColor(Color565(0x80, 0x80, 0x80), ILI9341_BLACK);
  gfx->tft->setFont(Arial_8);

  if (t_min) {
    sprintf(buf, "%d:  ", t_min);
    if (t_min < 10)
      gfx->tft->drawString(buf, 5, 180 + 17 + 8, 0);
    else
      gfx->tft->drawString(buf, 5, 180 + 17, 0);
  }

  gfx->tft->setTextColor(Color565(0x80, 0x80, 0x80), ILI9341_BLACK);
  if (t_sec != t_lastsec
      // && (!(t_sec % 5))
  ) {
    gfx->tft->setTextColor(Color565(0xc0, 0xc0, 0xc0), ILI9341_BLACK);
  }
  sprintf(buf, "%02d:  ", t_sec);
  gfx->tft->drawString(buf, 5, 180 + 17 + 1 * 16, 0);

  gfx->tft->setTextColor(Color565(0x50, 0x50, 0x50), ILI9341_BLACK);
  if (t_sec != t_lastsec && (!(t_sec % 5))) {
    gfx->tft->setTextColor(Color565(0x70, 0x70, 0x70), ILI9341_BLACK);
  }
  sprintf(buf, "%03d  ", t_millis);
  gfx->tft->drawString(buf, 2, 180 + 17 + 2 * 16, 0);

  if (t_sec != t_lastsec
      // && (!(t_sec % 5))
  ) {
    t_lastsec = t_sec;
  }
}

void m64_player_controller::display_fft_256() {
  TICK_CNT_FFT++;
  if (TICK_CNT_FFT == TICK_CNT_FFT_MAX) {
    TICK_CNT_FFT = 0;
  } else
    return;

  // Serial.println(" -- display_fft_256 --");

  int y_pos = 8 + 3;

  for (int i = 0; i < 120; i += 2) { // 128 !!
    int v = fft_data_256[i] > 20 ? 20 : fft_data_256[i];
    gfx->tft->drawFastVLine(i * 2, y_pos, v, ILI9341_PINK);
    // gfx->tft->drawFastVLine(
    //     i*2+1, y_pos,
    //     v,
    //     ILI9341_PINK);

    gfx->tft->drawFastVLine(i * 2, y_pos + v, 20 - v, ILI9341_BLACK);
    // gfx->tft->drawFastVLine(
    //     i*2+1, y_pos + v,
    //     20 - v,
    //     ILI9341_BLACK);
  }

  // tft->drawFastHLine((320-256) / 2, y_pos + 23, peak, COLOR_GRAY);
  // tft->drawFastHLine((320-256) / 2 + peak, y_pos + 23, 256 - peak - 1,
  // COLOR_BLACK);
}

void m64_player_controller::play() {
  char tmp_name[ENTRY_NAME_MAX_LEN + ENTRY_PATH_MAX_LEN + 2];

  strcpy(tmp_name, current_path);
  strncat(tmp_name, filelist.files[current_filenum].name, 127);
  mp3_player->audio_play_sd_mp3->play(tmp_name);
  STATE = CONTROLLER_STATE_PLAYING;
}

void m64_player_controller::pause() {}

void m64_player_controller::stop() {}

void m64_player_controller::update_display() {
  TICK_CNT_DISPUPDATE++;
  if (TICK_CNT_DISPUPDATE == TICK_CNT_DISPUPDATE_MAX) {
    TICK_CNT_DISPUPDATE = 0;
  } else
    return;

  // if(!gfx->tft->asyncUpdateActive()) {
  //     Serial.println(" updating!");
  //     gfx->tft->updateScreenAsync();
  // }

  // if(!gfx->tft->asyncUpdateActive()) {
  //     Serial.println(" updating!");
  //     gfx->tft->updateScreenAsync();
  // }
}

void m64_player_controller::tick() {
  // display_fft_256();
  if (STATE == CONTROLLER_STATE_PLAYING) {
    display_track_time();
    display_fft_256();
  }

  // update_display();
}
