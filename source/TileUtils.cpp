#include "TileUtils.h"
#include "type_defines.h"

int TileUtils::LongitudeToTileX(double lon, int z)
{
	return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
}

int TileUtils::LatitudeToTileY(double lat, int z)
{
	double latrad = lat * PI / 180.0;
	return (int)(floor((1.0 - asinh(tan(latrad)) / PI) / 2.0 * (1 << z)));
}

LatLong TileUtils::TileToLatLong(int x, int y, int z) {
	double n = PI - 2.0 * PI * y / (double)(1 << z);
	double latitude = 180.0 / PI * atan(0.5 * (exp(n) - exp(-n)));
	double longitude = x / (double)(1 << z) * 360.0 - 180;
	return { latitude, longitude };
}
