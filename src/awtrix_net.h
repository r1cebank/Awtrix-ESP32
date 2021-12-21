

// WiFi configuration
bool shouldSaveConfig = false;
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(AWTRIX_LOCAL_PORT);