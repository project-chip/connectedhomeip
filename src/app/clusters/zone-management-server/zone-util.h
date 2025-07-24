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

class ZoneUtil
{
public:
    using TwoDCartesianVertexStruct = Structs::TwoDCartesianVertexStruct::Type;

    static bool AreTwoDCartVerticesEqual(const TwoDCartesianVertexStruct & v1, const TwoDCartesianVertexStruct & v2)
    {
        return v1.x == v2.x && v1.y == v2.y;
    }

    /**
     * Algorithm for Checking Self-Intersection of a Zone
     * --------------------------------------------------
     *
     * 1. If any vertex is repeated in the ordered list of vertices:
     *      a. This causes smaller or no loops in the polygon and creates degenerate
     *         cases violating the definition of a simple polygon.
     *         Return "True" for self-intersection.
     *         Note: We are also simplifying by rejecting consecutive repeated vertices.
     * 2. Go through the list of vertices:
     *      a. Pick unique non-adjacent edges(p1q1 and p2q2) to check for self-intersection.
     *         For a zone with only 3 vertices, where all edges are adjacent to
     *         each other, pick them for checking self-intersection.
     *      b. If SegmentsIntersect(p1q1, p2q2):
     *           i. Return "True" for self-intersection.
     * 3. Return "False" for self-intersection.
     *
     * Algorithm for SegmentsIntersect(p1q1, p2q2)
     * -------------------------------------------
     * 1. If points p1 and q1 lie on the opposite sides of segment p2q2 AND
     *    points p2 and q2 lie on opposite sides of segment p1q1:
     *      a. Return True for segments p1q1 and p2q2 intersecting each other.
     *         This is figured out by evaluating the Orientation(OnLeft,
     *         OnRight, Collinear) of a point 'r' w.r.t a segment 'pq'.
     * 2. If p2 lies on p1q1(collinear and p2 lies on segment p1q1)
     *      a. Return True for segments p1q1 and p2q2 intersecting each other.
     * 3. If q2 lies on p1q1(collinear and q2 lies on segment p1q1)
     *      a. Return True for segments p1q1 and p2q2 intersecting each other.
     * 4. If p1 lies on p2q2(collinear and p1 lies on segment p2q2)
     *      a. Return True for segments p1q1 and p2q2 intersecting each other.
     * 5. If q1 lies on p2q2(collinear and q1 lies on segment p2q2)
     *      a. Return True for segments p1q1 and p2q2 intersecting each other.
     *
     * Algorithm for Orientation(p, q, r)
     * ----------------------------------
     * 1. Compute cross-product of vectors pq and pr.
     * 2. If cross-product > 0:
     *      Point r lies to the left of vector pq(Counter-clockwise from pq to pr)
     *    Else if cross-product < 0:
     *      Point r lies to the right of vector pq(Clockwise from pq to pr)
     *    Else
     *      Point r is collinear with pq
     */

