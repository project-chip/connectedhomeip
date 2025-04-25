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
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>

// DefaultSessionResumptionStorage is a partial implementation.
// Use SimpleSessionResumptionStorage, which extends it, to test.
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>

TEST(TestDefaultSessionResumptionStorage, TestSave)
{
    chip::SimpleSessionResumptionStorage sessionStorage;
    chip::TestPersistentStorageDelegate storage;
    sessionStorage.Init(&storage);
    struct
    {
        chip::SessionResumptionStorage::ResumptionIdStorage resumptionId;
        chip::Crypto::P256ECDHDerivedSecret sharedSecret;
        chip::ScopedNodeId node;
        chip::CATValues cats;
    } vectors[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE + 1];

    // Populate test vectors.
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(vectors); ++i)
    {
        EXPECT_EQ(chip::Crypto::DRBG_get_bytes(vectors[i].resumptionId.data(), vectors[i].resumptionId.size()), CHIP_NO_ERROR);
        *vectors[i].resumptionId.data() =
            static_cast<uint8_t>(i); // set first byte to our index to ensure uniqueness for the FindByResumptionId call
        vectors[i].sharedSecret.SetLength(vectors[i].sharedSecret.Capacity());
        EXPECT_EQ(chip::Crypto::DRBG_get_bytes(vectors[i].sharedSecret.Bytes(), vectors[i].sharedSecret.Length()), CHIP_NO_ERROR);
        vectors[i].node           = chip::ScopedNodeId(static_cast<chip::NodeId>(i + 1), static_cast<chip::FabricIndex>(i + 1));
        vectors[i].cats.values[0] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[1] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[2] = static_cast<chip::CASEAuthTag>(rand());
    }

    // Fill storage.
    for (size_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; ++i)
    {
        EXPECT_EQ(sessionStorage.Save(vectors[i].node, vectors[i].resumptionId, vectors[i].sharedSecret, vectors[i].cats),
                  CHIP_NO_ERROR);
    }

    // Verify behavior for over-fill.
    //
    // Currently, DefaultSessionResumptionStorage replaces index 0.
    // If more sophisticated LRU behavior is implemented, this test
    // case should be modified to match.
    {
        size_t last = MATTER_ARRAY_SIZE(vectors) - 1;
        EXPECT_EQ(
            sessionStorage.Save(vectors[last].node, vectors[last].resumptionId, vectors[last].sharedSecret, vectors[last].cats),
            CHIP_NO_ERROR);
        // Copy our data to our test vector index 0 to match
        // what is now in storage.
        vectors[0].node = vectors[last].node;
        vectors[0].cats = vectors[last].cats;
        memcpy(vectors[0].resumptionId.data(), vectors[last].resumptionId.data(), vectors[0].resumptionId.size());
        memcpy(vectors[0].sharedSecret.Bytes(), vectors[last].sharedSecret.Bytes(), vectors[0].sharedSecret.Length());
    }

    // Read back and verify values.
    for (auto & vector : vectors)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;

        // Verify retrieval by node.
        EXPECT_EQ(sessionStorage.FindByScopedNodeId(vector.node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(vector.resumptionId.data(), outResumptionId.data(), vector.resumptionId.size()), 0);
        EXPECT_EQ(memcmp(vector.sharedSecret.ConstBytes(), outSharedSecret.ConstBytes(), vector.sharedSecret.Length()), 0);
        EXPECT_EQ(vector.cats.values[0], outCats.values[0]);
        EXPECT_EQ(vector.cats.values[1], outCats.values[1]);
        EXPECT_EQ(vector.cats.values[2], outCats.values[2]);

        // Validate retrieval by resumption ID.
        EXPECT_EQ(sessionStorage.FindByResumptionId(vector.resumptionId, outNode, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_EQ(vector.node, outNode);
        EXPECT_EQ(memcmp(vector.sharedSecret.Bytes(), outSharedSecret.ConstBytes(), vector.sharedSecret.Length()), 0);
        EXPECT_EQ(vector.cats.values[0], outCats.values[0]);
        EXPECT_EQ(vector.cats.values[1], outCats.values[1]);
        EXPECT_EQ(vector.cats.values[2], outCats.values[2]);
    }
}

TEST(TestDefaultSessionResumptionStorage, TestInPlaceSave)
{
    chip::SimpleSessionResumptionStorage sessionStorage;
    chip::TestPersistentStorageDelegate storage;
    sessionStorage.Init(&storage);
    struct
    {
        chip::SessionResumptionStorage::ResumptionIdStorage resumptionId;
        chip::Crypto::P256ECDHDerivedSecret sharedSecret;
        chip::ScopedNodeId node;
        chip::CATValues cats;
    } vectors[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE + 10];

    // Construct only a few unique node identities to simulate talking to a
    // couple peers.
    chip::ScopedNodeId nodes[3];
    static_assert(MATTER_ARRAY_SIZE(nodes) < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE,
                  "must have fewer nodes than slots in session resumption storage");
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(nodes); ++i)
    {
        do
        {
            nodes[i] = chip::ScopedNodeId(static_cast<chip::NodeId>(rand()), static_cast<chip::FabricIndex>(i + 1));
        } while (!nodes[i].IsOperational());
    }

    // Populate test vectors.
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(vectors); ++i)
    {
        EXPECT_EQ(chip::Crypto::DRBG_get_bytes(vectors[i].resumptionId.data(), vectors[i].resumptionId.size()), CHIP_NO_ERROR);
        *vectors[i].resumptionId.data() =
            static_cast<uint8_t>(i); // set first byte to our index to ensure uniqueness for the FindByResumptionId call
        vectors[i].sharedSecret.SetLength(vectors[i].sharedSecret.Capacity());
        EXPECT_EQ(chip::Crypto::DRBG_get_bytes(vectors[i].sharedSecret.Bytes(), vectors[i].sharedSecret.Length()), CHIP_NO_ERROR);
        vectors[i].node           = nodes[i % MATTER_ARRAY_SIZE(nodes)];
        vectors[i].cats.values[0] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[1] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[2] = static_cast<chip::CASEAuthTag>(rand());
    }

    // Add one entry for each node.
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(nodes); ++i)
    {
        EXPECT_EQ(sessionStorage.Save(vectors[i].node, vectors[i].resumptionId, vectors[i].sharedSecret, vectors[i].cats),
                  CHIP_NO_ERROR);
    }

    // Read back and verify values.
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(nodes); ++i)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;

        // Verify retrieval by node.
        EXPECT_EQ(sessionStorage.FindByScopedNodeId(vectors[i].node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(vectors[i].resumptionId.data(), outResumptionId.data(), vectors[i].resumptionId.size()), 0);
        EXPECT_EQ(memcmp(vectors[i].sharedSecret.ConstBytes(), outSharedSecret.ConstBytes(), vectors[i].sharedSecret.Length()), 0);
        EXPECT_EQ(vectors[i].cats.values[0], outCats.values[0]);
        EXPECT_EQ(vectors[i].cats.values[1], outCats.values[1]);
        EXPECT_EQ(vectors[i].cats.values[2], outCats.values[2]);

        // Validate retrieval by resumption ID.
        EXPECT_EQ(sessionStorage.FindByResumptionId(vectors[i].resumptionId, outNode, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_EQ(vectors[i].node, outNode);
        EXPECT_EQ(memcmp(vectors[i].sharedSecret.ConstBytes(), outSharedSecret.ConstBytes(), vectors[i].sharedSecret.Length()), 0);
        EXPECT_EQ(vectors[i].cats.values[0], outCats.values[0]);
        EXPECT_EQ(vectors[i].cats.values[1], outCats.values[1]);
        EXPECT_EQ(vectors[i].cats.values[2], outCats.values[2]);
    }

    // Now add all test vectors.  This should overwrite each node's record
    // many times.
    for (auto & vector : vectors)
    {
        EXPECT_EQ(sessionStorage.Save(vector.node, vector.resumptionId, vector.sharedSecret, vector.cats), CHIP_NO_ERROR);
    }

    // Read back and verify that only the last record for each node was retained.
    for (size_t i = MATTER_ARRAY_SIZE(vectors) - MATTER_ARRAY_SIZE(nodes); i < MATTER_ARRAY_SIZE(vectors); ++i)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;

        // Verify retrieval by node.
        EXPECT_EQ(sessionStorage.FindByScopedNodeId(vectors[i].node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(vectors[i].resumptionId.data(), outResumptionId.data(), vectors[i].resumptionId.size()), 0);
        EXPECT_EQ(memcmp(vectors[i].sharedSecret.ConstBytes(), outSharedSecret.ConstBytes(), vectors[i].sharedSecret.Length()), 0);
        EXPECT_EQ(vectors[i].cats.values[0], outCats.values[0]);
        EXPECT_EQ(vectors[i].cats.values[1], outCats.values[1]);
        EXPECT_EQ(vectors[i].cats.values[2], outCats.values[2]);

        // Validate retrieval by resumption ID.
        EXPECT_EQ(sessionStorage.FindByResumptionId(vectors[i].resumptionId, outNode, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_EQ(vectors[i].node, outNode);
        EXPECT_EQ(memcmp(vectors[i].sharedSecret.ConstBytes(), outSharedSecret.ConstBytes(), vectors[i].sharedSecret.Length()), 0);
        EXPECT_EQ(vectors[i].cats.values[0], outCats.values[0]);
        EXPECT_EQ(vectors[i].cats.values[1], outCats.values[1]);
        EXPECT_EQ(vectors[i].cats.values[2], outCats.values[2]);
    }

    // Remove all records for all fabrics.  If all three tables of (index, state,
    // links) are in sync, deleting for each fabric should clean error free.
    for (const auto & node : nodes)
    {
        EXPECT_EQ(sessionStorage.DeleteAll(node.GetFabricIndex()), CHIP_NO_ERROR);
    }

    // Verify that no entries can be located any longer for any node or
    // resumption ID.
    for (auto & vector : vectors)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;

        // Verify all records for all nodes are gone.
        EXPECT_NE(sessionStorage.FindByScopedNodeId(vector.node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);

        // Verify all records for all resumption IDs are gone.
        EXPECT_NE(sessionStorage.FindByResumptionId(vector.resumptionId, outNode, outSharedSecret, outCats), CHIP_NO_ERROR);
    }

    // Verify no state table persistent storage entries were leaked.
    for (const auto & node : nodes)
    {
        uint16_t size = 0;
        auto rv       = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::GetStorageKey(node).KeyName(), nullptr, size);
        EXPECT_EQ(rv, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
    // Verify no link table persistent storage entries were leaked.
    for (auto & vector : vectors)
    {
        uint16_t size = 0;
        auto rv       = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::GetStorageKey(vector.resumptionId).KeyName(),
                                                nullptr, size);
        EXPECT_EQ(rv, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

TEST(TestDefaultSessionResumptionStorage, TestDelete)
{
    chip::SimpleSessionResumptionStorage sessionStorage;
    chip::TestPersistentStorageDelegate storage;
    sessionStorage.Init(&storage);
    chip::Crypto::P256ECDHDerivedSecret sharedSecret;
    struct
    {
        chip::SessionResumptionStorage::ResumptionIdStorage resumptionId;
        chip::ScopedNodeId node;
    } vectors[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE];

    // Create a shared secret.  We can use the same one for all entries.
    sharedSecret.SetLength(sharedSecret.Capacity());
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(sharedSecret.Bytes(), sharedSecret.Length()), CHIP_NO_ERROR);

    // Populate test vectors.
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(vectors); ++i)
    {
        EXPECT_EQ(chip::Crypto::DRBG_get_bytes(vectors[i].resumptionId.data(), vectors[i].resumptionId.size()), CHIP_NO_ERROR);
        *vectors[i].resumptionId.data() =
            static_cast<uint8_t>(i); // set first byte to our index to ensure uniqueness for the delete test
        vectors[i].node = chip::ScopedNodeId(static_cast<chip::NodeId>(i + 1), static_cast<chip::FabricIndex>(i + 1));
    }

    // Fill storage.
    for (auto & vector : vectors)
    {
        EXPECT_EQ(sessionStorage.Save(vector.node, vector.resumptionId, sharedSecret, chip::CATValues{}), CHIP_NO_ERROR);
    }

    // Delete values in turn from storage and verify they are removed.
    for (auto & vector : vectors)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;
        EXPECT_EQ(sessionStorage.Delete(vector.node), CHIP_NO_ERROR);
        EXPECT_NE(sessionStorage.FindByScopedNodeId(vector.node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);
        EXPECT_NE(sessionStorage.FindByResumptionId(vector.resumptionId, outNode, outSharedSecret, outCats), CHIP_NO_ERROR);
    }

    // Verify no state or link table persistent storage entries were leaked.
    for (auto & vector : vectors)
    {
        uint16_t size = 0;
        {
            auto rv =
                storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::GetStorageKey(vector.node).KeyName(), nullptr, size);
            EXPECT_EQ(rv, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        }
        {
            auto rv = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::GetStorageKey(vector.resumptionId).KeyName(),
                                              nullptr, size);
            EXPECT_EQ(rv, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        }
    }
}

TEST(TestDefaultSessionResumptionStorage, TestDeleteAll)
{
    chip::SimpleSessionResumptionStorage sessionStorage;
    chip::TestPersistentStorageDelegate storage;
    sessionStorage.Init(&storage);
    chip::Crypto::P256ECDHDerivedSecret sharedSecret;
    struct
    {
        chip::FabricIndex fabricIndex;
        struct
        {
            chip::SessionResumptionStorage::ResumptionIdStorage resumptionId;
            chip::ScopedNodeId node;
        } nodes[3];
    } vectors[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE / 3];

    // Create a shared secret.  We can use the same one for all entries.
    sharedSecret.SetLength(sharedSecret.Capacity());
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(sharedSecret.Bytes(), sharedSecret.Length()), CHIP_NO_ERROR);

    // Populate test vectors.
    for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i)
    {
        vectors[i].fabricIndex = static_cast<chip::FabricIndex>(i + 1);
        for (size_t j = 0; j < sizeof(vectors[0].nodes) / sizeof(vectors[0].nodes[0]); ++j)
        {
            EXPECT_EQ(
                chip::Crypto::DRBG_get_bytes(vectors[i].nodes[j].resumptionId.data(), vectors[i].nodes[j].resumptionId.size()),
                CHIP_NO_ERROR);
            vectors[i].nodes[j].node = chip::ScopedNodeId(static_cast<chip::NodeId>(j), vectors[i].fabricIndex);
        }
    }

    // Fill storage.
    for (auto & vector : vectors)
    {
        for (auto & node : vector.nodes)
        {
            EXPECT_EQ(sessionStorage.Save(node.node, node.resumptionId, sharedSecret, chip::CATValues{}), CHIP_NO_ERROR);
        }
    }

    // Validate Fabric deletion.
    for (const auto & vector : vectors)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;
        // Verify fabric node entries exist.
        for (const auto & node : vector.nodes)
        {
            EXPECT_EQ(sessionStorage.FindByScopedNodeId(node.node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);
        }
        // Delete fabric.
        EXPECT_EQ(sessionStorage.DeleteAll(vector.fabricIndex), CHIP_NO_ERROR);
        // Verify fabric node entries no longer exist.
        for (const auto & node : vector.nodes)
        {
            EXPECT_NE(sessionStorage.FindByScopedNodeId(node.node, outResumptionId, outSharedSecret, outCats), CHIP_NO_ERROR);
        }
    }
    // Verify no state or link table persistent storage entries were leaked.
    for (auto & vector : vectors)
    {
        for (auto & node : vector.nodes)
        {
            uint16_t size = 0;
            {
                auto rv = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::GetStorageKey(node.node).KeyName(), nullptr,
                                                  size);
                EXPECT_EQ(rv, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
            }
            {
                auto rv = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::GetStorageKey(node.resumptionId).KeyName(),
                                                  nullptr, size);
                EXPECT_EQ(rv, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
            }
        }
    }
}
