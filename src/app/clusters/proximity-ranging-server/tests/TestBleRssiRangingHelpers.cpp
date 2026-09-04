/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// Unit tests for BleRssiRangingHelpers: the advertisement payload
// encode/decode round-trip and the BLEDeviceID generation / persistence
// helpers.
//
// EncodeBeaconPayload / DecodeBeaconPayload default to the secure
// HmacObfuscateBleDeviceId strategy, where the ObfuscatedBLEDeviceId field is an
// HMAC-SHA256 tag over (BLEDeviceID || messageCounter). The obfuscation strategy
// is injectable, so the obfuscation helpers and the insecure plaintext strategy
// are exercised directly below rather than behind a build flag.

#include <pw_unit_test/framework.h>

#include <app/clusters/proximity-ranging-server/BleRssiRangingHelpers.h>
#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <cstring>

namespace {

using namespace chip;
using namespace chip::app::Clusters::ProximityRanging::BleRssi;

using Ble::ChipBLEProximityRangingIdentificationInfo;

constexpr uint64_t kDeviceId      = 0x0102030405060708ULL;
constexpr uint64_t kOtherDeviceId = 0x1112131415161718ULL;
constexpr uint16_t kMsgCounter    = 0x2A2B;
constexpr int8_t kTxPower         = -42;

// Two distinct HMAC keys; obfuscation must differ between them.
constexpr uint8_t kSessionKey[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};
constexpr uint8_t kOtherSessionKey[] = { 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
                                         0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };

constexpr size_t kObfuscatedLen = sizeof(ChipBLEProximityRangingIdentificationInfo::ObfuscatedBLEDeviceId);

class TestBleRssiRangingHelpers : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Encode populates the OpCode, message counter, and Tx power exactly as passed.
TEST_F(TestBleRssiRangingHelpers, EncodeSetsHeaderFields)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    EXPECT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    EXPECT_EQ(payload.OpCode, ChipBLEProximityRangingIdentificationInfo::kOpCode);
    EXPECT_EQ(payload.GetMsgCounter(), kMsgCounter);
    EXPECT_EQ(payload.GetTxPower(), kTxPower);
}

// In secure mode the obfuscated field must NOT be the plaintext big-endian
// BLEDeviceID: it is an HMAC tag, so the raw identifier does not appear on air.
TEST_F(TestBleRssiRangingHelpers, EncodeObfuscatesDeviceId)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    uint8_t plaintext[kObfuscatedLen] = {};
    Encoding::BigEndian::Put64(plaintext, kDeviceId);
    EXPECT_NE(memcmp(payload.GetObfuscatedBLEDeviceId(), plaintext, kObfuscatedLen), 0);
}

// Known-answer test: pins the exact on-air obfuscated bytes so the wire format
// cannot drift silently. The expected value is HMAC-SHA256(kSessionKey,
// kDeviceId_BE || kMsgCounter_BE) truncated to the leading kObfuscatedLen bytes,
// computed by an independent implementation (Python's hmac module) rather than
// by the code under test:
//
//   message  = 01 02 03 04 05 06 07 08 2A 2B
//   full tag = a9e1319f58876ae42c29485c6f6466866c4f296fca290a30650a355a18594d05
//
// The round-trip and property tests share EncodeBeaconPayload's internal HMAC
// helper, so a systematic layout bug (wrong endianness, wrong truncation,
// counter mis-placed) would pass those but fail here.
TEST_F(TestBleRssiRangingHelpers, EncodeMatchesKnownAnswerVector)
{
    static constexpr uint8_t kExpectedObfuscatedId[kObfuscatedLen] = {
        0xA9, 0xE1, 0x31, 0x9F, 0x58, 0x87, 0x6A, 0xE4, 0x2C, 0x29, 0x48, 0x5C, 0x6F, 0x64, 0x66, 0x86,
    };

    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    EXPECT_EQ(memcmp(payload.GetObfuscatedBLEDeviceId(), kExpectedObfuscatedId, kObfuscatedLen), 0);
}

