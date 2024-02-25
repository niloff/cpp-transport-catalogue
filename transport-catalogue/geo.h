#pragma once

#include <cmath>
#include <string>

namespace geo {
/**
 * Радиус Земли
 */
static constexpr int EARTH_RADIUS = 6371000;
/**
 * Величина 1 / радиан
 */
static const double DR = 3.1415926535 / 180.;
/**
 * Географические координаты
 */
struct Coordinates {
    /**
     * Широта
     */
    double lat;
    /**
     * Долгота
     */
    double lng;
    // Перегрузка операторов
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};
/**
 * Хэш географических координат
 */
struct CoordinatesHash {
    /**
     * Хэш географических координат
     */
    size_t operator() (const Coordinates& coordinates) const noexcept;
private:
    std::hash<double> hasher_;
};
/**
 * Вычислить расстояние между двумя точками с географическими координатами
 */
double ComputeDistance(Coordinates from, Coordinates to);

}
