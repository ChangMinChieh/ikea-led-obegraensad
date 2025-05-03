#include "plugins/HAWeatherPlugin.h"

#ifdef ESP8266
WiFiClient wiFiClient;
#endif

void HAWeatherPlugin::setup()
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

    if (this->lastUpdate == 0 || millis() >= this->lastUpdate + (1000 * 60 * 1))
    {
        this->lastUpdate = millis();
        this->update();
    }
    currentStatus = NONE;
}

void HAWeatherPlugin::loop()
{
    if (millis() >= this->lastUpdate + (1000 * 60 * 1))
    {
        this->update();
        this->lastUpdate = millis();
    };
}

void HAWeatherPlugin::update()
{
     String weatherApiString = String(HA_URL) + "/api/states/" + String(HA_ENTITY);
#ifdef ESP32
    http.begin(weatherApiString);
    http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
#endif
#ifdef ESP8266
    http.begin(wiFiClient, weatherApiString);
#endif

    int code = http.GET();

    if (code == HTTP_CODE_OK)
    {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, http.getStream());

        int temperature = round(doc["attributes"]["temperature"].as<float>());
        const char* state = doc["state"];
        int weatherIcon = this -> getCode(state);
        if(this -> lastTemperature != temperature || this -> lastWeather != weatherIcon){
            this -> lastTemperature = temperature;
            this -> lastWeather = weatherIcon;
            this -> drawWeatherAndTemperature(weatherIcon, temperature);
        }
    }

}

void HAWeatherPlugin::drawWeatherAndTemperature(int weatherIcon, int temperature) {
        std::vector<int> allIconY = {1, 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1};
        std::vector<int> allTempY = {10, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 10, 10};
        int iconY = allIconY[weatherIcon];
        int tempY = allTempY[weatherIcon];

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

int HAWeatherPlugin::getCode(const char* weather)
{
    if(strcmp(weather, "clear-night")==0) return 0;
    if(strcmp(weather, "cloudy")==0) return 1;
    if(strcmp(weather, "exceptional")==0) return 2;
    if(strcmp(weather, "fog")==0) return 3;
    if(strcmp(weather, "hail")==0) return 4;
    if(strcmp(weather, "lightning")==0) return 5;
    if(strcmp(weather, "lightning-rainy")==0) return 6;
    if(strcmp(weather, "partlycloudy")==0) return 7;
    if(strcmp(weather, "pouring")==0) return 8;
    if(strcmp(weather, "rainy")==0) return 8;
    if(strcmp(weather, "snowy")==0) return 9;
    if(strcmp(weather, "snowy-rainy")==0) return 10;
    if(strcmp(weather, "sunny")==0) return 11;
    if(strcmp(weather, "windy")==0) return 12;
    if(strcmp(weather, "windy-variant")==0) return 13;
    return 2;
}


const char *HAWeatherPlugin::getName() const
{
    return "Home Assistant Weather";
}
