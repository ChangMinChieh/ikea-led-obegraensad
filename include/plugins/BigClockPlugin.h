#pragma once

#include "PluginManager.h"
#include <vector>

class BigClockPlugin : public Plugin
{
private:
  struct tm timeinfo;

  // 確保這三個變數都存在
  int previousMinutes;
  int previousHour;
  int previousSecond; 

  std::vector<int> previousHH;
  std::vector<int> previousMM;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};