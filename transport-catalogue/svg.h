#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <variant>

namespace svg {
/**
 * Вспомогательные инструменты
 */
namespace utils {
/**
 * Передача значения в поток вывода
 */
template <typename T>
inline void RenderValue(std::ostream& out, const T& value) {
    out << value;
}
/**
 * Кодировать строку в соответствии с html
 */
void HtmlEncodeString(std::ostream& out, std::string_view sv);
/**
 * Передача стрового значения в поток вывода
 */
template <>
inline void RenderValue<std::string>(std::ostream& out, const std::string& s) {
    HtmlEncodeString(out, s);
}
/**
 * Передача шаблонного атрибута в поток вывода
 */
template <typename AttrType>
inline void RenderAttr(std::ostream& out, std::string_view name, const AttrType& value) {
    using namespace std::literals;
    out << name << "=\""sv;
    RenderValue(out, value);
    out.put('"');
}
/**
 * Передача опционального шаблонного атрибута в поток вывода
 */
template <typename AttrType>
inline void RenderOptionalAttr(std::ostream& out, std::string_view name,
                               const std::optional<AttrType>& value) {
    if (!value) return;
    RenderAttr(out, name, *value);
}

}  // namespace utils
/**
 * Точка на координатной плоскости
 */
struct Point {
    Point() = default;
    Point(double x, double y):
        x(x),
        y(y) { }
    double x = 0;
    double y = 0;
};
/**
 * Цветовая модель RGB
 */
struct Rgb {
    /**
     * Конструктор по-умолчанию.
     * rgb (0, 0, 0)
     */
    Rgb() = default;
    /**
     * Конструктор
     */
    Rgb(uint8_t red, uint8_t green, uint8_t blue):
        red(red),
        green(green),
        blue(blue) { }
    /**
     * Интенсивность красного (0-255)
     */
    uint8_t red = 0;
    /**
     * Интенсивность зеленого (0-255)
     */
    uint8_t green = 0;
    /**
     * Интенсивность синего (0-255)
     */
    uint8_t blue = 0;
};
/**
 * Цветовая модель RGBA
 */
struct Rgba : public Rgb {
    /**
     * Конструктор по-умолчанию.
     * rgba (0, 0, 0, 1.0)
     */
    Rgba() :
        Rgb(),
        opacity(1.0) { }
    /**
     * Конструктор
     */
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double alpha) :
        Rgb(red, green, blue),
        opacity(alpha) { }
    /**
     * Непрозрачность
     */
    double opacity = 1.0;
};
/**
 * Значение цвета.
 * Здесь задаем поддерживаемые типы данных для значения
 */
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
/**
 * Значение цвета - бесцветный
 */
inline const Color NoneColor{ std::monostate() };
/**
 * Перегрузка передачи цвета в поток вывода
 */
std::ostream& operator<<(std::ostream& out, const Color& color);
/**
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {

    RenderContext(std::ostream& out) :
        out(out) { }

    RenderContext(std::ostream& out, int indent_step, int indent = 0) :
        out(out),
        indent_step(indent_step),
        indent(indent) { }
    /**
     * Возвращает новый контекст вывода с увеличенным смещением
     */
    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }
    /**
     * Добавление отступов в соответствии с контекстом
     */
    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    /**
     * Поток вывода
     */
    std::ostream& out;
    /**
     * Шаг отступа
     */
    int indent_step = 0;
    /**
     * Отступ
     */
    int indent = 0;
};
/**
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    /**
     * Вывод содержимого тэга объекта
     */
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;
private:
    /**
     * Вывод содержимого тэга подкласса объекта.
     * Необходимо переопределить в наследованном классе.
     */
    virtual void RenderObject(const RenderContext& context) const = 0;
};
/**
 * Свойство определяет как будут выглядеть концы линий.
 */
enum class StrokeLineCap {
    /**
     * Встык
     */
    BUTT,
    /**
     * Скругленные
     */
    ROUND,
    /**
     * Квадратные
     */
    SQUARE,
};
/**
 * Перегрузка передачи вида конца линий в поток вывода
 */
std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
/**
 * Свойство определяет как будут выглядеть соединения линий на углах
 */
