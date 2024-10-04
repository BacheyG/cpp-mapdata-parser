#include "ShapeUtils.h"
#include "type_defines.h"

namespace ShapeUtils {

	/*void CalculateShapeOrientation(FShape& shape) {
		float result = 0;
		for (int i = 0; i < SIZE(shape.coordinates); ++i) {
			int32_t next = (i + 1) % SIZE(shape.coordinates);
			result += (shape.coordinates[next].localPosition.X - shape.coordinates[i].localPosition.X) * (shape.coordinates[next].localPosition.Y + shape.coordinates[i].localPosition.Y);
		}
		shape.orientation = result > 0;
	}
	*/
}
