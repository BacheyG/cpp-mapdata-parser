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
    double buildingArea;
    int buildingLanduseKind;
    int buildingCountInTile;
    double averageBuildingAreaInTile;
};

extern "C" {
    // Function to get a latitude/longitude pair for a building ID from its XML data
    EXPORT double* get_building_location(const char* osmData);

    // Function to get building count for a tile
    EXPORT BuildingEnvironmentData get_building_environment_data(const char* osmData);

    // Function to get average building size for a tile
    EXPORT int* get_coords_to_tile(double lat, double lon);
}
