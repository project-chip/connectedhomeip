/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/*
 * Utility Class providing functionality around various checks required for
 * zones, such as self-intersection, duplicates, etc.
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <set>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

enum class OrientationEnum : uint8_t
{
    kCollinear = 0x00,
    kOnLeft    = 0x01,
    kOnRight   = 0x02,
};

class ZoneGeometry
{
public:
    using TwoDCartesianVertexStruct = Structs::TwoDCartesianVertexStruct::Type;

    static bool AreTwoDCartVerticesEqual(const TwoDCartesianVertexStruct & v1, const TwoDCartesianVertexStruct & v2)
    {
        return v1.x == v2.x && v1.y == v2.y;
    }

    /**
     * The Spec requires zones to be a simple polygon.  That is a piecewise-linear
     * continuous map from a circle to the plane (a loop) that is not
     * self-intersecting (i.e. is a 1-1 or injective map).
     *
     * Being a map from a circle, continuity, and piecewise-linearity are guaranteed
     * by the way we define the zone in terms of an ordered set of vertices that we connect in
     * a cycle, so we only need to check that the map is 1-1.
     *
     * Algorithm for Checking Self-Intersection of a Zone
     * --------------------------------------------------
     *
     * 1. If any vertex appears more than once in the list of vertices, the mapping
     *    is not 1-1. So, the zone is self-intersecting.
     * 2. For a polygon of 3 vertices, flag it as self-intersecting if all
     *    vertices are collinear. Otherwise, it is a simple triangle and a valid
     *    polygon.
     * 3. For all polygons of > 3 vertices, go through all pairs of non-adjacent edges
     *    p1q1 and p2q2.  If any non-adjacent edges intersect, the zone is
     *    self-intersecting. If all such pairs do not intersect, the zone is not
     *    self-intersecting.
     *
     * Algorithm for SegmentsIntersect(p1q1, p2q2)
     * -------------------------------------------
     * 1. If points p1 and q1 lie on the opposite sides of segment p2q2 AND points p2
     *    and q2 lie on opposite sides of segment p1q1, the segments intersect.
     *    Orientation(pq, r) is used to determine which side of the vector pq point r
     *    lies on, or whether it lies on the line containing pq.
     * 2. If either endpoint of one segment lies on the other segment, the segments
     *    intersect.
     * 3. In all other cases, the segments do not intersect.
     *
     * Algorithm for Orientation(pq, r)
     * ----------------------------------
     * 1. Compute cross-product of vectors pq and pr.
     * 2. If cross-product > 0:
     *      Point r lies to the left of ray pq (Counter-clockwise from pq to pr)
     *    Else if cross-product < 0:
     *      Point r lies to the right of ray pq (Clockwise from pq to pr)
     *    Else
     *      Point r is collinear with pq
     */

    /**
     * Proof of Correctness of Overall Self-Intersection Algorithm
     * -----------------------------------------------------------
     *
     * 1. As noted in the algorithm description, if a vertex is repeated the zone is
     *    self-intersecting.  In what follows, we can, therefore, assume no repeated
     *    vertices and no zero-length edges(consecutive repeated vertices).
     * 2. For the 3-vertex case, since the edges are all adjacent to each other, the
     *    only way for two edges to intersect (i.e. share a point other than the vertex
     *    where they are adjacent), is for them to overlap. But then all three of the
     *    vertices have to be collinear, so it's enough to check for that to detect
     *    self-intersections.
     * 3. When there are 4 or more distinct vertices in the polygon, let's consider
     *    the case when two adjacent edges intersect.  As noted in point 2, the edges
     *    must overlap.  Since there are no repeated vertices, the two adjacent edges
     *    must have different lengths.  Let 'l' denote the longer edge and 's'
     *    denote the shorter edge. The non-shared end vertex of 's', thus, lies on 'l'.
     *    Let this point be P. Given the polygon is a cycle with each vertex having
     *    degree 2, there must be two edges that meet at P. One of them is 's';
     *    Let's call the other one 't'. Since there are at least 4 vertices, there
     *    are also at least 4 edges. Because 's' and 't' are adjacent, 't' is not
     *    adjacent to 'l' (otherwise, there would only be 3 edges!). So, in this case,
     *    we have non-adjacent edges 't' and 'l' that intersect (at point P).
     *
     *    Therefore, if there are any self-intersection in a polygon with 4 or more
     *    vertices, there must be intersecting non-adjacent edges.
     */

    /*
     * Proofs of correctness for SegmentsIntersect and Orientation
     * -----------------------------------------------------------
     *
     * 1. Correctness of SegmentsIntersect(p1q1, p2q2) :
     *    a. If points p1 and q1 lie on opposite sides of p2q2, then segment p1q1
     *       intersects the line containing p2q2.  Similarly, if points p2 and q2 lie
     *       on opposite sides of p1q1, then the segment p2q2 intersects the line
     *       containing p1q1.  Since the two lines can only intersect at one point,
     *       this point must also lie on both segments, and the two segments intersect.
     *    b. If the endpoint of one of the segments lies on the other segment, then
     *       they intersect at that point.
     *    c. If the two segments intersect at all at some point P, then either P
     *       is the endpoint of one of the segments (and thus we are in
     *       the case described in (b)), or it's in the interior of both segments.
     *       If P is in the interior of both segments and the segments are not
     *       collinear, then we are in the case described in (a).  If the two
     *       segments _are_ collinear and have a shared point P that is not an
     *       endpoint, then the endpoint (p1, q1, p2, or q2) closest to P has to lie
     *       on both segments and hence we are in the case described in (b).
     *
     * 2. Correctness of Orientation(p, q, r):
     *    a. Compute cross-product of vectors pq and pr.
     *       --The sign determines the direction of thumb in the right hand
     *         rule of turning from vector pq to pr.
     *    b. If cross-product > 0, r lies to the left of ray
     *       pq (Counter-clockwise turn from vector pq to pr)
     *    c. If cross-product < 0, r lies to the right of ray
     *       pq (Clockwise turn from vector pq to pr)
     *    d. If cross-product is 0, p, q, r are collinear.
     *
     */

    // Method to check for self-intersecting zones
    static bool IsZoneSelfIntersecting(const std::vector<TwoDCartesianVertexStruct> & vertices)
    {
        size_t vertexCount = vertices.size();

        // Check if there are duplicate vertices in the polygon
        if (ZoneHasDuplicates(vertices))
        {
            ChipLogDetail(Zcl, "Zone has duplicate vertices");
            return true;
        }

        // If there are only 3 vertices, it's enough to check whether they are collinear.
        if (vertexCount == 3)
        {
            if (Orientation(vertices[0], vertices[1], vertices[2]) == OrientationEnum::kCollinear)
            {
                ChipLogDetail(Zcl, "Degenerate case of 3 collinear vertices");
                return true;
            }

            return false;
        }

        // Iterate through all pairs of non-adjacent segments
        for (size_t i = 0; i < vertexCount; ++i)
        {
            auto & p1 = vertices[i];
            auto & q1 = vertices[(i + 1) % vertexCount];

            // j starts from i+2, skipping the adjacent edge (vertices[i+1], vertices[i+2])
            for (size_t j = i + 2; j < vertexCount; ++j)
            {
                // Skip edges that share an endpoint (i.e., adjacent edges)
                if ((j + 1) % vertexCount == i)
                {
                    continue;
                }

                auto & p2 = vertices[j];
                auto & q2 = vertices[(j + 1) % vertexCount];

                if (DoSegmentsIntersect(p1, q1, p2, q2))
                {
                    ChipLogDetail(Zcl, "Zone segment[(%u,%u),(%u,%u)] intersects with segment[(%u,%u),(%u,%u)]", p1.x, p1.y, q1.x,
                                  q1.y, p2.x, p2.y, q2.x, q2.y);
                    return true; // Found a self-intersection
                }
            }
        }

        return false; // No self-intersection found
    }