// Encoding is a pure function of its inputs: identical inputs yield identical
// obfuscated output.
TEST_F(TestBleRssiRangingHelpers, EncodeIsDeterministic)
{
    ChipBLEProximityRangingIdentificationInfo a;
    ChipBLEProximityRangingIdentificationInfo b;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), a), CHIP_NO_ERROR);
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), b), CHIP_NO_ERROR);

    EXPECT_EQ(memcmp(a.GetObfuscatedBLEDeviceId(), b.GetObfuscatedBLEDeviceId(), kObfuscatedLen), 0);
}

// The message counter is mixed into the HMAC, so bumping it changes the tag
// (this is what gives passive observers a rolling, uncorrelatable identifier).
TEST_F(TestBleRssiRangingHelpers, EncodeVariesWithMessageCounter)
{
    ChipBLEProximityRangingIdentificationInfo a;
    ChipBLEProximityRangingIdentificationInfo b;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), a), CHIP_NO_ERROR);
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, static_cast<uint16_t>(kMsgCounter + 1), kTxPower, ByteSpan(kSessionKey), b),
              CHIP_NO_ERROR);

    EXPECT_NE(memcmp(a.GetObfuscatedBLEDeviceId(), b.GetObfuscatedBLEDeviceId(), kObfuscatedLen), 0);
}

// A different session key produces a different tag for the same device ID.
TEST_F(TestBleRssiRangingHelpers, EncodeVariesWithSessionKey)
{
    ChipBLEProximityRangingIdentificationInfo a;
    ChipBLEProximityRangingIdentificationInfo b;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), a), CHIP_NO_ERROR);
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kOtherSessionKey), b), CHIP_NO_ERROR);

    EXPECT_NE(memcmp(a.GetObfuscatedBLEDeviceId(), b.GetObfuscatedBLEDeviceId(), kObfuscatedLen), 0);
}

// The core contract: a payload encoded for a device verifies against that same
// device ID and key.
TEST_F(TestBleRssiRangingHelpers, DecodeMatchesEncodedPayload)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeBeaconPayload(payload, kDeviceId, ByteSpan(kSessionKey)), CHIP_NO_ERROR);
}

// A candidate ID that did not emit the beacon does not verify.
TEST_F(TestBleRssiRangingHelpers, DecodeRejectsWrongDeviceId)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeBeaconPayload(payload, kOtherDeviceId, ByteSpan(kSessionKey)), CHIP_ERROR_NOT_FOUND);
}

// Verifying with the wrong session key fails: the peers must share the key.
TEST_F(TestBleRssiRangingHelpers, DecodeRejectsWrongSessionKey)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeBeaconPayload(payload, kDeviceId, ByteSpan(kOtherSessionKey)), CHIP_ERROR_NOT_FOUND);
}

// Decode recomputes the expected tag from the counter carried in the payload,
// so a tampered counter no longer matches the stored obfuscated field.
TEST_F(TestBleRssiRangingHelpers, DecodeRejectsTamperedMessageCounter)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    payload.SetMsgCounter(static_cast<uint16_t>(kMsgCounter + 1));
    EXPECT_EQ(DecodeBeaconPayload(payload, kDeviceId, ByteSpan(kSessionKey)), CHIP_ERROR_NOT_FOUND);
}

// Flipping a byte of the obfuscated field breaks verification.
TEST_F(TestBleRssiRangingHelpers, DecodeRejectsTamperedObfuscatedField)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(kSessionKey), payload), CHIP_NO_ERROR);

    payload.ObfuscatedBLEDeviceId[0] = static_cast<uint8_t>(payload.ObfuscatedBLEDeviceId[0] ^ 0xFF);
    EXPECT_EQ(DecodeBeaconPayload(payload, kDeviceId, ByteSpan(kSessionKey)), CHIP_ERROR_NOT_FOUND);
}

