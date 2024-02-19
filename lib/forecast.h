#include <cpr/cpr.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <thread>
#include <csignal>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/receiver.hpp>
#include <ftxui/dom/elements.hpp>

using json = nlohmann::json;
using namespace ftxui;

const std::string url_city_template = "https://api.api-ninjas.com/v1/city";
const std::string url_forecast_template =
    "https://api.open-meteo.com/v1/forecast";
static std::string api_ninjas_city_key;

static uint16_t count_days;
const int16_t to_parse_coords_begin = 1;
const int16_t to_parse_coords_end = 2;
const uint8_t start_hour_night = 0;
const uint8_t end_hour_night = 6;
const uint8_t start_hour_morning = 6;
const uint8_t end_hour_morning = 12;
const uint8_t start_hour_day = 12;
const uint8_t end_hour_day = 18;
const uint8_t start_hour_evening = 18;
const uint8_t end_hour_evening = 24;
const uint8_t part_count_hour = 6;
const uint8_t count_hour_in_day = 24;
const uint8_t len_format_date = 10;
const uint8_t index_night = 0;
const uint8_t index_morning = 1;
const uint8_t index_day = 2;
const uint8_t index_evening = 3;


struct CityInfo {
  std::string name;
  double longitude;
  double latitude;
};

struct WeatherInfo {
  int16_t weather_code = 0;
  float max_temperature = __FLT_MIN__;
  float min_temperature = __FLT_MAX__;
  int16_t wind_speed = INT16_MIN;
  float precipitation = 0;
  float precipitation_probability = 0;
  WeatherInfo() = default;
};

struct AllDayWetherInfo {
  std::string date;
  std::vector<WeatherInfo> parts_of_day = std::vector<WeatherInfo>(4);
};

static std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast;
static std::map<std::string, CityInfo> geographical_info;

json ReadConfig(const std::string& directory);

void PrintError(const std::string& text);

void GetCoords(CityInfo& city);

json SendRequest(CityInfo& city, json& data);

void GetInfoForForecast(const std::string& directory);