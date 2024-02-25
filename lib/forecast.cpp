#include "forecast.h"

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

void PrintError(const std::string_view& text) {
  std::cerr << text << '\n';
  exit(EXIT_FAILURE);
}

json ReadConfig(const std::string_view& directory) {
  std::filesystem::path path = directory;
  std::ifstream input_config(path);
  if (input_config) {
    json data = json::parse(input_config);
    api_ninjas_city_key = data["X-Api-Key"];
    return data;
  }
  PrintError("Not found config.json");
}

void GetCoords(CityInfo& city) {
  std::string url = kUrlCityTemplate;
  cpr::Response response =
      cpr::Get(cpr::Url(url), cpr::Parameters{{"name", city.name}},
               cpr::Header{{"X-Api-Key", api_ninjas_city_key}});
  if (response.status_code == 200) {
    response.text = response.text.substr(
        kToParseCoordsBegin, response.text.length() - kToParseCoordsEnd);
    json data = json::parse(response.text);
    city.latitude = data["latitude"];
    city.longitude = data["longitude"];
    return;
  }
  PrintError("Bad Gateway!");
}

json SendRequest(CityInfo& city, json& data, uint16_t& counter) {
  cpr::Response response =
      cpr::Get(cpr::Url(kUrlForecastTemplate),
               cpr::Parameters{{"longitude", std::to_string(city.longitude)},
                               {"latitude", std::to_string(city.latitude)},
                               {"forecast_days", std::to_string(counter)},
                               {"hourly", "weather_code"},
                               {"hourly", "temperature_2m"},
                               {"hourly", "wind_speed_10m"},
                               {"hourly", "precipitation"},
                               {"hourly", "precipitation_probability"}});
  if (response.status_code == 200) {
    json data = json::parse(response.text);
    return data;
  }
  PrintError("Unexpected error!");
  exit(EXIT_FAILURE);
}

