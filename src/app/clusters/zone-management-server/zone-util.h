/*
 * Utility Class providing functionality around various checks required for
 * zones, such as self-intersection, duplicates, etc.
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

enum class OrientationEnum : uint8_t
{
    kCollinear        = 0x00,
    kCounterClockwise = 0x01,
    kClockwise        = 0x02,
};

class ZoneUtil
{
public:
    using TwoDCartesianVertexStruct = Structs::TwoDCartesianVertexStruct::Type;

    static bool AreTwoDCartVerticesEqual(const TwoDCartesianVertexStruct & v1, const TwoDCartesianVertexStruct & v2)
    {
        return v1.x == v2.x && v1.y == v2.y;
    }

    // Method to check for self-intersecting zones
    static bool IsZoneSelfIntersecting(const std::vector<TwoDCartesianVertexStruct> & vertices)
    {
        size_t vertexCount = vertices.size();

        // Iterate through all pairs of non-adjacent segments
        for (size_t i = 0; i < vertexCount; ++i)
        {
            TwoDCartesianVertexStruct p1 = vertices[i];
            TwoDCartesianVertexStruct q1 = vertices[(i + 1) % vertexCount];

            // j starts from i+2, skipping the adjacent vertex i+1.
            for (size_t j = i + 2; j < vertexCount; ++j)
            {
                TwoDCartesianVertexStruct p2 = vertices[j];
                TwoDCartesianVertexStruct q2 = vertices[(j + 1) % vertexCount];

                // Skip segments that share an endpoint (i.e., adjacent segments)
                if (AreTwoDCartVerticesEqual(p1, p2) || AreTwoDCartVerticesEqual(p1, q2) || AreTwoDCartVerticesEqual(q1, p2) ||
                    AreTwoDCartVerticesEqual(q1, q2))
                {
                    continue;
                }

                if (DoSegmentsIntersect(p1, q1, p2, q2))
                {
                    return true; // Found a self-intersection
                }
            }
        }

        return false; // No self-intersection found
    }

private:
    // Helper function: Check if point q lies on segment pr (assuming p, q, r are collinear)
    // Checks if q is within the bounding box defined by p and r.
    // Ensures that the coordinates of q are greater than the min and lesser
    // than the max of p and r.
    static bool OnSegment(const TwoDCartesianVertexStruct & p, const TwoDCartesianVertexStruct & q,
                          const TwoDCartesianVertexStruct & r)
    {
        return q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y);
    }

    // Helper function: Determine the orientation of the ordered triplet (p, q, r) of vertices.
    // Employ the cross-product computation(Determinant of the coordinate matrix) of
    // vectors pq and pr to find the direction of the Z axis.
    // The sign of the Determinant is used to infer the direction of cross-product vector
    // and hence the orientation of the 3 coordinates. A value of 0 indicates that the points
    // are collinear.
    // Using the right-hand rule Z > 0 means pointing upward from the 2D
    // plane(counter-clockwise rotation) and Z < 0 means pointing downward into
    // the 2D plane(clock-wise rotation).
    // 0 --> p, q and r are collinear
    // 1 --> Counterclockwise
    // 2 --> Clockwise
    static OrientationEnum CrossProduct(const TwoDCartesianVertexStruct & p, const TwoDCartesianVertexStruct & q,
                                        const TwoDCartesianVertexStruct & r)
    {
        long long val = static_cast<long long>((q.x - p.x) * (r.y - p.y)) - static_cast<long long>((q.y - p.y) * (r.x - p.x));

        if (val == 0) // Collinear
        {
            return OrientationEnum::kCollinear;
        }

        return (val > 0) ? OrientationEnum::kCounterClockwise : OrientationEnum::kClockwise; // Counterclockwise or Clockwise
    }

    // Helper function: Check if segment p1q1 intersects segment p2q2
    static bool DoSegmentsIntersect(const TwoDCartesianVertexStruct & p1, const TwoDCartesianVertexStruct & q1,
                                    const TwoDCartesianVertexStruct & p2, const TwoDCartesianVertexStruct & q2)
    {
        // Segment 1(p1q1) and first vertex of Segment 2(p2)
        OrientationEnum o1 = CrossProduct(p1, q1, p2);
        // Segment 1(p1q1) and second vertex of Segment 2(q2)
        OrientationEnum o2 = CrossProduct(p1, q1, q2);
        // Segment 2(p2q2) and first vertex of Segment 1(p1)
        OrientationEnum o3 = CrossProduct(p2, q2, p1);
        // Segment 2(p2q2) and second vertex of Segment 1(q1)
        OrientationEnum o4 = CrossProduct(p2, q2, q1);

        // General case
        if (o1 != OrientationEnum::kCollinear && o2 != OrientationEnum::kCollinear && o3 != OrientationEnum::kCollinear &&
            o4 != OrientationEnum::kCollinear && o1 != o2 && o3 != o4)
        {
            return true;
        }

        // Special Cases (Collinear points)
        if (o1 == OrientationEnum::kCollinear && OnSegment(p1, p2, q1))
        {
            return true; // p1, q1 and p2 are collinear and p2 lies on segment p1q1
        }
        if (o2 == OrientationEnum::kCollinear && OnSegment(p1, q2, q1))
        {
            return true; // p1, q1 and q2 are collinear and q2 lies on segment p1q1
        }
        if (o3 == OrientationEnum::kCollinear && OnSegment(p2, p1, q2))
        {
            return true; // p2, q2 and p1 are collinear and p1 lies on segment p2q2
        }
        if (o4 == OrientationEnum::kCollinear && OnSegment(p2, q1, q2))
        {
            return true; // p2, q2 and q1 are collinear and q1 lies on segment p2q2
        }

        return false;
    }
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
