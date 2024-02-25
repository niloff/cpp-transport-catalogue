#include "domain.h"
/**
 * Сущности транспорта
 */
namespace transport {
/**
 * Хэш остановки
 */
size_t StopHasher::operator() (const Stop& stop) const noexcept {
    return s_hasher_(stop.name) * 37 * 37 + coord_hasher_(stop.coordinates);
}

}
