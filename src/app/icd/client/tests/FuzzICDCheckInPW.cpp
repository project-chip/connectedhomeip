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
 *      FuzzTest harnesses for the Check-In message client path:
 *      CheckinMessage::ParseCheckinMessagePayload and its caller
 *      DefaultICDClientStorage::ProcessCheckInPayload.
 *
 *      A flat-buffer fuzzer makes very little progress here. The parser
 *      accepts only payloads of exactly nonce(13) + counter(4) + appData(n) +
 *      MIC(16) bytes, and ProcessCheckInPayload's kAppDataLength(6) work
 *      buffer caps n at 2 -- so the whole accept window is 33..35 bytes and
 *      everything else returns before any crypto runs. Past the length gates
 *      sits an AES-CCM decrypt and an HMAC-derived nonce comparison, both of
 *      which need the entry's key to satisfy.
 *
 *      So the harnesses below construct payloads with GenerateCheckinMessagePayload
 *      under a known key and then let the fuzzer mutate them, rather than
 *      asking the mutator to discover a valid MIC. That puts the counter
 *      decode, the nonce recomputation and comparison, the appData memmove
 *      and span narrowing, and (for ProcessCheckInPayload) the multi-entry
 *      key-iteration loop over a fixed-size work buffer inside the reachable
 *      set.
 *
 *      Three properties:
 *        - ParseRejectsArbitraryPayload      -- unconstrained bytes; covers the
 *          length/bounds arms ahead of the crypto, seeded with valid payloads
 *          and with sizes on both sides of the accept window.
 *        - ParseHandlesMutatedValidPayload   -- a valid payload with one
 *          fuzzer-chosen byte overwritten; reaches the decrypt/nonce/appData
 *          tail. appData sizes here exceed what ProcessCheckInPayload's work
 *          buffer allows, so this is the only property that drives the
 *          non-empty memmove.
 *        - ProcessSearchesAllStoredEntries   -- the storage-level caller with
 *          several stored entries, exercising the iterate-every-key loop and
 *          the 6-byte work buffer at and past its appData limit.
 *
 *      Oracle: no sanitizer error, plus the API contracts that the callers
 *      rely on -- on success the counter is the one that was encoded and
 *      appData is narrowed to exactly the encoded application data; on
 *      failure the call returns an error rather than reporting success.
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <app/icd/client/DefaultICDClientStorage.h>
#include <app/icd/client/ICDClientInfo.h>
#include <crypto/CryptoBuildConfig.h>
#include <crypto/DefaultSessionKeystore.h>
#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/logging/Constants.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <protocols/secure_channel/CheckinMessage.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Crypto;
using namespace chip::Protocols::SecureChannel;
using namespace fuzztest;

constexpr size_t kKeyLength = sizeof(Symmetric128BitsKeyByteArray);

// Application data that ParseCheckinMessagePayload can accept when the caller
// supplies a work buffer larger than ProcessCheckInPayload's kAppDataLength.
constexpr size_t kMaxParseAppDataSize = 32;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
#if CHIP_CRYPTO_PSA
        // Fuzz binaries use gmock_main and so miss the unit-test main's PSA
        // initialization; the backend must be initialized before any crypto runs.
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif
        // The storage layer logs a line per stored entry, which at fuzzing rates dominates
        // both runtime and output volume.
        Logging::SetLogFilter(Logging::kLogCategory_None);
        return true;
    }();
    (void) sInitialized;
}

// Distinct key material per index so that a payload built for one entry does
// not authenticate against another. Both the AES and HMAC handle for an entry
// are derived from the same material, matching how SetKey provisions them.
void FillKeyMaterial(uint8_t index, uint8_t (&out)[kKeyLength])
{
    for (size_t i = 0; i < kKeyLength; i++)
    {
        out[i] = static_cast<uint8_t>(index * 0x11u + i);
    }
}

