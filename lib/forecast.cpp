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

void ParseRequest(CityInfo& city, json& info) {
  uint16_t time_iter = 0;
  for (uint16_t i = 0; i < count_days; ++i, time_iter += 24) {
    for (uint16_t j = 0; j < 6; ++j) {
      
    }
    for (uint16_t j = 6; j < 12; ++j) {
    }
    for (uint16_t j = 12; j < 18; ++j) {
    }
    for (uint16_t j = 18; j < 24; ++j) {
    }
    city_forecast[city.name][i].date = std::string format =
        info["hourly"]["time"][time_iter].substr(0, 10);
  }
}

void GetInfoForForecast(const std::string& directory) {
  json data_from_config = ReadConfig(directory);
  for (auto city : data_from_config["cities"]) {
    city_forecast[city] =
        std::vector<AllDayWetherInfo>(data_from_config["forecast_days"]);
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