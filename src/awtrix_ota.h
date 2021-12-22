#ifndef __AWTRIX_OTA_H__
#define __AWTRIX_OTA_H__
#include <ArduinoOTA.h>
static bool updating = false;

void initOTA();
void flashProgress(unsigned int progress, unsigned int total);
#endif
