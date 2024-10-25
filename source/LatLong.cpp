
#include "LatLong.h"

#include <limits>

LatLong::LatLong() : latitude(0.0), longitude(0.0) {}

LatLong::LatLong(double latitude, double longitude) : latitude(latitude), longitude(longitude) {}

bool LatLong::Equals(const LatLong& other) const {
    const double tolerance = std::numeric_limits<double>::epsilon() * 100;
    return std::abs(latitude - other.latitude) < tolerance
        && std::abs(longitude - other.longitude) < tolerance;
}