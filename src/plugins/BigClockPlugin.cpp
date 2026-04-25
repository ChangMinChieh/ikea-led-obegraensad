#include "plugins/BigClockPlugin.h"

void BigClockPlugin::setup()
{
  previousMinutes = -1;
  previousHour = -1;
  previousSecond = -1; // 現在對應到 .h 裡的定義了
  previousHH.clear();
  previousMM.clear();
  Screen.clear();
}

void BigClockPlugin::loop()
{
  if (getLocalTime(&timeinfo))
  {
    // 每分鐘更新一次畫面
    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min)
    {
      std::vector<int> hh = {timeinfo.tm_hour / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {timeinfo.tm_min / 10, timeinfo.tm_min % 10};

      Screen.clear();
      
      // 小時 (y=0)
      Screen.drawCharacter(2, 0, Screen.readBytes(fonts[1].data[hh[0]]), 8, Screen.getCurrentBrightness());
      Screen.drawCharacter(9, 0, Screen.readBytes(fonts[1].data[hh[1]]), 8, Screen.getCurrentBrightness());
      
      // 分鐘 (y=9)
      Screen.drawCharacter(2, 9, Screen.readBytes(fonts[1].data[mm[0]]), 8, Screen.getCurrentBrightness());
      Screen.drawCharacter(9, 9, Screen.readBytes(fonts[1].data[mm[1]]), 8, Screen.getCurrentBrightness());

      previousHH = hh;
      previousMM = mm;
      previousMinutes = timeinfo.tm_min;
      previousHour = timeinfo.tm_hour;
    }
    
    // 雖然沒用到秒數畫圖，但更新它避免邏輯錯誤
    if (previousSecond != timeinfo.tm_sec) {
        previousSecond = timeinfo.tm_sec;
    }
  }
}

const char *BigClockPlugin::getName() const
{
  return "Big Clock";
}