enum class StrokeLineJoin {
    /**
     * Дуга
     */
    ARCS,
    /**
     * Скошенный
     */
    BEVEL,
    /**
     * Под углом 45°
     */
    MITER,
    /**
     * Сглаженный под 45°
     */
    MITER_CLIP,
    /**
     * Скругленный
     */
    ROUND,
};
/**
 * Перегрузка передачи вида соединений линий в поток вывода
 */
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);
/**
 * Путь, представленный в виде последовательности различных контуров векторный объект,
 * который будет содержит свойства, управляющие параметрами заливки и контура
 */
template <typename Owner>
class PathProps {
public:
    /**
     * Установить цвет заливки
     */
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    /**
     * Установить цвет обводки
     */
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    /**
     * Установить толщину обводки
     */
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
    /**
     * Установить вид концов линий.
     */
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }
    /**
     * Установить вид соединения линий.
     */
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;
    /**
     * Передача свойств в поток вывода
     */
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        using utils::RenderOptionalAttr;
        RenderOptionalAttr(out, "fill"sv, fill_color_);
        RenderOptionalAttr(out, " stroke"sv, stroke_color_);
        RenderOptionalAttr(out, " stroke-width"sv, stroke_width_);
        RenderOptionalAttr(out, " stroke-linecap"sv, stroke_line_cap_);
        RenderOptionalAttr(out, " stroke-linejoin"sv, stroke_line_join_);
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};
/**
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    /**
     * Задать координаты центра окружности
     */
    Circle& SetCenter(Point center);
    /**
     * Задать радиус окружности
     */
    Circle& SetRadius(double radius);
private:
    /**
     * Вывод содержимого тэга подкласса объекта.
     */
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};
/**
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    /**
     * Добавить очередную вершину к ломаной линии
     */
    Polyline& AddPoint(Point point);
private:
    /**
     * Вывод содержимого тэга подкласса объекта.
     */
    void RenderObject(const RenderContext& context) const override;
    /**
     * Вершины ломаной линии
     */
    std::vector<Point> points_;
};
/**
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    /**
     * Задаёт координаты опорной точки (атрибуты x и y)
     */
    Text& SetPosition(Point pos);
    /**
     * Задаёт смещение относительно опорной точки (атрибуты dx, dy)
     */
    Text& SetOffset(Point offset);
    /**
     * Задаёт размеры шрифта (атрибут font-size)
     */
    Text& SetFontSize(uint32_t size);
    /**
     * Задаёт название шрифта (атрибут font-family)
     */
    Text& SetFontFamily(std::string font_family);
    /**
     * Задаёт толщину шрифта (атрибут font-weight)
     */
    Text& SetFontWeight(std::string font_weight);
    /**
     * Задаёт текстовое содержимое объекта (отображается внутри тега text)
     */
    Text& SetData(std::string data);
private:
    /**
     * Вывод содержимого тэга подкласса объекта.
     */
    void RenderObject(const RenderContext& context) const override;
    /**
     * Координаты опорной точки
     */
    Point pos_ = { 0.0, 0.0 };
    /**
     * Смещение относительно опорной точки
     */
    Point offset_ = { 0.0, 0.0 };
    /**
     * Размер шрифта
     */
    uint32_t size_ = 1;
    /**
     * Название шрифта
     */
    std::string font_family_;
    /**
     * Толщина шрифта
     */
    std::string font_weight_;
    /**
     * Текстовое содержимое объекта
     */
    std::string data_;
};
/**
 * ObjectContainer задаёт интерфейс для доступа к контейнеру SVG-объектов.
 * Через этот интерфейс Drawable-объекты могут визуализировать себя,
 * добавляя в контейнер SVG-примитивы
 */
class ObjectContainer {
public:
    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    ~ObjectContainer() = default;
};
/**
 * Интерфейс Drawable унифицирует работу с объектами, которые можно нарисовать,
 * подключив SVG-библиотеку. Для этого в нём есть метод Draw, принимающий ссылку
 * на интерфейс ObjectContainer
 */
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void Draw(ObjectContainer& container) const = 0;
};
/**
 * SVG-Документ, реализующий интерфейс для доступа к контейнеру SVG-объектов.
 */
class Document : public ObjectContainer {
public:
    /**
     * Добавляет в svg-документ объект-наследник svg::Object
     */
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    /**
     * Выводит в ostream svg-представление документа
     */
    void Render(std::ostream& out) const;
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg
