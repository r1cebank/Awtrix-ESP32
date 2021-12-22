#ifndef __AWTRIX_MATRIX_H__
#define __AWTRIX_MATRIX_H__
#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <Fonts/TomThumb.h>

enum MSG_TYPE
{
  WIFI,
  HOST
};


// Matrix Settings
static CRGB leds[256];
static FastLED_NeoMatrix *matrix;
static int matrixType = 0;
static int matrixTempCorrection = 0;

void initMatrix();
void hardwareAnimatedCheck(MSG_TYPE type, int x, int y);
void hardwareAnimatedSearch(MSG_TYPE searchType, int x, int y);
#endif
