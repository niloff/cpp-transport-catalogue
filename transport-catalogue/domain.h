#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include "geo.h"
/**
 * Сущности транспорта
 */
namespace transport {
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
/**
 * Хэш остановки
 */
struct StopHasher {
    /**
     * Хэш остановки
     */
    size_t operator() (const Stop& stop) const noexcept;
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
    /**
     * Кольцевой ли маршрут
     */
    bool is_roundtrip;
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
    /**
     * Извилистость маршрута
     */
    double curvature = 0;
};
}
