#include "ShapeUtils.h"

namespace ShapeUtils {

    bool CalculateShapeOrientation(FLine* shape) 
    {
		float result = 0;
		for (int i = 0; i < SIZE(shape->coordinates); ++i) {
			int32_t next = (i + 1) % SIZE(shape->coordinates);
			result += (shape->coordinates[next]->localPosition.X - shape->coordinates[i]->localPosition.X)
				    * (shape->coordinates[next]->localPosition.Y + shape->coordinates[i]->localPosition.Y);
		}
		return result > 0;
	}

    bool IsPointInShape(FLine* shape, VECTOR2D point) 
    {
        bool result = false;
        int numVertices = SIZE(shape->coordinates);

        // Loop through each edge of the polygon
        for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
            VECTOR2D currentPoint = shape->coordinates[i]->localPosition;
            VECTOR2D previousPoint = shape->coordinates[j]->localPosition;

            // Check if point is within the Y-range of the current edge
            if ((currentPoint.Y > point.Y) != (previousPoint.Y > point.Y) &&
                // Check if point is to the left of the edge
                (point.X < (previousPoint.X - currentPoint.X) *
                    (point.Y - currentPoint.Y) / (previousPoint.Y - currentPoint.Y) + currentPoint.X)) {
                result = !result; // Toggle result
            }
        }

        return result;
    }

}
