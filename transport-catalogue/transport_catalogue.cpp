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
void Catalogue::AddStop(const Stop& stop) {
    if (stopname_to_stop_.count(stop.name)) return;
    stops_.push_back(stop);
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
 * Статистика по остановке
 */
const transport::StopInfo* Catalogue::GetBusesByStop(const Stop* stop) const {
    const auto stop_it = stop_to_buses_.find(stop);
    if (stop_it == stop_to_buses_.end()) {
        return new StopInfo;
    }
    return &stop_it->second;
}
/**
 * Массив из отсортированных по номерам маршрутов.
 * Выводяться только непустые маршруты (с остановками)
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
 * Массив из отсортированных по наименованию остановок.
 * Выводятся только остановки, через которые проходит как минимум один маршрут.
 */
const std::vector<const Stop*> Catalogue::GetSortedStops() const {
    std::map<std::string_view, const Stop*> sorted_stops(stopname_to_stop_.begin(),
                                                        stopname_to_stop_.end());
    std::vector<const Stop*> stops;
    stops.reserve(sorted_stops.size());
    for (const auto& [stopname, stop] : sorted_stops) {
        auto it = stop_to_buses_.find(stop);
        if (it == stop_to_buses_.end() || it->second.empty()) continue;
        stops.push_back(stop);
    }
    return stops;
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
}
