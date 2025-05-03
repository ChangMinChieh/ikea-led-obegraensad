#include "plugins/WeatherPlugin.h"

// https://github.com/chubin/wttr.in/blob/master/share/translations/en.txt
#ifdef ESP8266
WiFiClient wiFiClient;
#endif

void WeatherPlugin::setup()
{
    // loading screen
    Screen.clear();
    currentStatus = LOADING;
    if(this->lastTemperature == 10000 || this->lastWeather==-1){
      Screen.setPixel(4, 7, 1);
      Screen.setPixel(5, 7, 1);
      Screen.setPixel(7, 7, 1);
      Screen.setPixel(8, 7, 1);
      Screen.setPixel(10, 7, 1);
      Screen.setPixel(11, 7, 1);
    }
    else {
        this->drawWeatherAndTemperature(this->lastWeather, this->lastTemperature);
    }

    if (this->lastUpdate == 0 || millis() >= this->lastUpdate + (1000 * 60 * 30))
    {
        this->lastUpdate = millis();
        this->update();
    }
    currentStatus = NONE;
}

void WeatherPlugin::loop()
{
    if (millis() >= this->lastUpdate + (1000 * 60 * 30))
    {
        this->update();
        this->lastUpdate = millis();
        Serial.println("updating weather");
    };
}

void WeatherPlugin::update()
{
    String weatherApiString = "https://wttr.in/" + String(WEATHER_LOCATION) + "?format=j2&lang=en";
#ifdef ESP32
    http.begin(weatherApiString);
#endif
#ifdef ESP8266
    http.begin(wiFiClient, weatherApiString);
#endif

    int code = http.GET();

    if (code == HTTP_CODE_OK)
    {
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, http.getString());

        int temperature = round(doc["current_condition"][0]["temp_C"].as<float>());
        int weatherCode = doc["current_condition"][0]["weatherCode"].as<int>();

        if(this -> lastTemperature != temperature || this -> lastWeather != weatherCode){
            this -> lastTemperature = temperature;
            this -> lastWeather = weatherCode;
            this -> drawWeatherAndTemperature(weatherCode, temperature);
        }
    }
}

void WeatherPlugin::drawWeatherAndTemperature(int weatherCode, int temperature) {
        int weatherIcon = 0;
        int iconY = 1;
        int tempY = 10;

        if (std::find(thunderCodes.begin(), thunderCodes.end(), weatherCode) != thunderCodes.end())
        {
            weatherIcon = 6;
        }
        else if (std::find(rainCodes.begin(), rainCodes.end(), weatherCode) != rainCodes.end())
        {
            weatherIcon = 8;
        }
        else if (std::find(snowCodes.begin(), snowCodes.end(), weatherCode) != snowCodes.end())
        {
            weatherIcon = 9;
        }
        else if (std::find(fogCodes.begin(), fogCodes.end(), weatherCode) != fogCodes.end())
        {
            weatherIcon = 3;
            iconY = 2;
        }
        else if (std::find(clearCodes.begin(), clearCodes.end(), weatherCode) != clearCodes.end())
        {
            weatherIcon = 11;
            iconY = 1;
            tempY = 9;
        }
        else if (std::find(cloudyCodes.begin(), cloudyCodes.end(), weatherCode) != cloudyCodes.end())
        {
            weatherIcon = 1;
            iconY = 2;
            tempY = 9;
        }
        else if (std::find(partyCloudyCodes.begin(), partyCloudyCodes.end(), weatherCode) != partyCloudyCodes.end())
        {
            weatherIcon = 7;
            iconY = 2;
        }

        Screen.clear();
        Screen.drawWeather(0, iconY, weatherIcon, 100);

        if (temperature >= 10)
        {
            Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(1, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
        }
        else if (temperature <= -10)
        {
            Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
            Screen.drawCharacter(11, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            temperature *= -1;
            Screen.drawNumbers(3, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
        }
        else if (temperature >= 0)
        {
            Screen.drawCharacter(7, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(4, tempY, {temperature});
        }
        else
        {
            Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
            Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(3, tempY, {-temperature});
        }
}

const char *WeatherPlugin::getName() const
{
    return "Weather";
}
