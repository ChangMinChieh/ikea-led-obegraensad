#include "plugins/ForecastPlugin.h"
#include <ArduinoJson.h>

#ifdef ESP32
#include <HTTPClient.h>
#include <WiFi.h>
#endif
#ifdef ENABLE_STORAGE
#include <Preferences.h>
#endif

// ==========================================
// Home Assistant 連線設定
// ==========================================
const char* haServer = HA_SERVER;
const char* haToken  = HA_TOKEN;

// ==========================================
// CWA 天氣代碼轉換
// ==========================================
int ForecastPlugin::mapCwaCode(int code) {
  if (code == 1) return 2;             // 晴
  if (code >= 2 && code <= 3) return 3; // 多雲
  if (code >= 4 && code <= 7) return 0; // 陰
  if (code >= 8 && code <= 22) return 4;// 雨
  return 1; // 雷雨
}

void ForecastPlugin::loadNightWindowConfig() {
#ifdef ENABLE_STORAGE
  Preferences prefs;
  prefs.begin("cityclock", true);
  int start = prefs.getInt("nightStart", 20);
  int end = prefs.getInt("nightEnd", 23);
  prefs.end();

  if (start >= 0 && start <= 23) nightStartHour = start;
  if (end >= 0 && end <= 23) nightEndHour = end;
#endif
}

// ==========================================
// 數據抓取 (包含氣象、濕度、PM2.5、CO2、明日趨勢)
// ==========================================
void ForecastPlugin::fetchHAData() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClient client;
  HTTPClient http;
  
  const char* entities[] = {
    "sensor.opencwa_xin_zhuang_qu_weather_code",
    "sensor.opencwa_xin_zhuang_qu_feels_like_temperature",
    "sensor.cwa_max_temp",
    "sensor.cwa_min_temp",
    "sensor.alpstuga_air_quality_monitor_shi_du_2",
    "sensor.alpstuga_air_quality_monitor_pm2_5_2",
    "sensor.alpstuga_air_quality_monitor_er_yang_hua_tan_2",
    "sensor.tomorrow_avg_temp_trend",
    "sensor.ming_ri_qi_wen_qu_shi"
  };

  bool trendUpdated = false;
  int newTrend = tomorrowTrend;

  for (int i = 0; i < 9; i++) {
    String url = String(haServer) + "/api/states/" + String(entities[i]);
    http.begin(client, url);
    http.addHeader("Authorization", "Bearer " + String(haToken));
    http.setTimeout(3000);

    if (http.GET() == HTTP_CODE_OK) {
      JsonDocument doc;
      deserializeJson(doc, http.getString());
      String state = doc["state"].as<String>();
      
      if (state != "unknown" && state != "unavailable") {
        if (i == 0) weatherIcon = mapCwaCode(state.toInt());
        if (i == 1) haFeelsLike = state.toFloat();
        if (i == 2) maxTemp = (int)roundf(state.toFloat());
        if (i == 3) minTemp = (int)roundf(state.toFloat());
        if (i == 4) haHumidity = state.toFloat();
        if (i == 5) haPM25 = state.toFloat();
        if (i == 6) haCO2 = state.toFloat();
        if (i == 7 || i == 8) { newTrend = (int)roundf(state.toFloat()); trendUpdated = true; }
        
        // 警告邏輯：PM2.5 > 35 或 CO2 > 1000
        showAQIWarning = (haPM25 > 35.0f || haCO2 > 1000.0f);
        hasData = true;
      }
    }
    http.end();
    delay(100); 
  }

  if (trendUpdated) {
    tomorrowTrend = newTrend;
    hasTomorrowTrend = true;
  }
}

// ==========================================
// 繪製數值 (支援 CO2 四位數)
// ==========================================
void ForecastPlugin::drawTempValue(int val, int y) {
  int absV = abs(val);
  if (val >= 1000) { 
    Screen.drawNumbers(2, y, {absV/1000, (absV/100)%10, (absV/10)%10, absV%10});
  } else if (val >= 100 || val <= -100) {
    Screen.drawNumbers(4, y, {absV/100, (absV/10)%10, absV%10});
  } else if (val >= 10 || val <= -10) {
    Screen.drawNumbers(6, y, {absV/10, absV%10});
    if (val < 0) Screen.setPixel(5, y + 2, 1, myBrightness);
  } else {
    if (val < 0) Screen.setPixel(6, y + 2, 1, myBrightness);
    Screen.drawNumbers(8, y, {absV});
  }
}

// ==========================================
// 繪製時鐘 (亮度補償)
// ==========================================
void ForecastPlugin::drawInternalClock() {
  if (getLocalTime(&_internalTime)) {
    if (_lastH != _internalTime.tm_hour || _lastM != _internalTime.tm_min) {
      Screen.clear();
      int clockB = (myBrightness + 25 > 255) ? 255 : myBrightness + 25;
      Screen.drawCharacter(2, 0, Screen.readBytes(fonts[1].data[_internalTime.tm_hour/10]), 8, clockB);
      Screen.drawCharacter(9, 0, Screen.readBytes(fonts[1].data[_internalTime.tm_hour%10]), 8, clockB);
      Screen.drawCharacter(2, 9, Screen.readBytes(fonts[1].data[_internalTime.tm_min/10]), 8, clockB);
      Screen.drawCharacter(9, 9, Screen.readBytes(fonts[1].data[_internalTime.tm_min%10]), 8, clockB);
      _lastH = _internalTime.tm_hour; _lastM = _internalTime.tm_min;
    }
  }
}

