#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <stdexcept>
#include "json.hpp" // nlohmann/json, один заголовочный файл

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std::chrono_literals;
using json = nlohmann::json;

enum TrafficLightsState { RED, YELLOW, GREEN };

struct Coordinates
{
    double latitude;
    double longitude;
};

struct Address
{
    std::string country;
    std::string city;
    std::string street;
    Coordinates coordinates;
};

class TrafficLight
{
private:
    Address address;

public:
    TrafficLight(const Address& addr) : address(addr) {}

    /// Создаёт светофор из json-файла
    static TrafficLight LoadFromFile(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Не удалось открыть файл: " + path);
        }

        json data = json::parse(file);

        Address addr;
        addr.country = data.at("country").get<std::string>();
        addr.city = data.at("city").get<std::string>();
        addr.street = data.at("street").get<std::string>();
        addr.coordinates.latitude = data.at("latitude").get<double>();
        addr.coordinates.longitude = data.at("longitude").get<double>();

        return TrafficLight(addr);
    }

    TrafficLightsState GetCurrentState() const
    {
        auto now = std::chrono::system_clock::now();
        auto now_sec = std::chrono::floor<std::chrono::seconds>(now).time_since_epoch();
        int seconds = static_cast<int>((now_sec % 60s).count());

        if (seconds >= 0 && seconds <= 5) return TrafficLightsState::YELLOW;
        if (seconds >= 6 && seconds <= 35) return TrafficLightsState::GREEN;
        return TrafficLightsState::RED;
    }

    void GetInfo() const
    {
        std::cout << "Страна: " << address.country << "\n"
                  << "Город: " << address.city << "\n"
                  << "Улица: " << address.street << "\n"
                  << "Координаты: " << address.coordinates.latitude << ", "
                  << address.coordinates.longitude << "\n"
                  << "Сейчас горит: ";

        switch (GetCurrentState())
        {
        case TrafficLightsState::RED:    std::cout << "\033[31mКРАСНЫЙ\033[0m"; break;
        case TrafficLightsState::YELLOW: std::cout << "\033[33mЖЁЛТЫЙ\033[0m";  break;
        case TrafficLightsState::GREEN:  std::cout << "\033[32mЗЕЛЁНЫЙ\033[0m"; break;
        }
        std::cout << "\n------------------------\n";
    }
};

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    const std::vector<std::string> files = { "light1.json", "light2.json", "light3.json" };
    std::vector<TrafficLight> lights;

    for (const auto& path : files)
    {
        try
        {
            lights.push_back(TrafficLight::LoadFromFile(path));
        }
        catch (const std::exception& e)
        {
            std::cout << "Ошибка при чтении " << path << ": " << e.what() << "\n";
        }
    }

    if (lights.empty())
    {
        std::cout << "Ни один светофор не загружен.\n";
        return 1;
    }

    // Мониторинг светофоров
    while (true)
    {
        for (const auto& light : lights)
        {
            light.GetInfo();
        }
        std::cout << "==========================\n";
        std::this_thread::sleep_for(23000ms);
    }
    return 0;
}
