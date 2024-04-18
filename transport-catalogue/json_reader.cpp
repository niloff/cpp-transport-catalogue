#include "json_reader.h"
#include "json_builder.h"
#include <sstream>
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
/**
 * Ключ запросов - Массив с описанием автобусных маршрутов и остановок
 */
const char* JsonReader::KEY_BASE_REQUESTS = "base_requests";
/**
 * Ключ запросов - Массив с запросами к транспортному справочнику
 */
const char* JsonReader::KEY_STAT_REQUESTS = "stat_requests";
/**
 * Ключ запросов - Настройки рендеринга
 */
const char* JsonReader::KEY_RENDER_SETTINGS = "render_settings";
/**
 * Ключ запросов - Настройки маршрутизации
 */
const char* JsonReader::KEY_ROUTING_SETTINGS = "routing_settings";

namespace  {

std::vector<const json::Dict*> CommandsFromNode(const json::Node* node, std::string_view command_type) {
    using namespace std::literals;
    if (!node->IsArray()) return {};
    const auto& node_requests = node->AsArray();
    std::vector<const json::Dict*> results;
    results.reserve(node_requests.size());
    for (const auto& request : node_requests) {
        if (!request.IsMap()) continue;
        const auto& command = request.AsMap();
        if (command_type != command.at("type"s).AsString()) continue;
        results.push_back(&command);
    }
    return results;
}
/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> RouteFromNode(const json::Node& node, bool is_roundtrip) {
    const auto& stops_array = node.AsArray();
    std::vector<std::string_view> stops(stops_array.size());
    for (size_t i = 0; i < stops_array.size(); ++i) {
        stops[i] = stops_array[i].AsString();
    }
    if (is_roundtrip) {
        return stops;
    }
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    return results;
}
/**
 * Парсинг цвета из ноды
 */
svg::Color ColorFromNode(const json::Node& node) {
    using namespace std::literals;
    if (node.IsNull()) {
        return svg::NoneColor;
    }
    if (node.IsString()) {
        return node.AsString();
    }
    if (node.IsArray()) {
        const json::Array& color = node.AsArray();
        if (color.size() == 3) {
            return svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
        }
        else if (color.size() == 4) {
            return svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
        }
    }
    throw std::logic_error("Missing node type for color parsing"s);
}
/**
 * Парсинг точки из ноды
 */
svg::Point PointFromNode(const json::Node& node) {
    using namespace std::literals;
    if (node.IsNull()) {
        return svg::Point();
    }
    if (node.IsArray()) {
        const auto& points = node.AsArray();
        if (points.size() == 2) {
            return { points[0].AsDouble(), points[1].AsDouble() };
        }
    }
    throw std::logic_error("Missing node type for point parsing"s);
}

}
/**
 * Чтение данных из потока
 */
void JsonReader::ReadInput(std::istream &input) {
    auto doc = json::Load(input);
    commands_ = doc.GetRoot().AsMap();
}
/**
 * Наполняет данными транспортный справочник, используя команды из commands_
 */
void JsonReader::UploadData(RequestHandler& handler) {
    using namespace std::literals;
    const json::Node* requests = GetRequests(KEY_BASE_REQUESTS);
    if (requests == nullptr) return;
    // ищем массив с маршрутами и остановками
    const auto& commands_stops = CommandsFromNode(requests, "Stop"s);
    // разбираем остановки
    for (const auto& command : commands_stops) {
        handler.AddStop(command->at("name"s).AsString(),
                          {command->at("latitude"s).AsDouble(),
                           command->at("longitude"s).AsDouble()});
    }
    // добавляем расстояния между остановками
    for (const auto& command : commands_stops) {
        if (command->count("road_distances"s) == 0) continue;
        auto& distances = command->at("road_distances"s).AsMap();
        auto stop_from = command->at("name"s).AsString();
        for (auto& [stop_to, distance] : distances) {
            handler.SetDistance(stop_from, stop_to, distance.AsInt());
        }
    }
    // разбираем маршруты
    const auto& commands_buses = CommandsFromNode(requests, "Bus"s);
    for (const auto& command : commands_buses) {
        std::string_view bus_number = command->at("name"s).AsString();
        bool circular_route = command->at("is_roundtrip"s).AsBool();
        const auto& stops = RouteFromNode(command->at("stops"s), circular_route);
        handler.AddRoute(bus_number, stops, circular_route);
    }
    // обновляем данные в инструментах
    handler.UpdateInternalData();
}
/**
 * Возвращает статистику в соответствии с запросами
 */
void JsonReader::PrintResponses(RequestHandler& handler, std::ostream &output) {
    using namespace std::literals;
    const json::Node* requests = GetRequests(KEY_STAT_REQUESTS);
    if (requests == nullptr) return;
    json::Array responses;
    for (auto& request : requests->AsArray()) {
        const auto& type = request.AsMap().at("type"s).AsString();
        if (type == "Stop"s) {
            responses.emplace_back(PrintStop(request, handler).AsMap());
        }
        else if (type == "Bus"s) {
            responses.emplace_back(PrintRoute(request, handler).AsMap());
        }
        else if (type == "Map"s) {
            responses.emplace_back(PrintMap(request, handler).AsMap());
        }
        else if (type == "Route"s) {
            responses.emplace_back(PrintRouting(request, handler).AsMap());
        }
    }
    json::Print(json::Document{ responses }, output);
}
/**
 * Парсит настройки для рендеринга
 */
