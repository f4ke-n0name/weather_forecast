#include <cpr/cpr.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

const std::string url_city_template = "https://api.api-ninjas.com/v1/city";
const std::string url_forecast_template =
    "https://api.open-meteo.com/v1/forecast";
static std::string api_ninjas_city_key;

const int16_t to_parse_coords_begin = 1;
const int16_t to_parse_coords_end = 2;
static uint16_t count_days;

struct CityInfo {
  std::string name;
  double longitude;
  double latitude;
};

struct WeatherInfo {
  std::string status = "";
  int16_t max_temperature = INT_MIN;
  int16_t min_temperature = INT_MAX;
  int16_t wind_speed = INT_MIN;
  int16_t precipitation = INT_MIN;
  int16_t precipitation_probability = INT_MIN;
};

struct AllDayWetherInfo {
  std::string date;
  WeatherInfo morning;
  WeatherInfo day;
  WeatherInfo evening;
  WeatherInfo night;
};

static std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast;
static std::map<std::string, CityInfo> geographical_info;

json ReadConfig(const std::string& directory);

void PrintError(const std::string& text);

void GetCoords(CityInfo& city);

json SendRequest(CityInfo& city, json& data);

void GetInfoForForecast(const std::string& directory);