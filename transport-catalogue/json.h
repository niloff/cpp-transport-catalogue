#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
/**
 * Библиотека для чтения JSON
 */
namespace json {

class Node;
/**
 * Словарь из узлов JSON
 */
using Dict = std::map<std::string, Node>;
/**
 * Массив из узлов JSON
 */
using Array = std::vector<Node>;
/**
 * Ошибка, выбрасываемая при ошибке парсинга JSON
 */
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};
/**
 * Узел JSON-файла
 */
class Node final {
public:
    /**
     * Значение узла.
     * Здесь задаем поддерживаемые типы данных для значения
     */
    using Value = std::variant<std::nullptr_t, std::string, int, double, bool, Array, Dict>;
    /**
     * Конструктор по-умолчанию
     */
    Node() = default;
    /**
     * Конструктор ноды.
     * Тип данных - нулевой указатель
     */
    Node(std::nullptr_t) :
        value_(nullptr) { }
    /**
     * Конструктор ноды.
     * Тип данных - integer
     */
    Node(int val) :
        value_(val) { }
    /**
     * Конструктор ноды.
     * Тип данных - double
     */
    Node(double val) :
        value_(val) { }
    /**
     * Конструктор ноды.
     * Тип данных - строка
     */
    Node(std::string val) :
        value_(std::move(val)) { }
    /**
     * Конструктор ноды.
     * Тип данных - массив из узлов JSON
     */
    Node(Array val) :
        value_(std::move(val)) { }
    /**
     * Конструктор ноды.
     * Тип данных - словарь из узлов JSON
     */
    Node(Dict val) :
        value_(std::move(val)) { }
    /**
     * Конструктор ноды.
     * Тип данных - boolean
     */
    Node(bool val) :
        value_(val) { }
    /**
     * Хранится ли в узле значение типа integer
     */
    bool IsInt() const;
    /**
     * Хранится ли в узле значение типа double
     */
    bool IsPureDouble() const;
    /**
     * Хранится ли в узле значение типа integer или double
     */
    bool IsDouble() const;
    /**
     * Хранится ли в узле значение типа boolean
     */
    bool IsBool() const;
    /**
     * Хранится ли в узле значение строкового типа
     */
    bool IsString() const;
    /**
     * Хранится ли в узле значение null
     */
    bool IsNull() const;
    /**
     * Хранится ли в узле значение типа Массив из узлов JSON
     */
    bool IsArray() const;
    /**
     * Хранится ли в узле значение типа Словарь из узлов JSON
     */
    bool IsMap() const;
    /**
     * Данные как integer
     */
    int AsInt() const;
    /**
     * Данные как boolean
     */
    bool AsBool() const;
    /**
     * Данные как double
     */
    double AsDouble() const;
    /**
     * Данные как строка
     */
    const std::string& AsString() const;
    /**
     * Данные как массив из узлов JSON
     */
    const Array& AsArray() const;
    /**
     * Данные как словарь из узлов JSON
     */
    const Dict& AsMap() const;
    /**
     * Извлечь данные узла
     */
    const Value& GetValue() const;
    /**
     * Перегрузка оператора
     */
    bool operator==(const Node& rhs) const;
private:
    /**
     * Хранимое значение узла JSON
     */
    Value value_;
};
/**
 * Перегрузка оператора
 */
bool operator!=(const Node& lhs, const Node& rhs);
/**
 * Документ JSON
 */
class Document {
public:
    /**
     * Конструктор.
     * Инициализация корневым узлом.
     */
    explicit Document(Node root) :
        root_(std::move(root)) { }
    /**
     * Корневой узел документа
     */
    const Node& GetRoot() const;
private:
    /**
     * Корневой узел документа
     */
    Node root_;
};
/**
 * Перегрузка операторов
 */
bool operator==(const Document& lhs, const Document& rhs);

bool operator!=(const Document& lhs, const Document& rhs);
/**
 * Загрузить документ из потока
 */
Document Load(std::istream& input);
/**
 * Вывод документа в поток
 */
void Print(const Document& doc, std::ostream& output);

}  // namespace json