// GenerateBleDeviceId always yields a valid (non-reserved) identifier.
TEST_F(TestBleRssiRangingHelpers, GenerateProducesValidId)
{
    for (int i = 0; i < 100; ++i)
    {
        uint64_t id = kInvalidBleDeviceId;
        ASSERT_EQ(GenerateBleDeviceId(id), CHIP_NO_ERROR);
        EXPECT_NE(id, kInvalidBleDeviceId);
    }
}

// The HMAC obfuscator rejects an empty session key rather than producing a
// keyless tag.
TEST_F(TestBleRssiRangingHelpers, HmacObfuscateRejectsEmptySessionKey)
{
    uint8_t out[kObfuscatedLen] = {};
    EXPECT_EQ(HmacObfuscateBleDeviceId(kDeviceId, kMsgCounter, ByteSpan(), MutableByteSpan(out)), CHIP_ERROR_INVALID_ARGUMENT);
}

// The HMAC obfuscator rejects an output buffer smaller than the
// ObfuscatedBLEDeviceId field rather than writing past its end. A valid key is
// passed so the size guard (checked before the key guard) is what fires.
TEST_F(TestBleRssiRangingHelpers, HmacObfuscateRejectsTooSmallBuffer)
{
    uint8_t out[kObfuscatedLen - 1] = {};
    EXPECT_EQ(HmacObfuscateBleDeviceId(kDeviceId, kMsgCounter, ByteSpan(kSessionKey), MutableByteSpan(out)),
              CHIP_ERROR_BUFFER_TOO_SMALL);
}

// EncodeBeaconPayload surfaces the empty-key failure from the default HMAC
// strategy rather than emitting an unkeyed beacon.
TEST_F(TestBleRssiRangingHelpers, EncodeRejectsEmptySessionKey)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    EXPECT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(), payload), CHIP_ERROR_INVALID_ARGUMENT);
}

// The plaintext strategy writes the raw big-endian BLEDeviceID and ignores the
// session key (it is the insecure bring-up path).
TEST_F(TestBleRssiRangingHelpers, PlaintextObfuscateWritesRawBigEndianId)
{
    uint8_t out[kObfuscatedLen] = {};
    ASSERT_EQ(PlaintextObfuscateBleDeviceId(kDeviceId, kMsgCounter, ByteSpan(), MutableByteSpan(out)), CHIP_NO_ERROR);

    uint8_t expected[kObfuscatedLen] = {};
    Encoding::BigEndian::Put64(expected, kDeviceId);
    EXPECT_EQ(memcmp(out, expected, kObfuscatedLen), 0);
}

// The plaintext obfuscator also rejects an output buffer smaller than the
// ObfuscatedBLEDeviceId field rather than writing past its end.
TEST_F(TestBleRssiRangingHelpers, PlaintextObfuscateRejectsTooSmallBuffer)
{
    uint8_t out[kObfuscatedLen - 1] = {};
    EXPECT_EQ(PlaintextObfuscateBleDeviceId(kDeviceId, kMsgCounter, ByteSpan(), MutableByteSpan(out)), CHIP_ERROR_BUFFER_TOO_SMALL);
}

// A payload encoded with an injected strategy round-trips when decoded with the
// same strategy: this is what lets two peers interoperate on the plaintext path
// without a shared key.
TEST_F(TestBleRssiRangingHelpers, EncodeDecodeRoundTripsWithInjectedPlaintextStrategy)
{
    ChipBLEProximityRangingIdentificationInfo payload;
    ASSERT_EQ(EncodeBeaconPayload(kDeviceId, kMsgCounter, kTxPower, ByteSpan(), payload, PlaintextObfuscateBleDeviceId),
              CHIP_NO_ERROR);

    EXPECT_EQ(DecodeBeaconPayload(payload, kDeviceId, ByteSpan(), PlaintextObfuscateBleDeviceId), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeBeaconPayload(payload, kOtherDeviceId, ByteSpan(), PlaintextObfuscateBleDeviceId), CHIP_ERROR_NOT_FOUND);
}

