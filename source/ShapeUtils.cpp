#include "ShapeUtils.h"

namespace ShapeUtils {

    bool CalculateShapeOrientation(FLine* shape) {
		float result = 0;
		for (int i = 0; i < SIZE(shape->coordinates); ++i) {
			int32_t next = (i + 1) % SIZE(shape->coordinates);
			result += (shape->coordinates[next].localPosition.X - shape->coordinates[i].localPosition.X) 
				    * (shape->coordinates[next].localPosition.Y + shape->coordinates[i].localPosition.Y);
		}
		return result > 0;
	}
}
