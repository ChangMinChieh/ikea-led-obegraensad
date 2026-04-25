#include "plugins/ClockPlugin.h"

void ClockPlugin::setup()
{
  // loading screen
  Screen.setPixel(4, 7, 1);
  Screen.setPixel(5, 7, 1);
  Screen.setPixel(7, 7, 1);
  Screen.setPixel(8, 7, 1);
  Screen.setPixel(10, 7, 1);
  Screen.setPixel(11, 7, 1);

  previousMinutes = -1;
  previousHour = -1;
  previousHH.clear();
  previousMM.clear();
}

void ClockPlugin::loop()
{
  if (getLocalTime(&timeinfo))
  {
    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min)
    {
      std::vector<int> hh = {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10};

      // 關鍵修復 1：每次更新都必須清空畫布，否則數字會重疊成發光方塊
      Screen.clear();

      // 關鍵修復 2：垂直堆疊佈局
      // x=3 是為了讓 4+1+4=9 像素寬的數字組在 16 像素寬度中視覺置中
      // y 座標設定為 1 和 9，拉開上下距離
      Screen.drawNumbers(3, 1, hh);
      Screen.drawNumbers(3, 9, mm);

      previousHH = hh;
      previousMM = mm;
      previousMinutes = timeinfo.tm_min;
      previousHour = timeinfo.tm_hour;
    }
  }
}

const char *ClockPlugin::getName() const
{
  return "Clock";
}
