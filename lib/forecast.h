#include <cpr/cpr.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

const std::string url_city_template = "https://api.api-ninjas.com/v1/city?name=";
const std::string url_forecast_template = "https://api.open-meteo.com/v1/forecast?latitude=59.94&longitude=30.31&hourly=temperature_2m&forecast_days=16";

struct CityInfo {
  std::string name;
  double longitude;
  double latitude;
};

void PrintError(const std::string& text );

void GetCoords (CityInfo & city);