renderer::RenderSettings JsonReader::GetRenderSettings() {
    const json::Node* settings = GetRequests(KEY_RENDER_SETTINGS);
    if (settings == nullptr) {
        return {};
    }
    json::Dict settings_map = settings->AsMap();
    renderer::RenderSettings render_settings;
    render_settings.width = settings_map.at("width").AsDouble();
    render_settings.height = settings_map.at("height").AsDouble();
    render_settings.padding = settings_map.at("padding").AsDouble();
    render_settings.stop_radius = settings_map.at("stop_radius").AsDouble();
    render_settings.line_width = settings_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = settings_map.at("bus_label_font_size").AsInt();
    render_settings.bus_label_offset = PointFromNode(settings_map.at("bus_label_offset"));
    render_settings.stop_label_font_size = settings_map.at("stop_label_font_size").AsInt();
    render_settings.stop_label_offset = PointFromNode(settings_map.at("stop_label_offset"));
    render_settings.underlayer_color = ColorFromNode(settings_map.at("underlayer_color"));
    render_settings.underlayer_width = settings_map.at("underlayer_width").AsDouble();
    const json::Array& color_palette = settings_map.at("color_palette").AsArray();
    for (const auto& color : color_palette) {
        render_settings.color_palette.emplace_back(ColorFromNode(color));
    }
    return render_settings;
}
/**
 * Получить считанные настройки для маршрутизации
 */
transport::RoutingSettings JsonReader::GetRoutingSettings() {
    const json::Node* settings = GetRequests(KEY_ROUTING_SETTINGS);
    if (settings == nullptr) {
        return {};
    }
    json::Dict settings_map = settings->AsMap();
    transport::RoutingSettings routing_settings;
    routing_settings.bus_wait_time = settings_map.at("bus_wait_time").AsInt();
    routing_settings.bus_velocity = settings_map.at("bus_velocity").AsDouble();
    return routing_settings;
}
/**
 * Получить запросы по ключу
 */
const json::Node* JsonReader::GetRequests(const char* request_key) const {
    auto it = commands_.find(request_key);
    if (it != commands_.end()) {
        return &it->second;
    }
    return nullptr;
}
/**
 * Вывод информации о маршруте
 */
const json::Node JsonReader::PrintRoute(const json::Node& request_map, RequestHandler& handler) {
    using namespace std::literals;
    const std::string& route_number = request_map.AsMap().at("name"s).AsString();
    const int request_id = request_map.AsMap().at("id"s).AsInt();
    auto bus_info = handler.GetBusStat(route_number);
    if (!bus_info) {
        return json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    }
    return json::Builder{}
                .StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("curvature").Value(bus_info->curvature)
                    .Key("route_length").Value(bus_info->route_length)
                    .Key("stop_count").Value(static_cast<int>(bus_info->stops_count))
                    .Key("unique_stop_count").Value(static_cast<int>(bus_info->unique_stops_count))
                .EndDict()
            .Build();
}
/**
 * Вывод информации об остановке
 */
const json::Node JsonReader::PrintStop(const json::Node& request_map, RequestHandler& handler) {
    using namespace std::literals;
    const std::string& stop_name = request_map.AsMap().at("name"s).AsString();
    const int request_id = request_map.AsMap().at("id"s).AsInt();
    auto buses = handler.GetBusesByStop(stop_name);
    if (!buses) {
        return json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request_id)
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                .Build();
    }
    return json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("buses"s).Value(json::Array(buses->begin(), buses->end()))
                .EndDict()
            .Build();
}
/**
 * Вывод изображения
 */
const json::Node JsonReader::PrintMap(const json::Node& request_map, RequestHandler& handler) {
    using namespace std::literals;
    const int request_id = request_map.AsMap().at("id"s).AsInt();
    std::ostringstream strm;
    handler.RenderMap(strm);
    return json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("map"s).Value(strm.str())
                .EndDict()
            .Build();
}

const json::Node JsonReader::PrintRouting(const json::Node& request_map, RequestHandler& handler) {
    using namespace std::literals;
    const int request_id = request_map.AsMap().at("id"s).AsInt();
    const std::string_view stop_from = request_map.AsMap().at("from"s).AsString();
    const std::string_view stop_to = request_map.AsMap().at("to"s).AsString();
    const auto& routing = handler.GetOptimalRoute(stop_from, stop_to);

    if (!routing) {
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("error_message"s).Value("not found"s)
            .EndDict()
        .Build();
    }
    json::Array items;
    double total_time = 0.0;
    items.reserve(routing.value().edges.size());
    for (auto& edge_id : routing.value().edges) {
        const graph::Edge<double> edge = handler.GetRouterGraph().GetEdge(edge_id);
        if (edge.quantity == 0) {
            items.emplace_back(json::Node(json::Builder{}
                                            .StartDict()
                                                .Key("stop_name"s).Value(edge.title)
                                                .Key("time"s).Value(edge.weight)
                                                .Key("type"s).Value("Wait"s)
                                            .EndDict()
                                          .Build()));

            total_time += edge.weight;
            continue;
        }
        items.emplace_back(json::Node(json::Builder{}
                                        .StartDict()
                                            .Key("bus"s).Value(edge.title)
                                            .Key("span_count"s).Value(static_cast<int>(edge.quantity))
                                            .Key("time"s).Value(edge.weight)
                                            .Key("type"s).Value("Bus"s)
                                        .EndDict()
                                      .Build()));

        total_time += edge.weight;
    }

    return json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("total_time"s).Value(total_time)
                    .Key("items"s).Value(items)
                .EndDict()
            .Build();
}
