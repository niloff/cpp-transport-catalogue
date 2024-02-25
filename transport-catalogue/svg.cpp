#include "svg.h"

namespace svg {

using namespace std::literals;

namespace  {

void PrintColor(std::ostream& out, std::monostate) {
    out << "none"s;
}

void PrintColor(std::ostream& out, const std::string& value) {
    out << value;
}

void PrintColor(std::ostream& out, Rgb rgb) {
    out << "rgb("sv << static_cast<int>(rgb.red)
        << ',' << static_cast<int>(rgb.green)
        << ',' << static_cast<int>(rgb.blue) << ')';
}

void PrintColor(std::ostream& out, Rgba rgba) {
    out << "rgba("sv << static_cast<int>(rgba.red)
        << ',' << static_cast<int>(rgba.green)
        << ',' << static_cast<int>(rgba.blue)
        << ',' << rgba.opacity << ')';
}


}
/**
 * Перегрузка передачи цвета в поток вывода
 */
std::ostream& operator<<(std::ostream& out, const Color &color) {
    std::visit([&out](const auto& value) {
        PrintColor(out, value);
    },
    color);
    return out;
}
/**
 * Перегрузка передачи вида конца линий в поток вывода
 */
std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}
/**
 * Перегрузка передачи вида соединений линий в поток вывода
 */
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}
/**
 * Вывод содержимого тэга объекта
 */
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    // Делегируем вывод тега своим подклассам
    RenderObject(context);
    context.out << std::endl;
}

// ---------- Circle ------------------
/**
 * Задать координаты центра окружности
 */
Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}
/**
 * Задать радиус окружности
 */
Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}
/**
 * Вывод содержимого тэга подкласса объекта.
 */
void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(out);
    out << "/>"sv;
}
/**
 * Добавить очередную вершину к ломаной линии
 */
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}
/**
 * Вывод содержимого тэга подкласса объекта.
 */
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const auto& point : points_) {
        if (is_first) {
            is_first = false;
        } else {
            out << ' ';
        }
        out << point.x << ',' << point.y;
    }
    out << "\" "sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(out);
    out << "/>"sv;
}

/**
 * Задаёт координаты опорной точки (атрибуты x и y)
 */
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}
/**
 * Задаёт смещение относительно опорной точки (атрибуты dx, dy)
 */
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
/**
 * Задаёт размеры шрифта (атрибут font-size)
 */
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}
/**
 * Задаёт название шрифта (атрибут font-family)
 */
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}
/**
 * Задаёт толщину шрифта (атрибут font-weight)
 */
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}
/**
 * Задаёт текстовое содержимое объекта (отображается внутри тега text)
 */
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(out);
    using utils::RenderAttr;
    RenderAttr(out, " x"sv, pos_.x);
    RenderAttr(out, " y"sv, pos_.y);
    RenderAttr(out, " dx"sv, offset_.x);
    RenderAttr(out, " dy"sv, offset_.y);
    RenderAttr(out, " font-size"sv, size_);
    if (!font_family_.empty()) {
        RenderAttr(out, " font-family"sv, font_family_);
    }
    if (!font_weight_.empty()) {
        RenderAttr(out, " font-weight"sv, font_weight_);
    }
    out.put('>');
    utils::HtmlEncodeString(out, data_);
    out << "</text>"sv;
}
/**
 * Добавляет в svg-документ объект-наследник svg::Object
 */
void Document::AddPtr(std::unique_ptr<Object>&& object) {
    objects_.push_back(std::move(object));
}
/**
 * Выводит в ostream svg-представление документа
 */
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

namespace utils {
/**
 * Кодировать строку в соответствии с html
 */
void HtmlEncodeString(std::ostream& out, std::string_view sv) {
    for (char c : sv) {
        switch (c) {
        case '"':
            out << "&quot;"sv;
            break;
        case '<':
            out << "&lt;"sv;
            break;
        case '>':
            out << "&gt;"sv;
            break;
        case '&':
            out << "&amp;"sv;
            break;
        case '\'':
            out << "&apos;"sv;
            break;
        default:
            out.put(c);
        }
    }
}

}  // namespace utils

}  // namespace svg
