/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      FuzzTest harness for GroupDataProviderImpl's group, key-set and endpoint
 *      management API -- the storage side of group state, reached in production from
 *      the GroupKeyManagement and Groups clusters.
 *
 *      The API is driven as an operation sequence rather than one call at a time
 *      because the behaviour worth exercising is the index arithmetic and the
 *      persisted TLV round-trip across operations: entries written by one operation
 *      are read, overwritten, compacted and removed by later ones. Provider state
 *      persists between inputs, so a sequence continues from what earlier sequences
 *      built.
 *
 *      The receive-side use of the same provider (key lookup during group message
 *      decryption) is covered separately by FuzzSessionManagerGroupPW.
 */

#include <cstdint>
#include <cstring>
#include <mutex>
#include <optional>
#include <tuple>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

using namespace chip;
using namespace chip::Credentials;
using namespace fuzztest;

using GroupInfo = GroupDataProvider::GroupInfo;
using GroupKey  = GroupDataProvider::GroupKey;
using KeySet    = GroupDataProvider::KeySet;

// The provider keys everything by fabric index; no FabricTable is needed because no
// operation here resolves the index to a fabric.
constexpr FabricIndex kFuzzFabric = 1;

// Persistent one-time state. The provider is created once and reused so that a sequence
// acts on state accumulated by previous inputs; its pools are fixed-size, so the state
// cannot grow without bound.
struct Fixture
{
    TestPersistentStorageDelegate storage;
    Crypto::DefaultSessionKeystore keystore;
    GroupDataProviderImpl provider{ /*maxGroupsPerFabric*/ 5, /*maxGroupKeysPerFabric*/ 8 };
};

Fixture & GetFixture()
{
    static Fixture * fixture = nullptr;
    static std::once_flag once;
    std::call_once(once, [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        auto * fx = new Fixture();
        fx->provider.SetStorageDelegate(&fx->storage);
        fx->provider.SetSessionKeystore(&fx->keystore);
        VerifyOrDie(fx->provider.Init() == CHIP_NO_ERROR);
        fixture = fx;
        // The fixture is intentionally leaked (built once, reused across inputs), so Finish()
        // runs from an exit hook rather than a destructor.
        std::atexit([] {
            if (fixture != nullptr)
            {
                fixture->provider.Finish();
            }
        });
    });
    return *fixture;
}

