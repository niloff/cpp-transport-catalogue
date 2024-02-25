#pragma once
#include <optional>
#include <string>
#include <unordered_set>
#include "transport_catalogue.h"
#include "map_renderer.h"
/**
 * Класс-фасад, упрощающий взаимодействие JSON reader-а
 * с другими подсистемами приложения.
 */
class RequestHandler {
public:
    /**
     * Конструктор
     */
    RequestHandler(const transport::Catalogue& db, const renderer::MapRenderer& renderer);
    /**
     * Возвращает информацию о маршруте (запрос Bus)
     */
    std::optional<transport::BusInfo> GetBusStat(const std::string_view& bus_name) const;
    /**
     * Возвращает маршруты, проходящие через остановку
     */
    const std::unordered_set<transport::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;
    /**
     * Отрисовка карты
     */
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport::Catalogue& db_;
    const renderer::MapRenderer& renderer_;
};

