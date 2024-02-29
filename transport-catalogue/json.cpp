#include "json.h"

using namespace std;

namespace json {
/**
 * Хранится ли в узле значение типа integer
 */
bool Node::IsInt() const {
    return holds_alternative<int>(*this);
}
/**
 * Хранится ли в узле значение типа double
 */
bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this);
}
/**
 * Хранится ли в узле значение типа integer или double
 */
bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}
/**
 * Хранится ли в узле значение типа boolean
 */
bool Node::IsBool() const {
    return holds_alternative<bool>(*this);
}
/**
 * Хранится ли в узле значение строкового типа
 */
bool Node::IsString() const {
    return holds_alternative<std::string>(*this);
}
/**
 * Хранится ли в узле значение null
 */
bool Node::IsNull() const {
    return holds_alternative<std::nullptr_t>(*this);
}
/**
 * Хранится ли в узле значение типа Массив из узлов JSON
 */
bool Node::IsArray() const {
    return holds_alternative<Array>(*this);
}
/**
 * Хранится ли в узле значение типа Словарь из узлов JSON
 */
bool Node::IsMap() const {
    return holds_alternative<Dict>(*this);
}
/**
 * Данные как integer
 */
int Node::AsInt() const {
    using namespace std::literals;
    if (!IsInt()) {
        throw logic_error("Node value is not int"s);
    }
    return std::get<int>(*this);
}
/**
 * Данные как boolean
 */
bool Node::AsBool() const {
    using namespace std::literals;
    if (!IsBool()) {
        throw logic_error("Node value is not bool"s);
    }
    return std::get<bool>(*this);
}
/**
 * Данные как double
 */
double Node::AsDouble() const {
    using namespace std::literals;
    if (!IsDouble()) {
        throw std::logic_error("Node value is not double"s);
    }
    return IsPureDouble() ? std::get<double>(*this) : AsInt();
}
/**
 * Данные как строка
 */
const std::string& Node::AsString() const {
    using namespace std::literals;
    if (!IsString()) {
        throw logic_error("Node value is not string"s);
    }
    return std::get<std::string>(*this);
}
/**
 * Данные как массив из узлов JSON
 */
const Array& Node::AsArray() const {
    using namespace std::literals;
    if (!IsArray()) {
        throw logic_error("Node value is not array"s);
    }
    return std::get<Array>(*this);
}
/**
 * Данные как словарь из узлов JSON
 */
const Dict& Node::AsMap() const {
    using namespace std::literals;
    if (!IsMap()) {
        throw logic_error("Node value is not map"s);
    }
    return std::get<Dict>(*this);
}
/**
 * Извлечь данные узла
 */
const Node::Value& Node::GetValue() const {
    return *this;
}
/**
 *  Извлечь данные узла
 */
Node::Value& Node::GetValue() {
    return *this;
}
/**
 * Перегрузка оператора
 */
bool Node::operator==(const Node& rhs) const {
    return GetValue() == rhs.GetValue();
}
/**
 * Перегрузка оператора
 */
bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}
/**
 * Корневой узел документа
 */
const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

namespace {

using namespace std::literals;

Node LoadNode(istream& input);
/**
 * Загрузка буквенной последовательности символов из потока
 */
std::string LoadAlphaSymbols(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}
/**
 * Считывает null JSON-документа
 */
Node LoadNull(istream& input) {
    const auto& s = LoadAlphaSymbols(input);
    if (s != "null"sv) {
        throw ParsingError("Failed to convert "s + s + "' to null"s);
    }
    return Node{nullptr};
}
/**
 * Считывает содержимое строкового литерала JSON-документа
 * Функцию следует использовать после считывания открывающего символа ":
 */
Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error"s);
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error"s);
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    // возвращаем узел со строковым значением
    return Node(std::move(s));
}
/**
 * Считывает число (integer либо double) JSON-документа
 */
Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}
/**
 * Считывает булевое значение (true либо false) JSON-документа
 */
Node LoadBool(istream& input) {
    const auto& s = LoadAlphaSymbols(input);
    if (s == "true"sv) {
        return Node{ true };
    }
    if (s == "false"sv) {
        return Node{ false };
    }
    throw ParsingError("Failed to convert "s + s + "' to bool"s);
}
/**
 * Считывает массив из узлов JSON-документа.
 * Функцию следует использовать после считывания открывающего символа ]
 */
Node LoadArray(istream& input) {
    Array array;
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        array.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Failed to convert data to array"s);
    }
    return Node(std::move(array));
}
/**
 * Считывает словарь из узлов JSON-документа.
 * Функцию следует использовать после считывания открывающего символа }
 */
Node LoadDict(istream& input) {
    Dict dict;
    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (dict.count(key) == 0) {
                   dict.emplace(std::move(key), LoadNode(input));
                   continue;
                }
                throw ParsingError("Duplicate key '"s + key + "' have been found"s);
            }
            throw ParsingError(": is expected but '"s + c + "' has been found"s);
        }
        if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}
/**
 * Считывает узел JSON-документа.
 */
Node LoadNode(istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Unexpected end of file"s);
    }
    switch (c) {
    case '[':
        return LoadArray(input);
    case '{':
        return LoadDict(input);
    case '"':
        return LoadString(input);
    case 't':
    case 'f':
        input.putback(c);
        return LoadBool(input);
    case 'n':
        input.putback(c);
        return LoadNull(input);
    default:
        input.putback(c);
        return LoadNumber(input);
    }
}

/**
 * Контекст вывода.
 * Хранит ссылку на поток вывода и текущий отсуп
 */
struct PrintContext {
    /**
     * Поток вывода
     */
    std::ostream& out;
    /**
     * Шаг отступа
     */
    int indent_step = 4;
    /**
     * Отступ
     */
    int indent = 0;
    /**
     * Добавление отступов в соответствии с контекстом
     */
    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    /**
     * Возвращает новый контекст вывода с увеличенным смещением
     */
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};
/**
 * Вывод узла в соответствии с контекстом
 */
void PrintNode(const Node& value, const PrintContext& ctx);
/**
 * Вывод значения узла в соответствии с контекстом.
 * Шаблон, подходящий для вывода double и int
 */
template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}
/**
 * Вывод строкового значения узла
 */
void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '"':
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
        }
    }
    out.put('"');
}
/**
 * Вывод строкового значения узла в соответствии с контекстом.
 */
template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}
/**
 * Вывод null значения узла в соответствии с контекстом.
 */
template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}
/**
 * Вывод булевого значения узла в соответствии с контекстом.
 */
template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << (value ? "true"sv : "false"sv);
}
/**
 * Вывод Массива узлов в соответствии с контекстом
 */
template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}
/**
 * Вывод Словаря из узлов в соответствии с контекстом
 */
template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}
/**
 * Вывод узла в соответствии с контекстом
 */
void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit([&ctx](const auto& value) { PrintValue(value, ctx); },
                node.GetValue());
}

}  // namespace

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json
