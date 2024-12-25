#include "ShapeUtils.h"

namespace ShapeUtils {

    double CalculateShapeArea(FLine* shape, bool global) {
        double result = 0;
        for (size_t i = 0; i < SIZE(shape->coordinates); ++i) {
            int32_t next = (i + 1) % SIZE(shape->coordinates);
            VECTOR2D pCurrent = global ? VECTOR2D(shape->coordinates[i]->globalPosition.latitude, shape->coordinates[i]->globalPosition.longitude) : shape->coordinates[i]->localPosition;
            VECTOR2D pNext = global ? VECTOR2D(shape->coordinates[next]->globalPosition.latitude, shape->coordinates[next]->globalPosition.longitude) : shape->coordinates[next]->localPosition;

            result += (pNext.X - pCurrent.X) * (pNext.Y + pCurrent.Y);
        }
        return result;
    }

    bool CalculateShapeOrientation(FLine* shape) 
    {
        double result = CalculateShapeArea(shape, false);
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
