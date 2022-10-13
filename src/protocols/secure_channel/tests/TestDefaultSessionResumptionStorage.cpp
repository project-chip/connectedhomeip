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

#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

// DefaultSessionResumptionStorage is a partial implementation.
// Use SimpleSessionResumptionStorage, which extends it, to test.
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>

void TestSave(nlTestSuite * inSuite, void * inContext)
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
    for (size_t i = 0; i < ArraySize(vectors); ++i)
    {
        NL_TEST_ASSERT(
            inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(vectors[i].resumptionId.data(), vectors[i].resumptionId.size()));
        *vectors[i].resumptionId.data() =
            static_cast<uint8_t>(i); // set first byte to our index to ensure uniqueness for the FindByResumptionId call
        vectors[i].sharedSecret.SetLength(vectors[i].sharedSecret.Capacity());
        NL_TEST_ASSERT(inSuite,
                       CHIP_NO_ERROR ==
                           chip::Crypto::DRBG_get_bytes(vectors[i].sharedSecret.Bytes(), vectors[i].sharedSecret.Length()));
        vectors[i].node           = chip::ScopedNodeId(static_cast<chip::NodeId>(i + 1), static_cast<chip::FabricIndex>(i + 1));
        vectors[i].cats.values[0] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[1] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[2] = static_cast<chip::CASEAuthTag>(rand());
    }

    // Fill storage.
    for (size_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; ++i)
    {
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.Save(vectors[i].node, vectors[i].resumptionId, vectors[i].sharedSecret, vectors[i].cats) ==
                           CHIP_NO_ERROR);
    }

    // Verify behavior for over-fill.
    //
    // Currently, DefaultSessionResumptionStorage replaces index 0.
    // If more sophisticated LRU behavior is implemented, this test
    // case should be modified to match.
    {
        size_t last = ArraySize(vectors) - 1;
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.Save(vectors[last].node, vectors[last].resumptionId, vectors[last].sharedSecret,
                                           vectors[last].cats) == CHIP_NO_ERROR);
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
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByScopedNodeId(vector.node, outResumptionId, outSharedSecret, outCats) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(vector.resumptionId.data(), outResumptionId.data(), vector.resumptionId.size()) == 0);
        NL_TEST_ASSERT(inSuite, memcmp(vector.sharedSecret.ConstBytes(), outSharedSecret, vector.sharedSecret.Length()) == 0);
        NL_TEST_ASSERT(inSuite, vector.cats.values[0] == outCats.values[0]);
        NL_TEST_ASSERT(inSuite, vector.cats.values[1] == outCats.values[1]);
        NL_TEST_ASSERT(inSuite, vector.cats.values[2] == outCats.values[2]);

        // Validate retrieval by resumption ID.
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByResumptionId(vector.resumptionId, outNode, outSharedSecret, outCats) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, vector.node == outNode);
        NL_TEST_ASSERT(inSuite, memcmp(vector.sharedSecret.Bytes(), outSharedSecret, vector.sharedSecret.Length()) == 0);
        NL_TEST_ASSERT(inSuite, vector.cats.values[0] == outCats.values[0]);
        NL_TEST_ASSERT(inSuite, vector.cats.values[1] == outCats.values[1]);
        NL_TEST_ASSERT(inSuite, vector.cats.values[2] == outCats.values[2]);
    }
}

