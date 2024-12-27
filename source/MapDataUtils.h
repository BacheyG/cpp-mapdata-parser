// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "type_defines.h"
#include "FTileMapData.h"

class MapDataUtils
{
public:
	static constexpr int32_t k_tileSizeWorld = 30000;
	static bool ProcessMapDataFromGeoJson(const STRING& mapDataJson, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY, int32_t zoom);
	static bool ProcessMapDataFromOsm(const STRING& mapDataOsm, FTileMapData* parsedMapData, int32_t tileX = 0, int32_t tileY = 0, int32_t zoom = 14);
	static STRING LanduseKindToString(LanduseKind kind);
	static PathType StringToPathType(const STRING& typeStr);
	static PathSurfaceMaterial StringToPathSurfaceMaterial(const STRING& materialStr);
	static LanduseKind StringToLanduseKind(const STRING& landuseStr);
	static BuildingKind StringToBuildingKind(const STRING& buildingStr);
	static RoofShape StringToRoofShape(const STRING& roofStr);
	static MaterialProperty StringToMaterial(const STRING& materialStr);
	static ColorProperty StringToColor(const STRING& colorStr);
};
