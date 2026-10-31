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

#include <pw_unit_test/framework.h>

#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

using TwoDCartesianVertexStruct = ZoneGeometry::TwoDCartesianVertexStruct;

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

TEST_F(TestZonePolygon, TestPointInPolygonAndCoordinateScaling)
{
    // 1. Coordinate scaling from sensor max (1919, 1079) to 640x480 and 1920x1080
    const TwoDCartesianVertexStruct maxCartesian{ 1919, 1079 };
    const std::vector<TwoDCartesianVertexStruct> sensorPolygon = {
        { 0, 0 },
        { 1919, 0 },
        { 1919, 1079 },
        { 0, 1079 },
    };

    const auto scaledToVga = ZoneGeometry::ScaleVertices(sensorPolygon, maxCartesian, 640, 480);
    ASSERT_EQ(scaledToVga.size(), 4u);
    EXPECT_EQ(scaledToVga[0].x, 0);
    EXPECT_EQ(scaledToVga[0].y, 0);
    EXPECT_EQ(scaledToVga[1].x, 639);
    EXPECT_EQ(scaledToVga[1].y, 0);
    EXPECT_EQ(scaledToVga[2].x, 639);
    EXPECT_EQ(scaledToVga[2].y, 479);
    EXPECT_EQ(scaledToVga[3].x, 0);
    EXPECT_EQ(scaledToVga[3].y, 479);

    const auto scaledIdentity = ZoneGeometry::ScaleVertices(sensorPolygon, maxCartesian, 1920, 1080);
    ASSERT_EQ(scaledIdentity.size(), 4u);
    for (size_t i = 0; i < sensorPolygon.size(); ++i)
    {
        EXPECT_EQ(scaledIdentity[i].x, sensorPolygon[i].x);
        EXPECT_EQ(scaledIdentity[i].y, sensorPolygon[i].y);
    }

    // Degenerate zero-dimension scaling guard
    EXPECT_EQ(ZoneGeometry::ScaleVertex({ 100, 100 }, maxCartesian, 0, 480).x, 0);
    EXPECT_EQ(ZoneGeometry::ScaleVertex({ 100, 100 }, { 0, 0 }, 640, 480).y, 0);

    // 2. 3-vertex triangle point-in-polygon (interior, boundary, exterior)
    const std::vector<TwoDCartesianVertexStruct> triangle = { { 10, 10 }, { 30, 10 }, { 20, 30 } };
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(triangle, { 20, 15 }));
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(triangle, { 10, 10 })); // vertex
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(triangle, { 20, 10 })); // horizontal edge
    EXPECT_FALSE(ZoneGeometry::IsPointInPolygon(triangle, { 5, 15 }));
    EXPECT_FALSE(ZoneGeometry::IsPointInPolygon(triangle, { 20, 35 }));

    // 3. 6-vertex concave L-shaped polygon
    const std::vector<TwoDCartesianVertexStruct> concaveL = {
        { 10, 10 }, { 50, 10 }, { 50, 30 }, { 30, 30 }, { 30, 50 }, { 10, 50 },
    };
    EXPECT_FALSE(ZoneGeometry::IsZoneSelfIntersecting(concaveL));
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(concaveL, { 20, 20 }));
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(concaveL, { 40, 20 }));
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(concaveL, { 20, 40 }));
    EXPECT_FALSE(ZoneGeometry::IsPointInPolygon(concaveL, { 40, 40 })); // concave cutout

    // 4. 12-vertex simple polygon (maximum vertex count allowed by Matter spec)
    const std::vector<TwoDCartesianVertexStruct> dodecagon = {
        { 20, 5 },  { 30, 5 },  { 40, 10 }, { 45, 20 }, { 45, 30 }, { 40, 40 },
        { 30, 45 }, { 20, 45 }, { 10, 40 }, { 5, 30 },  { 5, 20 },  { 10, 10 },
    };
    EXPECT_FALSE(ZoneGeometry::IsZoneSelfIntersecting(dodecagon));
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(dodecagon, { 25, 25 }));
    EXPECT_TRUE(ZoneGeometry::IsPointInPolygon(dodecagon, { 10, 25 }));
    EXPECT_FALSE(ZoneGeometry::IsPointInPolygon(dodecagon, { 2, 2 }));
    EXPECT_FALSE(ZoneGeometry::IsPointInPolygon(dodecagon, { 48, 48 }));
}

TEST_F(TestZonePolygon, TestPrivacyMaskingRGB24AndI420)
{
    constexpr uint16_t kWidth                         = 32;
    constexpr uint16_t kHeight                        = 24;
    const std::vector<TwoDCartesianVertexStruct> rect = {
        { 8, 6 },
        { 20, 6 },
        { 20, 16 },
        { 8, 16 },
    };

    // RGB24 masking verification
    std::vector<uint8_t> rgb(static_cast<size_t>(kWidth) * kHeight * 3U, 150);
    ZoneGeometry::ApplyPrivacyMaskRGB24(rgb.data(), kWidth, kHeight, static_cast<size_t>(kWidth) * 3U, rect);

    for (uint16_t y = 0; y < kHeight; ++y)
    {
        for (uint16_t x = 0; x < kWidth; ++x)
        {
            const size_t idx   = (static_cast<size_t>(y) * kWidth + x) * 3U;
            const bool inRect  = (x >= 8 && x <= 20 && y >= 6 && y <= 16);
            const uint8_t want = inRect ? 0 : 150;
            EXPECT_EQ(rgb[idx], want);
            EXPECT_EQ(rgb[idx + 1U], want);
            EXPECT_EQ(rgb[idx + 2U], want);
        }
    }

    // I420 masking verification
    const size_t ySize  = static_cast<size_t>(kWidth) * kHeight;
    const size_t uvSize = (static_cast<size_t>(kWidth) / 2U) * (static_cast<size_t>(kHeight) / 2U);
    std::vector<uint8_t> i420(ySize + 2U * uvSize, 200);
    ZoneGeometry::ApplyPrivacyMaskI420(i420.data(), kWidth, kHeight, rect);

    for (uint16_t y = 0; y < kHeight; ++y)
    {
        for (uint16_t x = 0; x < kWidth; ++x)
        {
            const bool inRect = (x >= 8 && x <= 20 && y >= 6 && y <= 16);
            EXPECT_EQ(i420[static_cast<size_t>(y) * kWidth + x], inRect ? 0 : 200);
        }
    }
    // Check interior chroma at (12, 10) -> U=128, V=128, and exterior chroma at (2, 2) -> U=200, V=200
    const size_t uvStride = kWidth / 2U;
    EXPECT_EQ(i420[ySize + 5U * uvStride + 6U], 128);
    EXPECT_EQ(i420[ySize + uvSize + 5U * uvStride + 6U], 128);
    EXPECT_EQ(i420[ySize + 1U * uvStride + 1U], 200);
    EXPECT_EQ(i420[ySize + uvSize + 1U * uvStride + 1U], 200);
}

} // namespace app
} // namespace chip
