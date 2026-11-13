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

TEST_F(TestZonePolygon, TestPrivacyMaskingI420NonAxisAlignedAndDownscaled)
{
    // 1. Planar ApplyPrivacyMaskI420 with a width not divisible by 8 (37x29), explicit padded strides,
    //    and a non-axis-aligned slanted polygon checked pixel-by-pixel against IsPointInPolygon.
    constexpr uint16_t kWidth   = 37;
    constexpr uint16_t kHeight  = 29;
    constexpr size_t kYStride   = 45;
    constexpr size_t kUVHeight  = (kHeight + 1U) / 2U;
    constexpr size_t kUStride   = 23;
    constexpr size_t kVStride   = 25;
    constexpr uint8_t kInitLuma = 200;
    constexpr uint8_t kInitU    = 190;
    constexpr uint8_t kInitV    = 210;

    const std::vector<TwoDCartesianVertexStruct> slantedPolygon = {
        { 5, 3 }, { 29, 7 }, { 33, 21 }, { 19, 17 }, { 8, 25 },
    };
    EXPECT_FALSE(ZoneGeometry::IsZoneSelfIntersecting(slantedPolygon));

    std::vector<uint8_t> yPlane(kYStride * kHeight, kInitLuma);
    std::vector<uint8_t> uPlane(kUStride * kUVHeight, kInitU);
    std::vector<uint8_t> vPlane(kVStride * kUVHeight, kInitV);

    // Exact scanline fill (padPixels = 0) must match IsPointInPolygon at every pixel (x, y)
    ZoneGeometry::ApplyPrivacyMaskI420(yPlane.data(), kYStride, uPlane.data(), kUStride, vPlane.data(), kVStride, kWidth, kHeight,
                                       slantedPolygon, /* padPixels = */ 0);

    for (uint16_t y = 0; y < kHeight; ++y)
    {
        for (uint16_t x = 0; x < kWidth; ++x)
        {
            const bool inside  = ZoneGeometry::IsPointInPolygon(slantedPolygon, { x, y });
            const uint8_t want = inside ? 0 : kInitLuma;
            EXPECT_EQ(yPlane[static_cast<size_t>(y) * kYStride + x], want);
            if (inside)
            {
                EXPECT_EQ(uPlane[static_cast<size_t>(y / 2U) * kUStride + (x / 2U)], 128);
                EXPECT_EQ(vPlane[static_cast<size_t>(y / 2U) * kVStride + (x / 2U)], 128);
            }
        }
        // Verify stride padding bytes past kWidth remain untouched
        for (size_t padX = kWidth; padX < kYStride; ++padX)
        {
            EXPECT_EQ(yPlane[static_cast<size_t>(y) * kYStride + padX], kInitLuma);
        }
    }

    // Chroma samples whose 2x2 luma block contains a masked pixel are 128; untouched blocks and stride padding keep kInitU/kInitV.
    constexpr size_t kUVWidth = (kWidth + 1U) / 2U;
    for (size_t cy = 0; cy < kUVHeight; ++cy)
    {
        for (size_t cx = 0; cx < kUVWidth; ++cx)
        {
            bool blockMasked = false;
            for (size_t dy = 0; dy < 2U; ++dy)
            {
                for (size_t dx = 0; dx < 2U; ++dx)
                {
                    const size_t x = 2U * cx + dx;
                    const size_t y = 2U * cy + dy;
                    if (x < kWidth && y < kHeight && yPlane[y * kYStride + x] == 0)
                    {
                        blockMasked = true;
                    }
                }
            }
            EXPECT_EQ(uPlane[cy * kUStride + cx], blockMasked ? 128 : kInitU);
            EXPECT_EQ(vPlane[cy * kVStride + cx], blockMasked ? 128 : kInitV);
        }
        for (size_t padCx = kUVWidth; padCx < kUStride; ++padCx)
        {
            EXPECT_EQ(uPlane[cy * kUStride + padCx], kInitU);
        }
        for (size_t padCx = kUVWidth; padCx < kVStride; ++padCx)
        {
            EXPECT_EQ(vPlane[cy * kVStride + padCx], kInitV);
        }
    }

    // 2. Conservative downscaled masking (default padPixels = 1):
    //    Thin slanted zone (100,100)-(700,400)-(700,402)-(100,102) on 1920x1080 scaled to 320x180
    //    must mask every column it crosses [17..116] and every downscaled sensor point in the zone.
    constexpr uint16_t kSmallWidth  = 320;
    constexpr uint16_t kSmallHeight = 180;
    const TwoDCartesianVertexStruct maxCartesian{ 1919, 1079 };
    const std::vector<TwoDCartesianVertexStruct> thinSensorZone = {
        { 100, 100 },
        { 700, 400 },
        { 700, 402 },
        { 100, 102 },
    };
    EXPECT_FALSE(ZoneGeometry::IsZoneSelfIntersecting(thinSensorZone));

    const auto scaledThinZone = ZoneGeometry::ScaleVertices(thinSensorZone, maxCartesian, kSmallWidth, kSmallHeight);
    std::vector<uint8_t> smallY(static_cast<size_t>(kSmallWidth) * kSmallHeight, kInitLuma);
    std::vector<uint8_t> smallU(static_cast<size_t>(kSmallWidth / 2U) * (kSmallHeight / 2U), kInitU);
    std::vector<uint8_t> smallV(static_cast<size_t>(kSmallWidth / 2U) * (kSmallHeight / 2U), kInitV);

    ZoneGeometry::ApplyPrivacyMaskI420(smallY.data(), kSmallWidth, smallU.data(), kSmallWidth / 2U, smallV.data(), kSmallWidth / 2U,
                                       kSmallWidth, kSmallHeight, scaledThinZone);

    for (uint16_t col = 17; col <= 116; ++col)
    {
        bool colHasMaskedPixel = false;
        for (uint16_t row = 0; row < kSmallHeight; ++row)
        {
            if (smallY[static_cast<size_t>(row) * kSmallWidth + col] == 0)
            {
                colHasMaskedPixel = true;
                break;
            }
        }
        EXPECT_TRUE(colHasMaskedPixel);
    }

    // Pixels outside the 1-px dilated bounding box (x in [16..117], y in [16..68]) keep their initial luma.
    for (uint16_t row = 0; row < kSmallHeight; ++row)
    {
        for (uint16_t col = 0; col < kSmallWidth; ++col)
        {
            if (col < 16U || col >= 118U || row < 16U || row >= 69U)
            {
                EXPECT_EQ(smallY[static_cast<size_t>(row) * kSmallWidth + col], kInitLuma);
            }
        }
    }
    EXPECT_EQ(smallY[20U * kSmallWidth + 110U], kInitLuma);
    EXPECT_EQ(smallY[65U * kSmallWidth + 20U], kInitLuma);

    for (size_t cy = 0; cy < kSmallHeight / 2U; ++cy)
    {
        for (size_t cx = 0; cx < kSmallWidth / 2U; ++cx)
        {
            bool blockMasked = false;
            for (size_t dy = 0; dy < 2U; ++dy)
            {
                for (size_t dx = 0; dx < 2U; ++dx)
                {
                    if (smallY[(2U * cy + dy) * kSmallWidth + (2U * cx + dx)] == 0)
                    {
                        blockMasked = true;
                    }
                }
            }
            EXPECT_EQ(smallU[cy * (kSmallWidth / 2U) + cx], blockMasked ? 128 : kInitU);
            EXPECT_EQ(smallV[cy * (kSmallWidth / 2U) + cx], blockMasked ? 128 : kInitV);
        }
    }

    for (uint16_t sx = 100; sx <= 700; ++sx)
    {
        const uint16_t baseSy = static_cast<uint16_t>(100U + (sx - 100U) / 2U);
        for (uint16_t sy = baseSy; sy <= baseSy + 2U; ++sy)
        {
            if (ZoneGeometry::IsPointInPolygon(thinSensorZone, { sx, sy }))
            {
                const auto framePt = ZoneGeometry::ScaleVertex({ sx, sy }, maxCartesian, kSmallWidth, kSmallHeight);
                EXPECT_EQ(smallY[static_cast<size_t>(framePt.y) * kSmallWidth + framePt.x], 0);
            }
        }
    }
}

} // namespace app
} // namespace chip