// Builds a Check-In payload under `index`'s key. Returns an empty vector when
// the parameters cannot produce one, so callers can simply skip the iteration.
std::vector<uint8_t> BuildPayload(uint8_t index, uint32_t counter, const std::vector<uint8_t> & appData)
{
    DefaultSessionKeystore keystore;

    uint8_t material[kKeyLength];
    FillKeyMaterial(index, material);

    Symmetric128BitsKeyByteArray aesMaterial;
    memcpy(aesMaterial, material, kKeyLength);
    Symmetric128BitsKeyByteArray hmacMaterial;
    memcpy(hmacMaterial, material, kKeyLength);

    Aes128KeyHandle aesHandle;
    Hmac128KeyHandle hmacHandle;
    if (keystore.CreateKey(aesMaterial, aesHandle) != CHIP_NO_ERROR)
    {
        return {};
    }
    if (keystore.CreateKey(hmacMaterial, hmacHandle) != CHIP_NO_ERROR)
    {
        keystore.DestroyKey(aesHandle);
        return {};
    }

    std::vector<uint8_t> payload(CheckinMessage::GetCheckinPayloadSize(appData.size()));
    MutableByteSpan output(payload.data(), payload.size());

    CHIP_ERROR err = CheckinMessage::GenerateCheckinMessagePayload(aesHandle, hmacHandle, counter,
                                                                   ByteSpan(appData.data(), appData.size()), output);

    keystore.DestroyKey(aesHandle);
    keystore.DestroyKey(hmacHandle);

    if (err != CHIP_NO_ERROR)
    {
        return {};
    }
    payload.resize(output.size());
    return payload;
}

// A handful of well-formed payloads plus sizes either side of the accept
// window, so the mutator starts from inputs that already clear the length
// gates instead of having to find a 33-byte shape on its own.
std::vector<std::vector<uint8_t>> PayloadSeeds()
{
    // Called while the FUZZ_TEST domains are registered, which happens before
    // any property body runs -- so this, not the property, is the first thing
    // to construct a keystore and must do the initialization itself.
    EnsureInitialized();

    std::vector<std::vector<uint8_t>> seeds;

    for (uint8_t index = 0; index < 2; index++)
    {
        for (uint32_t counter : { 0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu })
        {
            for (size_t appDataSize : { 0u, 1u, 2u, 3u })
            {
                std::vector<uint8_t> appData(appDataSize, 0xAB);
                std::vector<uint8_t> payload = BuildPayload(index, counter, appData);
                if (!payload.empty())
                {
                    seeds.push_back(std::move(payload));
                }
            }
        }
    }

    // Boundary sizes around kMinPayloadSize that carry no valid MIC.
    seeds.push_back({});
    seeds.push_back(std::vector<uint8_t>(CheckinMessage::kMinPayloadSize - 1, 0x00));
    seeds.push_back(std::vector<uint8_t>(CheckinMessage::kMinPayloadSize, 0x00));
    seeds.push_back(std::vector<uint8_t>(CheckinMessage::kMinPayloadSize + 1, 0xFF));

    return seeds;
}

// ---------------------------------------------------------------------------

