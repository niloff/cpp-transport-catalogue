#include "transport_router.h"

namespace transport {
/**
 * Сборка сервиса
 */
void Router::Build(const Catalogue& catalogue) {
    const auto& stops_sorted = catalogue.GetStops(SortMode::SORTED);
    graph_ = std::move(graph::DirectedWeightedGraph<double>(stops_sorted.size() * 2));
    FillStops(std::move(stops_sorted));
    FillBuses(catalogue);
    router_ = std::make_unique<graph::Router<double>>(graph_);
}
/**
 * Получить оптимальный маршрут
 */
std::optional<RouterResponse> Router::GetOptimalRoute(const Stop* from, const Stop* to) const {
    const auto route = router_->BuildRoute(stop_ids_.at(from), stop_ids_.at(to));
    if (!route) {
        return std::nullopt;
    }
    RouterResponse response;
    response.route.reserve(route->edges.size());
    for (const auto edge_id : route->edges) {
        const auto& edge = graph_.GetEdge(edge_id);
        if (edge.quantity == 0) {
            response.route.emplace_back(RouterResponse::Departure{std::string(edge.title),
                                                                  edge.weight});
        }
        else {
            response.route.emplace_back(RouterResponse::Route{std::string(edge.title),
                                                              static_cast<int>(edge.quantity),
                                                              edge.weight});
        }
        response.total_time += edge.weight;
    }
    return response;
}
/**
 * Заполнить данные об остановках
 */
void Router::FillStops(const std::vector<const Stop*>& stops) {
    stop_ids_.clear();
    graph::VertexId vertex_id = 0;
    for (const auto stop : stops) {
        stop_ids_[stop] = vertex_id;
        graph_.AddEdge({
                stop->name,
                0,
                vertex_id,
                ++vertex_id,
                static_cast<double>(routing_settings_.bus_wait_time)
            });
        ++vertex_id;
    }
}
/**
 * Заполнить данные о маршрутах
 */
void Router::FillBuses(const Catalogue& catalogue) {
    const auto& buses = catalogue.GetBuses(SortMode::SORTED);
    for (const auto bus : buses) {
        const auto& bus_stops = bus->stops;
        auto stops_end = bus_stops.end();
        for (auto it_from = bus_stops.begin(); it_from != stops_end; ++it_from) {
            const transport::Stop* stop_from = *it_from;
            const transport::Stop* stop_prev = stop_from;
            int distance = 0;
            for (auto it_to = std::next(it_from); it_to != stops_end; ++it_to) {
                const transport::Stop* stop_to = *it_to;
                distance += catalogue.GetDistance(stop_prev, stop_to);
                stop_prev = stop_to;
                graph_.AddEdge({bus->route,
                                static_cast<size_t>(std::distance(it_from, it_to)),
                                stop_ids_.at(stop_from) + 1,
                                stop_ids_.at(stop_to),
                                (static_cast<double>(distance) / routing_settings_.bus_velocity) * KOEF_MINUTES_PER_METRES
                               });
            }
        }
    }
}

}
