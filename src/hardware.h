#ifndef PINS_H
#define PINS_H

#define SPI_SPEED           40000000

// -- pins DISPLAY

#define PIN_TFT_SCK         13      // mandatory
#define PIN_TFT_MISO        12      // mandatory
#define PIN_TFT_MOSI        11      // mandatory
#define PIN_TFT_DC          10      // mandatory, can be any pin but using pin 10 
                                    // (or 36 or 37 on T4.1) provides greater 
                                    // performance

#define PIN_TFT_CS          9       // optional (but recommended), can be any pin.  
#define PIN_TFT_RST         6       // optional (but recommended), can be any pin. 
#define PIN_BACKLIGHT       29   

// -- pins AUDIO

#define PIN_I2S_BCLK        21
#define PIN_I2S_MCLK        23
#define PIN_I2S_TX          7
#define PIN_I2S_LRCLK       20

#endif

