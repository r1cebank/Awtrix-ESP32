#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "FS.h"
#include <LittleFS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <Fonts/TomThumb.h>
#include <LightDependentResistor.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include <WiFiManager.h>
#include <Wire.h>
#include <BME280_t.h>
#include "Adafruit_HTU21DF.h"
#include <Adafruit_BMP280.h>

#include <DFMiniMp3.h>

#include <rom/rtc.h>

const int AWTRIX_LOCAL_PORT = 80;
const int SLEEP_SEC = 5;

char awtrix_server_ip[16] = "0.0.0.0";
char awtrix_server_port[6] = "7001"; // AWTRIX Host Port, default = 7001
const char *CONFIG_FILE = "/awtrix.json";

// WiFi configuration
bool shouldSaveConfig = false;
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(AWTRIX_LOCAL_PORT);

// Server content
const char *UPLOAD_INDEX = "<html><head></head><body><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form></body></html>";
const char *RESET_INDEX = "<html><head></head><body><h2>Reset complete</h2></body></html>";

// Matrix Settings
CRGB leds[256];
FastLED_NeoMatrix *matrix;
int matrixType = 0;
int matrixTempCorrection = 0;
bool updating = false;

enum MSG_TYPE
{
  WIFI,
  HOST
};

void saveConfigCallback()
{
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *wifiManager)
{

  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(wifiManager->getConfigPortalSSID());
  matrix->clear();
  matrix->setCursor(3, 6);
  matrix->setTextColor(matrix->Color(0, 255, 50));
  matrix->print("Hotspot");
  matrix->show();
}

bool loadConfig()
{
  File configFile = LITTLEFS.open(CONFIG_FILE);
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
    return false;
  }
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }
  else
  {
    serializeJson(doc, Serial);
    Serial.println();
  }

  if (doc.containsKey("awtrix_server_ip"))
  {
    strcpy(awtrix_server_ip, doc["awtrix_server_ip"]);
  }

  if (doc.containsKey("matrixType"))
  {
    matrixType = doc["matrixType"].as<int>();
  }

  if (doc.containsKey("matrixCorrection"))
  {
    matrixTempCorrection = doc["matrixCorrection"].as<int>();
  }

  if (doc.containsKey("awtrix_server_port"))
  {
    strcpy(awtrix_server_port, doc["awtrix_server_port"]);
  }
  configFile.close();
  return true;
}

bool saveConfig()
{
  LITTLEFS.remove(CONFIG_FILE);
  File file = LITTLEFS.open(CONFIG_FILE, FILE_WRITE);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to write file"));
  }

  doc["matrixType"] = matrixType;
  doc["matrixCorrection"] = matrixTempCorrection;
  doc["awtrix_server_port"] = awtrix_server_port;
  doc["awtrix_server_ip"] = awtrix_server_ip;

  serializeJson(doc, Serial);
  Serial.println();
  if (serializeJson(doc, file) == 0)
  {
    Serial.println(F("Failed to write to file"));
    return false;
  }
  Serial.println("saved config");
  file.close();
  return true;
}

void initWiFi()
{
  WiFiManagerParameter custom_awtrix_server("server", "AWTRIX Host", awtrix_server_ip, 16);
  WiFiManagerParameter custom_port("port", "Matrix Port", awtrix_server_port, 6);
  WiFiManagerParameter custom_matrix_type("matrixType", "MatrixType", "0", 1);
  // Just a quick hint
  WiFiManagerParameter host_hint("<small>AWTRIX Host IP (without Port)<br></small><br><br>");
  WiFiManagerParameter port_hint("<small>Communication Port (default: 7001)<br></small><br><br>");
  WiFiManagerParameter matrix_hint("<small>0: Columns; 1: Tiles; 2: Rows <br></small><br><br>");
  WiFiManagerParameter p_lineBreak_notext("<p></p>");

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);

  wifiManager.addParameter(&p_lineBreak_notext);
  wifiManager.addParameter(&host_hint);
  wifiManager.addParameter(&custom_awtrix_server);
  wifiManager.addParameter(&port_hint);
  wifiManager.addParameter(&custom_port);
  wifiManager.addParameter(&matrix_hint);
  wifiManager.addParameter(&custom_matrix_type);
  wifiManager.addParameter(&p_lineBreak_notext);

  if (!wifiManager.autoConnect("AWTRIX Controller", "awtrixxx"))
  {
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
  }
  WiFi.mode(WIFI_STA);
  if (shouldSaveConfig)
  {

    strcpy(awtrix_server_ip, custom_awtrix_server.getValue());
    matrixType = atoi(custom_matrix_type.getValue());
    strcpy(awtrix_server_port, custom_port.getValue());
    if (!saveConfig())
    {
      Serial.println("Config write failed.");
    }
    ESP.restart();
  }
}

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

