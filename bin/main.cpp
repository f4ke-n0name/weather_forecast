#include <lib/forecast.h>

std::string round_double_to_string(std::string number, int count) {
  std::string result;
  for (int i = 0; i < number.size(); i++) {
    if (number[i] != '.') {
      result.push_back(number[i]);
    } else {
      for (int j = i; j < i + count + 1; j++) {
        result.push_back(number[j]);
      }
      break;
    }
  }
  return result;
}

Element design_part_of_day(AllDayWetherInfo& info, const uint8_t& index,
                           json& data) {
  Element design = window(
      text(output_part_of_day[index]) | center | bold,
      vbox(
          text(data["weather_type"]
                   [std::to_string(info.parts_of_day[index].weather_code)]
                       .get<std::string>()),
          text(
              round_double_to_string(
                  std::to_string(info.parts_of_day[index].min_temperature), 1) +
              " ... " +
              round_double_to_string(
                  std::to_string(info.parts_of_day[index].max_temperature), 1) +
              " °C"),
          text(round_double_to_string(
                   std::to_string(info.parts_of_day[index].wind_speed), 1) +
               " km/h"),
          text(round_double_to_string(
                   std::to_string(info.parts_of_day[index].precipitation), 1) +
               " mm |" +
               std::to_string(
                   info.parts_of_day[index].precipitation_probability) +
               "%")) |
          flex);
  return design;
}

Element design_of_day(AllDayWetherInfo& info, json& data) {
  Element design = hbox(
      design_part_of_day(info, 0, data), design_part_of_day(info, 1, data),
      design_part_of_day(info, 2, data), design_part_of_day(info, 3, data));
  return window(text(info.date) | center | bold, design);
}

int main(int argc, char** argv) {
  auto screen = ScreenInteractive::TerminalOutput();
  json data_from_config = ReadConfig(
      "/home/danil/github-classroom/is-itmo-c-23/labwork7-f4ke-n0name/build/"
      "config.json");
  uint16_t counter = count_days;
  uint16_t counter_max = counter;
  uint16_t update_seconds = update_time;
  std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast =
      GetInfoForForecast(data_from_config, counter);
  auto current_city = city_forecast.begin();
  std::string city = current_city->first;
  auto renderer = Renderer([&] {
    Elements forecasts;
    for (int i = 0; i < counter; ++i) {
      forecasts.push_back(
          design_of_day(city_forecast[city][i], data_from_config));
    }
    return window(text(city) | bold | center, vbox(forecasts));
  });

  Component events = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Escape) {
      screen.ExitLoopClosure();
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('+')) {
      if (counter <= 30) {
        ++counter;
      }
      if (counter > counter_max) {
        counter_max = counter;
        city_forecast = GetInfoForForecast(data_from_config, counter);
      }
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('-')) {
      if (counter >= 1) {
        --counter;
      }
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('n')) {
      current_city = (current_city == (city_forecast.end()))
                         ? city_forecast.begin()
                         : ++current_city;
      city = current_city->first;
      screen.PostEvent(Event::Custom);
      return true;
    } else if (event == Event::Character('p')) {
      current_city = (current_city == city_forecast.begin())
                         ? city_forecast.end()
                         : --current_city;
      city = current_city->first;
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
  return 0;
}