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
  int16_t time_iter = 0;
  float max_temperature_part = -__FLT_MAX__;
  float min_temperature_part = __FLT_MAX__;
  int16_t wind_speed_average = 0;
  float precipitation_average = 0.0;
  int16_t precipitation_probability_average = 0;
  std::vector<uint16_t> max_entry_weather_code;
  uint8_t start_hour = 0;
  uint8_t end_hour = 0;
  uint16_t time_iter = 0;
};

void ParseInfoPartOfDay(DayPartWeatherInfo& data, std::string& part_of_day,
                        uint16_t& iteration, json& info,
                        std::string& city_name) {
  for (uint16_t i = data.start_hour + iteration * count_hour_in_day;
       j < data.end_hour + iteration * count_hour_in_day; ++i) {
    data.precipitation_average +=
        info["hourly"]["precipitation"][j].get<float>();
    data.precipitation_probability_average +=
        info["hourly"]["precipitation_probability"][j].get<int16_t>();
    data.wind_speed_average +=
        info["hourly"]["wind_speed_10m"][j].get<int16_t>();
    data.max_entry_weather_code.push_back(
        info["hourly"]["weather_code"][j].get<uint16_t>());
    data.max_temperature_part = std::max(
        max_temperature_part, info["hourly"]["temperature_2m"][j].get<float>());
    data.min_temperature_part = std::min(
        min_temperature_part, info["hourly"]["temperature_2m"][j].get<float>());
  }
  if (part_of day == "night"){
        city_forecast[city_name][iteration].night.precipitation =
        std::round(data.precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city_name][iteration].night.precipitation_probability =
        data.precipitation_probability_average / part_count_hour;
    city_forecast[city_name][iteration].night.wind_speed =
        data.wind_speed_average / part_count_hour;
    city_forecast[city_name][iteration].night.weather_code =
        GetWeatherCode(data.max_entry_weather_code);
    city_forecast[city_name][iteration].night.max_temperature =
        std::round(data.max_temperature_part * 100) / 100;
    city_forecast[city_name][iteration].night.min_temperature =
        std::round(data.min_temperature_part * 100) / 100;
  } 
  else if (part_of day == "morning"){
        city_forecast[city_name][iteration].morning.precipitation =
        std::round(data.precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city_name][iteration].morning.precipitation_probability =
        data.precipitation_probability_average / part_count_hour;
    city_forecast[city_name][iteration].morning.wind_speed =
        data.wind_speed_average / part_count_hour;
    city_forecast[city_name][iteration].morning.weather_code =
        GetWeatherCode(data.max_entry_weather_code);
    city_forecast[city_name][iteration].morning.max_temperature =
        std::round(data.max_temperature_part * 100) / 100;
    city_forecast[city_name][iteration].morning.min_temperature =
        std::round(data.min_temperature_part * 100) / 100;
  }
  else if (part_of_day == "day"){
        city_forecast[city_name][iteration].day.precipitation =
        std::round(data.precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city_name][iteration].day.precipitation_probability =
        data.precipitation_probability_average / part_count_hour;
    city_forecast[city_name][iteration].day.wind_speed =
        data.wind_speed_average / part_count_hour;
    city_forecast[city_name][iteration].day.weather_code =
        GetWeatherCode(data.max_entry_weather_code);
    city_forecast[city_name][iteration].day.max_temperature =
        std::round(data.max_temperature_part * 100) / 100;
    city_forecast[city_name][iteration].day.min_temperature =
        std::round(data.min_temperature_part * 100) / 100;
  }
  else if (part_of_day == "evening"){
        city_forecast[city_name][iteration].evening.precipitation =
        std::round(data.precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city_name][iteration].evening.precipitation_probability =
        data.precipitation_probability_average / part_count_hour;
    city_forecast[city_name][iteration].evening.wind_speed =
        data.wind_speed_average / part_count_hour;
    city_forecast[city_name][iteration].evening.weather_code =
        GetWeatherCode(data.max_entry_weather_code);
    city_forecast[city_name][iteration].evening.max_temperature =
        std::round(data.max_temperature_part * 100) / 100;
    city_forecast[city_name][iteration].evening.min_temperature =
        std::round(data.min_temperature_part * 100) / 100;
  }
}

