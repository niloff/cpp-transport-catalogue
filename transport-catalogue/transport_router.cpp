#include "transport_router.h"

namespace transport {

const graph::DirectedWeightedGraph<double>& Router::BuildGraph(const Catalogue& catalogue) {
    stop_ids_.clear();
    const auto& stops_sorted = catalogue.GetStops(SortMode::SORTED);
    graph::DirectedWeightedGraph<double> stops_graph(stops_sorted.size() * 2);
    graph::VertexId vertex_id = 0;
    for (const auto stop_info : stops_sorted) {
        stop_ids_[stop_info->name] = vertex_id;
        stops_graph.AddEdge({
                stop_info->name,
                0,
                vertex_id,
                ++vertex_id,
                static_cast<double>(routing_settings_.bus_wait_time)
            });
        ++vertex_id;
    }
    const auto& buses_sorted = catalogue.GetBuses(SortMode::SORTED);
    for (const auto bus : buses_sorted) {
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
                stops_graph.AddEdge({bus->route,
                                     static_cast<size_t>(std::distance(it_from, it_to)),
                                     ++stop_ids_.at(stop_from->name),
                                     stop_ids_.at(stop_to->name),
                                     static_cast<double>(distance) / (routing_settings_.bus_velocity * (100.0 / 6.0))
                                    });
            }
        }
    }

    graph_ = std::move(stops_graph);
    router_ = std::make_unique<graph::Router<double>>(graph_);

    return graph_;
}

const std::optional<graph::Router<double>::RouteInfo> Router::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
    return router_->BuildRoute(stop_ids_.at(std::string(stop_from)),stop_ids_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
    return graph_;
}

}
