#include "request_handler.h"
/**
 * Конструктор
 */
RequestHandler::RequestHandler(renderer::MapRenderer& renderer,
                               transport::Router& router) :
    renderer_(renderer),
    router_(router) { }
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
std::optional<std::vector<std::string>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    const transport::Stop* stop = db_.FindStop(stop_name);
    if (stop == nullptr) return std::nullopt;
    const transport::StopInfo* buses = db_.GetBusesByStop(stop);
    std::set<std::string> sorted_buses;
    for (transport::Bus* bus : *buses) {
        sorted_buses.emplace(bus->route);
    }
    return std::vector<std::string>(sorted_buses.begin(), sorted_buses.end());
}
/**
 * Отрисовка карты
 */
void RequestHandler::RenderMap(std::ostream& output) const {
    // подготавливаем необходимые данные
    const auto& sorted_buses = db_.GetBuses();
    const auto& sorted_stops = db_.GetStops();
    auto doc = renderer_.GetSVG();
    doc.Render(output);
}
/**
 * Получить информацию об оптимальном маршруте между остановками
 */
const std::optional<graph::Router<double>::RouteInfo>
RequestHandler::GetOptimalRoute(const std::string_view stop_from, const std::string_view stop_to) const {
    return router_.FindRoute(stop_from, stop_to);
}
/**
 * Граф
 */
const graph::DirectedWeightedGraph<double>& RequestHandler::GetRouterGraph() const {
    return router_.GetGraph();
}
/**
 * Обновить данные агрегированных объектов
 */
void RequestHandler::UpdateInternalData() {
    // подготавливаем необходимые данные
    const auto& sorted_buses = db_.GetBuses();
    const auto& sorted_stops = db_.GetStops();
    renderer_.SetBuses(sorted_buses).SetStops(sorted_stops);
    router_.BuildGraph(db_);
}
