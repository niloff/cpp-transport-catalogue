#pragma once

#include <cmath>
#include <string>

namespace geo {
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
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}

}
