#include "input_reader.h"
#include <algorithm>
#include <cassert>
#include <iterator>

namespace reader {

namespace utils {
/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}
}

namespace input {
/**
 * Наименование команды - остановка
 */
const char* CommandDescription::COMMAND_NAME_STOP = "Stop";
/**
 * Наименование команды - маршрут
 */
const char* CommandDescription::COMMAND_NAME_BUS = "Bus";
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}
/**
 * Парсит строку вида "55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"
 * и возвращает вектор с расстояниями до остановок
 */
std::vector<DistanceDescription> ParseDistances(std::string_view str) {
    std::vector<DistanceDescription> distances;
    for (auto delim = str.find("m to "); delim != str.npos; delim = str.find("m to ", delim)) {
        // парсим расстояние
        auto comma = str.rfind(',', delim);
        auto pos_start = str.find_first_not_of(' ', comma + 1);
        int distance = std::stoi(std::string(str.substr(pos_start, delim - pos_start)));
        // парсим название остановки
        delim += 5;
        comma = str.find_first_of(',', delim);
        auto pos_end = comma != str.npos ? comma - delim : str.npos;
        auto stop_name = utils::Trim(str.substr(delim, pos_end));
        distances.push_back({std::string(stop_name), distance});
    }
    return distances;
}
/**
 * Парсинг команды-запроса из строки
 */
CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}
/**
 * Чтение данных из потока
 */
void InputReader::ReadInput(std::istream& input) {
    int base_request_count;
    input >> base_request_count >> std::ws;
    if (base_request_count <= 0) return;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseLine(line);
    }
}
/**
 * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
 */
void InputReader::ParseLine(std::string_view line) {
    const auto& command_description = ParseCommandDescription(line);
    if (!command_description) return;
    commands_.push_back(std::move(command_description));
    name_to_commands_[commands_.back().command].push_back(&commands_.back());
}
/**
 * Наполняет данными транспортный справочник, используя команды из commands_
 */
void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue &catalogue) const {
    // разбираем остановки
    auto it = name_to_commands_.find(CommandDescription::COMMAND_NAME_STOP);
    if (it != name_to_commands_.end()) {
        const auto& stops_commands = it->second;
        // добавляем остановки
        for(const auto& command : stops_commands) {
            const geo::Coordinates& coordinates = ParseCoordinates(command->description);
            catalogue.AddStop(command->id, coordinates);
        }
        //добавляем расстояния между остановками
        for(const auto& command : stops_commands) {
            const auto& distances = ParseDistances(command->description);
            if (distances.empty()) continue;
            const auto from = catalogue.FindStop(command->id);
            for (auto distance : distances) {
                const auto to = catalogue.FindStop(distance.stop_name);
                if (to == nullptr) continue;
                catalogue.SetDistance(from, to, distance.distance);
            }
        }
    }
    // разбираем маршруты
    it = name_to_commands_.find(CommandDescription::COMMAND_NAME_BUS);
    if (it != name_to_commands_.end()) {
        const auto& buses_commands = it->second;
        for(const auto& command : buses_commands) {
            const auto& stops = utils::ParseRoute(command->description);
            catalogue.AddRoute(command->id, stops);
        }
    }
}
}
}
