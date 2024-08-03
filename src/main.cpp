// #include <Arduino.h>

#include <Audio.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>

#include <TeensyTimerTool.h>

#include "hardware.h"
#include "logo.h"
#include "m64_player_controller.hpp"
#include "m64_t4_gfx.hpp"
#include "main.hpp"
#include "startup_sample.h"

// -- SD
#define ROOT_DIRECTORY "/\0"

// -- TFT / gfx
ILI9341_t3n tft = ILI9341_t3n(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST, PIN_TFT_MOSI,
                              PIN_TFT_SCK, PIN_TFT_MISO);

DMAMEM uint16_t fb[320 * 240];

m64gfx gfx;

// -- AUDIO
m64_mp3_player mp3_player;

AudioOutputI2S i2s1;

AudioPlaySdMp3 playMp31;
AudioPlayMemory playMem1;

AudioAnalyzePeak analyzePeak1;
AudioAnalyzeFFT256 analyzeFFT2561;

AudioMixer4 mixerL;
AudioMixer4 mixerR;

AudioConnection patchCord1(playMp31, 0, mixerL, 0);
AudioConnection patchCord2(playMp31, 1, mixerR, 0);

AudioConnection patchCord3(playMem1, 0, mixerL, 1);
AudioConnection patchCord4(playMem1, 0, mixerR, 1);

AudioConnection patchCord5(playMp31, 0, analyzePeak1, 0);
AudioConnection patchCord6(playMp31, 0, analyzeFFT2561, 0);

AudioConnection patchCord7(mixerL, 0, i2s1, 0);
AudioConnection patchCord8(mixerR, 0, i2s1, 1);

// -- timing
using namespace TeensyTimerTool;
PeriodicTimer t_sid;  // SID update timer : 50.125Hz
PeriodicTimer t_tick; // tick timer

int tick_lock = 0;

// -- controller
m64_player_controller player_controller;

// -- init

void init_hw() {
  Serial.println(" -- INIT HW: pins --");
  pinMode(PIN_BTN_PREV, INPUT_PULLUP);
  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_VOL_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_VOL_DWN, INPUT_PULLUP);
  pinMode(PIN_BTN_PLAY_PAUSE, INPUT_PULLUP);
  Serial.println(" * BTN pins PULLUP activated");
  Serial.println();
}

void init_tft() {
  Serial.println(" -- INIT DISPLAY HW --");
  tft.begin(SPI_SPEED, 0);
  Serial.printf(" * tft initialized with speed %d\n", SPI_SPEED);
  // tft.useFrameBuffer(true);
  // tft.setFrameBuffer(fb);
  // Serial.println(" * tft framebuffers active");

  tft.fillScreen(ILI9341_BLACK);
  Serial.println(" * tft READY.");

  tft.setClipRect();
  tft.setRotation(2);

  pinMode(PIN_BACKLIGHT, OUTPUT);
  analogWrite(PIN_BACKLIGHT, 1023 / 1);

  Serial.println();
}

void init_gfx() {
  Serial.println(" -- INIT GFX --");

  gfx.init(&tft, fb, 240, 320, 2);
}

void init_audio() {
  Serial.println(" -- INIT AUDIO --");

  player_controller.init(&gfx, &mp3_player, &playMp31, &mixerL, &mixerR, 0.2);
  Serial.println(" * CONTROLLER initialized");
  Serial.println(" * audio READY.");

  Serial.print(" * initializing fft2 ...");
  analyzeFFT2561.windowFunction(AudioWindowHanning256);
  Serial.println("DONE");

  Serial.print(" * playing startup sound ... ");
  playMem1.play(AudioSampleStartupmonow);
  Serial.println("DONE");
  Serial.println();
}

void init_sd() {
  Serial.println(" -- INIT SD CARD --");

  while (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println(" - attemtting to access SD card");
    delay(200);
  }
  delay(200);
  Serial.println(" * SD card READY.");
  Serial.println();
}

void init_controller() {
  Serial.println(" -- START CONTROLLER --");

  Serial.println(" * loading file list ...");
  player_controller.load_files((char *)"/");
  Serial.printf(" * loaded %d files in '/'\n",
                player_controller.filelist.count);
  Serial.println();
}

void init_timing() {
  Serial.println(" -- INIT TIMING --");

  // t_tick.begin(tick_controller, 10ms);
  // Serial.println(" * main tick initialized 10ms");
  // t_sid.begin(tick_sids, 19950us); // 19 950 us =  50.125 Hz
  // Serial.println(" * sid tick initialized 19950us");
  Serial.println();
}

void init_all() {
  Serial.println(" -- INIT --");
  init_hw();
  init_tft();
  init_gfx();
  init_sd();
  init_audio();
  init_controller();
  init_timing();
  Serial.println();
}

// -- startup / run

void setup() {
  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);
  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);
  AudioMemory(8);
  Serial.begin(115200);
  unsigned long int m = millis();
  while (!Serial && ((millis() - m) < 2000))
    ;
  SPI.begin();

  Serial.println(" -- M64 MP3/TRK SID PLAYER 2.0 --");
  init_all();

  // dimWriteScreen_part(&tft,(uint16_t *) logo2_320x240, 0,
  //                     0, 20,
  //                     150,
  //                     240);

  player_controller.display_filelist();
  // player_controller.play();
}

void loop(void) {
  // if (Serial.available()) {
  //   wait4UserInput();
  //   player_controller.play();
  //   while (tick_lock)
  //     ;
  //   tick_lock = 1;
  //   player_controller.display_filelist();
  //   tick_lock = 0;
  //   delay(250);
  // }

  tick_controller();

  delay(10);
}

void tick_controller() {
  if (tick_lock)
    return;

  tick_lock = 1;

  if (player_controller.STATE == CONTROLLER_STATE_PLAYING) {
    if (analyzeFFT2561.available()) {
      for (int i = 0; i < 128; i++) {
        float f = analyzeFFT2561.read(i);
        int v = f * 256 * 4;
        player_controller.fft_data_256[i] = v;
      }
    }
  }

  if (player_controller.STATE == CONTROLLER_STATE_PLAYING) {
    if (analyzePeak1.available()) {
      float f = analyzePeak1.read();
      int v = f * 200;
      player_controller.peak = v;
    }
  }

  player_controller.tick();

  tick_lock = 0;
}

void tick_sids() {}

// -- MP3

void playFile(const char *filename) {
  // Serial.print("Playing file: ");
  // Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playMp31.play(filename);

#if 0	
	 Serial.print("Max Usage: ");
	 Serial.print(playMp31.processorUsageMax());
	 Serial.print("% Audio, ");
	 Serial.print(playMp31.processorUsageMaxDecoder());	 	 
	 Serial.print("% Decoding max, ");
	 
	 Serial.print(playMp31.processorUsageMaxSD());	 
	 Serial.print("% SD max, ");
	 	 
	 Serial.print(AudioProcessorUsageMax());	 
	 Serial.println("% All");
	 
	 AudioProcessorUsageMaxReset();
	 playMp31.processorUsageMaxReset();
	 playMp31.processorUsageMaxResetDecoder();
#endif
}

// -- helpers n stuff

void wait4UserInput() {
  Serial.clear();
  Serial.println("Press any key to continue");
  while (Serial.read() == -1)
    ;
  while (Serial.read() != -1)
    ;
  Serial.clear();
  Serial.println("Continued");
}
