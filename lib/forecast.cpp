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
    return data;
  }
  PrintError("Not found config.json");
}

void GetCoords(CityInfo& city) {
  std::string url = url_city_template + city.name;
  cpr::Response response = cpr::Get(
      cpr::Url(url),
      cpr::Header{{"X-Api-Key", "49Y8Nc078fsxTIB8UgYZ2w==5yXdu1a2oKJcnwdb"}});
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

json SendRequest(CityInfo& city) {
  std::string url = url_forecast_template + std::to_string(city.latitude) +
                    "&longitude=" + std::to_string(city.longitude) +
                    "&hourly=temperature_2m&forecast_days=3";
  cpr::Response response = cpr::Get(cpr::Url(url));
  if (response.status_code == 200) {
    json data = json::parse(response.text);
    return data;
  }
  PrintError("Error!");
}

void GetInfoForForecast(const std::string& directory) {
  json data_from_config = ReadConfig(directory);
  for (auto element : data_from_config["cities"]) {
    city_forecast[element] =
        std::vector<AllDayWetherInfo>(data_from_config["count_days"]);
    geographical_info[element].name = element;
    GetCoords(geographical_info[element]);
    std::cout << geographical_info[element].name << ' '
              << geographical_info[element].latitude << ' '
              << geographical_info[element].longitude << '\n';
  }
}