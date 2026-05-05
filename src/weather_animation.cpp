#include "weather_animation.h"
#include "screen.h"

void drawWeatherAnimation(int icon, int x, int y, int animationFrame, int brightness) {
  if (icon == 0) {
    // 陰天：整個雲朵循環移動
    int xOffset = 0;
    if (animationFrame == 1) {
      xOffset = -2;
    } else if (animationFrame == 2) {
      xOffset = 2;
    }
    
    Screen.drawWeather(x + xOffset, y, icon, brightness);
} else if (icon == 4) {
    // 1. 先畫出雲朵基礎圖標（這會把圖片裡原本的雨滴也畫出來）
    Screen.drawWeather(x, y, icon, brightness);
    
    // 2. 徹底清空雲朵下方（y+5 到 y+7）的所有可能出現雨滴的區域
    // 假設圖標總寬度是 16 像素，我們直接清空整片橫向區域
    for (int col = 0; col < 16; col++) {
      Screen.setPixel(x + col, y + 5, 0, 0);
      Screen.setPixel(x + col, y + 6, 0, 0);
      Screen.setPixel(x + col, y + 7, 0, 0);
    }

    // 3. 定義你要的 4 條雨滴橫向位置
    int rainPositions[] = {4, 7, 10, 12};
    
    // 4. 繪製你自定義的動態雨滴
    for (int i = 0; i < 4; i++) {
      int px = x + rainPositions[i];
      int state = (animationFrame + i) % 4; 
      
      if (state == 0) {
        Screen.setPixel(px, y + 5, 255, brightness);
      } 
      else if (state == 1) {
        Screen.setPixel(px, y + 5, 255, brightness);
        Screen.setPixel(px, y + 6, 255, brightness);
      } 
      else if (state == 2) {
        Screen.setPixel(px, y + 6, 255, brightness);
        Screen.setPixel(px, y + 7, 255, brightness);
      }
    }
  } else if (icon == 1) {
    // 雷雨：在雲朵下方閃爍的閃電
    Screen.drawWeather(x, y, icon, brightness);
    
    // 閃電動畫：每隔 500ms 切換一次閃電狀態
    if (animationFrame % 2 == 0) {
      // 畫出閃電（假設閃電位於圖標的特定位置）
      Screen.setPixel(x + 7, y + 5, 255, brightness); // 閃電頂部
      Screen.setPixel(x + 6, y + 6, 255, brightness);
      Screen.setPixel(x + 8, y + 6, 255, brightness);
      Screen.setPixel(x + 5, y + 7, 255, brightness);
      Screen.setPixel(x + 9, y + 7, 255, brightness);
    } else {
      // 清除閃電
      for (int col = 5; col <= 9; col++) {
        Screen.setPixel(x + col, y + 5, 0, 0);
        Screen.setPixel(x + col, y + 6, 0, 0);
        Screen.setPixel(x + col, y + 7, 0, 0);
      }
    }
  }
} 