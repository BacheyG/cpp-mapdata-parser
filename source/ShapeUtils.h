#pragma once

#include "FTileMapData.h"

namespace ShapeUtils {

double CalculateShapeArea(FLine* shape, bool global = false);
bool CalculateShapeOrientation(FLine* shape);
bool IsPointInShape(FLine* shape, VECTOR2D point);

}