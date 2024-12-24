#include "TileBuildingDataUtils.hpp"

#include "TileUtils.h"

static constexpr int kZoomLevel = 14;

int get_building_count(const std::string& osmData) {
    return 42; // Example output; replace with actual logic
}

int* get_coords_to_tile(double lat, double lon) {
    int tileX = TileUtils::LongitudeToTileX(lon, kZoomLevel);
    int tileY = TileUtils::LatitudeToTileY(lat, kZoomLevel);
    int result[2];
    result[0] = tileX;
    result[1] = tileY;
    return result;
}