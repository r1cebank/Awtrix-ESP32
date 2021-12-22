#ifndef __AWTRIX_NET_H__
#define __AWTRIX_NET_H__
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

static const int AWTRIX_LOCAL_PORT = 80;

static char awtrix_server_ip[16] = "0.0.0.0";
static char awtrix_server_port[6] = "7001"; // AWTRIX Host Port, default = 7001

// Server content
static const char *UPLOAD_INDEX = "<html><head></head><body><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form></body></html>";
static const char *RESET_INDEX = "<html><head></head><body><h2>Reset complete</h2></body></html>";

// WiFi configuration
static bool shouldSaveConfig = false;
static WiFiManager wifiManager;
static WiFiClient espClient;
static PubSubClient client(espClient);
static WebServer server(AWTRIX_LOCAL_PORT);

void configModeCallback(WiFiManager *wifiManager);
void initWiFi();
void initServer();
void serverSearch(int rounds, int x, int y);
void initConnection();
void reconnectClient();
#endif