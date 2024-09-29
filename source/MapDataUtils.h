// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "type_defines.h"
#include "FTileMapData.h"

class MapDataUtils
{
public:
	static constexpr int32_t k_tileSizeWorld = 30000;
	static bool ProcessMapDataFromGeoJson(const STRING& mapDataJson, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY);
	static bool ProcessMapDataFromOsm(const STRING& mapDataOsm, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY);
};
