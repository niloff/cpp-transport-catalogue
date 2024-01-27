#include <iomanip>
#include "stat_reader.h"
namespace reader::stat {
/**
 * Наименование запроса - остановка
 */
const char* QueryDescription::QUERY_NAME_STOP = "Stop";
/**
 * Наименование запроса - маршрут
 */
const char* QueryDescription::QUERY_NAME_BUS = "Bus";
/**
 * Парсинг команды-запроса из строки
 */
QueryDescription ParseQuery(std::string_view line) {
    auto space_pos = line.find(' ');
    if (space_pos == line.npos) {
        return {};
    }
    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space == line.npos) {
        return {};
    }
    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space))};
}

void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    QueryDescription query_desc = ParseQuery(request);
    if (!query_desc) return;
    if (query_desc.query == QueryDescription::QUERY_NAME_BUS) {
        PrintRoute(query_desc.id, transport_catalogue, output);
        return;
    }
    if (query_desc.query == QueryDescription::QUERY_NAME_STOP) {
        PrintStop(query_desc.id, transport_catalogue, output);
    }
}

void PrintRoute(const std::string& bus_number, const catalogue::TransportCatalogue& catalogue, std::ostream& out) {
    using namespace catalogue;
    BusInfo* stat = catalogue.GetBusInfo(bus_number);
    if(stat == nullptr) {
        out << "Bus " << bus_number << ": not found\n";
        return;
    }
    out << "Bus " << bus_number << ": " << stat->stops_count << " stops on route, "
        << stat->unique_stops_count << " unique stops, " << std::setprecision(6)
        << stat->route_length << " route length\n";
}

void PrintStop(const std::string& stop_name, const catalogue::TransportCatalogue& catalogue, std::ostream& out)  {
    using namespace catalogue;
    const StopInfo* stat = catalogue.GetStopInfo(stop_name);
    if (stat == nullptr) {
        out << "Stop " << stop_name << ": not found\n";
        return;
    }
    const auto& route_numbers = stat->GetRouteNumbers();
    if (route_numbers.empty()) {
        out << "Stop " << stop_name << ": no buses\n";
        return;
    }
    out << "Stop " << stop_name << ": buses";
    for (const auto& number : route_numbers) {
        out << " " << number;
    }
    out << "\n";
}
}
