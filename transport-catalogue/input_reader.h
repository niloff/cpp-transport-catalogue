#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <deque>
#include "geo.h"
#include "transport_catalogue.h"

namespace reader::input {
/**
 * Команда-запрос
 */
struct CommandDescription {
    /**
     * Наименование команды - остановка
     */
    static const char* COMMAND_NAME_STOP;
    /**
     * Наименование команды - маршрут
     */
    static const char* COMMAND_NAME_BUS;
    /**
     * Определяет, задана ли команда (поле command непустое)
     */
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }
    /**
     * Название команды
     */
    std::string command;
    /**
     * id маршрута или остановки
     */
    std::string id;
    /**
     * Параметры команды
     */
    std::string description;
};
/**
 * Интерфейс для ввода данных
 */
class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
     */
    void ParseLine(std::string_view line);
    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(catalogue::TransportCatalogue& catalogue) const;
private:
    /**
     * Команды на добавление информации
     */
    std::deque<CommandDescription> commands_;
    /**
     * Соответствие наименования команды списку команд
     */
    std::unordered_map<std::string_view, std::vector<CommandDescription*>> name_to_commands_;
};
}