// First use on empty storage: generate a valid ID and persist it.
TEST_F(TestBleRssiRangingHelpers, RetrieveGeneratesAndPersistsOnFirstUse)
{
    TestPersistentStorageDelegate storage;
    // Hold the StorageKeyName in a local: its destructor zeroes the underlying
    // buffer, so a raw const char * captured from a temporary would dangle.
    StorageKeyName key = DefaultStorageKeyAllocator::ProximityRangingBleDeviceId();

    uint64_t id = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, id), CHIP_NO_ERROR);
    EXPECT_NE(id, kInvalidBleDeviceId);
    EXPECT_TRUE(storage.HasKey(key.KeyName()));
}

// The generated ID is stable across calls (i.e. across simulated reboots).
TEST_F(TestBleRssiRangingHelpers, RetrieveReturnsStableIdAcrossCalls)
{
    TestPersistentStorageDelegate storage;

    uint64_t first = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, first), CHIP_NO_ERROR);

    uint64_t second = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, second), CHIP_NO_ERROR);

    EXPECT_EQ(first, second);
}

// A previously persisted valid ID is returned verbatim, without regeneration.
TEST_F(TestBleRssiRangingHelpers, RetrieveReturnsPersistedValue)
{
    TestPersistentStorageDelegate storage;
    StorageKeyName key         = DefaultStorageKeyAllocator::ProximityRangingBleDeviceId();
    constexpr uint64_t kStored = 0xDEADBEEFCAFEF00DULL;
    ASSERT_EQ(storage.SyncSetKeyValue(key.KeyName(), &kStored, sizeof(kStored)), CHIP_NO_ERROR);

    uint64_t id = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, id), CHIP_NO_ERROR);
    EXPECT_EQ(id, kStored);
}

// A stored reserved value (kInvalidBleDeviceId) is treated as absent and a
// fresh valid ID is generated and persisted in its place.
TEST_F(TestBleRssiRangingHelpers, RetrieveRegeneratesWhenStoredValueIsInvalid)
{
    TestPersistentStorageDelegate storage;
    StorageKeyName key          = DefaultStorageKeyAllocator::ProximityRangingBleDeviceId();
    constexpr uint64_t kInvalid = kInvalidBleDeviceId;
    ASSERT_EQ(storage.SyncSetKeyValue(key.KeyName(), &kInvalid, sizeof(kInvalid)), CHIP_NO_ERROR);

    uint64_t id = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, id), CHIP_NO_ERROR);
    EXPECT_NE(id, kInvalidBleDeviceId);
}

// A stored value of the wrong size is treated as corrupt and regenerated.
TEST_F(TestBleRssiRangingHelpers, RetrieveRegeneratesWhenStoredValueHasWrongSize)
{
    TestPersistentStorageDelegate storage;
    StorageKeyName key          = DefaultStorageKeyAllocator::ProximityRangingBleDeviceId();
    const uint8_t kTruncated[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    ASSERT_EQ(storage.SyncSetKeyValue(key.KeyName(), kTruncated, sizeof(kTruncated)), CHIP_NO_ERROR);

    uint64_t id = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, id), CHIP_NO_ERROR);
    EXPECT_NE(id, kInvalidBleDeviceId);

    // The regenerated ID must now be persisted as a full 8-byte value.
    uint64_t reread = kInvalidBleDeviceId;
    ASSERT_EQ(RetrieveGenerateBleDeviceId(storage, reread), CHIP_NO_ERROR);
    EXPECT_EQ(reread, id);
}

// If persisting the freshly generated ID fails, the failure is propagated.
TEST_F(TestBleRssiRangingHelpers, RetrievePropagatesPersistFailure)
{
    TestPersistentStorageDelegate storage;
    storage.SetRejectWrites(true);

    uint64_t id = kInvalidBleDeviceId;
    EXPECT_EQ(RetrieveGenerateBleDeviceId(storage, id), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
}

} // namespace
