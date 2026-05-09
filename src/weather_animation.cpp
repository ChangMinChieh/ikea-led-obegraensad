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
    // 1. 先畫出基礎雲塊
    Screen.drawWeather(x, y, icon, brightness);
    
    // 2. 清空閃電活動區域
    // 閃電現在從 y+3 開始，高度 5px，加上位移 1px，範圍約在 y+3 ~ y+9
    for (int col = 5; col <= 9; col++) {
      for (int row = 3; row <= 9; row++) {
        Screen.setPixel(x + col, y + row, 0, 0);
      }
    }

    // 3. 決定位移 (0 或 1 像素)
    int yOffset = (animationFrame % 2 == 0) ? 0 : 1;
    
    // 4. 向上移動 2px：將基礎起始點改為 y + 3
    int startY = y + 3 + yOffset; 
    int startX = x + 5;

    // 5. 繪製閃電 (1x1, 1x1, 1x3, 1x1, 1x1)
    // 第一列：--#--
    Screen.setPixel(startX + 3, startY + 0, 255, brightness);
    
    // 第二列：-#---
    Screen.setPixel(startX + 2, startY + 1, 255, brightness);
    
    // 第三列：-###-
    Screen.setPixel(startX + 1, startY + 2, 255, brightness);
    Screen.setPixel(startX + 2, startY + 2, 255, brightness);
    Screen.setPixel(startX + 3, startY + 2, 255, brightness);
    
    // 第四列：-#---
    Screen.setPixel(startX + 2, startY + 3, 255, brightness);
    
    // 第五列：-#---
    Screen.setPixel(startX + 1, startY + 4, 255, brightness);
} else if (icon == 3) {
    // 晴時多雲：太陽固定，雲朵擺動
    int sequence[] = {0, 1, 0, -1};
    int xOffset = sequence[animationFrame % 4];

    // 1. 清空 16x6 區域
    for (int row = 0; row <= 5; row++) {
        for (int col = 0; col < 16; col++) {
            Screen.setPixel(x + col, y + row, 0, 0);
        }
    }

    // 2. 繪製太陽 (固定座標) - 這是你原始檔案中 index 2 (clear) 的變體
    const int sunPixels[][2] = {
        {10, 0}, {11, 0}, {12, 0},
        {9, 1}, {10, 1}, {11, 1}, {12, 1}, {13, 1},
        {8, 2}, {9, 2}, {10, 2}, {11, 2}, {12, 2}, {13, 2},
        {10, 3}, {11, 3}, {12, 3}, {12, 4}, {13, 4}
    };
    for (auto &p : sunPixels) {
        Screen.setPixel(x + p[0], y + p[1], 255, brightness);
    }

    // 3. 繪製雲朵 (動態偏移)
    // 這裡我們定義 Frame 1 的基礎點
    std::vector<std::pair<int, int>> cloud = {
        {5, 1}, {6, 1}, {7, 1}, {4, 2}, {5, 2}, {2, 3}, {3, 3}, {4, 3},
        {1, 4}, {2, 4}, {6, 4}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {6, 5}, 
        {7, 5}, {8, 5}, {9, 5}, {10, 5}, {11, 5}, {12, 5}
    };

    for (auto &p : cloud) {
        int cx = p.first;
        int cy = p.second;

        // 【關鍵：轉換為 Frame 3 的邏輯】
        // 當偏移量為 1 時，我們微調最右邊的像素，讓它看起來像 Frame 3
        if (xOffset == 1) {
            if (cx == 12 && cy == 4) continue; // 移除這個點，讓右側變纖細
            if (cx == 6 && cy == 4) cx = 7;    // 讓雲朵的中間結構稍作拉伸
        }

        Screen.setPixel(x + cx + xOffset, y + cy, 255, brightness);
    }
} else {
    // 預設靜態圖示
    Screen.drawWeather(x, y, icon, brightness);
  }
} 
