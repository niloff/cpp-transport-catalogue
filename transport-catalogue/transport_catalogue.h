#pragma once
#include <unordered_map>
#include <map>
#include <optional>
#include <deque>
#include <vector>
#include <set>
#include <ostream>
#include <unordered_set>
#include "domain.h"
/**
 * Сущности транспорта
 */
namespace transport {
/**
 * Статистика по остановке.
 * Содержит указатели на маршруты, проходящие через остановку.
 */
using StopInfo = std::unordered_set<Bus*>;
/**
 * Каталог
 */
class Catalogue {
public:
    /**
     * Конструктор
     */
    Catalogue() = default;
    /**
     * Добавить остановку в каталог
     */
    void AddStop(const Stop& stop);
    /**
     * Добавить маршрут в каталог
     */
    void AddRoute(std::string_view bus_number,
                  const std::vector<const transport::Stop*>& stops,
                  bool is_roundtrip);
    /**
     * Найти маршрут по его номеру
     */
    const transport::Bus* FindRoute(std::string_view bus_number) const;
    /**
     * Найти остановку по наименованию
     */
    const transport::Stop* FindStop(std::string_view stop_name) const;
    /**
     * Статистика по остановке
     */
    const transport::StopInfo* GetBusesByStop(const Stop* stop) const;
    /**
     * Получить каталог из отсортированных по номерам маршрутов.
     * Выводяться только непустые маршруты (с остановками)
     */
    const std::vector<const Bus*> GetSortedBuses() const;
    /**
     * Установить расстояние между двумя остановками
     */
    void SetDistance(const transport::Stop* from,
                     const transport::Stop* to,
                     int distance);
    /**
     * Получить расстояние между двумя остановками
     */
    int GetDistance(const transport::Stop* from, const transport::Stop* to) const;
private:

    struct DistanceHasher {
        size_t operator() (const std::pair<const transport::Stop*, const transport::Stop*>& stops) const noexcept {
            return hasher_(*stops.first) + hasher_(*stops.second) * 37 * 37 * 37;
        }
    private:
        transport::StopHasher hasher_;
    };
    /**
     * Остановки
     */
    std::deque<transport::Stop> stops_;
    /**
     * Автобусы
     */
    std::deque<transport::Bus> buses_;
    /**
     * Остановки по их наименованию
     */
    std::unordered_map<std::string_view, const transport::Stop*> stopname_to_stop_;
    /**
     * Автобусы по номерам маршрутов
     */
    std::unordered_map<std::string_view, const transport::Bus*> busname_to_bus_;
    /**
     * Расстояния между парами остановок
     */
    std::unordered_map<std::pair<const transport::Stop*, const transport::Stop*>, int, DistanceHasher> distances_;
    /**
     * Маршруты, проходящие через остановки
     */
    std::unordered_map<const transport::Stop*, StopInfo> stop_to_buses_;
};
}
