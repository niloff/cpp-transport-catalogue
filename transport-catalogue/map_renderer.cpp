#include "map_renderer.h"

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
/**
 * Возвращает ломаные линии маршрутов
 */
std::vector<svg::Polyline> MapRenderer::GetRouteLines(const std::vector<const transport::Bus*>& buses, const SphereProjector& sp) const {
    std::vector<svg::Polyline> route_lines;
    route_lines.reserve(buses.size());
    size_t color_num = 0;
    for (const auto& bus : buses) {
        svg::Polyline line;
        for (const auto& stop : bus->stops) {
            line.AddPoint(sp(stop->coordinates));
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
std::vector<svg::Text> MapRenderer::GetRoutesLabels(const std::vector<const transport::Bus*>& buses, const SphereProjector& sp) const {
    std::vector<svg::Text> routes_labels;
    routes_labels.reserve(buses.size());
    size_t color_num = 0;
    for (const auto& bus : buses) {
        // подложка
        svg::Text underlayer;
        underlayer.SetPosition(sp(bus->stops[0]->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus->route)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        routes_labels.push_back(underlayer);
        // текст
        svg::Text text;
        text.SetPosition(sp(bus->stops[0]->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus->route)
                .SetFillColor(render_settings_.color_palette[color_num]);
        routes_labels.push_back(text);
        color_num = color_num < render_settings_.color_palette.size() - 1 ?
                    color_num + 1 : 0;
        if (bus->is_roundtrip) continue;
        auto stop_end = bus->stops[bus->stops.size() / 2];
        if (bus->stops.front() == stop_end) continue;
        svg::Text underlayer2 {underlayer};
        routes_labels.push_back(underlayer2.SetPosition(sp(stop_end->coordinates)));
        svg::Text text2 {text};
        routes_labels.push_back(text2.SetPosition(sp(stop_end->coordinates)));
    }

    return routes_labels;
}
/**
 * Возвращает круги, обозначающие остановки
 */
std::vector<svg::Circle> MapRenderer::GetStopsSymbols(const std::vector<const transport::Stop*>& stops, const SphereProjector& sp) const {
    std::vector<svg::Circle> stops_symbols;
    stops_symbols.reserve(stops.size());
    for (const auto& stop : stops) {
        stops_symbols.push_back(svg::Circle()
                                .SetCenter(sp(stop->coordinates))
                                .SetRadius(render_settings_.stop_radius)
                                .SetFillColor("white"));
    }
    return stops_symbols;
}
/**
 * Возвращает названия остановок
 */
std::vector<svg::Text> MapRenderer::GetStopsLabels(const std::vector<const transport::Stop*>& stops, const SphereProjector& sp) const {
    std::vector<svg::Text> stops_labels;
    stops_labels.reserve(stops.size());
    for (const auto& stop : stops) {
        // подложка
        stops_labels.push_back(svg::Text()
                               .SetPosition(sp(stop->coordinates))
                               .SetOffset(render_settings_.stop_label_offset)
                               .SetFontSize(render_settings_.stop_label_font_size)
                               .SetFontFamily("Verdana")
                               .SetData(stop->name)
                               .SetFillColor(render_settings_.underlayer_color)
                               .SetStrokeColor(render_settings_.underlayer_color)
                               .SetStrokeWidth(render_settings_.underlayer_width)
                               .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                               .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        // текст
        stops_labels.push_back(svg::Text().SetData(stop->name)
                               .SetPosition(sp(stop->coordinates))
                               .SetOffset(render_settings_.stop_label_offset)
                               .SetFontSize(render_settings_.stop_label_font_size)
                               .SetFontFamily("Verdana")
                               .SetData(stop->name)
                               .SetFillColor("black"));
    }
    return stops_labels;
}
/**
 * Возвращает векторное изображение маршрутов каталога
 */
svg::Document MapRenderer::GetSVG(const std::vector<const transport::Bus*>& buses,
                                  const std::vector<const transport::Stop*>& stops,
                                  const std::vector<geo::Coordinates> &route_stops_coord) const {
    svg::Document doc;
    SphereProjector sp(route_stops_coord.begin(),
                       route_stops_coord.end(),
                       render_settings_.width,
                       render_settings_.height,
                       render_settings_.padding);
    for (const auto& line : GetRouteLines(buses, sp)) {
        doc.Add(line);
    }
    for (const auto& text : GetRoutesLabels(buses, sp)) {
        doc.Add(text);
    }
    for (const auto& circle : GetStopsSymbols(stops, sp)) {
        doc.Add(circle);
    }
    for (const auto& text : GetStopsLabels(stops, sp)) {
        doc.Add(text);
    }

    return doc;
}

} // namespace renderer
