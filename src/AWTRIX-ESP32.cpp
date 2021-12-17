#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
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

const int AWTRIX_LOCAL_PORT = 80;
const int SLEEP_SEC = 5;

char awtrix_server_ip[16] = "0.0.0.0";
char awtrix_server_port[6] = "7001"; // AWTRIX Host Port, default = 7001

bool shouldSaveConfig = false;
WiFiManager wifiManager;
WebServer server(AWTRIX_LOCAL_PORT);

const char *UPLOAD_INDEX = "<html><head></head><body><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form></body></html>";
const char *RESET_INDEX = "<html><head></head><body><h2>Reset complete</h2></body></html>";

void saveConfigCallback()
{
	shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *wifiManager)
{

	Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(wifiManager->getConfigPortalSSID());
	// matrix->clear();
	// matrix->setCursor(3, 6);
	// matrix->setTextColor(matrix->Color(0, 255, 50));
	// matrix->print("Hotspot");
	// matrix->show();
}


void initWiFi() {
  wifiManager.setAPStaticIPConfig(IPAddress(172, 217, 28, 1), IPAddress(172, 217, 28, 1), IPAddress(255, 255, 255, 0));
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
  Serial.println(WiFi.localIP());
}

void initServer() {
  server.on("/", HTTP_GET, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/html", UPLOAD_INDEX);
	});

	server.on("/reset", HTTP_GET, []() {
		server.send(200, "text/html", RESET_INDEX);
		wifiManager.resetSettings();
		ESP.restart();
	});
	
	// server.on(
	// 	"/update", HTTP_POST, []() {
  //     server.sendHeader("Connection", "close");
  //     server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  //     ESP.restart(); }, []() {
  //     HTTPUpload& upload = server.upload();

  //     if (upload.status == UPLOAD_FILE_START) {
  //       Serial.setDebugOutput(true);

  //       uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  //       if (!Update.begin(maxSketchSpace)) { //start with max available size
  //         Update.printError(Serial);
  //       }
  //     } else if (upload.status == UPLOAD_FILE_WRITE) {
	// 	  matrix->clear();
	// 	  flashProgress((int)upload.currentSize,(int)upload.buf);
  //       if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
  //         Update.printError(Serial);
  //       }
  //     } else if (upload.status == UPLOAD_FILE_END) {
  //       if (Update.end(true)) { //true to set the size to the current progress
	// 	  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");


  //       } else {
  //         Update.printError(Serial);
  //       }
  //       Serial.setDebugOutput(false);
  //     }
  //     yield(); 
  // });

	server.begin();
}

void setup(void)
{
  Serial.begin(115200);
  Serial.print("Sleeping for ");
  Serial.println(SLEEP_SEC);
  sleep(SLEEP_SEC);
  initWiFi();
  initServer();
}

void loop() 
{
  // Handle potential HTTP requests
  server.handleClient();
}