// Unconstrained bytes against the parser under a fixed key. Almost every input
// is rejected before the crypto; the value is in covering which arm rejects it.
void ParseRejectsArbitraryPayload(const std::vector<uint8_t> & payload)
{
    EnsureInitialized();

    DefaultSessionKeystore keystore;

    uint8_t material[kKeyLength];
    FillKeyMaterial(0, material);

    Symmetric128BitsKeyByteArray aesMaterial;
    memcpy(aesMaterial, material, kKeyLength);
    Symmetric128BitsKeyByteArray hmacMaterial;
    memcpy(hmacMaterial, material, kKeyLength);

    Aes128KeyHandle aesHandle;
    Hmac128KeyHandle hmacHandle;
    ASSERT_EQ(keystore.CreateKey(aesMaterial, aesHandle), CHIP_NO_ERROR);
    ASSERT_EQ(keystore.CreateKey(hmacMaterial, hmacHandle), CHIP_NO_ERROR);

    uint8_t appDataBuffer[sizeof(CounterType) + kMaxParseAppDataSize];
    MutableByteSpan appData(appDataBuffer);
    CounterType counter = 0;

    CHIP_ERROR err = CheckinMessage::ParseCheckinMessagePayload(aesHandle, hmacHandle, ByteSpan(payload.data(), payload.size()),
                                                                counter, appData);

    // A payload too short to hold nonce + counter + MIC, or one whose application
    // data cannot fit the work buffer alongside the counter, can never be accepted.
    if (payload.size() < CheckinMessage::kMinPayloadSize || payload.size() > CheckinMessage::kMinPayloadSize + kMaxParseAppDataSize)
    {
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    if (err == CHIP_NO_ERROR)
    {
        // A success implies the payload was well formed: appData must have been
        // narrowed to exactly the application-data length the size arithmetic
        // predicts, and must still sit inside the buffer that was handed in.
        EXPECT_EQ(appData.size(), CheckinMessage::GetAppDataSize(ByteSpan(payload.data(), payload.size())));
        EXPECT_LE(appData.size(), sizeof(appDataBuffer));
        EXPECT_EQ(appData.data(), appDataBuffer);
    }

    keystore.DestroyKey(aesHandle);
    keystore.DestroyKey(hmacHandle);
}

// A valid payload with one byte overwritten. Mutating a byte in the nonce, the
// ciphertext or the MIC selects which of the post-length checks rejects it;
// leaving the payload intact (when the mutation is a no-op) drives the success
// tail including the appData memmove.
void ParseHandlesMutatedValidPayload(uint32_t encodedCounter, const std::vector<uint8_t> & appDataIn, uint16_t mutationIndex,
                                     uint8_t mutationValue)
{
    EnsureInitialized();

    std::vector<uint8_t> payload = BuildPayload(0, encodedCounter, appDataIn);
    if (payload.empty())
    {
        return;
    }

    const bool mutated = mutationIndex < payload.size() && payload[mutationIndex] != mutationValue;
    if (mutationIndex < payload.size())
    {
        payload[mutationIndex] = mutationValue;
    }

    DefaultSessionKeystore keystore;

    uint8_t material[kKeyLength];
    FillKeyMaterial(0, material);

    Symmetric128BitsKeyByteArray aesMaterial;
    memcpy(aesMaterial, material, kKeyLength);
    Symmetric128BitsKeyByteArray hmacMaterial;
    memcpy(hmacMaterial, material, kKeyLength);

    Aes128KeyHandle aesHandle;
    Hmac128KeyHandle hmacHandle;
    ASSERT_EQ(keystore.CreateKey(aesMaterial, aesHandle), CHIP_NO_ERROR);
    ASSERT_EQ(keystore.CreateKey(hmacMaterial, hmacHandle), CHIP_NO_ERROR);

    uint8_t appDataBuffer[sizeof(CounterType) + kMaxParseAppDataSize];
    MutableByteSpan appData(appDataBuffer);
    CounterType decodedCounter = 0;

    CHIP_ERROR err = CheckinMessage::ParseCheckinMessagePayload(aesHandle, hmacHandle, ByteSpan(payload.data(), payload.size()),
                                                                decodedCounter, appData);

    // Any byte actually changed invalidates the MIC, so the parse must fail. A
    // mutated payload that still authenticates would itself be a finding.
    if (mutated)
    {
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
    else
    {
        // Conversely an untouched payload was generated under the same key the
        // parser is given, so rejecting it is a defect. Without this a parser
        // that always fails would satisfy every other assertion here.
        ASSERT_EQ(err, CHIP_NO_ERROR);
    }

    if (err == CHIP_NO_ERROR)
    {
        // An untouched payload must round-trip exactly.
        if (!mutated)
        {
            EXPECT_EQ(decodedCounter, encodedCounter);
            ASSERT_EQ(appData.size(), appDataIn.size());
            EXPECT_EQ(memcmp(appData.data(), appDataIn.data(), appDataIn.size()), 0);
        }
        EXPECT_EQ(appData.size(), CheckinMessage::GetAppDataSize(ByteSpan(payload.data(), payload.size())));
    }

    keystore.DestroyKey(aesHandle);
    keystore.DestroyKey(hmacHandle);
}

// The storage-level caller: several entries stored across distinct fabrics,
// with a payload built for one of them. ProcessCheckInPayload has to walk every
// stored key until one decrypts, using a kAppDataLength work buffer that is
// smaller than what the parser alone accepts.
void ProcessSearchesAllStoredEntries(uint8_t entryCount, uint8_t targetEntry, uint32_t encodedCounter,
                                     const std::vector<uint8_t> & appDataIn, uint16_t mutationIndex, uint8_t mutationValue)
{
    EnsureInitialized();

    // Keep the fabric count inside what the storage will accept and make sure
    // the target names one of the entries actually stored.
    const uint8_t entries = static_cast<uint8_t>(1 + (entryCount % 4));
    const uint8_t target  = static_cast<uint8_t>(targetEntry % entries);

    TestPersistentStorageDelegate clientInfoStore;
    DefaultSessionKeystore keystore;
    DefaultICDClientStorage storage;

    ASSERT_EQ(storage.Init(&clientInfoStore, &keystore), CHIP_NO_ERROR);

    for (uint8_t index = 0; index < entries; index++)
    {
        const FabricIndex fabricIndex = static_cast<FabricIndex>(index + 1);

        uint8_t material[kKeyLength];
        FillKeyMaterial(index, material);

        ICDClientInfo clientInfo;
        clientInfo.peer_node         = ScopedNodeId(static_cast<NodeId>(0x1000 + index), fabricIndex);
        clientInfo.check_in_node     = clientInfo.peer_node;
        clientInfo.start_icd_counter = encodedCounter;
        clientInfo.offset            = 0;

        ASSERT_EQ(storage.UpdateFabricList(fabricIndex), CHIP_NO_ERROR);
        ASSERT_EQ(storage.SetKey(clientInfo, ByteSpan(material, kKeyLength)), CHIP_NO_ERROR);
        ASSERT_EQ(storage.StoreEntry(clientInfo), CHIP_NO_ERROR);
    }

    std::vector<uint8_t> payload = BuildPayload(target, encodedCounter, appDataIn);
    const bool mutated           = !payload.empty() && mutationIndex < payload.size() && payload[mutationIndex] != mutationValue;
    if (!payload.empty() && mutationIndex < payload.size())
    {
        payload[mutationIndex] = mutationValue;
    }

    ICDClientInfo matchedInfo;
    CounterType decodedCounter = 0;
    CHIP_ERROR err = storage.ProcessCheckInPayload(ByteSpan(payload.data(), payload.size()), matchedInfo, decodedCounter);

    // A changed byte breaks the MIC, and application data larger than the
    // kAppDataLength work buffer leaves alongside the counter cannot be accepted.
    if (mutated || appDataIn.size() > DefaultICDClientStorage::kAppDataLength - sizeof(CounterType))
    {
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    else if (!payload.empty())
    {
        // An untouched payload whose application data fits the work buffer is
        // valid for exactly one stored entry, so it must be found -- and must
        // resolve to that entry rather than any other.
        ASSERT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(decodedCounter, encodedCounter);
        EXPECT_EQ(matchedInfo.peer_node, ScopedNodeId(static_cast<NodeId>(0x1000 + target), static_cast<FabricIndex>(target + 1)));
    }

    if (err == CHIP_NO_ERROR)
    {
        EXPECT_LE(appDataIn.size(), DefaultICDClientStorage::kAppDataLength - sizeof(CounterType));
    }

    // SetKey imports two key handles per entry and Shutdown does not release
    // them, so under a keystore with finite slots they would accumulate for the
    // life of the run.
    for (uint8_t index = 0; index < entries; index++)
    {
        EXPECT_EQ(storage.DeleteAllEntries(static_cast<FabricIndex>(index + 1)), CHIP_NO_ERROR);
    }

    storage.Shutdown();
}

// ---------------------------------------------------------------------------

FUZZ_TEST(ICDCheckInPW, ParseRejectsArbitraryPayload)
    .WithDomains(Arbitrary<std::vector<uint8_t>>().WithSeeds(PayloadSeeds()).WithMaxSize(128));

FUZZ_TEST(ICDCheckInPW, ParseHandlesMutatedValidPayload)
    .WithDomains(Arbitrary<uint32_t>(), Arbitrary<std::vector<uint8_t>>().WithMaxSize(kMaxParseAppDataSize), Arbitrary<uint16_t>(),
                 Arbitrary<uint8_t>());

FUZZ_TEST(ICDCheckInPW, ProcessSearchesAllStoredEntries)
    .WithDomains(Arbitrary<uint8_t>(), Arbitrary<uint8_t>(), Arbitrary<uint32_t>(),
                 Arbitrary<std::vector<uint8_t>>().WithMaxSize(8), Arbitrary<uint16_t>(), Arbitrary<uint8_t>());

} // namespace
