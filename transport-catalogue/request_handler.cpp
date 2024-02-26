#include "request_handler.h"
/**
 * Конструктор
 */
RequestHandler::RequestHandler(const renderer::MapRenderer &renderer) :
    renderer_(renderer) { }
/**
 * Добавить остановку в каталог
 */
void RequestHandler::AddStop(std::string_view stop_name, const geo::Coordinates &coordinates) {
    if (db_.FindStop(stop_name) != nullptr) return;
    db_.AddStop({std::string(stop_name), coordinates});
}
/**
 * Добавить маршрут в каталог.
 */
void RequestHandler::AddRoute(std::string_view bus_number,
              const std::vector<std::string_view>& stop_names,
              bool is_roundtrip) {
    if (bus_number.empty() || stop_names.empty()) return;
    std::vector<const transport::Stop*> route_stops(stop_names.size());
    for (size_t i = 0; i < stop_names.size(); ++i) {
        const transport::Stop* stop = db_.FindStop(stop_names[i]);
        if(stop == nullptr) continue;
        route_stops[i] = stop;
    }
    db_.AddRoute(bus_number, std::move(route_stops), is_roundtrip);
}
/**
 * Установить расстояние между двумя остановками
 */
void RequestHandler::SetDistance(std::string_view from,
                                 std::string_view to,
                                 int distance) {
    auto stop_from = db_.FindStop(from);
    auto stop_to = db_.FindStop(to);
    if (stop_from && stop_to) {
        db_.SetDistance(stop_from, stop_to, distance);
    }
}
/**
 * Возвращает информацию о маршруте (запрос Bus)
 */
std::optional<transport::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    const transport::Bus* bus = db_.FindRoute(bus_name);
    if(bus == nullptr) return std::nullopt;
    transport::BusInfo bus_stat;
    bus_stat.stops_count = bus->stops.size();
    std::unordered_set<std::string_view> unique_stops;
    double geo_length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        const auto from = bus->stops[i];
        const auto to = bus->stops[i + 1];
        const double geo_distance = ComputeDistance(from->coordinates, to->coordinates);
        auto route_distance = db_.GetDistance(from, to);
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
 * Возвращает маршруты, проходящие через остановку
 */
const std::unordered_set<transport::Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    const transport::Stop* stop = db_.FindStop(stop_name);
    if (stop == nullptr) return nullptr;
    return db_.GetBusesByStop(stop);
}
/**
 * Отрисовка карты
 */
void RequestHandler::RenderMap(std::ostream& output) const {
    // подготавливаем необходимые данные
    std::vector<geo::Coordinates> route_stops_coord;
    std::set<std::string_view> stops;
    const auto& sorted_buses = db_.GetSortedBuses();
    // формируем вектор координат остановок маршрута
    for (const auto& bus : sorted_buses) {
        if (bus->stops.empty()) continue;
        for (const auto& stop : bus->stops) {
            if (stops.count(stop->name) > 0) continue;
            route_stops_coord.push_back(stop->coordinates);
            // добавляем в контейнер для сортировки
            stops.insert(stop->name);
        }
    }
    // извлекаем указатели на остановки после сортировки
    std::vector<const transport::Stop*> sorted_stops;
    std::transform(stops.begin(), stops.end(), std::back_inserter(sorted_stops),
            [&](const auto &stop_name){
                return db_.FindStop(stop_name);
            });
    auto doc = renderer_.GetSVG(sorted_buses, sorted_stops, route_stops_coord);
    doc.Render(output);
}