private:
    // Helper function: Check if point q lies on segment pr (assuming p, q, r are collinear)
    // Checks if q is within the bounding box defined by p and r.
    // Returns true if the coordinates of q are greater than the min and smaller
    // than the max of the corresponding coordinates of p and r.
    static bool OnSegment(const TwoDCartesianVertexStruct & p, const TwoDCartesianVertexStruct & q,
                          const TwoDCartesianVertexStruct & r)
    {
        return q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y);
    }

    // Helper function: Determine the orientation of the point r with respect to ray pq: in
    // the left half-plane when facing in the direction of the ray, in the right half-plane,
    // or collinear with the ray.
    // Employs the cross-product computation (determinant of the coordinate matrix) of
    // vectors pq and pr to find the direction of the Z axis. The rotation
    // considered is from vector pq to pr.
    // The sign of the determinant is used to infer the direction of the cross-product vector
    // and hence the orientation of the point with respect to the ray. A value of 0 indicates that the points
    // are collinear.
    // Using the right-hand rule Z > 0 means pointing upward from the 2D
    // plane (counter-clockwise rotation) and Z < 0 means pointing downward into
    // the 2D plane (clock-wise rotation).
    // 0 --> p, q and r are collinear
    // 1 --> Counterclockwise
    // 2 --> Clockwise
    static OrientationEnum Orientation(const TwoDCartesianVertexStruct & p, const TwoDCartesianVertexStruct & q,
                                       const TwoDCartesianVertexStruct & r)
    {
        long long val = (static_cast<long long>(q.x - p.x) * static_cast<long long>(r.y - p.y)) -
            (static_cast<long long>(q.y - p.y) * static_cast<long long>(r.x - p.x));

        if (val == 0) // Collinear
        {
            return OrientationEnum::kCollinear;
        }

        return (val > 0) ? OrientationEnum::kOnLeft : OrientationEnum::kOnRight;
    }

    // Helper function: Check if segment p1q1 intersects segment p2q2
    static bool DoSegmentsIntersect(const TwoDCartesianVertexStruct & p1, const TwoDCartesianVertexStruct & q1,
                                    const TwoDCartesianVertexStruct & p2, const TwoDCartesianVertexStruct & q2)
    {
        // Segment 1 (p1q1) and first vertex of Segment 2 (p2)
        OrientationEnum o1 = Orientation(p1, q1, p2);
        // Segment 1(p1q1) and second vertex of Segment 2(q2)
        OrientationEnum o2 = Orientation(p1, q1, q2);
        // Segment 2(p2q2) and first vertex of Segment 1(p1)
        OrientationEnum o3 = Orientation(p2, q2, p1);
        // Segment 2(p2q2) and second vertex of Segment 1(q1)
        OrientationEnum o4 = Orientation(p2, q2, q1);

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

    struct VertexComparator
    {
        bool operator()(const TwoDCartesianVertexStruct & a, const TwoDCartesianVertexStruct & b) const
        {
            if (a.x != b.x)
            {
                return a.x < b.x;
            }
            return a.y < b.y;
        }
    };

    static bool ZoneHasDuplicates(const std::vector<TwoDCartesianVertexStruct> & zoneVertices)
    {
        std::set<TwoDCartesianVertexStruct, VertexComparator> seenVertices;

        for (const auto & vertex : zoneVertices)
        {
            if (!seenVertices.insert(vertex).second)
            {
                return true;
            }
        }

        return false;
    }
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
