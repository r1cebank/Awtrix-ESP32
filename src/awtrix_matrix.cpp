#include <awtrix_matrix.h>

void initMatrix()
{
  switch (matrixType)
  {
  case 0:
    matrix = new FastLED_NeoMatrix(leds, 32, 8, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
    break;
  case 1:
    matrix = new FastLED_NeoMatrix(leds, 8, 8, 4, 1, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE);
    break;
  case 2:
    matrix = new FastLED_NeoMatrix(leds, 32, 8, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);
    break;
  default:
    matrix = new FastLED_NeoMatrix(leds, 32, 8, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
    break;
  }
  switch (matrixTempCorrection)
  {
  case 0:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setCorrection(TypicalLEDStrip);
    break;
  case 1:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(Candle);
    break;
  case 2:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(Tungsten40W);
    break;
  case 3:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(Tungsten100W);
    break;
  case 4:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(Halogen);
    break;
  case 5:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(CarbonArc);
    break;
  case 6:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(HighNoonSun);
    break;
  case 7:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(DirectSunlight);
    break;
  case 8:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(OvercastSky);
    break;
  case 9:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(ClearBlueSky);
    break;
  case 10:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(WarmFluorescent);
    break;
  case 11:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(StandardFluorescent);
    break;
  case 12:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(CoolWhiteFluorescent);
    break;
  case 13:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(FullSpectrumFluorescent);
    break;
  case 14:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(GrowLightFluorescent);
    break;
  case 15:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(BlackLightFluorescent);
    break;
  case 16:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(MercuryVapor);
    break;
  case 17:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(SodiumVapor);
    break;
  case 18:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(MetalHalide);
    break;
  case 19:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(HighPressureSodium);
    break;
  case 20:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setTemperature(UncorrectedTemperature);
    break;
  default:
    FastLED.addLeds<NEOPIXEL, D2>(leds, 256).setCorrection(TypicalLEDStrip);
    break;
  }

  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(30);
  matrix->setFont(&TomThumb);
  matrix->clear();
  matrix->setTextColor(matrix->Color(255, 0, 255));
  matrix->setCursor(9, 6);
  matrix->print("BOOT");
  matrix->show();
}

void hardwareAnimatedCheck(MSG_TYPE type, int x, int y)
{
	int wifiCheckTime = millis();
	int wifiCheckPoints = 0;
	while (millis() - wifiCheckTime < 2000)
	{
		while (wifiCheckPoints < 7)
		{
			matrix->clear();
			switch (type)
			{
			case WIFI:
				matrix->setCursor(7, 6);
				matrix->print("WiFi");
				break;
			case HOST:
				matrix->setCursor(5, 6);
				matrix->print("Host");
				break;
			// case MsgType_Temp:
			// 	matrix->setCursor(7, 6);
			// 	matrix->print("Temp");
			// 	break;
			// case MsgType_Audio:
			// 	matrix->setCursor(3, 6);
			// 	matrix->print("Audio");
			// 	break;
			// case MsgType_Gest:
			// 	matrix->setCursor(3, 6);
			// 	matrix->print("Gest.");
			// 	break;
			// case MsgType_LDR:
			// 	matrix->setCursor(7, 6);
			// 	matrix->print("LDR");
			// 	break;
			}

			switch (wifiCheckPoints)
			{
			case 6:
				matrix->drawPixel(x, y, CRGB(0x00FF00));
			case 5:
				matrix->drawPixel(x - 1, y + 1, CRGB(0x00FF00));
			case 4:
				matrix->drawPixel(x - 2, y + 2, CRGB(0x00FF00));
			case 3:
				matrix->drawPixel(x - 3, y + 3, CRGB(0x00FF00));
			case 2:
				matrix->drawPixel(x - 4, y + 4, CRGB(0x00FF00));
			case 1:
				matrix->drawPixel(x - 5, y + 3, CRGB(0x00FF00));
			case 0:
				matrix->drawPixel(x - 6, y + 2, CRGB(0x00FF00));
				break;
			}
			wifiCheckPoints++;
			matrix->show();
			delay(100);
		}
	}
}


void hardwareAnimatedSearch(MSG_TYPE searchType, int x, int y)
{
  for (int i = 0; i < 4; i++)
  {
    matrix->clear();
    matrix->setTextColor(0xFFFF);
    if (searchType == WIFI)
    {
      matrix->setCursor(7, 6);
      matrix->print("WiFi");
    }
    else if (searchType == HOST)
    {
      matrix->setCursor(5, 6);
      matrix->print("Host");
    }
    switch (i)
    {
    case 3:
      matrix->drawPixel(x, y, CRGB(0x22ff));
      matrix->drawPixel(x + 1, y + 1, CRGB(0x22ff));
      matrix->drawPixel(x + 2, y + 2, CRGB(0x22ff));
      matrix->drawPixel(x + 3, y + 3, CRGB(0x22ff));
      matrix->drawPixel(x + 2, y + 4, CRGB(0x22ff));
      matrix->drawPixel(x + 1, y + 5, CRGB(0x22ff));
      matrix->drawPixel(x, y + 6, CRGB(0x22ff));
    case 2:
      matrix->drawPixel(x - 1, y + 2, CRGB(0x22ff));
      matrix->drawPixel(x, y + 3, CRGB(0x22ff));
      matrix->drawPixel(x - 1, y + 4, CRGB(0x22ff));
    case 1:
      matrix->drawPixel(x - 3, y + 3, CRGB(0x22ff));
    case 0:
      break;
    }
    matrix->show();
    delay(100);
  }
}

