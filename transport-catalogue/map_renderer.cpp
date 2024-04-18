#include "map_renderer.h"

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

MapRenderer& MapRenderer::SetBuses(const std::vector<const transport::Bus*>& buses) {
    buses_ = buses;
    return *this;
}

MapRenderer& MapRenderer::SetStops(const std::vector<const transport::Stop*>& stops) {
    stops_ = stops;
    return *this;
}
/**
 * Возвращает векторное изображение маршрутов каталога
 */
svg::Document MapRenderer::GetSVG() const {
    const auto projector = BuildProjector();
    svg::Document doc;
    for (const auto& line : GetRouteLines(projector)) {
        doc.Add(line);
    }
    for (const auto& text : GetRoutesLabels(projector)) {
        doc.Add(text);
    }
    for (const auto& circle : GetStopsSymbols(projector)) {
        doc.Add(circle);
    }
    for (const auto& text : GetStopsLabels(projector)) {
        doc.Add(text);
    }

    return doc;
}
/**
 * Возвращает проектор сферических координат на карту.
 * Создается на основе координат остановок.
 */
SphereProjector MapRenderer::BuildProjector() const {
    // извлекаем координаты
    std::vector<geo::Coordinates> stops_coord(stops_.size());
    for (size_t i = 0; i < stops_.size(); ++i) {
        stops_coord[i] = stops_[i]->coordinates;
    }
    return {stops_coord.begin(),
                stops_coord.end(),
                render_settings_.width,
                render_settings_.height,
                render_settings_.padding };
}
/**
 * Возвращает ломаные линии маршрутов
 */
std::vector<svg::Polyline> MapRenderer::GetRouteLines(const SphereProjector& projector) const {
    std::vector<svg::Polyline> route_lines;
    route_lines.reserve(buses_.size());
    size_t color_num = 0;
    for (const auto& bus : buses_) {
        svg::Polyline line;
        for (const auto& stop : bus->stops) {
            line.AddPoint(projector(stop->coordinates));
        }
        line.SetStrokeColor(render_settings_.color_palette[color_num])
                .SetFillColor("none")
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        color_num = color_num < render_settings_.color_palette.size() - 1 ?
                    color_num + 1 : 0;
        route_lines.push_back(line);
    }
    return route_lines;
}
/**
 * Возвращает названия маршрутов
 */
std::vector<svg::Text> MapRenderer::GetRoutesLabels(const SphereProjector& projector) const {
    std::vector<svg::Text> routes_labels;
    routes_labels.reserve(buses_.size());
    size_t color_num = 0;
    for (const auto& bus : buses_) {
        // подложка
        svg::Text underlayer;
        underlayer.SetData(bus->route)
                .SetPosition(projector(bus->stops[0]->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        routes_labels.push_back(underlayer);
        // текст
        svg::Text text;
        text.SetData(bus->route)
                .SetPosition(projector(bus->stops[0]->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(render_settings_.color_palette[color_num]);
        routes_labels.push_back(text);
        color_num = color_num < render_settings_.color_palette.size() - 1 ?
                    color_num + 1 : 0;
        if (bus->is_roundtrip) continue;
        auto stop_end = bus->stops[bus->stops.size() / 2];
        if (bus->stops.front() == stop_end) continue;
        svg::Text underlayer2 {underlayer};
        routes_labels.push_back(underlayer2.SetPosition(projector(stop_end->coordinates)));
        svg::Text text2 {text};
        routes_labels.push_back(text2.SetPosition(projector(stop_end->coordinates)));
    }

    return routes_labels;
}
/**
 * Возвращает круги, обозначающие остановки
 */
std::vector<svg::Circle> MapRenderer::GetStopsSymbols(const SphereProjector& projector) const {
    std::vector<svg::Circle> stops_symbols;
    stops_symbols.reserve(stops_.size());
    for (const auto& stop : stops_) {
        stops_symbols.push_back(svg::Circle()
                                .SetCenter(projector(stop->coordinates))
                                .SetRadius(render_settings_.stop_radius)
                                .SetFillColor("white"));
    }
    return stops_symbols;
}
/**
 * Возвращает названия остановок
 */
std::vector<svg::Text> MapRenderer::GetStopsLabels(const SphereProjector& projector) const {
    std::vector<svg::Text> stops_labels;
    stops_labels.reserve(stops_.size());
    for (const auto& stop : stops_) {
        // подложка
        stops_labels.push_back(svg::Text()
                               .SetData(stop->name)
                               .SetPosition(projector(stop->coordinates))
                               .SetOffset(render_settings_.stop_label_offset)
                               .SetFontSize(render_settings_.stop_label_font_size)
                               .SetFontFamily("Verdana")
                               .SetFillColor(render_settings_.underlayer_color)
                               .SetStrokeColor(render_settings_.underlayer_color)
                               .SetStrokeWidth(render_settings_.underlayer_width)
                               .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                               .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        // текст
        stops_labels.push_back(svg::Text()
                               .SetData(stop->name)
                               .SetPosition(projector(stop->coordinates))
                               .SetOffset(render_settings_.stop_label_offset)
                               .SetFontSize(render_settings_.stop_label_font_size)
                               .SetFontFamily("Verdana")
                               .SetFillColor("black"));
    }
    return stops_labels;
}

} // namespace renderer
