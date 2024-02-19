#include <lib/forecast.h>

int main(int argc, char** argv) {
  GetInfoForForecast(
      "/home/danil/github-classroom/is-itmo-c-23/labwork7-f4ke-n0name/build/"
      "config.json");
        // Define the document
  Element data = hbox({
      text("night")   | border | flex,
      text("morning") | border | flex,
      text("day")  | border | flex,
      text("evening") | border | flex
  });
  Element document =
  window(text( "Ufa" ) | center | bold ,  data);
 
  auto screen = ScreenInteractive::Create(
    Dimension::Full(),       // Width
    Dimension::Fit(document) // Height
  );

  Render(screen, document);
  screen.Print();
  return 0;
}