#include "json_reader.h"
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
 * Наполняет данными транспортный справочник, используя команды из commands_
 */
void JsonReader::UploadData(transport::Catalogue& catalogue, const json::Node* requests) {
    using namespace std::literals;
    if (requests == nullptr) return;
    // ищем массив с маршрутами и остановками
    const auto& commands_stops = CommandsFromNode(requests, "Stop"s);
    // разбираем остановки
    for (const auto& command : commands_stops) {
        catalogue.AddStop(command->at("name"s).AsString(),
                          {command->at("latitude"s).AsDouble(),
                           command->at("longitude"s).AsDouble()});
    }
    // добавляем расстояния между остановками
    for (const auto& command : commands_stops) {
        if (command->count("road_distances"s) == 0) continue;
        auto& distances = command->at("road_distances"s).AsMap();
        auto stop_from = command->at("name"s).AsString();
        for (auto& [stop_to, distance] : distances) {
            catalogue.SetDistance(stop_from, stop_to, distance.AsInt());
        }
    }
    // разбираем маршруты
    const auto& commands_buses = CommandsFromNode(requests, "Bus"s);
    for (const auto& command : commands_buses) {
        std::string_view bus_number = command->at("name"s).AsString();
        bool circular_route = command->at("is_roundtrip"s).AsBool();
        const auto& stops = RouteFromNode(command->at("stops"s), circular_route);
        catalogue.AddRoute(bus_number, stops, circular_route);
    }
}
/**
 * Возвращает статистику в соответствии с запросами
 */
void JsonReader::GetStatInfo(RequestHandler& handler, const json::Node* requests) {
    using namespace std::literals;
    if (requests == nullptr) return;
    std::vector<json::Node> result;
    for (auto& request : requests->AsArray()) {
        const auto& type = request.AsMap().at("type"s).AsString();
        if (type == "Stop"s) {
            result.emplace_back(PrintStop(request, handler).AsMap());
        }
        else if (type == "Bus"s) {
            result.emplace_back(PrintRoute(request, handler).AsMap());
        }
        else if (type == "Map"s) {
            result.emplace_back(PrintMap(request, handler).AsMap());
        }
    }
    json::Print(json::Document{ result }, std::cout);
}
/**
 * Парсит настройки для рендеринга
 */
renderer::MapRenderer JsonReader::ParseRenderSettings(const json::Node *settings) {
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
 * Вывод информации о маршруте
 */
const json::Node JsonReader::PrintRoute(const json::Node& request_map, RequestHandler& rh) {
    using namespace std::literals;
    json::Dict result;
    const std::string& route_number = request_map.AsMap().at("name"s).AsString();
    result["request_id"s] = request_map.AsMap().at("id"s).AsInt();
    auto bus_info = rh.GetBusStat(route_number);
    if (!bus_info) {
        result["error_message"s] = json::Node{ static_cast<std::string>("not found"s) };
    }
    else {
        result["curvature"s] = bus_info->curvature;
        result["route_length"s] = bus_info->route_length;
        result["stop_count"s] = static_cast<int>(bus_info->stops_count);
        result["unique_stop_count"s] = static_cast<int>(bus_info->unique_stops_count);
    }
    return json::Node{ result };
}
/**
 * Вывод информации об остановке
 */
const json::Node JsonReader::PrintStop(const json::Node& request_map, RequestHandler& rh) {
    using namespace std::literals;
    json::Dict result;
    const std::string& stop_name = request_map.AsMap().at("name"s).AsString();
    result["request_id"s] = request_map.AsMap().at("id"s).AsInt();
    auto stops_info = rh.GetBusesByStop(stop_name);
    if (!stops_info) {
        result["error_message"s] = json::Node{ static_cast<std::string>("not found"s) };
    }
    else {
        std::set<std::string> sorted_routes;
        for (auto info : *stops_info) {
            sorted_routes.insert(info->route);
        }
        json::Array buses;
        for (auto& bus : sorted_routes) {
            buses.push_back(bus);
        }
        result["buses"s] = buses;
    }
    return json::Node{ result };
}
/**
 * Вывод изображения
 */
const json::Node JsonReader::PrintMap(const json::Node& request_map, RequestHandler& rh) {
    using namespace std::literals;
    json::Dict result;
    result["request_id"s] = request_map.AsMap().at("id"s).AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result["map"s] = strm.str();
    return json::Node{ result };
}
