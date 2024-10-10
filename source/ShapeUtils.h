#pragma once

#include "FTileMapData.h"

namespace ShapeUtils {

bool CalculateShapeOrientation(FLine* shape);
bool IsPointInShape(FLine* shape, VECTOR2D point);

}