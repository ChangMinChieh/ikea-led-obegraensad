#pragma once

#ifdef ESP32
#include <HTTPClient.h>
#endif
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#endif
#include <ArduinoJson.h>
#include "PluginManager.h"
#include "secrets.h"
class HAWeatherPlugin : public Plugin
{
private:
  unsigned long lastUpdate = 0;
  HTTPClient http;

  int lastWeather = -1;
  int lastTemperature = 10000;
  void drawWeatherAndTemperature(int weather, int temperature); 
  int getCode(const char* weather);
public:
    void update();
    void setup() override;
    void loop() override;
    const char *getName() const override;
};
