#include "forecast.h"

void PrintError(const std::string& text) {
    std::cerr << text << '\n';
    exit(EXIT_FAILURE);
}

json ReadConfig(const std::string& directory){
    std::filesystem::path path = directory;
    std::ifstream input_config(path);
    if(input_config){
        json data = json::parse(input_config);
        return data;
    }
    PrintError("Not found config.json");
}

void GetCoords(CityInfo& city) { 
    std::string url = url_city_template + city.name;
    cpr::Response response = cpr::Get(cpr::Url(url), cpr::Header{{"X-Api-Key ", "49Y8Nc078fsxTIB8UgYZ2w==5yXdu1a2oKJcnwdb"}});
    if (response.status_code == 200){
        int16_t len = response.text.length();
        response.text = response.text.substr(1, len-2);
        json data = json::parse(response.text);
        city.latitude = data["latitude"];
        city.longitude = data["longitude"];
    }
    PrintError("Bad Gateway!"); 
}

void SendRequest(CityInfo& city){
    std::string url = url_forecast_template;
    cpr::Response response = cpr::Get(cpr::Url(url));
    if (response.status_code == 200){
        int16_t len = response.text.length();
        response.text = response.text.substr(1, len-2);
        json data = json::parse(response.text);
        std:: cout << data << '\n';
    }
    PrintError("Error!");

}
