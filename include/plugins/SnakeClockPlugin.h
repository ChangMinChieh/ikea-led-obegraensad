#pragma once

#include "PluginManager.h"

class SnakeClockPlugin : public Plugin
{
private:
  bool enabled = false;
  static const uint8_t X_MAX = 16;
  static const uint8_t Y_MAX = 16;

  struct tm timeinfo;

  int
      previousMinutes,
      previousHour,
      current_minute = 0,
      current_hour = 0;

  void drawCharacter(int x, int y, std::vector<int> bits, int bitCount, uint8_t brightness = 255);
  void drawDigits();

  static const uint8_t LED_TYPE_OFF = 0;
  static const uint8_t LED_TYPE_ON = 1;
  static const uint8_t GAME_STATE_RUNNING = 1;
  static const uint8_t GAME_STATE_END = 2;

  unsigned char gameState;
  unsigned char lastDirection = 0; // 0=unset 1=up 2=right 3=down 4 =left

  std::vector<uint> position = {240, 241, 242};

  uint8_t dot;

  void initGame();
  void newDot();
  void findDirection();
  void moveSnake(uint newpos);
  void end();
  void delayWithClockUpdate(int delay);

public:
  void setup() override;
  void loop() override;
  void teardown() override;
  const char *getName() const override;
};