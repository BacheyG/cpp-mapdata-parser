#pragma once

#include <string>
#include <vector>

// Export macro for cross-platform compatibility
#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" {
    // Function to get building count for a tile
    EXPORT int get_building_count(const std::string& osmData);

    // Function to get average building size for a tile
    EXPORT int* get_coords_to_tile(double lat, double lon);
}
