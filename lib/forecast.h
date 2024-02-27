#include <cpr/cpr.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;
using namespace ftxui;

const std::string kUrlCityTemplate = "https://api.api-ninjas.com/v1/city";
const std::string kUrlForecastTemplate =
    "https://api.open-meteo.com/v1/forecast";
static std::string api_ninjas_city_key;

const int16_t kToParseCoordsBegin = 1;
const int16_t kToParseCoordsEnd = 2;
const uint8_t kStartHourNight = 0;
const uint8_t kEndHourNight = 6;
const uint8_t kStartHourMorning = 6;
const uint8_t kEndHourMorning = 12;
const uint8_t kStartHourDay = 12;
const uint8_t kEndHourDay = 18;
const uint8_t kStartHourEvening = 18;
const uint8_t kEndHourEvening = 24;
const uint8_t kPartCountHour = 6;
const uint8_t kCountHourInDay = 24;
const uint8_t kLenFormatDate = 10;
const uint8_t kIndexNight = 0;
const uint8_t kIndexMorning = 1;
const uint8_t kIndexDay = 2;
const uint8_t kIndexEvening = 3;
const uint8_t kMaxDaysInMonth = 30;
const uint8_t kMinDaysInMonth = 1;

std::map<uint8_t, std::string> output_part_of_day{
    {0, "Night"}, {1, "Morning"}, {2, "Day"}, {3, "Evening"}};

enum class IndexPartsOfDay {Night, Morning, Day, Evening};

struct CityInfo {
  std::string name;
  double longitude;
  double latitude;
};

struct WeatherInfo {
  uint16_t weather_code = 0;
  float max_temperature = __FLT_MIN__;
  float min_temperature = __FLT_MAX__;
  uint16_t wind_speed = INT16_MIN;
  float precipitation = 0;
  uint16_t precipitation_probability = 0;
};

struct AllDayWetherInfo {
  std::string date;
  std::vector<WeatherInfo> parts_of_day = std::vector<WeatherInfo>(4);
};

std::map<std::string, CityInfo> geographical_info;

json ReadConfig(std::string_view directory);

void PrintError(const std::string_view& text);

void SetCoords(CityInfo& city);

json SendRequest(CityInfo& city, json& data, uint16_t counter);

std::map<std::string, std::vector<AllDayWetherInfo>> GetInfoForForecast(
    json& data_from_config, uint16_t& counter);

void DrawForecast(const std::string& directory);