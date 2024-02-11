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
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};
/**
 * Хэш географических координат
 */
struct CoordinatesHash {
    size_t operator() (const Coordinates& coordinates) const noexcept {
        return hasher_(coordinates.lat) + hasher_(coordinates.lng) * 37;
    }
private:
    std::hash<double> hasher_;
};
/**
 * Вычислить расстояние между двумя точками с географическими координатами
 */
inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    return acos(sin(from.lat * DR) * sin(to.lat * DR)
                + cos(from.lat * DR) * cos(to.lat * DR) * cos(abs(from.lng - to.lng) * DR))
        * EARTH_RADIUS;
}

}
