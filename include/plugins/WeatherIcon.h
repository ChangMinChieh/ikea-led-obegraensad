#pragma once
#include "PluginManager.h"
#include "timing.h"
#include <time.h>

class WeatherIconPlugin : public Plugin {
private:
  NonBlockingDelay timer;
  NonBlockingDelay animationTimer;
  int currentIcon = 0;
  int animationFrame = 0;
  int myBrightness = 180; // 統一亮度設定

public:
  void setup() override;
  void loop() override;
  void websocketHook(JsonDocument &request) override;
  const char *getName() const override;
};