#pragma once

#include "LatLong.h"

class TileUtils {
public:
	static int LongitudeToTileX(double lon, int z);
	static int LatitudeToTileY(double lat, int z);
	static LatLong TileToLatLong(int x, int y, int z);
};