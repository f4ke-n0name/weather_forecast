#include <lib/forecast.h>

Element design_part_of_day(AllDayWetherInfo& info, const uint8_t& index) {
  Element design = window(
      text(output_part_of_day[index]) | center | bold,
      vbox(text(std::to_string(info.parts_of_day[index].weather_code)),
           text(std::to_string(info.parts_of_day[index].min_temperature) +
                " ... " +
                std::to_string(info.parts_of_day[index].max_temperature) +
                " °C"),
           text(std::to_string(info.parts_of_day[index].wind_speed) + " km/h"),
           text(std::to_string(info.parts_of_day[index].precipitation) +
                " mm |" +
                std::to_string(
                    info.parts_of_day[index].precipitation_probability) +
                "%")) |
          flex);
  return design;
}

Element design_of_day(AllDayWetherInfo& info) {
  Element design =
      hbox(design_part_of_day(info, 0), design_part_of_day(info, 1),
           design_part_of_day(info, 2), design_part_of_day(info, 3));
  return design;
}

int main(int argc, char** argv) {
  auto screen = ScreenInteractive::TerminalOutput();
  uint16_t index = 0;
  std::map<std::string, std::vector<AllDayWetherInfo>> city_forecast =
      GetInfoForForecast(
          "/home/danil/github-classroom/is-itmo-c-23/labwork7-f4ke-n0name/"
          "build/"
          "config.json");
  Elements forecasts;
  std::string city = city_forecast.begin()->first;
  for (int i = 0; i < 3; ++i) {
    forecasts.push_back(design_of_day(city_forecast[city][i]));
  }
  auto renderer = Renderer([&] {

    std::cout << "Current city is " << city << ' ' << '('
              << geographical_info[city].latitude << " ; "
              << geographical_info[city].longitude << ')' << '\n';

    return window(text(city_forecast[city][index].date) | bold | center,
                  vbox(forecasts));
  });
  screen.Loop(renderer);
  return 0;
}