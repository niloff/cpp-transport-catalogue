#pragma once

#include "json.h"
#include "request_handler.h"
#include <iostream>

class JsonReader {
    /**
     * Ключ запросов - Массив с описанием автобусных маршрутов и остановок
     */
    static const char* KEY_BASE_REQUESTS;
    /**
     * Ключ запросов - Массив с запросами к транспортному справочнику
     */
    static const char* KEY_STAT_REQUESTS;
    /**
     * Ключ запросов - Настройки рендеринга
     */
    static const char* KEY_RENDER_SETTINGS;
    /**
     * Ключ запросов - Настройки маршрутизации
     */
    static const char* KEY_ROUTING_SETTINGS;
public:
    /**
     * Чтение данных из потока
     */
    void ReadInput(std::istream &input);
    /**
     * Наполняет данными транспортный справочник в соответствии с запросами
     */
    void UploadData(RequestHandler& handler);
    /**
     * Вывод информации в соответствии со считанными запросами.
     */
    void PrintResponses(RequestHandler& handler, std::ostream& output);
    /**
     * Получить считанные настройки для рендеринга
     */
    renderer::RenderSettings GetRenderSettings();
    /**
     * Получить считанные настройки для маршрутизации
     */
    transport::RoutingSettings GetRoutingSettings();
private:
    /**
     * Получить запросы по ключу
     */
    const json::Node* GetRequests(const char* request_key) const;
    /**
     * Вывод информации о маршруте
     */
    static const json::Node PrintRoute(const json::Node& request_map, RequestHandler& handler);
    /**
     * Вывод информации об остановке
     */
    static const json::Node PrintStop(const json::Node& request_map, RequestHandler& handler);
    /**
     * Вывод изображения
     */
    static const json::Node PrintMap(const json::Node& request_map, RequestHandler& handler);
    /**
     * Вывод оптимального маршрута
     */
    static const json::Node PrintRouting(const json::Node& request_map, RequestHandler& handler);
private:
    /**
     * Считанные запросы
     */
    json::Dict commands_;
};