// The provider's group/keyset/endpoint management API, reached in production from the
// GroupKeyManagement and Groups clusters rather than from an inbound datagram. It is driven
// as an operation sequence because the interesting behaviour is in the index arithmetic and
// the persisted TLV round-trip across operations, not in any single call: entries survive
// between iterations, so later inputs act on state earlier ones built.
void ProviderSequenceDoesNotCrash(const std::vector<std::tuple<uint8_t, uint16_t, uint16_t, uint16_t>> & ops)
{
    GroupDataProvider * provider = &GetFixture().provider;

    constexpr FabricIndex fabric = kFuzzFabric;

    for (const auto & op : ops)
    {
        const uint8_t selector = std::get<0>(op);
        const uint16_t a       = std::get<1>(op);
        const uint16_t b       = std::get<2>(op);
        const uint16_t c       = std::get<3>(op);

        switch (selector % 24)
        {
        case 0: {
            GroupInfo info(static_cast<GroupId>(a), "G");
            (void) provider->SetGroupInfo(fabric, info);
            break;
        }
        case 1: {
            GroupInfo info;
            (void) provider->GetGroupInfo(fabric, static_cast<GroupId>(a), info);
            break;
        }
        case 2:
            (void) provider->RemoveGroupInfo(fabric, static_cast<GroupId>(a));
            break;
        case 3: {
            GroupInfo info(static_cast<GroupId>(a), "G");
            if (provider->SetGroupInfoAt(fabric, b, info) == CHIP_NO_ERROR)
            {
                // A stored entry must read back at the index it was written to: this is the
                // index arithmetic and the persisted TLV round-trip, not just a no-crash check.
                GroupInfo readBack;
                ASSERT_EQ(provider->GetGroupInfoAt(fabric, b, readBack), CHIP_NO_ERROR);
                ASSERT_EQ(readBack.group_id, info.group_id);
            }
            break;
        }
        case 4: {
            GroupInfo info;
            (void) provider->GetGroupInfoAt(fabric, b, info);
            break;
        }
        case 5:
            (void) provider->RemoveGroupInfoAt(fabric, b);
            break;
        case 6:
            if (provider->AddEndpoint(fabric, static_cast<GroupId>(a), static_cast<EndpointId>(b)) == CHIP_NO_ERROR)
            {
                ASSERT_TRUE(provider->HasEndpoint(fabric, static_cast<GroupId>(a), static_cast<EndpointId>(b)));
            }
            break;
        case 7:
            if (provider->RemoveEndpoint(fabric, static_cast<GroupId>(a), static_cast<EndpointId>(b)) == CHIP_NO_ERROR)
            {
                ASSERT_FALSE(provider->HasEndpoint(fabric, static_cast<GroupId>(a), static_cast<EndpointId>(b)));
            }
            break;
        case 8:
            (void) provider->HasEndpoint(fabric, static_cast<GroupId>(a), static_cast<EndpointId>(b));
            break;
        case 9:
            (void) provider->RemoveEndpointAllGroups(fabric, static_cast<EndpointId>(b),
                                                     (c & 1) ? GroupDataProvider::GroupCleanupPolicy::kKeepGroupIfEmpty
                                                             : GroupDataProvider::GroupCleanupPolicy::kDeleteGroupIfEmpty);
            break;
        case 10:
            (void) provider->RemoveEndpoints(fabric, static_cast<GroupId>(a));
            break;
        case 11:
            if (provider->SetGroupKey(fabric, static_cast<GroupId>(a), c) == CHIP_NO_ERROR)
            {
                KeysetId readBack = 0;
                ASSERT_EQ(provider->GetGroupKey(fabric, static_cast<GroupId>(a), readBack), CHIP_NO_ERROR);
                ASSERT_EQ(readBack, c);
            }
            break;
        case 12: {
            GroupKey key(static_cast<GroupId>(a), c);
            (void) provider->SetGroupKeyAt(fabric, b, key);
            break;
        }
        case 13: {
            KeysetId keysetId = 0;
            (void) provider->GetGroupKey(fabric, static_cast<GroupId>(a), keysetId);
            break;
        }
        case 14: {
            GroupKey key;
            (void) provider->GetGroupKeyAt(fabric, b, key);
            break;
        }
        case 15:
            (void) provider->RemoveGroupKeyAt(fabric, b);
            break;
        case 16: {
            // num_keys_used is clamped by the API; c drives which epoch-key count is stored.
            KeySet keySet(c, GroupDataProvider::SecurityPolicy::kTrustFirst, static_cast<uint8_t>(1 + (a % KeySet::kEpochKeysMax)));
            for (uint8_t i = 0; i < keySet.num_keys_used; i++)
            {
                memset(keySet.epoch_keys[i].key, static_cast<int>(a + i), sizeof(keySet.epoch_keys[i].key));
                keySet.epoch_keys[i].start_time = b;
            }
            const uint8_t compressedFabricId[8] = { 0xC3, 0x50, 0xBA, 0x91, 0x42, 0xE1, 0x6E, 0xF6 };
            (void) provider->SetKeySet(fabric, ByteSpan(compressedFabricId), keySet);
            break;
        }
        case 17: {
            KeySet keySet;
            (void) provider->GetKeySet(fabric, c, keySet);
            break;
        }
        case 18:
            (void) provider->RemoveKeySet(fabric, c);
            break;
        case 19: {
            // Drain each iterator; Release is mandatory, the pools are fixed-size.
            if (auto * it = provider->IterateGroupInfo(fabric))
            {
                GroupInfo info;
                (void) it->Count();
                while (it->Next(info))
                {
                }
                it->Release();
            }
            if (auto * it = provider->IterateGroupKeys(fabric))
            {
                GroupKey key;
                (void) it->Count();
                while (it->Next(key))
                {
                }
                it->Release();
            }
            if (auto * it = provider->IterateKeySets(fabric))
            {
                KeySet keySet;
                (void) it->Count();
                while (it->Next(keySet))
                {
                }
                it->Release();
            }
            if (auto * it =
                    provider->IterateEndpoints(fabric, (a & 1) ? std::make_optional(static_cast<GroupId>(b)) : std::nullopt))
            {
                GroupDataProvider::GroupEndpoint endpoint;
                (void) it->Count();
                while (it->Next(endpoint))
                {
                }
                it->Release();
            }
            break;
        }
        case 20:
            if (provider->RemoveFabric(fabric) == CHIP_NO_ERROR)
            {
                // Removing the fabric must leave nothing enumerable behind it.
                auto * it = provider->IterateGroupInfo(fabric);
                ASSERT_NE(it, nullptr);
                ASSERT_EQ(it->Count(), 0u);
                it->Release();
            }
            break;
        case 21:
            (void) provider->RemoveGroupKeys(fabric);
            break;
        case 22:
            // The endpoint-only overload, distinct from the (group, endpoint) one above.
            (void) provider->RemoveEndpoint(fabric, static_cast<EndpointId>(b));
            break;
        case 23:
            if (auto * it = provider->IterateGroupSessions(a))
            {
                GroupDataProvider::GroupSession session;
                (void) it->Count();
                while (it->Next(session))
                {
                }
                it->Release();
            }
            break;
        default:
            break;
        }
    }

    KeySet ipk;
    (void) provider->GetIpkKeySet(fabric, ipk);
}

FUZZ_TEST(FuzzGroupDataProviderPW, ProviderSequenceDoesNotCrash)
    .WithDomains(VectorOf(TupleOf(Arbitrary<uint8_t>(), Arbitrary<uint16_t>(), Arbitrary<uint16_t>(), Arbitrary<uint16_t>()))
                     .WithMaxSize(48));

} // namespace
