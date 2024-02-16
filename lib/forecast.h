#include <cpr/cpr.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

const std::string url_city_template =
    "https://api.api-ninjas.com/v1/city";
const std::string url_forecast_template =
    "https://api.open-meteo.com/v1/forecast";

const int16_t to_parse_coords_begin = 1;
const int16_t to_parse_coords_end = 2;

struct CityInfo {
  std::string name;
  double longitude;
  double latitude;
};

struct WeatherInfo {
  std::string date;
  int16_t temperature;
  std::string wind_direction;
  int16_t wind_speed;
  int16_t fallout;
  int16_t wet;
};

struct AllDayWetherInfo {
  WeatherInfo morning;
  WeatherInfo day;
  WeatherInfo evening;
  WeatherInfo night;
};

static std::map<std::string , std::vector<AllDayWetherInfo>> city_forecast;
static std::map<std::string , CityInfo> geographical_info;

json ReadConfig(const std::string& directory);

void PrintError(const std::string& text);

void GetCoords(CityInfo& city);

json SendRequest(CityInfo& city);
void GetInfoForForecast(const std::string& directory);