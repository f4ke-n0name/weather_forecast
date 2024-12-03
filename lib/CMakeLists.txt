add_library(forecast forecast.h forecast.cpp)

target_link_libraries(forecast PUBLIC cpr::cpr)

target_link_libraries(forecast PUBLIC nlohmann_json::nlohmann_json)

target_link_libraries(forecast
  PUBLIC ftxui::screen
  PUBLIC ftxui::dom
  PUBLIC ftxui::component
)