void TestInPlaceSave(nlTestSuite * inSuite, void * inContext)
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
    static_assert(ArraySize(nodes) < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE,
                  "must have fewer nodes than slots in session resumption storage");
    for (size_t i = 0; i < ArraySize(nodes); ++i)
    {
        do
        {
            nodes[i] = chip::ScopedNodeId(static_cast<chip::NodeId>(rand()), static_cast<chip::FabricIndex>(i + 1));
        } while (!nodes[i].IsOperational());
    }

    // Populate test vectors.
    for (size_t i = 0; i < ArraySize(vectors); ++i)
    {
        NL_TEST_ASSERT(
            inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(vectors[i].resumptionId.data(), vectors[i].resumptionId.size()));
        *vectors[i].resumptionId.data() =
            static_cast<uint8_t>(i); // set first byte to our index to ensure uniqueness for the FindByResumptionId call
        vectors[i].sharedSecret.SetLength(vectors[i].sharedSecret.Capacity());
        NL_TEST_ASSERT(inSuite,
                       CHIP_NO_ERROR ==
                           chip::Crypto::DRBG_get_bytes(vectors[i].sharedSecret.Bytes(), vectors[i].sharedSecret.Length()));
        vectors[i].node           = nodes[i % ArraySize(nodes)];
        vectors[i].cats.values[0] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[1] = static_cast<chip::CASEAuthTag>(rand());
        vectors[i].cats.values[2] = static_cast<chip::CASEAuthTag>(rand());
    }

    // Add one entry for each node.
    for (size_t i = 0; i < ArraySize(nodes); ++i)
    {
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.Save(vectors[i].node, vectors[i].resumptionId, vectors[i].sharedSecret, vectors[i].cats) ==
                           CHIP_NO_ERROR);
    }

    // Read back and verify values.
    for (size_t i = 0; i < ArraySize(nodes); ++i)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;

        // Verify retrieval by node.
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByScopedNodeId(vectors[i].node, outResumptionId, outSharedSecret, outCats) ==
                           CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite,
                       memcmp(vectors[i].resumptionId.data(), outResumptionId.data(), vectors[i].resumptionId.size()) == 0);
        NL_TEST_ASSERT(inSuite, memcmp(vectors[i].sharedSecret.Bytes(), outSharedSecret, vectors[i].sharedSecret.Length()) == 0);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[0] == outCats.values[0]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[1] == outCats.values[1]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[2] == outCats.values[2]);

        // Validate retrieval by resumption ID.
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByResumptionId(vectors[i].resumptionId, outNode, outSharedSecret, outCats) ==
                           CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, vectors[i].node == outNode);
        NL_TEST_ASSERT(inSuite, memcmp(vectors[i].sharedSecret.Bytes(), outSharedSecret, vectors[i].sharedSecret.Length()) == 0);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[0] == outCats.values[0]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[1] == outCats.values[1]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[2] == outCats.values[2]);
    }

    // Now add all test vectors.  This should overwrite each node's record
    // many times.
    for (auto & vector : vectors)
    {
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.Save(vector.node, vector.resumptionId, vector.sharedSecret, vector.cats) == CHIP_NO_ERROR);
    }

    // Read back and verify that only the last record for each node was retained.
    for (size_t i = ArraySize(vectors) - ArraySize(nodes); i < ArraySize(vectors); ++i)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;

        // Verify retrieval by node.
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByScopedNodeId(vectors[i].node, outResumptionId, outSharedSecret, outCats) ==
                           CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite,
                       memcmp(vectors[i].resumptionId.data(), outResumptionId.data(), vectors[i].resumptionId.size()) == 0);
        NL_TEST_ASSERT(inSuite, memcmp(vectors[i].sharedSecret.Bytes(), outSharedSecret, vectors[i].sharedSecret.Length()) == 0);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[0] == outCats.values[0]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[1] == outCats.values[1]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[2] == outCats.values[2]);

        // Validate retrieval by resumption ID.
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByResumptionId(vectors[i].resumptionId, outNode, outSharedSecret, outCats) ==
                           CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, vectors[i].node == outNode);
        NL_TEST_ASSERT(inSuite, memcmp(vectors[i].sharedSecret.Bytes(), outSharedSecret, vectors[i].sharedSecret.Length()) == 0);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[0] == outCats.values[0]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[1] == outCats.values[1]);
        NL_TEST_ASSERT(inSuite, vectors[i].cats.values[2] == outCats.values[2]);
    }

    // Remove all records for all fabrics.  If all three tables of (index, state,
    // links) are in sync, deleting for each fabric should clean error free.
    for (const auto & node : nodes)
    {
        NL_TEST_ASSERT(inSuite, sessionStorage.DeleteAll(node.GetFabricIndex()) == CHIP_NO_ERROR);
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
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByScopedNodeId(vector.node, outResumptionId, outSharedSecret, outCats) != CHIP_NO_ERROR);

        // Verify all records for all resumption IDs are gone.
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByResumptionId(vector.resumptionId, outNode, outSharedSecret, outCats) != CHIP_NO_ERROR);
    }

    // Verify no state table persistent storage entries were leaked.
    for (const auto & node : nodes)
    {
        uint16_t size = 0;
        chip::DefaultStorageKeyAllocator keyAlloc;
        auto rv = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::StorageKey(keyAlloc, node), nullptr, size);
        NL_TEST_ASSERT(inSuite, rv == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
    // Verify no link table persistent storage entries were leaked.
    for (auto & vector : vectors)
    {
        uint16_t size = 0;
        chip::DefaultStorageKeyAllocator keyAlloc;
        auto rv =
            storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::StorageKey(keyAlloc, vector.resumptionId), nullptr, size);
        NL_TEST_ASSERT(inSuite, rv == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

void TestDelete(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(sharedSecret.Bytes(), sharedSecret.Length()));

    // Populate test vectors.
    for (size_t i = 0; i < ArraySize(vectors); ++i)
    {
        NL_TEST_ASSERT(
            inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(vectors[i].resumptionId.data(), vectors[i].resumptionId.size()));
        *vectors[i].resumptionId.data() =
            static_cast<uint8_t>(i); // set first byte to our index to ensure uniqueness for the delete test
        vectors[i].node = chip::ScopedNodeId(static_cast<chip::NodeId>(i + 1), static_cast<chip::FabricIndex>(i + 1));
    }

    // Fill storage.
    for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i)
    {
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.Save(vectors[i].node, vectors[i].resumptionId, sharedSecret, chip::CATValues{}) ==
                           CHIP_NO_ERROR);
    }

    // Delete values in turn from storage and verify they are removed.
    for (size_t i = 0; i < ArraySize(vectors); ++i)
    {
        chip::ScopedNodeId outNode;
        chip::SessionResumptionStorage::ResumptionIdStorage outResumptionId;
        chip::Crypto::P256ECDHDerivedSecret outSharedSecret;
        chip::CATValues outCats;
        NL_TEST_ASSERT(inSuite, sessionStorage.Delete(vectors[i].node) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByScopedNodeId(vectors[i].node, outResumptionId, outSharedSecret, outCats) !=
                           CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite,
                       sessionStorage.FindByResumptionId(vectors[i].resumptionId, outNode, outSharedSecret, outCats) !=
                           CHIP_NO_ERROR);
    }

    // Verify no state or link table persistent storage entries were leaked.
    for (auto & vector : vectors)
    {
        uint16_t size = 0;
        chip::DefaultStorageKeyAllocator keyAlloc;
        {
            auto rv =
                storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::StorageKey(keyAlloc, vector.node), nullptr, size);
            NL_TEST_ASSERT(inSuite, rv == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        }
        {
            auto rv = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::StorageKey(keyAlloc, vector.resumptionId),
                                              nullptr, size);
            NL_TEST_ASSERT(inSuite, rv == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        }
    }
}

