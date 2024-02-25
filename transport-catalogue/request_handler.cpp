#include "request_handler.h"
/**
 * Конструктор
 */
RequestHandler::RequestHandler(const transport::Catalogue& db, const renderer::MapRenderer &renderer) :
    db_(db),
    renderer_(renderer) { }
/**
 * Возвращает информацию о маршруте (запрос Bus)
 */
std::optional<transport::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.GetBusInfo(bus_name);
}
/**
 * Возвращает маршруты, проходящие через остановку
 */
const std::unordered_set<transport::Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetBusesByStop(stop_name);
}
/**
 * Отрисовка карты
 */
svg::Document RequestHandler::RenderMap() const {
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
    return renderer_.GetSVG(sorted_buses, sorted_stops, route_stops_coord);
}