void ForecastPlugin::setup() {
  Screen.clear(); displayMode = 5; modeStart = millis();
  _lastH = -1; _lastM = -1; lastFetch = millis();
  loadNightWindowConfig();
  lastNightConfigLoad = millis();
  fetchHAData();
}

void ForecastPlugin::loop() {
  if (millis() - lastFetch > 600000UL) {
    lastFetch = millis();
    fetchHAData();
  }
  if (millis() - lastNightConfigLoad > 5000UL) {
    loadNightWindowConfig();
    lastNightConfigLoad = millis();
  }

  unsigned long elapsed = millis() - modeStart;
  int currentHour = _internalTime.tm_hour;
  if (getLocalTime(&_internalTime)) {
    currentHour = _internalTime.tm_hour;
  } else {
    time_t now = time(nullptr);
    struct tm fallbackTime;
    localtime_r(&now, &fallbackTime);
    currentHour = fallbackTime.tm_hour;
  }
  bool showNightTrend = false;
  if (nightStartHour == nightEndHour) {
    showNightTrend = (currentHour == nightStartHour);
  } else if (nightStartHour < nightEndHour) {
    showNightTrend = (currentHour >= nightStartHour && currentHour <= nightEndHour);
  } else {
    showNightTrend = (currentHour >= nightStartHour || currentHour <= nightEndHour);
  }

  switch (displayMode) {
    case 2: // 溫度面板：白天高低溫、晚間明日趨勢
      if (displayTimer.isReady(1000)) {
        static unsigned long lastDebugPrint = 0;
        if (millis() - lastDebugPrint > 30000UL) {
          Serial.printf("[ForecastPlugin] hour=%d night=%d range=%d~%d hasTrend=%d trend=%d\n",
                        currentHour, showNightTrend, nightStartHour, nightEndHour, hasTomorrowTrend, tomorrowTrend);
          lastDebugPrint = millis();
        }
        Screen.clear();
        if (showNightTrend && hasTomorrowTrend) {
          // 晚間顯示明日趨勢：上升/下降/持平 + 差值
          if (tomorrowTrend > 0) {
            Screen.setPixel(2, 4, 1, myBrightness);
            Screen.setPixel(1, 5, 1, myBrightness); Screen.setPixel(2, 5, 1, myBrightness); Screen.setPixel(3, 5, 1, myBrightness);
            Screen.setPixel(2, 6, 1, myBrightness); Screen.setPixel(2, 7, 1, myBrightness);
          } else if (tomorrowTrend < 0) {
            Screen.setPixel(2, 4, 1, myBrightness); Screen.setPixel(2, 5, 1, myBrightness);
            Screen.setPixel(1, 6, 1, myBrightness); Screen.setPixel(2, 6, 1, myBrightness); Screen.setPixel(3, 6, 1, myBrightness);
            Screen.setPixel(2, 7, 1, myBrightness);
          } else {
            Screen.setPixel(1, 5, 1, myBrightness); Screen.setPixel(2, 5, 1, myBrightness); Screen.setPixel(3, 5, 1, myBrightness);
          }
          drawTempValue(tomorrowTrend, 4);
        } else {
          // 白天顯示今日高低溫
          Screen.setPixel(2, 1, 1, myBrightness);
          Screen.setPixel(1, 2, 1, myBrightness); Screen.setPixel(2, 2, 1, myBrightness); Screen.setPixel(3, 2, 1, myBrightness);
          Screen.setPixel(2, 3, 1, myBrightness); Screen.setPixel(2, 4, 1, myBrightness);
          drawTempValue(maxTemp, 2);

          Screen.setPixel(2, 9, 1, myBrightness); Screen.setPixel(2, 10, 1, myBrightness);
          Screen.setPixel(1, 11, 1, myBrightness); Screen.setPixel(2, 11, 1, myBrightness); Screen.setPixel(3, 11, 1, myBrightness);
          Screen.setPixel(2, 12, 1, myBrightness);
          drawTempValue(minTemp, 9);
        }
      }
      if (elapsed >= 10000) { displayMode = 5; modeStart = millis(); _lastM = -1; displayTimer.forceReady(); }
      break;

    case 5: // 時鐘
      drawInternalClock();
      if (elapsed >= 60000) { displayMode = 2; modeStart = millis(); displayTimer.forceReady(); Screen.clear(); }
      break;
  }
}

void ForecastPlugin::websocketHook(JsonDocument &request) {}
const char *ForecastPlugin::getName() const { return "CWA HA Clock Plus"; }