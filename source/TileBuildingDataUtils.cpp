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

BuildingEnvironmentData get_building_environment_data(const char* osmData, int64_t buildingId) {
	BuildingEnvironmentData data;
	FTileMapData mapData;
	std::string osmDataStr(osmData);
	MapDataUtils::ProcessMapDataFromOsm(osmData, &mapData);
	bool foundBuilding = false;
	double averageBuildingAreaNearby = 0.0;
	int buildingCountNearby = 0;
	for (size_t i = 0; i < SIZE(mapData.buildings); ++i) {
		FBuildingData* buildingData = mapData.buildings[i];
		if (buildingId == buildingData->id) {
			FLine* mainComponent = buildingData->geometry->GetMainSegment();
			if (mainComponent != nullptr && SIZE(mainComponent->coordinates) > 0) {
				data.latitude = mainComponent->coordinates[0]->globalPosition.latitude;
				data.longitude = mainComponent->coordinates[0]->globalPosition.longitude;
				data.buildingArea = std::abs(ShapeUtils::CalculateShapeArea(mainComponent, true)) * 1000000;
				data.buildingKind = static_cast<int>(buildingData->kind);
				data.roofShape = static_cast<int>(buildingData->roofShape);
				data.isRoofShapeKnown = buildingData->roofShape != RoofShape::Unknown;
				data.isHeightKnown = buildingData->isHeightKnown;
				data.height = data.isHeightKnown ? buildingData->height : 0;
				data.buildingLanduseKind = static_cast<int>(buildingData->belongingLanduse->kind);
				data.buildingColor = static_cast<int>(buildingData->buildingColor);
				data.isBuildingColorKnown = buildingData->buildingColor != ColorProperty::Unknown;
				data.roofColor = static_cast<int>(buildingData->roofColor);
				data.isRoofColorKnown = buildingData->roofColor != ColorProperty::Unknown;
				foundBuilding = true;
			}
		}
		else {
			FLine* mainComponent = buildingData->geometry->GetMainSegment();
			if (mainComponent != nullptr && SIZE(mainComponent->coordinates) > 0) {
				averageBuildingAreaNearby += std::abs(ShapeUtils::CalculateShapeArea(mainComponent, true)) * 1000000;
				++buildingCountNearby;
			}
		}
	}
	if (!foundBuilding) {
		printf("Building not found in query. This should not happen.");
	}
	if (buildingCountNearby > 0) {
		data.averageBuildingAreaNearby = averageBuildingAreaNearby / buildingCountNearby;
		data.buildingCountNearby = buildingCountNearby;
	}

	return data;
}

double* get_building_tile_bounds(const char* osmData) {
	BuildingEnvironmentData data;
	FTileMapData mapData;
	std::string osmDataStr(osmData);
	MapDataUtils::ProcessMapDataFromOsm(osmData, &mapData);
	double result[4] = { -1, -1, -1, -1 };
	double latitude = 0;
	double longitude = 0;
	if(SIZE(mapData.buildings) > 0 ) {
		FLine* mainSegment = mapData.buildings[0]->geometry->GetMainSegment();
		if (mainSegment != nullptr && SIZE(mainSegment->coordinates) > 0) {
			latitude = mainSegment->coordinates[0]->globalPosition.latitude;
			longitude = mainSegment->coordinates[0]->globalPosition.longitude;
		}
	}
	int tileX = TileUtils::LongitudeToTileX(longitude, kZoomLevel);
	int tileY = TileUtils::LatitudeToTileY(latitude, kZoomLevel);

	LatLong tileCornerLow = TileUtils::TileToLatLong(tileX, tileY, kZoomLevel);
	LatLong tileCornerHigh = TileUtils::TileToLatLong(tileX + 1, tileY + 1, kZoomLevel);
	result[2] = tileCornerLow.latitude;
	result[1] = tileCornerLow.longitude;
	result[0] = tileCornerHigh.latitude;
	result[3] = tileCornerHigh.longitude;
	return result;
}