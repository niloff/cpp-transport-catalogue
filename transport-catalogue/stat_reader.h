#pragma once
#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"

namespace reader::stat {
/**
 * Команда-запроc на вывод информации
 */
struct QueryDescription {
    /**
     * Наименование команды - остановка
     */
    static const char* QUERY_NAME_STOP;
    /**
     * Наименование команды - маршрут
     */
    static const char* QUERY_NAME_BUS;
    /**
     * Определяет, задан ли запрос (поле query непустое)
     */
    explicit operator bool() const {
        return !query.empty();
    }

    bool operator!() const {
        return !operator bool();
    }
    /**
     * Название запроса
     */
    std::string query;
    /**
     * id маршрута или остановки
     */
    std::string id;
};

void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::istream& input,
                       std::ostream& output);

void PrintRoute(const std::string& bus_number, const catalogue::TransportCatalogue& catalogue, std::ostream& out);

void PrintStop(const std::string& stop_name, const catalogue::TransportCatalogue& catalogue, std::ostream& out);

}
