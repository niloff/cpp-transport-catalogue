#include "transport_catalogue.h"
#include "router.h"
#include <memory>
#include <unordered_map>
#include <variant>

namespace transport {
/**
 * Настройки маршрутизации
 */
struct RoutingSettings {
    /**
     * Время ожидания автобуса на остановке, в минутах.
     * Значение — целое число от 1 до 1000.
     */
    int bus_wait_time = 0;
    /**
     * Скорость автобуса, в км/ч.
     * Значение — вещественное число от 1 до 1000.
     */
    double bus_velocity = 0.0;
};
struct RouterResponse {
    double total_time = 0.0;

    struct Departure {
        std::string stop_name;
        double time;
    };
    struct Route {
        std::string bus;
        int span_count;
        double time = 0.0;
    };

    std::vector<std::variant<Departure, Route>> route;
};
/**
 * Средство маршрутизации
 */
class Router {
    /**
     * Коэффициент для перевода км/ч в м/мин
     */
    static constexpr double KOEF_MINUTES_PER_METRES = 0.06;
public:
    /**
     * Конструктор
     */
    Router() = default;
    /**
     * Конструктор
     */
    Router(const RoutingSettings& routing_settings)
        : routing_settings_(routing_settings) {}
    /**
     * Сборка сервиса
     */
    void Build(const Catalogue &catalogue);
    /**
     * Получить оптимальный маршрут
     */
    std::optional<RouterResponse> GetOptimalRoute(const Stop* from, const Stop* to) const;
private:
    /**
     * Заполнить данные об остановках
     */
    void FillStops(const std::vector<const Stop*>& stops);
    /**
     * Заполнить данные о маршрутах
     */
    void FillBuses(const Catalogue& catalogue);
private:
    /**
     * Настройки маршрутизации
     */
    RoutingSettings routing_settings_;
    /**
     * Граф
     */
    graph::DirectedWeightedGraph<double> graph_;
    /**
     * Идентификатор вершины графа по указателю на остановку
     */
    std::unordered_map<const transport::Stop*, graph::VertexId> stop_ids_;
    /**
     * Маршрутизация по графу
     */
    std::unique_ptr<graph::Router<double>> router_;
};

}