void ParseRequest(CityInfo& city, json& info) {
  uint16_t time_iter = 0;
  float max_temperature_part = -__FLT_MAX__;
  float min_temperature_part = __FLT_MAX__;
  int16_t wind_speed_average = 0;
  float precipitation_average = 0.0;
  int16_t precipitation_probability_average = 0;
  std::vector<uint16_t> max_entry_weather_code;

  for (uint16_t i = 0; i < count_days; ++i, time_iter += count_hour_in_day) {
    for (uint16_t j = start_hour_night + i * count_hour_in_day;
         j < end_hour_night + i * count_hour_in_day; ++j) {
      precipitation_average += info["hourly"]["precipitation"][j].get<float>();
      precipitation_probability_average +=
          info["hourly"]["precipitation_probability"][j].get<int16_t>();
      wind_speed_average += info["hourly"]["wind_speed_10m"][j].get<int16_t>();
      max_entry_weather_code.push_back(
          info["hourly"]["weather_code"][j].get<uint16_t>());
      max_temperature_part =
          std::max(max_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
      min_temperature_part =
          std::min(min_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
    }

    city_forecast[city.name][i].night.precipitation =
        std::round(precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city.name][i].night.precipitation_probability =
        precipitation_probability_average / part_count_hour;
    city_forecast[city.name][i].night.wind_speed =
        wind_speed_average / part_count_hour;
    city_forecast[city.name][i].night.weather_code =
        GetWeatherCode(max_entry_weather_code);
    city_forecast[city.name][i].night.max_temperature =
        std::round(max_temperature_part * 100) / 100;
    city_forecast[city.name][i].night.min_temperature =
        std::round(min_temperature_part * 100) / 100;

    max_entry_weather_code.clear();
    max_temperature_part = -__FLT_MAX__;
    min_temperature_part = __FLT_MAX__;
    wind_speed_average = 0;
    precipitation_average = 0.0;
    precipitation_probability_average = 0;

    for (uint16_t j = start_hour_morning + i * count_hour_in_day;
         j < end_hour_morning + i * count_hour_in_day; ++j) {
      precipitation_average += info["hourly"]["precipitation"][j].get<float>();
      precipitation_probability_average +=
          info["hourly"]["precipitation_probability"][j].get<int16_t>();
      wind_speed_average += info["hourly"]["wind_speed_10m"][j].get<int16_t>();
      max_entry_weather_code.push_back(
          info["hourly"]["weather_code"][j].get<uint16_t>());
      max_temperature_part =
          std::max(max_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
      min_temperature_part =
          std::min(min_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
    }

    city_forecast[city.name][i].morning.precipitation =
        std::round(precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city.name][i].morning.precipitation_probability =
        precipitation_probability_average / part_count_hour;
    city_forecast[city.name][i].morning.wind_speed =
        wind_speed_average / part_count_hour;
    city_forecast[city.name][i].morning.weather_code =
        GetWeatherCode(max_entry_weather_code);
    city_forecast[city.name][i].morning.max_temperature =
        std::round(max_temperature_part * 100) / 100;
    city_forecast[city.name][i].morning.min_temperature =
        std::round(min_temperature_part * 100) / 100;

    max_entry_weather_code.clear();
    max_temperature_part = -__FLT_MAX__;
    min_temperature_part = __FLT_MAX__;
    wind_speed_average = 0;
    precipitation_average = 0.0;
    precipitation_probability_average = 0;

    for (uint16_t j = start_hour_day + i * count_hour_in_day;
         j < end_hour_day + i * count_hour_in_day; ++j) {
      precipitation_average += info["hourly"]["precipitation"][j].get<float>();
      precipitation_probability_average +=
          info["hourly"]["precipitation_probability"][j].get<int16_t>();
      wind_speed_average += info["hourly"]["wind_speed_10m"][j].get<int16_t>();
      max_entry_weather_code.push_back(
          info["hourly"]["weather_code"][j].get<uint16_t>());
      max_temperature_part =
          std::max(max_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
      min_temperature_part =
          std::min(min_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
    }

    city_forecast[city.name][i].day.precipitation =
        std::round(precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city.name][i].day.precipitation_probability =
        precipitation_probability_average / part_count_hour;
    city_forecast[city.name][i].day.wind_speed =
        wind_speed_average / part_count_hour;
    city_forecast[city.name][i].day.weather_code =
        GetWeatherCode(max_entry_weather_code);
    city_forecast[city.name][i].day.max_temperature =
        std::round(max_temperature_part * 100) / 100;
    city_forecast[city.name][i].day.min_temperature =
        std::round(min_temperature_part * 100) / 100;

    max_entry_weather_code.clear();
    max_temperature_part = -__FLT_MAX__;
    min_temperature_part = __FLT_MAX__;
    wind_speed_average = 0;
    precipitation_average = 0.0;
    precipitation_probability_average = 0;

    for (uint16_t j = start_hour_evening + i * count_hour_in_day;
         j < end_hour_evening + i * count_hour_in_day; ++j) {
      precipitation_average += info["hourly"]["precipitation"][j].get<float>();
      precipitation_probability_average +=
          info["hourly"]["precipitation_probability"][j].get<int16_t>();
      wind_speed_average += info["hourly"]["wind_speed_10m"][j].get<int16_t>();
      max_entry_weather_code.push_back(
          info["hourly"]["weather_code"][j].get<uint16_t>());
      max_temperature_part =
          std::max(max_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
      min_temperature_part =
          std::min(min_temperature_part,
                   info["hourly"]["temperature_2m"][j].get<float>());
    }

    city_forecast[city.name][i].evening.precipitation =
        std::round(precipitation_average / part_count_hour * 100) / 100;
    city_forecast[city.name][i].evening.precipitation_probability =
        precipitation_probability_average / part_count_hour;
    city_forecast[city.name][i].evening.wind_speed =
        wind_speed_average / part_count_hour;
    city_forecast[city.name][i].evening.weather_code =
        GetWeatherCode(max_entry_weather_code);
    city_forecast[city.name][i].evening.max_temperature =
        std::round(max_temperature_part * 100) / 100;
    city_forecast[city.name][i].evening.min_temperature =
        std::round(min_temperature_part * 100) / 100;

    city_forecast[city.name][i].date =
        info["hourly"]["time"][time_iter].get<std::string>().substr(
            0, len_format_date);
  }
}

void GetInfoForForecast(const std::string& directory) {
  json data_from_config = ReadConfig(directory);
  for (auto city : data_from_config["cities"]) {
    city_forecast[city] = std::vector<AllDayWetherInfo>(
        data_from_config["forecast_days"].get<int16_t>());
    geographical_info[city].name = city;
    GetCoords(geographical_info[city]);
    std::cout << geographical_info[city].name << ' '
              << geographical_info[city].latitude << ' '
              << geographical_info[city].longitude << '\n';
    json info = SendRequest(geographical_info[city], data_from_config);
    std::cout << info << '\n' << '\n';
    ParseRequest(geographical_info[city], info);
  }
}