#include "transport_catalogue.h"
#include "router.h"
#include <memory>

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
/**
 * Средство маршрутизации
 */
class Router {
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
     * Построить граф
     */
    const graph::DirectedWeightedGraph<double>& BuildGraph(const Catalogue &catalogue);
    const std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
    const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    /**
     * Настройки маршрутизации
     */
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    std::map<std::string, graph::VertexId> stop_ids_;
    std::unique_ptr<graph::Router<double>> router_;
};

}