void initFS()
{
  if (LITTLEFS.begin(true))
  {
    if (!loadConfig())
    {
      Serial.println("Config read failed.");
    }
    Serial.println("Config read success.");
  }
  else
  {
    Serial.println("LittleFS mount failure");
  }
}

uint32_t Wheel(byte WheelPos, int pos)
{
	if (WheelPos < 85)
	{
		return matrix->Color((WheelPos * 3) - pos, (255 - WheelPos * 3) - pos, 0);
	}
	else if (WheelPos < 170)
	{
		WheelPos -= 85;
		return matrix->Color((255 - WheelPos * 3) - pos, 0, (WheelPos * 3) - pos);
	}
	else
	{
		WheelPos -= 170;
		return matrix->Color(0, (WheelPos * 3) - pos, (255 - WheelPos * 3) - pos);
	}
}

void flashProgress(unsigned int progress, unsigned int total)
{
	matrix->setBrightness(80);
	long num = 32 * 8 * progress / total;
	for (unsigned char y = 0; y < 8; y++)
	{
		for (unsigned char x = 0; x < 32; x++)
		{
			if (num-- > 0)
				matrix->drawPixel(x, 8 - y - 1, Wheel((num * 16) & 255, 0));
		}
	}
	matrix->setCursor(1, 6);
	matrix->setTextColor(matrix->Color(200, 200, 200));
	matrix->print("FLASHING");
	matrix->show();
}

void initOTA()
{
  ArduinoOTA.onStart([&]() {
		updating = true;
		matrix->clear();
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		flashProgress(progress, total);
	});

	ArduinoOTA.begin();
}

void initServer()
{
  server.on("/", HTTP_GET, []()
            {
              server.sendHeader("Connection", "close");
              server.send(200, "text/html", UPLOAD_INDEX);
            });

  server.on("/reset", HTTP_GET, []()
            {
              server.send(200, "text/html", RESET_INDEX);
              wifiManager.resetSettings();
              LITTLEFS.format();
              ESP.restart();
            });

  server.on(
      "/update", HTTP_POST, []()
      {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      },
      []()
      {
        HTTPUpload &upload = server.upload();

        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.setDebugOutput(true);

          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace))
          { //start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          matrix->clear();
          flashProgress((int)upload.currentSize, (int)upload.buf);
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { //true to set the size to the current progress
            server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
          }
          else
          {
            Update.printError(Serial);
          }
          Serial.setDebugOutput(false);
        }
        yield();
      });

  server.begin();
}

void printStatus()
{
  Serial.print("Current ip ");
  Serial.println(WiFi.localIP());
  Serial.print("Awtrix server ip ");
  Serial.println(awtrix_server_ip);
  Serial.print("Awtrix server port ");
  Serial.println(awtrix_server_port);
  for (int x = 32; x >= -90; x--)
	{
		matrix->clear();
		matrix->setCursor(x, 6);
		matrix->print("Host-IP: " + String(awtrix_server_ip) + ":" + String(awtrix_server_port));
		matrix->setTextColor(matrix->Color(0, 255, 50));
		matrix->show();
		delay(40);
	}
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


void pubsubCallback(char *topic, byte *payload, unsigned int length)
{
	// updateMatrix(payload, length);
}


void setup(void)
{
  Serial.begin(115200);
  Serial.print("Sleeping for ");
  Serial.println(SLEEP_SEC);
  sleep(SLEEP_SEC);
  initFS();
  initMatrix();
  hardwareAnimatedSearch(WIFI, 24, 0);
  initWiFi();
  hardwareAnimatedCheck(WIFI, 27, 2);
  initServer();
  initOTA();
  printStatus();
  client.setServer(awtrix_server_ip, atoi(awtrix_server_port));
	client.setCallback(pubsubCallback);
}

void serverSearch(int rounds, int x, int y)
{
	matrix->clear();
	matrix->setTextColor(0xFFFF);
	matrix->setCursor(5, 6);
	matrix->print("Host");

	switch (rounds)
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
}

void reconnectClient()
{
  String clientId = "AWTRIXController-";
	clientId += String(random(0xffff), HEX);
	hardwareAnimatedSearch(HOST, 28, 0);
	if (client.connect(clientId.c_str()))
	{
		//Serial.println("connected to server!");
		client.subscribe("awtrixmatrix/#");
		client.publish("matrixClient", "connected");
		matrix->fillScreen(matrix->Color(0, 0, 0));
		matrix->show();
	}
}

void loop()
{
  // Handle potential HTTP requests
  server.handleClient();
  ArduinoOTA.handle();

  if (WiFi.isConnected())
		{
			if (!client.connected())
			{
				reconnectClient();
			}
			else
			{
				client.loop();
			}
		}
}