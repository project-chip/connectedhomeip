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

/**
 *    @file
 *      This file implements a unit test suite for testing different zone
 *      polygons for self-intersection.
 *
 */

#include <vector>

#include <app/clusters/zone-management-server/zone-geometry.h>
#include <app/clusters/zone-management-server/zone-management-server.h>

#include <pw_unit_test/framework.h>

#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::Testing;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

// =================================
//      Unit tests
// =================================

namespace chip {
namespace app {

class TestZonePolygon : public ::testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

TEST_F(TestZonePolygon, TestValidZones)
{
    // Simple triangle
    std::vector<TwoDCartesianVertexStruct> zone1 = { { 10, 10 }, { 20, 10 }, { 20, 20 } };
    bool res                                     = ZoneGeometry::IsZoneSelfIntersecting(zone1);
    EXPECT_FALSE(res);

    // Simple rectangle
    std::vector<TwoDCartesianVertexStruct> zone2 = { { 10, 10 }, { 20, 10 }, { 20, 20 }, { 10, 20 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone2);
    EXPECT_FALSE(res);

    // Convex Pentagon
    std::vector<TwoDCartesianVertexStruct> zone3 = { { 10, 10 }, { 20, 10 }, { 20, 20 }, { 15, 25 }, { 10, 20 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone3);
    EXPECT_FALSE(res);

    // Concave Pentagon
    std::vector<TwoDCartesianVertexStruct> zone4 = { { 10, 10 }, { 20, 10 }, { 20, 20 }, { 15, 15 }, { 10, 20 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone4);
    EXPECT_FALSE(res);

    // 4-vertex Arrow head
    std::vector<TwoDCartesianVertexStruct> zone5 = { { 10, 10 }, { 20, 20 }, { 30, 10 }, { 20, 15 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone5);
    EXPECT_FALSE(res);

    // 4-vertex polygon with 3 vertices collinear
    std::vector<TwoDCartesianVertexStruct> zone6 = { { 10, 10 }, { 20, 10 }, { 30, 10 }, { 20, 15 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone6);
    EXPECT_FALSE(res);
}

TEST_F(TestZonePolygon, TestSelfIntersectingZones)
{
    // Hourglass rectangle
    std::vector<TwoDCartesianVertexStruct> zone1 = { { 10, 10 }, { 20, 20 }, { 20, 10 }, { 10, 20 } };
    bool res                                     = ZoneGeometry::IsZoneSelfIntersecting(zone1);
    EXPECT_TRUE(res);

    // Pentagon with apex pulled down crossing base.
    std::vector<TwoDCartesianVertexStruct> zone2 = { { 10, 10 }, { 20, 10 }, { 20, 20 }, { 15, 5 }, { 10, 20 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone2);
    EXPECT_TRUE(res);

    // Pentagon with overlapping first and second edges
    std::vector<TwoDCartesianVertexStruct> zone3 = { { 2, 2 }, { 10, 2 }, { 7, 2 }, { 10, 1 }, { 1, 1 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone3);
    EXPECT_TRUE(res);

    // Degenerate zone with duplicate vertices and inner loops
    std::vector<TwoDCartesianVertexStruct> zone4 = { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 2, 1 }, { 1, 0 }, { 0, 1 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone4);
    EXPECT_TRUE(res);

    // Degenerate zone with duplicate vertices and overlapping edges
    std::vector<TwoDCartesianVertexStruct> zone5 = { { 0, 0 }, { 2, 0 }, { 2, 1 }, { 2, 0 }, { 1, 0 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone5);
    EXPECT_TRUE(res);

    // Degenerate zone with duplicate vertices and overlapping edges
    std::vector<TwoDCartesianVertexStruct> zone6 = { { 0, 0 }, { 1, 0 }, { 0, 0 }, { 0, 1 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone6);
    EXPECT_TRUE(res);

    // Degenerate zone with adjacent duplicate vertices
    std::vector<TwoDCartesianVertexStruct> zone7 = { { 0, 0 }, { 0, 0 }, { 1, 0 }, { 0, 1 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone7);
    EXPECT_TRUE(res);

    // Degenerate zone with duplicate first and last vertices
    std::vector<TwoDCartesianVertexStruct> zone8 = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 0 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone8);
    EXPECT_TRUE(res);

    // Degenerate zone with 3 vertices and overlapping edges
    std::vector<TwoDCartesianVertexStruct> zone9 = { { 0, 0 }, { 2, 0 }, { 1, 0 } };
    res                                          = ZoneGeometry::IsZoneSelfIntersecting(zone9);
    EXPECT_TRUE(res);

    // Degenerate zone with 4 collinear vertices and overlapping edges
    std::vector<TwoDCartesianVertexStruct> zone10 = { { 0, 0 }, { 2, 0 }, { 1, 0 }, { 3, 0 } };
    res                                           = ZoneGeometry::IsZoneSelfIntersecting(zone10);
    EXPECT_TRUE(res);
}

} // namespace app
} // namespace chip
