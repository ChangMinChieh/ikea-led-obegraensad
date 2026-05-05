#include "plugins/WeatherIcon.h"
#include "signs.h" // 包含 weatherIcons
#include "weather_animation.h"

void WeatherIconPlugin::setup() {
  Screen.clear();
  timer.reset();
  animationTimer.reset();
  currentIcon = 0;
  animationFrame = 0;
  Screen.drawWeather(0, 0, currentIcon, myBrightness);
  drawWeatherAnimation(currentIcon, 0, 0, animationFrame, myBrightness);
}

void WeatherIconPlugin::loop() {
  bool redraw = false;

  if (timer.isReady(7000)) { // 每隔 7 秒切換圖示
    currentIcon = (currentIcon + 1) % (int)weatherIcons.size();
    redraw = true;
  }

  if (animationTimer.isReady(1000)) { // 每 1000ms 切換動畫幀
    animationFrame = (animationFrame + 1) % 3;
    redraw = true;
  }

  if (redraw) {
    Screen.clear();
    Screen.drawWeather(0, 0, currentIcon, myBrightness);
    drawWeatherAnimation(currentIcon, 0, 0, animationFrame, myBrightness);
  }
}

void WeatherIconPlugin::websocketHook(JsonDocument &request) {}

const char *WeatherIconPlugin::getName() const { return "WeatherIcon"; }