void TestDeleteAll(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(sharedSecret.Bytes(), sharedSecret.Length()));

    // Populate test vectors.
    for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i)
    {
        vectors[i].fabricIndex = static_cast<chip::FabricIndex>(i + 1);
        for (size_t j = 0; j < sizeof(vectors[0].nodes) / sizeof(vectors[0].nodes[0]); ++j)
        {
            NL_TEST_ASSERT(
                inSuite,
                CHIP_NO_ERROR ==
                    chip::Crypto::DRBG_get_bytes(vectors[i].nodes[j].resumptionId.data(), vectors[i].nodes[j].resumptionId.size()));
            vectors[i].nodes[j].node = chip::ScopedNodeId(static_cast<chip::NodeId>(j), vectors[i].fabricIndex);
        }
    }

    // Fill storage.
    for (auto & vector : vectors)
    {
        for (auto & node : vector.nodes)
        {
            NL_TEST_ASSERT(inSuite,
                           sessionStorage.Save(node.node, node.resumptionId, sharedSecret, chip::CATValues{}) == CHIP_NO_ERROR);
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
            NL_TEST_ASSERT(
                inSuite, sessionStorage.FindByScopedNodeId(node.node, outResumptionId, outSharedSecret, outCats) == CHIP_NO_ERROR);
        }
        // Delete fabric.
        NL_TEST_ASSERT(inSuite, sessionStorage.DeleteAll(vector.fabricIndex) == CHIP_NO_ERROR);
        // Verify fabric node entries no longer exist.
        for (const auto & node : vector.nodes)
        {
            NL_TEST_ASSERT(
                inSuite, sessionStorage.FindByScopedNodeId(node.node, outResumptionId, outSharedSecret, outCats) != CHIP_NO_ERROR);
        }
    }
    // Verify no state or link table persistent storage entries were leaked.
    for (auto & vector : vectors)
    {
        for (auto & node : vector.nodes)
        {
            uint16_t size = 0;
            chip::DefaultStorageKeyAllocator keyAlloc;
            {
                auto rv =
                    storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::StorageKey(keyAlloc, node.node), nullptr, size);
                NL_TEST_ASSERT(inSuite, rv == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
            }
            {
                auto rv = storage.SyncGetKeyValue(chip::SimpleSessionResumptionStorage::StorageKey(keyAlloc, node.resumptionId),
                                                  nullptr, size);
                NL_TEST_ASSERT(inSuite, rv == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
            }
        }
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestSave", TestSave),
    NL_TEST_DEF("TestInPlaceSave", TestInPlaceSave),
    NL_TEST_DEF("TestDelete", TestDelete),
    NL_TEST_DEF("TestDeleteAll", TestDeleteAll),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-DefaultSessionResumptionStorage",
    &sTests[0],
    nullptr,
    nullptr,
};
// clang-format on

/**
 *  Main
 */
int TestDefaultSessionResumptionStorage()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDefaultSessionResumptionStorage)
