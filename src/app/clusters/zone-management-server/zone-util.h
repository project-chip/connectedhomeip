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

            for (size_t j = i + 1; j < vertexCount; ++j)
            {
                TwoDCartesianVertexStruct p2 = vertices[j];
                TwoDCartesianVertexStruct q2 = vertices[(j + 1) % vertexCount];

                // Skip segments that share an endpoint (i.e., adjacent segments)
                bool segments_are_adjacent = false;
                if (AreTwoDCartVerticesEqual(p1, p2) || AreTwoDCartVerticesEqual(p1, q2) || AreTwoDCartVerticesEqual(q1, p2) ||
                    AreTwoDCartVerticesEqual(q1, q2))
                {
                    segments_are_adjacent = true;
                }

                // Also a logical adjacency check for sequence
                if ((j == (i + 1) % vertexCount) || (i == (j + 1) % vertexCount))
                {
                    segments_are_adjacent = true;
                }

                if (!segments_are_adjacent)
                {
                    if (DoSegmentsIntersect(p1, q1, p2, q2))
                    {
                        return true; // Found a self-intersection
                    }
                }
            }
        }

        return false; // No self-intersection found
    }

private:
    // Helper function: Check if point q lies on segment pr (assuming p, q, r are collinear)
    // Checks if q is within the bounding box defined by p and r.
    static bool OnSegment(const TwoDCartesianVertexStruct & p, const TwoDCartesianVertexStruct & q,
                          const TwoDCartesianVertexStruct & r)
    {
        return q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y);
    }

    // Helper function: Determine the orientation of the ordered triplet (p, q, r) of vertices.
    // Employ the cross-product computation to find the direction of the Z axis.
    // Using the right-hand rule Z > 0 means pointing upward from the 2D
    // plane(counter-clockwise rotation) and Z < 0 means pointing downward into
    // the 2D plane(clock-wise rotation).
    // 0 --> p, q and r are collinear
    // 1 --> Counterclockwise
    // 2 --> Clockwise
    static int Orientation(const TwoDCartesianVertexStruct & p, const TwoDCartesianVertexStruct & q,
                           const TwoDCartesianVertexStruct & r)
    {
        long long val = static_cast<long long>((q.y - p.y) * (r.x - q.x)) - static_cast<long long>((q.x - p.x) * (r.y - p.y));

        if (val == 0) // Collinear
        {
            return 0;
        }

        return (val > 0) ? 1 : 2; // Counterclockwise or Clockwise
    }

    // Helper function: Check if segment p1q1 intersects segment p2q2
    static bool DoSegmentsIntersect(const TwoDCartesianVertexStruct & p1, const TwoDCartesianVertexStruct & q1,
                                    const TwoDCartesianVertexStruct & p2, const TwoDCartesianVertexStruct & q2)
    {
        int o1 = Orientation(p1, q1, p2);
        int o2 = Orientation(p1, q1, q2);
        int o3 = Orientation(p2, q2, p1);
        int o4 = Orientation(p2, q2, q1);

        // General case
        if (o1 != 0 && o2 != 0 && o3 != 0 && o4 != 0 && o1 != o2 && o3 != o4)
        {
            return true;
        }

        // Special Cases (Collinear points)
        if (o1 == 0 && OnSegment(p1, p2, q1))
        {
            return true; // p1, q1 and p2 are collinear and p2 lies on segment p1q1
        }
        if (o2 == 0 && OnSegment(p1, q2, q1))
        {
            return true; // p1, q1 and q2 are collinear and q2 lies on segment p1q1
        }
        if (o3 == 0 && OnSegment(p2, p1, q2))
        {
            return true; // p2, q2 and p1 are collinear and p1 lies on segment p2q2
        }
        if (o4 == 0 && OnSegment(p2, q1, q2))
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
