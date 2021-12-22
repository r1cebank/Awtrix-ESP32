#include <awtrix_net.h>
#include <awtrix_ota.h>
#include <awtrix_config.h>
#include <WiFiManager.h>
#include <awtrix_matrix.h>

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

void saveConfigCallback()
{
  shouldSaveConfig = true;
}

void pubsubCallback(char *topic, byte *payload, unsigned int length)
{
	// updateMatrix(payload, length);
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

void initConnection()
{
    client.setServer(awtrix_server_ip, atoi(awtrix_server_port));
	client.setCallback(pubsubCallback);
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
