#include "plugins/DrawEndpointPlugin.h"

void DrawEndpointPlugin::setup()
{
  delay(50);
  Screen.clear();
  if (Screen.isEndpointCacheEmpty())
  {
    Screen.loadFromEndpointStorage();
  }
  else
  {
    Screen.endpointRestoreCache();
  }
#ifdef ENABLE_SERVER
  sendInfo();
#endif
}

void DrawEndpointPlugin::teardown()
{
  Screen.endpointCacheCurrent();
}

const char* DrawEndpointPlugin::restHook(const char *command, const char *dataJson)
{
  if(strcmp(command, "draw")==0)
  {
    DynamicJsonDocument doc(6144);
    DeserializationError error = deserializeJson(doc, dataJson);
    if (error)
    {
      return "error";
    }
    JsonArray schedule = doc.as<JsonArray>();
    int counter = 0;
    for (JsonVariant item : schedule)
    {
      int pixelData = item.as<int>();
      Screen.setPixelAtIndex(counter, 1, pixelData);
      counter++;
    }
    Screen.persistEndpoint();
    return "ok";
  }
  return "invalid";
}

const char *DrawEndpointPlugin::getName() const
{
  return "Draw Endpoint";
}