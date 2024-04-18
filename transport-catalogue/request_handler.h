#pragma once
#include <optional>
#include <string>
#include <unordered_set>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
/**
 * Класс-фасад, упрощающий взаимодействие JSON reader-а
 * с другими подсистемами приложения.
 */
class RequestHandler {
public:
    /**
     * Конструктор
     */
    RequestHandler(renderer::MapRenderer& renderer,
                   transport::Router& router);
    /**
     * Добавить остановку в каталог
     */
    void AddStop(std::string_view stop_name, const geo::Coordinates &coordinates);
    /**
     * Добавить маршрут в каталог
     */
    void AddRoute(std::string_view bus_number,
                  const std::vector<std::string_view>& stop_names,
                  bool is_roundtrip);
    /**
     * Установить расстояние между двумя остановками
     */
    void SetDistance(std::string_view from,
                     std::string_view to,
                     int distance);
    /**
     * Возвращает информацию о маршруте (запрос Bus)
     */
    std::optional<transport::BusInfo> GetBusStat(const std::string_view& bus_name) const;
    /**
     * Возвращает маршруты, проходящие через остановку
     */
    std::optional<std::vector<std::string>> GetBusesByStop(const std::string_view& stop_name) const;
    /**
     * Отрисовка карты
     */
    void RenderMap(std::ostream &output) const;
    /**
     * Получить информацию об оптимальном маршруте между остановками
     */
    const std::optional<graph::Router<double>::RouteInfo>
    GetOptimalRoute(const std::string_view stop_from, const std::string_view stop_to) const;
    /**
     * Граф
     */
    const graph::DirectedWeightedGraph<double>& GetRouterGraph() const;
    /**
     * Обновить данные агрегированных объектов
     */
    void UpdateInternalData();
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transport::Catalogue db_;
    renderer::MapRenderer& renderer_;
    transport::Router& router_;
};

