#include "transport_catalogue.h"
#include <unordered_set>
#include <stdexcept>
namespace catalogue {
/**
 * Добавить маршрут в статистику остановки
 */
void StopInfo::AddRouteNumber(std::string_view bus) {
    if(buses_.find(bus) != buses_.end()) return;
    buses_.insert(bus);
}
/**
 * Проходящие через остановку маршруты
 */
std::set<std::string_view> StopInfo::GetRouteNumbers() const {
    std::set<std::string_view> buses;
    for (auto bus : buses_) {
        buses.insert(bus);
    }
    return buses;
}
/**
 * Добавить остановку в каталог
 */
void TransportCatalogue::AddStop(std::string_view stop_name, const geo::Coordinates &coordinates) {
    if (stopname_to_stop_.count(stop_name)) return;
    stops_.push_back({ std::string(stop_name), coordinates });
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}
/**
 * Добавить маршрут в каталог
 */
void TransportCatalogue::AddRoute(std::string_view bus_number,
                                  const std::vector<const Stop*>& stops) {
    if (busname_to_bus_.count(bus_number)) return;
    buses_.push_back({ std::string(bus_number), stops });
    busname_to_bus_[buses_.back().route] = &buses_.back();
    // добавляем маршрут в набор каждой остановки, через которую он проходит
    for (auto stop : stops) {
        stop_to_info_[stop].AddRouteNumber(buses_.back().route);
    }
}
/**
 * Добавить маршрут в каталог
 * Версия с вектором имен остановок
 */
void TransportCatalogue::AddRoute(std::string_view bus_number,
              const std::vector<std::string_view>& stop_names) {
    if (bus_number.empty() || stop_names.empty()) return;
    std::vector<const Stop*> route_stops(stop_names.size());
    for (size_t i = 0; i < stop_names.size(); ++i) {
        const Stop* stop = FindStop(stop_names[i]);
        if(stop == nullptr) continue;
        route_stops[i] = stop;
    }
    AddRoute(bus_number, std::move(route_stops));
}
/**
 * Найти маршрут по его номеру
 */
const Bus* TransportCatalogue::FindRoute(std::string_view bus_number) const {
    auto bus_it = busname_to_bus_.find(bus_number);
    return bus_it != busname_to_bus_.end() ? bus_it->second : nullptr;
}
/**
 * Найти остановку по наименованию
 */
const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
    auto stop_it = stopname_to_stop_.find(stop_name);
    return stop_it != stopname_to_stop_.end() ? stop_it->second : nullptr;
}
/**
 * Статистика по автобусу
 */
BusInfo* TransportCatalogue::GetBusInfo(std::string_view bus_number) const {
    BusInfo* bus_stat = nullptr;
    const Bus* bus = FindRoute(bus_number);
    if(bus == nullptr) return bus_stat;
    bus_stat = new BusInfo;
    bus_stat->stops_count = bus->stops.size();
    std::unordered_set<std::string_view> unique_stops;
    double geo_length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        const auto from = bus->stops[i];
        const auto to = bus->stops[i + 1];
        const double geo_distance = ComputeDistance(from->coordinates, to->coordinates);
        auto route_distance = GetDistance(from, to);
        bus_stat->route_length += route_distance != 0 ? route_distance : geo_distance;
        geo_length += geo_distance;
        unique_stops.insert(from->name);
        unique_stops.insert(to->name);
    }
    bus_stat->unique_stops_count = unique_stops.size();
    bus_stat->curvature = bus_stat->route_length / geo_length;
    return bus_stat;
}
/**
 * Статистика по остановке
 */
const StopInfo* TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    const Stop* stop = FindStop(stop_name);
    if (stop == nullptr) return nullptr;
    const auto stop_it = stop_to_info_.find(stop);
    if (stop_it == stop_to_info_.end()) {
        return new StopInfo;
    }
    return &stop_it->second;
}
/**
 * Установить расстояние между двумя остановками
 */
void TransportCatalogue::SetDistance(const Stop* from,
                                     const Stop* to,
                                     int distance) {
    if (from == nullptr || to == nullptr) return;
    distances_[{from, to}] = distance;
}
/**
 * Получить расстояние между двумя остановками
 */
int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
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
