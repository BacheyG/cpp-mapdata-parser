#pragma once

#include <string>
#include <vector>

// Export macro for cross-platform compatibility
#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

struct BuildingEnvironmentData {
    double latitude;
    double longitude;
    double buildingArea;
    int buildingKind;
    int buildingLanduseKind;
    bool isHeightKnown;
    double height;
    bool isRoofShapeKnown;
    int roofShape;
    int buildingCountNearby;
    double averageBuildingAreaNearby;
    BuildingEnvironmentData() : latitude(-1), longitude(-1), buildingArea(-1), buildingKind(0), buildingLanduseKind(0),
        isHeightKnown(false), height(0), isRoofShapeKnown(false), roofShape(0),
        buildingCountNearby(-1), averageBuildingAreaNearby(-1) {}
};

extern "C" {
    // Function to get a latitude/longitude pair for a building ID from its XML data
    EXPORT double* get_building_location(const char* osmData);

    // Function to get building count for a tile
    EXPORT BuildingEnvironmentData get_building_environment_data(const char* osmData, int64_t buildingId);

    // Function to get building tile location
    EXPORT double* get_building_tile_bounds(const char* osmData);
}
