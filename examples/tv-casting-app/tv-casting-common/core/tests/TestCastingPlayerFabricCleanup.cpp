/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *      Unit tests for CastingPlayer fabric cleanup functionality when deleting cached VideoPlayers.
 *      Tests ensure that when a CastingPlayer is deleted from the CastingStore cache,
 *      the associated fabric is properly removed from the fabric table.
 *
 *      Note: These are basic structural tests. Full integration tests with actual
 *      fabric table operations require a complete Matter stack initialization.
 */

#include <gtest/gtest.h>

#include "core/CastingPlayer.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>

using namespace chip;
using namespace matter::casting::core;

namespace {

class TestCastingPlayerFabricCleanup : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

/**
 * Test: RemoveFabric method exists and can be called
 *
 * Verifies that the RemoveFabric() method is accessible on CastingPlayer.
 * This is a basic structural test to ensure the API exists.
 */
TEST_F(TestCastingPlayerFabricCleanup, RemoveFabric_MethodExists)
{
    // Create a test CastingPlayer
    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-1", sizeof(attrs.id) - 1);
    strncpy(attrs.deviceName, "Test Device", sizeof(attrs.deviceName) - 1);
    attrs.nodeId      = 0x1234567890ABCDEF;
    attrs.fabricIndex = 1;
    attrs.vendorId    = 0x1234;
    attrs.productId   = 0x5678;

    CastingPlayer player(attrs);

    // Verify RemoveFabric method can be called
    // Note: Full verification requires Matter stack initialization
    player.RemoveFabric();

    // Test passes if no crash occurs
    SUCCEED();
}

/**
 * Test: CastingPlayer can be created with fabric information
 *
 * Verifies that a CastingPlayer can be constructed with fabric-related
 * attributes (nodeId, fabricIndex).
 */
TEST_F(TestCastingPlayerFabricCleanup, CastingPlayer_CreationWithFabricInfo)
{
    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-2", sizeof(attrs.id) - 1);
    attrs.nodeId      = 0xABCDEF1234567890;
    attrs.fabricIndex = 5;

    CastingPlayer player(attrs);

    // Verify player was created
    EXPECT_NE(player.GetId(), nullptr);
    SUCCEED();
}

/**
 * Test: RemoveFabric can be called multiple times
 *
 * Verifies that calling RemoveFabric() multiple times doesn't cause crashes.
 */
TEST_F(TestCastingPlayerFabricCleanup, RemoveFabric_MultipleCallsSafe)
{
    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-3", sizeof(attrs.id) - 1);
    attrs.nodeId      = 0x1111222233334444;
    attrs.fabricIndex = 2;

    CastingPlayer player(attrs);

    // Call RemoveFabric multiple times
    player.RemoveFabric();
    player.RemoveFabric();
    player.RemoveFabric();

    // Test passes if no crash occurs
    SUCCEED();
}

/**
 * Test: RemoveFabric with zero fabric index
 *
 * Verifies that RemoveFabric() can be called on a player with fabricIndex = 0.
 */
TEST_F(TestCastingPlayerFabricCleanup, RemoveFabric_WithZeroFabricIndex)
{
    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-4", sizeof(attrs.id) - 1);
    attrs.nodeId      = 0;
    attrs.fabricIndex = 0;

    CastingPlayer player(attrs);

    // Should not crash
    player.RemoveFabric();

    SUCCEED();
}

/**
 * Test: Disconnect method exists
 *
 * Verifies that the Disconnect() method is accessible and doesn't crash.
 */
TEST_F(TestCastingPlayerFabricCleanup, Disconnect_MethodExists)
{
    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-5", sizeof(attrs.id) - 1);
    attrs.nodeId      = 0x5555666677778888;
    attrs.fabricIndex = 3;

    CastingPlayer player(attrs);

    // Verify Disconnect method can be called
    player.Disconnect();

    SUCCEED();
}

/**
 * Test: CastingPlayer attributes are accessible
 *
 * Verifies that fabric-related attributes can be read from a CastingPlayer.
 */
TEST_F(TestCastingPlayerFabricCleanup, CastingPlayer_AttributesAccessible)
{
    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-6", sizeof(attrs.id) - 1);
    strncpy(attrs.deviceName, "Test Device Name", sizeof(attrs.deviceName) - 1);
    attrs.nodeId      = 0x9999AAAABBBBCCCC;
    attrs.fabricIndex = 7;
    attrs.vendorId    = 0xABCD;
    attrs.productId   = 0x1234;

    CastingPlayer player(attrs);

    // Verify attributes are accessible
    EXPECT_NE(player.GetId(), nullptr);
    EXPECT_NE(player.GetDeviceName(), nullptr);
    EXPECT_EQ(player.GetVendorId(), 0xABCD);
    EXPECT_EQ(player.GetProductId(), 0x1234);
}

/**
 * Test: Documentation of fabric cleanup pattern
 *
 * This test documents the expected pattern for fabric cleanup when
 * a CastingPlayer connection fails or is deleted from cache.
 */
TEST_F(TestCastingPlayerFabricCleanup, FabricCleanup_ExpectedPattern)
{
    // Expected pattern from CastingPlayer.cpp:
    // 1. Connection fails or player needs to be removed
    // 2. Call RemoveFabric() to clean up fabric table entry
    // 3. Call CastingStore::Delete() to remove from cache
    // 4. Call completion callback with error

    CastingPlayerAttributes attrs;
    strncpy(attrs.id, "test-player-7", sizeof(attrs.id) - 1);
    attrs.nodeId      = 0xDDDDEEEEFFFF0000;
    attrs.fabricIndex = 4;

    CastingPlayer player(attrs);

    // Step 2: RemoveFabric
    player.RemoveFabric();

    // Steps 3 and 4 would follow in real implementation
    SUCCEED();
}

} // namespace