uint16_t GetWeatherCode(std::vector<uint16_t>& max_entry_weather_code) {
  std::map<uint16_t, uint16_t> codes;
  for (size_t i = 0; i < max_entry_weather_code.size(); ++i) {
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

void SetWeatherInfoForPartDay(
    DayPartWeatherInfo& data, size_t& iteration, const uint8_t& part_of_day,
    std::string& city_name,
    std::map<std::string, std::vector<AllDayWetherInfo>>& city_forecast) {
  city_forecast[city_name][iteration].parts_of_day[part_of_day].precipitation =
      std::round(data.precipitation_average / kPartCountHour);
  city_forecast[city_name][iteration]
      .parts_of_day[part_of_day]
      .precipitation_probability =
      data.precipitation_probability_average / kPartCountHour;
  city_forecast[city_name][iteration].parts_of_day[part_of_day].wind_speed =
      data.wind_speed_average / kPartCountHour;
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
    DayPartWeatherInfo& data, const std::string_view& part_of_day, size_t& iteration,
    json& info, std::string& city_name,
    std::map<std::string, std::vector<AllDayWetherInfo>>& city_forecast) {
  for (size_t i = data.start_hour + iteration * kCountHourInDay;
       i < data.end_hour + iteration * kCountHourInDay; ++i) {
    data.precipitation_average +=
        info["hourly"]["precipitation"][i].get<float>();
    data.precipitation_probability_average +=
        info["hourly"]["precipitation_probability"][i].get<uint16_t>();
    data.wind_speed_average +=
        info["hourly"]["wind_speed_10m"][i].get<uint16_t>();
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
    SetWeatherInfoForPartDay(data, iteration, kIndexNight, city_name,
                             city_forecast);
  } else if (part_of_day == "morning") {
    SetWeatherInfoForPartDay(data, iteration, kIndexMorning, city_name,
                             city_forecast);
  } else if (part_of_day == "day") {
    SetWeatherInfoForPartDay(data, iteration, kIndexDay, city_name,
                             city_forecast);
  } else if (part_of_day == "evening") {
    SetWeatherInfoForPartDay(data, iteration, kIndexEvening, city_name,
                             city_forecast);
  }
}

void ParseRequest(
    CityInfo& city, json& info,
    std::map<std::string, std::vector<AllDayWetherInfo>>& city_forecast,
    uint16_t& counter) {
  uint16_t time_iter = 0;
  DayPartWeatherInfo data;
  for (size_t i = 0; i < counter; ++i, time_iter += kCountHourInDay) {
    data.start_hour = kStartHourNight;
    data.end_hour = kEndHourNight;
    ParseInfoPartOfDay(data, "night", i, info, city.name, city_forecast);
    data = DayPartWeatherInfo();
    data.start_hour = kStartHourMorning;
    data.end_hour = kEndHourMorning;
    ParseInfoPartOfDay(data, "morning", i, info, city.name, city_forecast);
    data = DayPartWeatherInfo();
    data.start_hour = kStartHourDay;
    data.end_hour = kEndHourDay;
    ParseInfoPartOfDay(data, "day", i, info, city.name, city_forecast);
    data = DayPartWeatherInfo();
    data.start_hour = kStartHourEvening;
    data.end_hour = kEndHourEvening;
    ParseInfoPartOfDay(data, "evening", i, info, city.name, city_forecast);
    city_forecast[city.name][i].date =
        info["hourly"]["time"][time_iter].get<std::string>().substr(
            0, kLenFormatDate);
  }
}

std::map<std::string, std::vector<AllDayWetherInfo>> GetInfoForForecast(
    json& data_from_config, uint16_t& counter) {
  std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast;
  for (auto city : data_from_config["cities"]) {
    city_forecast[city] = std::vector<AllDayWetherInfo>(counter);
    geographical_info[city].name = city;
    GetCoords(geographical_info[city]);
    json info = SendRequest(geographical_info[city], data_from_config, counter);
    ParseRequest(geographical_info[city], info, city_forecast, counter);
  }
  return city_forecast;
}

std::string round_double_to_string(std::string number, uint16_t count) {
  std::string result;
  for (size_t i = 0; i < number.size(); ++i) {
    if (number[i] != '.') {
      result.push_back(number[i]);
    } else {
      for (size_t j = i; j < i + count + 1; ++j) {
        result.push_back(number[j]);
      }
      break;
    }
  }
  return result;
}

Element design_part_of_day(AllDayWetherInfo& info, const uint8_t& index,
                           json& data) {
  std::string weather_code =
      data["weather_type"]
          [std::to_string(info.parts_of_day[index].weather_code)]
              .get<std::string>();
  Elements weather_design;
  for (auto& element :
       data["weather_design"][weather_code].get<std::vector<std::string>>()) {
    weather_design.push_back(text(element));
  }
  Element design =
      window(
          text(output_part_of_day[index]) | center | bold,
          hbox(
              vbox(weather_design) | flex | center,
              vbox(
                  text(weather_code) | center,
                  text(round_double_to_string(
                           std::to_string(
                               info.parts_of_day[index].min_temperature),
                           1) +
                       " ... " +
                       round_double_to_string(
                           std::to_string(
                               info.parts_of_day[index].max_temperature),
                           1) +
                       " °C"),
                  text(round_double_to_string(
                           std::to_string(info.parts_of_day[index].wind_speed),
                           1) +
                       " km/h"),
                  text(round_double_to_string(
                           std::to_string(
                               info.parts_of_day[index].precipitation),
                           1) +
                       " mm |" +
                       std::to_string(
                           info.parts_of_day[index].precipitation_probability) +
                       "%")) |
                  flex | center)) |
      flex;
  return design;
}

Element design_of_day(AllDayWetherInfo& info, json& data) {
  Element design = hbox(design_part_of_day(info, kIndexNight, data),
                        design_part_of_day(info, kIndexMorning, data),
                        design_part_of_day(info, kIndexDay, data),
                        design_part_of_day(info, kIndexEvening, data));
  return window(text(info.date) | center | bold, design);
}

void DrawForecast(const std::string& directory) {
  auto screen = ScreenInteractive::TerminalOutput();
  json data_from_config = ReadConfig(directory);
  uint16_t counter = data_from_config["forecast_days"];
  uint16_t counter_max = counter;
  uint16_t update_seconds = data_from_config["update_seconds"];
  std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast =
      GetInfoForForecast(data_from_config, counter);
  std::vector<std::string> cities;
  for (auto element : city_forecast) {
    cities.push_back(element.first);
  }
  uint16_t index = 0;
  std::string city = cities[index];
  auto renderer = Renderer([&] {
    Elements forecasts;
    for (size_t i = 0; i < counter; ++i) {
      forecasts.push_back(
          design_of_day(city_forecast[city][i], data_from_config));
    }
    return window(text(city) | bold | center, vbox(forecasts));
  });

  Component events = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Escape) {
      screen.Exit();
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('+')) {
      if (counter <= kMaxDaysInMonth) {
        ++counter;
      }
      if (counter > counter_max) {
        counter_max = counter;
        city_forecast = GetInfoForForecast(data_from_config, counter);
      }
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('-')) {
      if (counter >= kMinDaysInMonth) {
        --counter;
      }
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('n')) {
      index = (index == cities.size() - 1 ? 0 : ++index);
      city = cities[index];
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('p')) {
      index = (index == 0 ? cities.size() - 1 : --index);
      city = cities[index];
      screen.PostEvent(Event::Custom);
      return true;
    }
    return false;
  });

  std::thread update([&] {
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(update_seconds));
      city_forecast = GetInfoForForecast(data_from_config, counter);
      screen.Post(Event::Custom);
    }
  });
  screen.Loop(events);
  update.detach();
}