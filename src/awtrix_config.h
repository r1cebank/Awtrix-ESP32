#ifndef __AWTRIX_CONFIG_H__
#define __AWTRIX_CONFIG_H__
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

static const char *CONFIG_FILE = "/awtrix.json";

bool loadConfig();
bool saveConfig();
void initFS();
#endif