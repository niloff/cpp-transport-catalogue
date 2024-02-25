#pragma once

#include "json.h"
#include "request_handler.h"
#include <iostream>

class JsonReader {
public:
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
     * Чтение данных из потока
     */
    void ReadInput(std::istream &input);
    /**
     * Получить запросы по ключу
     */
    const json::Node* GetRequests(const char* request_key) const;
    /**
     * Наполняет данными транспортный справочник в соответствии с запросами
     */
    static void UploadData(transport::Catalogue& catalogue, const json::Node* requests);
    /**
     * Возвращает статистику в соответствии с запросами
     */
    static void GetStatInfo(RequestHandler& handler, const json::Node* requests);
    /**
     * Парсит настройки для рендеринга
     */
    static renderer::MapRenderer ParseRenderSettings(const json::Node* settings);
private:
    /**
     * Вывод информации о маршруте
     */
    static const json::Node PrintRoute(const json::Node& request_map, RequestHandler& rh);
    /**
     * Вывод информации об остановке
     */
    static const json::Node PrintStop(const json::Node& request_map, RequestHandler& rh);
    /**
     * Вывод изображения
     */
    static const json::Node PrintMap(const json::Node& request_map, RequestHandler& rh);
private:
    json::Dict commands_;
};