    /**
     * Proof of Correctness
     * --------------------
     *
     * 1. Filtering vertex repetition(including consecutive and non-consecutive)
     *    a. Consecutive repeated vertices(e.g., A, B, B, C.):
     *       If V_i == V_i+1, then we have an edge(V_i, V_i+1) of zero length
     *       and it is a degenerate polygon. Note, we are not collapsing all
     *       consecutive identical vertices into a single one.
     *    b. Non-consecutive repeated vertices(e.g., A, B, C, D, B, E):
     *       If a vertex V appears at index i and again at index k(i < k and not
     *       consecutive), the polygon's path revisits a point before
     *       completing its cycle. In a cycle, the degree of each vertex is 2,
     *       whereas with vertex repetition, there are vertices with higher degree.
     *       This means the polygon's boundary self-intersects, and creates
     *       these smaller loops that degenerates its form.
     *
     * 2. Correctness of SegmentsIntersect(p1q1, p2q2):
     *    a. General case: where points p1 and q1 lie on opposite sides of
     *       p2q2, AND points p2 and q2 lie on opposite sides of p1q1.
     *       --This is the standard geometric test for proper intersection at a
     *         point that is not one of the vertices of the segments.
     *       --Orientation(p, q, r) is used here: To check if p1 and q1 are on
     *         opposite sides of p2q2, Orientation(p2, q2, p1) and
     *         Orientation(p2, q2, q1) must be different(one OnLeft and one
     *         OnRight), thus p1q1 crossing over p2q2. Applying this
     *         symmetrically to both p1q1 and p2q2 ensures they intersect each
     *         other.
     *    b. Collinearity and Overlap:
     *       Orientation(p, q, r) returning 0 indicates collinearity of the 3
     *       points.
     *       --The check for a point 'q' lying on segment 'pr'[OnSegment(p, q, r)] involves
     *         verifying that 'q' is collinear with 'pr', AND the coordinates of
     *         'q' lies on 'pr'(fall within the bounding box defined by 'pr').
     *         This correctly identifies overlapping segments when they are
     *         collinear, or shared endpoints.
     *    c. Thus, SegmentsIntersect correctly identifies if two given line
     *       segments intersect in any way(proper crossing, collinear overlap,
     *       or shared endpoint).
     *
     * 3. Correctness of Orientation(p, q, r):
     *    a. Compute cross-product of vectors pq and pr.
     *       --The sign determines the direction of thumb in the right hand
     *         rule of turning from vector pq to pr.
     *    b. If cross-product > 0, r lies to the left of vector
     *       pq(Counter-clockwise turn from vector pq to pr)
     *    c. If cross-product < 0, r lies to the right of vector
     *       pq(Clockwise turn from vector pq to pr)
     *    d. If cross-product is 0, p, q, r are collinear.
     *
     * 4. Correctness of overall iteration over non-adjacent edges to check for
     *    self-intersection:
     *    a. Adjacent edges share a common vertex by definition, and intersect
     *       at that common vertex. This is an expected part of a simple polygon
     *       (cycle graph) with each vertex having degree 2, and does not
     *       constitute self-intersection.
     *    b. If a polygon self-intersects, it must have at least one pair of
     *       non-adjacent edges that intersect. Thus, by checking every unique
     *       pair of non-adjacent edges, the algorithm guarantees that if a
     *       self-intersection exists, it will be found.
     *    c. Exception of 3 vertices:
     *       A polygon of 3 vertices has all edges adjacent to each other. In
     *       the normal case, it is a triangle and a cycle.
     *       But in a degenerate case, one vertex can lie on the segment formed
     *       by the other 2 vertices. So, the collinearity and overlapping
     *       check must be performed to verify if that is true, and flag it as
     *       self-intersecting.
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
                if (vertexCount > 3 &&
                    (AreTwoDCartVerticesEqual(p1, p2) || AreTwoDCartVerticesEqual(p1, q2) || AreTwoDCartVerticesEqual(q1, p2) ||
                     AreTwoDCartVerticesEqual(q1, q2)))
                {
                    continue;
                }

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

    // Helper function: Determine the orientation of the ordered triplet (p, q, r) of vertices.
    // Employ the cross-product computation (determinant of the coordinate matrix) of
    // vectors pq and pr to find the direction of the Z axis. The rotation
    // considered is from vector pq to pr.
    // The sign of the determinant is used to infer the direction of the cross-product vector
    // and hence the orientation of the 3 coordinates. A value of 0 indicates that the points
    // are collinear.
    // Using the right-hand rule Z > 0 means pointing upward from the 2D
    // plane (counter-clockwise rotation) and Z < 0 means pointing downward into
    // the 2D plane (clock-wise rotation).
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

        return (val > 0) ? OrientationEnum::kOnLeft : OrientationEnum::kOnRight;
    }

    // Helper function: Check if segment p1q1 intersects segment p2q2
    static bool DoSegmentsIntersect(const TwoDCartesianVertexStruct & p1, const TwoDCartesianVertexStruct & q1,
                                    const TwoDCartesianVertexStruct & p2, const TwoDCartesianVertexStruct & q2)
    {
        // Segment 1 (p1q1) and first vertex of Segment 2 (p2)
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
