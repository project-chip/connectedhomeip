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
 *      management API, reached in production from the GroupKeyManagement and Groups
 *      clusters.
 *
 *      Driven as an operation sequence because the behaviour worth exercising is the
 *      index arithmetic and the persisted TLV round-trip across operations, not any
 *      single call. State persists between inputs.
 *
 *      The same provider's receive-side key lookup is covered by
 *      FuzzSessionManagerGroupPW.
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
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif
#include <lib/core/CHIPCore.h>
#include <lib/support/AutoRelease.h>
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

// No FabricTable: nothing here resolves the index to a fabric.
constexpr FabricIndex kFuzzFabric = 1;

// Reused across inputs so a sequence acts on accumulated state; the pools are fixed-size,
// so it cannot grow without bound. Storage left behind by RemoveFabric therefore builds up
// over many inputs, which a per-input reset would hide. The cost is that a crash may need its
// predecessors to reproduce; re-run the corpus in order.
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
#if CHIP_CRYPTO_PSA
        // A fuzz binary links gmock_main, which does not init PSA; key derivation needs it.
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif
        auto * fx = new Fixture();
        fx->provider.SetStorageDelegate(&fx->storage);
        fx->provider.SetSessionKeystore(&fx->keystore);
        VerifyOrDie(fx->provider.Init() == CHIP_NO_ERROR);
        fixture = fx;
        // The fixture is leaked, so Finish() needs an exit hook rather than a destructor.
        std::atexit([] {
            if (fixture != nullptr)
            {
                fixture->provider.Finish();
            }
        });
    });
    return *fixture;
}

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
                // A stored entry must read back at the index it was written to.
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
            // c is the keyset id; a picks how many epoch keys are stored.
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
            // AutoRelease matters: the iterator pools are fixed-size.
            if (AutoRelease it(provider->IterateGroupInfo(fabric)); it)
            {
                GroupInfo info;
                (void) it->Count();
                while (it->Next(info))
                {
                }
            }
            if (AutoRelease it(provider->IterateGroupKeys(fabric)); it)
            {
                GroupKey key;
                (void) it->Count();
                while (it->Next(key))
                {
                }
            }
            if (AutoRelease it(provider->IterateKeySets(fabric)); it)
            {
                KeySet keySet;
                (void) it->Count();
                while (it->Next(keySet))
                {
                }
            }
            if (AutoRelease it(
                    provider->IterateEndpoints(fabric, (a & 1) ? std::make_optional(static_cast<GroupId>(b)) : std::nullopt));
                it)
            {
                GroupDataProvider::GroupEndpoint endpoint;
                (void) it->Count();
                while (it->Next(endpoint))
                {
                }
            }
            break;
        }
        case 20:
            (void) provider->RemoveFabric(fabric);
            break;
        case 21:
            (void) provider->RemoveGroupKeys(fabric);
            break;
        case 22:
            // The endpoint-only overload, distinct from the (group, endpoint) one.
            (void) provider->RemoveEndpoint(fabric, static_cast<EndpointId>(b));
            break;
        case 23:
            if (AutoRelease it(provider->IterateGroupSessions(a)); it)
            {
                GroupDataProvider::GroupSession session;
                (void) it->Count();
                while (it->Next(session))
                {
                }
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
