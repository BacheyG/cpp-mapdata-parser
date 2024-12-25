#include "TileBuildingDataUtils.hpp"

#include "MapDataUtils.h"
#include "ShapeUtils.h"
#include "TileUtils.h"

static constexpr int kZoomLevel = 14;

double* get_building_location(const char* osmData) {
    FTileMapData mapData;
    std::string osmDataStr(osmData);
    MapDataUtils::ProcessMapDataFromOsm(osmData, &mapData);
    static double result[3] = { -1, -1, -1 };
    if (SIZE(mapData.buildings) > 0) {
        FLine* mainComponent = mapData.buildings[0]->geometry->GetMainSegment();
        if (mainComponent != nullptr && SIZE(mainComponent->coordinates) > 0) {
            result[0] = mainComponent->coordinates[0]->globalPosition.latitude;
            result[1] = mainComponent->coordinates[0]->globalPosition.longitude;
            result[2] = ShapeUtils::CalculateShapeArea(mainComponent, true);
        }
    }
    return result;
}

BuildingEnvironmentData get_building_environment_data(const char* osmData) {
    BuildingEnvironmentData data;
    data.averageBuildingAreaInTile = 2.5;
    data.buildingArea = 3;
    data.buildingCountInTile = 20;
    data.buildingLanduseKind = 2;
    return data;
}

int* get_coords_to_tile(double lat, double lon) {
    int tileX = TileUtils::LongitudeToTileX(lon, kZoomLevel);
    int tileY = TileUtils::LatitudeToTileY(lat, kZoomLevel);
    int result[2];
    result[0] = tileX;
    result[1] = tileY;
    return result;
}