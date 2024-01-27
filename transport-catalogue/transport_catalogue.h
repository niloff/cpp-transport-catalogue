#pragma once
#include <unordered_map>
#include <deque>
#include <vector>
#include <set>
#include <ostream>
#include <unordered_set>
#include "geo.h"

namespace catalogue {
/**
 * Остановка
 */
struct Stop {
    /**
     * Название остановки
     */
    std::string name;
    /**
     * Координаты остановки
     */
    geo::Coordinates coordinates;
};

struct StopHasher {
    size_t operator() (const Stop& stop) const noexcept {
        return s_hasher_(stop.name) * 37 * 37 + coord_hasher_(stop.coordinates);
    }
private:
    std::hash<std::string> s_hasher_;
    geo::CoordinatesHash coord_hasher_;
};
/**
 * Автобус
 */
struct Bus {
    /**
     * Номер маршрута
     */
    std::string route;
    /**
     * Остановки, через которые проходит автобус
     */
    std::vector<const Stop*> stops;
};
/**
 * Статистика по маршруту
 */
struct BusInfo {
    /**
     * Количество остановок
     */
    size_t stops_count = 0;
    /**
     * Количество уникальных остановок
     */
    size_t unique_stops_count = 0;
    /**
     * Длина маршрута
     */
    double route_length = 0;
};
/**
 * Статистика по остановке
 */
class StopInfo {
public:
    StopInfo() = default;
    /**
     * Добавить маршрут в статистику остановки
     */
    void AddRouteNumber(std::string_view bus);
    /**
     * Проходящие через остановку маршруты
     */
    std::set<std::string_view> GetRouteNumbers() const;
private:
    std::unordered_set<std::string_view> buses_;
};
/**
 * Транспортный каталог
 */
class TransportCatalogue {
public:
    /**
     * Конструктор
     */
    TransportCatalogue() = default;
    /**
     * Добавить остановку в каталог
     */
    void AddStop(std::string_view stop_name, const geo::Coordinates& coordinates);
    /**
     * Добавить маршрут в каталог
     */
    void AddRoute(std::string_view bus_number,
                  const std::vector<const Stop*>& stops);
    /**
     * Добавить маршрут в каталог
     * Версия с вектором имен остановок
     */
    void AddRoute(std::string_view bus_number,
                  const std::vector<std::string_view>& stop_names);
    /**
     * Найти маршрут по его номеру
     */
    const Bus* FindRoute(std::string_view bus_number) const;
    /**
     * Найти остановку по наименованию
     */
    const Stop* FindStop(std::string_view stop_name) const;
    /**
     * Статистика по автобусу
     */
    BusInfo* GetBusInfo(std::string_view bus_number) const;
    /**
     * Статистика по остановке
     */
    const StopInfo* GetStopInfo(std::string_view stop_name) const;
    /**
     * Установить расстояние между двумя остановками
     */
    void SetDistance(const Stop* from,
                     const Stop* to,
                     int distance);
    /**
     * Получить расстояние между двумя остановками
     */
    int GetDistance(const Stop* from, const Stop* to) const;
private:
    struct DistanceHasher {
        size_t operator() (const std::pair<const Stop*, const Stop*>& stops) const noexcept {
            return hasher_(*stops.first) + hasher_(*stops.second) * 37 * 37 * 37;
        }
    private:
        StopHasher hasher_;
    };
    /**
     * Остановки
     */
    std::deque<Stop> stops_;
    /**
     * Автобусы
     */
    std::deque<Bus> buses_;
    /**
     * Остановки по их наименованию
     */
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    /**
     * Автобусы по номерам маршрутов
     */
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    /**
     * Расстояния между парами остановок
     */
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher> distances_;
    /**
     * Информация об остановках
     */
    std::unordered_map<const Stop*, StopInfo> stop_to_info_;
};
}
