#include <awtrix_config.h>
#include <awtrix_matrix.h>
#include <awtrix_net.h>

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
