#include <Arduino.h>

#include <awtrix_config.h>
#include <awtrix_matrix.h>
#include <awtrix_net.h>
#include <awtrix_ota.h>
#include <awtrix_sensors.h>

const int SLEEP_SEC = 5;

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
	initConnection();
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