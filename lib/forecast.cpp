#include "forecast.h"

void PrintError(const std::string& text) {
  std::cerr << text << '\n';
  exit(EXIT_FAILURE);
}

json ReadConfig(const std::string& directory) {
  std::filesystem::path path = directory;
  std::ifstream input_config(path);
  if (input_config) {
    json data = json::parse(input_config);
    api_ninjas_city_key = data["X-Api-Key"];
    count_days = data["forecast_days"];
    return data;
  }
  PrintError("Not found config.json");
}

void GetCoords(CityInfo& city) {
  std::string url = url_city_template;
  cpr::Response response =
      cpr::Get(cpr::Url(url), cpr::Parameters{{"name", city.name}},
               cpr::Header{{"X-Api-Key", api_ninjas_city_key}});
  if (response.status_code == 200) {
    response.text = response.text.substr(
        to_parse_coords_begin, response.text.length() - to_parse_coords_end);
    json data = json::parse(response.text);
    city.latitude = data["latitude"];
    city.longitude = data["longitude"];
    return;
  }
  PrintError("Bad Gateway!");
}

json SendRequest(CityInfo& city, json& data) {
  cpr::Response response =
      cpr::Get(cpr::Url(url_forecast_template),
               cpr::Parameters{{"longitude", std::to_string(city.longitude)},
                               {"latitude", std::to_string(city.latitude)},
                               {"forecast_days", std::to_string(count_days)},
                               {"hourly", "weather_code"},
                               {"hourly", "temperature_2m"},
                               {"hourly", "wind_speed_10m"},
                               {"hourly", "precipitation"},
                               {"hourly", "precipitation_probability"}});
  if (response.status_code == 200) {
    json data = json::parse(response.text);
    return data;
  }
  PrintError("Error!");
}

uint16_t GetWeatherCode(std::vector<uint16_t>& max_entry_weather_code) {
  std::map<uint16_t, uint16_t> codes;
  for (uint16_t i = 0; i < max_entry_weather_code.size(); ++i) {
    ++codes[max_entry_weather_code[i]];
  }
  int16_t max_entry = INT16_MIN;
  uint16_t max_entry_code = 0;
  for (auto element : codes) {
    if (element.first > max_entry) {
      max_entry = element.second;
      max_entry_code = element.first;
    }
  }
  return max_entry_code;
}

struct DayPartWeatherInfo {
  float max_temperature_part = -__FLT_MAX__;
  float min_temperature_part = __FLT_MAX__;
  int16_t wind_speed_average = 0;
  float precipitation_average = 0.0;
  int16_t precipitation_probability_average = 0;
  std::vector<uint16_t> max_entry_weather_code;
  uint8_t start_hour = 0;
  uint8_t end_hour = 0;
};

void SetWeatherInfoForPartDay(
    DayPartWeatherInfo& data, uint16_t& iteration, const uint8_t& part_of_day,
    std::string& city_name,
    std::map<std::string, std::vector<AllDayWetherInfo>>& city_forecast) {
  city_forecast[city_name][iteration].parts_of_day[part_of_day].precipitation =
      std::round(data.precipitation_average / part_count_hour);
  city_forecast[city_name][iteration]
      .parts_of_day[part_of_day]
      .precipitation_probability =
      data.precipitation_probability_average / part_count_hour;
  city_forecast[city_name][iteration].parts_of_day[part_of_day].wind_speed =
      data.wind_speed_average / part_count_hour;
  city_forecast[city_name][iteration].parts_of_day[part_of_day].weather_code =
      GetWeatherCode(data.max_entry_weather_code);
  city_forecast[city_name][iteration]
      .parts_of_day[part_of_day]
      .max_temperature = std::round(data.max_temperature_part);
  city_forecast[city_name][iteration]
      .parts_of_day[part_of_day]
      .min_temperature = std::round(data.min_temperature_part);
}

void ParseInfoPartOfDay(
    DayPartWeatherInfo& data, const std::string& part_of_day,
    uint16_t& iteration, json& info, std::string& city_name,
    std::map<std::string, std::vector<AllDayWetherInfo>>& city_forecast) {
  for (uint16_t i = data.start_hour + iteration * count_hour_in_day;
       i < data.end_hour + iteration * count_hour_in_day; ++i) {
    data.precipitation_average +=
        info["hourly"]["precipitation"][i].get<float>();
    data.precipitation_probability_average +=
        info["hourly"]["precipitation_probability"][i].get<int16_t>();
    data.wind_speed_average +=
        info["hourly"]["wind_speed_10m"][i].get<int16_t>();
    data.max_entry_weather_code.push_back(
        info["hourly"]["weather_code"][i].get<uint16_t>());
    data.max_temperature_part =
        std::max(data.max_temperature_part,
                 info["hourly"]["temperature_2m"][i].get<float>());
    data.min_temperature_part =
        std::min(data.min_temperature_part,
                 info["hourly"]["temperature_2m"][i].get<float>());
  }
  if (part_of_day == "night") {
    SetWeatherInfoForPartDay(data, iteration, index_night, city_name,
                             city_forecast);
  } else if (part_of_day == "morning") {
    SetWeatherInfoForPartDay(data, iteration, index_morning, city_name,
                             city_forecast);
  } else if (part_of_day == "day") {
    SetWeatherInfoForPartDay(data, iteration, index_day, city_name,
                             city_forecast);
  } else if (part_of_day == "evening") {
    SetWeatherInfoForPartDay(data, iteration, index_evening, city_name,
                             city_forecast);
  }
}

void ParseRequest(
    CityInfo& city, json& info,
    std::map<std::string, std::vector<AllDayWetherInfo>>& city_forecast) {
  uint16_t time_iter = 0;
  DayPartWeatherInfo data;
  for (uint16_t i = 0; i < count_days; ++i, time_iter += count_hour_in_day) {
    data.start_hour = start_hour_night;
    data.end_hour = end_hour_night;
    ParseInfoPartOfDay(data, "night", i, info, city.name, city_forecast);
    data = DayPartWeatherInfo();
    data.start_hour = start_hour_morning;
    data.end_hour = end_hour_morning;
    ParseInfoPartOfDay(data, "morning", i, info, city.name, city_forecast);
    data = DayPartWeatherInfo();
    data.start_hour = start_hour_day;
    data.end_hour = end_hour_day;
    ParseInfoPartOfDay(data, "day", i, info, city.name, city_forecast);
    data = DayPartWeatherInfo();
    data.start_hour = start_hour_evening;
    data.end_hour = end_hour_evening;
    ParseInfoPartOfDay(data, "evening", i, info, city.name, city_forecast);
    city_forecast[city.name][i].date =
        info["hourly"]["time"][time_iter].get<std::string>().substr(
            0, len_format_date);
  }
}

std::map<std::string, std::vector<AllDayWetherInfo>> GetInfoForForecast(
    const std::string& directory) {
  std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast;
  json data_from_config = ReadConfig(directory);
  for (auto city : data_from_config["cities"]) {
    city_forecast[city] = std::vector<AllDayWetherInfo>(
        data_from_config["forecast_days"].get<int16_t>());
    geographical_info[city].name = city;
    GetCoords(geographical_info[city]);
    json info = SendRequest(geographical_info[city], data_from_config);
    ParseRequest(geographical_info[city], info, city_forecast);
  }
  return city_forecast;
}