/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <transport/raw/PeerTCPParamsStorage.h>

TEST(TestPeerTCPParamsStorage, TestSaveTCPParams)
{
    chip::Transport::PeerTCPParamsStorage tcpParamStorage;
    chip::TestPersistentStorageDelegate storage;
    tcpParamStorage.Init(&storage);
    struct
    {
        uint16_t supportedTransports;
        uint32_t maxTCPMessageSize;
        chip::ScopedNodeId node;
    } vectors[CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE];

    // Populate test vectors.
    for (size_t i = 0; i < ArraySize(vectors); ++i)
    {
        vectors[i].supportedTransports = static_cast<uint16_t>(i % 6);
        vectors[i].maxTCPMessageSize   = static_cast<uint32_t>(i * 1000);
        vectors[i].node = chip::ScopedNodeId(static_cast<chip::NodeId>(i + 1), static_cast<chip::FabricIndex>(i + 1));
    }

    // Fill storage.
    for (size_t i = 0; i < CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE; ++i)
    {
        EXPECT_EQ(tcpParamStorage.SaveTCPParams(vectors[i].node, vectors[i].supportedTransports, vectors[i].maxTCPMessageSize),
                  CHIP_NO_ERROR);
    }

    // Verify behavior for cache spillover, where index 0 entity is replaced.
    {
        size_t last = ArraySize(vectors) - 1;
        EXPECT_EQ(
            tcpParamStorage.SaveTCPParams(vectors[last].node, vectors[last].supportedTransports, vectors[last].maxTCPMessageSize),
            CHIP_NO_ERROR);
        // Copy our data to our test vector index 0 to match
        // what is now in storage.
        vectors[0].node                = vectors[last].node;
        vectors[0].supportedTransports = vectors[last].supportedTransports;
        vectors[0].maxTCPMessageSize   = vectors[last].maxTCPMessageSize;
    }

    // Read back and verify values.
    for (auto & vector : vectors)
    {
        uint16_t outSupportedTransports;
        uint32_t outMaxTCPMessageSize;

        // Verify retrieval by node.
        EXPECT_EQ(tcpParamStorage.FindByScopedNodeId(vector.node, outSupportedTransports, outMaxTCPMessageSize), CHIP_NO_ERROR);
        EXPECT_EQ(vector.supportedTransports, outSupportedTransports);
        EXPECT_EQ(vector.maxTCPMessageSize, outMaxTCPMessageSize);
    }
}

TEST(TestPeerTCPParamsStorage, TestDeleteTCPParams)
{
    chip::Transport::PeerTCPParamsStorage tcpParamStorage;
    chip::TestPersistentStorageDelegate storage;
    tcpParamStorage.Init(&storage);
    struct
    {
        uint16_t supportedTransports;
        uint32_t maxTCPMessageSize;
        chip::ScopedNodeId node;
    } vectors[CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE];

    // Populate test vectors.
    for (size_t i = 0; i < ArraySize(vectors); ++i)
    {
        vectors[i].supportedTransports = static_cast<uint16_t>(i);
        vectors[i].maxTCPMessageSize   = static_cast<uint32_t>(i * 1000);
        vectors[i].node = chip::ScopedNodeId(static_cast<chip::NodeId>(i + 1), static_cast<chip::FabricIndex>(i + 1));
    }

    // Fill storage.
    for (auto & vector : vectors)
    // for (size_t i = 0; i < CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE; ++i)
    {
        EXPECT_EQ(tcpParamStorage.SaveTCPParams(vector.node, vector.supportedTransports, vector.maxTCPMessageSize), CHIP_NO_ERROR);
    }

    // Delete values in turn from storage and verify they are removed.
    for (auto & vector : vectors)
    {
        uint16_t outSupportedTransports;
        uint32_t outMaxTCPMessageSize;
        EXPECT_EQ(tcpParamStorage.DeleteTCPParams(vector.node), CHIP_NO_ERROR);
        EXPECT_NE(tcpParamStorage.FindByScopedNodeId(vector.node, outSupportedTransports, outMaxTCPMessageSize), CHIP_NO_ERROR);
    }
}

TEST(TestDefaultSessionResumptionStorage, TestDeleteAllTCPParams)
{
    chip::Transport::PeerTCPParamsStorage tcpParamStorage;
    chip::TestPersistentStorageDelegate storage;
    tcpParamStorage.Init(&storage);

    struct
    {
        chip::FabricIndex fabricIndex;
        struct
        {
            chip::ScopedNodeId node;
        } nodes[3];
    } vectors[CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE / 3];

    // Populate test vectors.
    for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i)
    {
        vectors[i].fabricIndex = static_cast<chip::FabricIndex>(i + 1);
        for (size_t j = 0; j < sizeof(vectors[0].nodes) / sizeof(vectors[0].nodes[0]); ++j)
        {
            vectors[i].nodes[j].node = chip::ScopedNodeId(static_cast<chip::NodeId>(j), vectors[i].fabricIndex);
        }
    }

    // Create sample maxTCPMessageSize and supportedTransports values. We can use the same one for all entries.
    uint16_t supportedTransports = 0x06; // TCP Server and Client
    uint32_t maxTCPMessageSize   = 48000;

    // Fill storage.
    for (auto & vector : vectors)
    {
        for (auto & node : vector.nodes)
        {
            EXPECT_EQ(tcpParamStorage.SaveTCPParams(node.node, supportedTransports, maxTCPMessageSize), CHIP_NO_ERROR);
        }
    }

    // Validate Fabric deletion.
    for (const auto & vector : vectors)
    {
        uint16_t outSupportedTransports;
        uint32_t outMaxTCPMessageSize;
        // Verify fabric node entries exist.
        for (const auto & node : vector.nodes)
        {
            EXPECT_EQ(tcpParamStorage.FindByScopedNodeId(node.node, outSupportedTransports, outMaxTCPMessageSize), CHIP_NO_ERROR);
        }
        // Delete fabric.
        EXPECT_EQ(tcpParamStorage.DeleteAllTCPParams(vector.fabricIndex), CHIP_NO_ERROR);
        // Verify fabric node entries no longer exist.
        for (const auto & node : vector.nodes)
        {
            EXPECT_NE(tcpParamStorage.FindByScopedNodeId(node.node, outSupportedTransports, outMaxTCPMessageSize), CHIP_NO_ERROR);
        }
    }
}

// Test Suite
