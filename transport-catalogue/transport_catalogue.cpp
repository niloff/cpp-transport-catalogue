#include <unordered_set>
#include <stdexcept>
#include <algorithm>
#include "transport_catalogue.h"
/**
 * Справочник
 */
namespace transport {
/**
 * Добавить остановку в каталог
 */
void Catalogue::AddStop(std::string_view stop_name, const geo::Coordinates &coordinates) {
    if (stopname_to_stop_.count(stop_name)) return;
    stops_.push_back({ std::string(stop_name), coordinates });
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}
/**
 * Добавить маршрут в каталог
 */
void Catalogue::AddRoute(std::string_view bus_number,
                         const std::vector<const transport::Stop*> &stops,
                         bool is_roundtrip) {
    if (busname_to_bus_.count(bus_number)) return;
    buses_.push_back({ std::string(bus_number), stops, is_roundtrip});
    Bus* bus = &buses_.back();
    busname_to_bus_[buses_.back().route] = bus;
    // добавляем маршрут в набор каждой остановки, через которую он проходит
    for (auto stop : stops) {
        StopInfo& buses_by_stop = stop_to_buses_[stop];
        if (buses_by_stop.count(bus) != 0) continue;
        buses_by_stop.insert(bus);
    }
}
/**
 * Добавить маршрут в каталог
 * Версия с вектором имен остановок
 */
void Catalogue::AddRoute(std::string_view bus_number,
                         const std::vector<std::string_view>& stop_names,
                         bool is_roundtrip) {
    using namespace transport;
    if (bus_number.empty() || stop_names.empty()) return;
    std::vector<const Stop*> route_stops(stop_names.size());
    for (size_t i = 0; i < stop_names.size(); ++i) {
        const Stop* stop = FindStop(stop_names[i]);
        if(stop == nullptr) continue;
        route_stops[i] = stop;
    }
    AddRoute(bus_number, std::move(route_stops), is_roundtrip);
}
/**
 * Найти маршрут по его номеру
 */
const transport::Bus *Catalogue::FindRoute(std::string_view bus_number) const {
    auto bus_it = busname_to_bus_.find(bus_number);
    return bus_it != busname_to_bus_.end() ? bus_it->second : nullptr;
}
/**
 * Найти остановку по наименованию
 */
const transport::Stop* Catalogue::FindStop(std::string_view stop_name) const {
    auto stop_it = stopname_to_stop_.find(stop_name);
    return stop_it != stopname_to_stop_.end() ? stop_it->second : nullptr;
}
/**
 * Статистика по автобусу
 */
std::optional<transport::BusInfo> Catalogue::GetBusInfo(std::string_view bus_number) const {
    using namespace transport;
    const Bus* bus = FindRoute(bus_number);
    if(bus == nullptr) return std::nullopt;
    BusInfo bus_stat;
    bus_stat.stops_count = bus->stops.size();
    std::unordered_set<std::string_view> unique_stops;
    double geo_length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        const auto from = bus->stops[i];
        const auto to = bus->stops[i + 1];
        const double geo_distance = ComputeDistance(from->coordinates, to->coordinates);
        auto route_distance = GetDistance(from, to);
        bus_stat.route_length += route_distance != 0 ? route_distance : geo_distance;
        geo_length += geo_distance;
        unique_stops.insert(from->name);
        unique_stops.insert(to->name);
    }
    bus_stat.unique_stops_count = unique_stops.size();
    bus_stat.curvature = bus_stat.route_length / geo_length;
    return bus_stat;
}
/**
 * Статистика по остановке
 */
const transport::StopInfo* Catalogue::GetBusesByStop(std::string_view stop_name) const {
    const Stop* stop = FindStop(stop_name);
    if (stop == nullptr) return nullptr;
    const auto stop_it = stop_to_buses_.find(stop);
    if (stop_it == stop_to_buses_.end()) {
        return new StopInfo;
    }
    return &stop_it->second;
}
/**
 * Получить каталог из отсортированных по номерам маршрутов.
 * Выводяться только непустые маршруты (с остановками).
 */
const std::vector<const Bus*> Catalogue::GetSortedBuses() const {
    std::map<std::string_view, const Bus*> sorted_buses(busname_to_bus_.begin(),
                                                        busname_to_bus_.end());
    std::vector<const Bus*> routes;
    routes.reserve(sorted_buses.size());
    for (const auto& [busname, bus] : sorted_buses) {
        if (bus->stops.size() == 0) continue;
        routes.push_back(bus);
    }
    return routes;
}
/**
 * Установить расстояние между двумя остановками
 */
void Catalogue::SetDistance(std::string_view from,
                            std::string_view to,
                            int distance) {
    auto stop_from = FindStop(from);
    auto stop_to = FindStop(to);
    if (stop_from && stop_to) {
        SetDistance(stop_from, stop_to, distance);
    }
}
/**
 * Получить расстояние между двумя остановками
 */
int Catalogue::GetDistance(const transport::Stop *from, const transport::Stop *to) const {
    auto it = distances_.find({from, to});
    if (it != distances_.end()) {
        return it->second;
    }
    it = distances_.find({to, from});
    if (it != distances_.end()) {
        return it->second;
    }
    else return 0;
}
/**
 * Установить расстояние между двумя остановками
 */
void Catalogue::SetDistance(const transport::Stop *from,
                            const transport::Stop *to,
                            int distance) {
    if (from == nullptr || to == nullptr) return;
    distances_[{from, to}] = distance;
}
}
